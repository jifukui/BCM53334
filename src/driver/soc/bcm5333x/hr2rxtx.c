/*
 * $Id: hr2rxtx.c,v 1.10 Broadcom SDK $
 *
 * $Copyright: Copyright 2016 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 */

#include "system.h"
#include "soc/bcm5333x.h"
#include "utils/net.h"

#if CFG_RXTX_SUPPORT_ENABLED

#define RXTX_DEBUG                  (0)

/* Max number of packet buffers we can accept */
#define SOC_MAX_RX_BUF_POSTED       (1)

/* Max time for TX to be done */
#define TX_WAIT_TIMEOUT             (500000UL)     /* in us */

/* Time for RX error detection */
#define RX_CHECK_INTERVAL           (1000000UL)    /* in us */

/* Make use of flags for packet state */
#define RX_FLAG_STATE_MASK      (3U << 8)
#define RX_FLAG_STATE_DMA       (1U << 8)
#define RX_FLAG_STATE_RECEIVED  (3U << 8)
#define RX_PKT_RECEIVED(pkt)    \
            (((pkt)->flags & RX_FLAG_STATE_MASK) == RX_FLAG_STATE_RECEIVED)
#define RX_PKT_AVAILABLE(pkt)   \
            (((pkt)->flags & RX_FLAG_STATE_MASK) == 0)

/* TX/RX enabled */
static BOOL             rxtx_initialized;

/* RX variables */
static SOC_RX_HANDLER   rx_handler;
static BOOL             rx_handler_in_intr;
static soc_rx_packet_t  *rx_packets[SOC_MAX_RX_BUF_POSTED];
static BOOL             rx_pkt_valid[SOC_MAX_RX_BUF_POSTED];
static soc_rx_packet_t  *rx_pkt_in_dma = NULL;

/* TX variables */
static soc_tx_packet_t  *tx_pkt_in_dma;
static soc_tx_packet_t  *tx_pkts_list;
static tick_t           tx_timeout_ticks;
static tick_t           tx_start_time;

/* Use address range 0x8000-0000~0x8FFF-FFFF to have DMA access through ACP */
#define PCI_TO_PTR(a)   ((uint32)a)
#define PTR_TO_PCI(x)   ((uint32)x)

/*
 * DMA Control Block - Type 26
 * 16 words
 */
typedef struct dcb_s {
    uint32 mem_addr;  /* buffer or chain address */
    uint32 w1;
    uint32 w2;
    uint32 w3;
    uint32 w4;
    uint32 w5;
    uint32 w6;
    uint32 w7;
    uint32 w8;
    uint32 w9;
    uint32 w10;
    uint32 w11;
    uint32 w12;
    uint32 w13;
    uint32 w14;
    uint32 w15;
} dcb_t;


static dcb_t tx_dcb __attribute__ ((section(".packet_buf"), aligned (32)));
static dcb_t rx_dcb __attribute__ ((section(".packet_buf"), aligned (32)));

/* Forwards */
static void bcm5333x_rx_direct(void);
static void bcm5333x_rx_refill(void);
static void bcm5333x_rx_retrieve(soc_rx_packet_t *pkt);
static void bcm5333x_rxtx_task(void *data);
static sys_error_t bcm5333x_tx_internal(soc_tx_packet_t *pkt);

static void
bcm5333x_rx_direct(void)
{
    soc_rx_packet_t *pkt = rx_pkt_in_dma;

    if (rx_pkt_in_dma != NULL) {
        if (rxtx_initialized == FALSE) {
            /* DMA may has been stopped */
            rx_pkt_in_dma = NULL;
            return;
        }

        /* Gather packet information */
        bcm5333x_rx_retrieve(pkt);

        /* Call packet handler */
        pkt->flags &= ~RX_FLAG_STATE_MASK;

        /* Call packet handler */
        if (rx_handler != NULL) {
            /*
             * Assuem we use only one buffer here. Need to change the way to
             * handle it if more than one buffer used.
             */
            rx_pkt_valid[0] = FALSE;
            rx_pkt_in_dma = NULL;
            (*rx_handler)(pkt);
        } else {
            /* Try to restart DMA */
            bcm5333x_rx_refill();
        }
    }
}

APISTATIC void
bcm5333x_rx_refill(void)
{
    uint8 i;
    uint32 val;

    for(i=0; i<SOC_MAX_RX_BUF_POSTED; i++) {
        if (rx_pkt_valid[i]) {
            soc_rx_packet_t *pkt = rx_packets[i];
            if (RX_PKT_AVAILABLE(pkt)) {
                pkt->flags |= RX_FLAG_STATE_DMA;
                rx_pkt_in_dma = pkt;
                sal_memset(&rx_dcb, 0, sizeof(dcb_t));
                rx_dcb.mem_addr = PTR_TO_PCI(pkt->buffer);
                rx_dcb.w1 = (V_DCB1_BYTE_COUNT(pkt->buflen)) ;
#if RXTX_DEBUG
                sal_printf("dcb:buf addr = 0x%08x:0x%08x\n", &rx_dcb, PTR_TO_PCI(pkt->buffer));
#endif
                SYS_REG_WRITE32(CMIC_CMC1_DMA_DESC(RX_CH1), PTR_TO_PCI(&rx_dcb));
                val = SYS_REG_READ32(CMIC_CMC1_CH1_DMA_CTRL);
                val |= PKTDMA_ENABLE;
                SYS_REG_WRITE32(CMIC_CMC1_CH1_DMA_CTRL, val);
                return;
            }
        }
    }

    rx_pkt_in_dma = NULL;
}

APISTATIC void
bcm5333x_rx_retrieve(soc_rx_packet_t *pkt)
{
    /* Gather information for the received packet */
    pkt->pktlen = G_DCB1_BYTE_COUNT(rx_dcb.w15);
    pkt->flags = 0;
#if RXTX_DEBUG
    sal_printf("\n0x%08x-%08x-%08x-%08x\n", rx_dcb.w2, rx_dcb.w3, rx_dcb.w4, rx_dcb.w5);
#endif
    pkt->lport = G_DCB1_SRC_PORT(rx_dcb.w3);
    pkt->traffic_class = G_DCB1_OUTER_PRI(rx_dcb.w11);
    pkt->timestamp = rx_dcb.w13;
}

APISTATIC void
bcm5333x_rxtx_task(void* data)
{
    uint32 irqstat = SYS_REG_READ32(CMIC_CMC1_DMA_STAT);
    uint32 val;

    /* RX polling mode */
    if (rx_pkt_in_dma != NULL) {
        if (irqstat & M_IRQ_CHAIN_DONE(RX_CH1)) {
            /* Clear bits. */
            val = SYS_REG_READ32(CMIC_CMC1_CH1_DMA_CTRL);
            val &= ~PKTDMA_ENABLE; /* clearing enable will also clear CHAIN_DONE */
            SYS_REG_WRITE32(CMIC_CMC1_CH1_DMA_CTRL, val);

            val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT_CLR);
            SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, (val | (DS_DESCRD_CMPLT_CLR(RX_CH1))));
            SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, val);
            bcm5333x_rx_direct();
        }
    }

    /* TX */
    if (tx_pkt_in_dma != NULL) {
        soc_tx_packet_t *pkt;
        if (irqstat & M_IRQ_CHAIN_DONE(TX_CH)) {
            /* Clear bits. */
            val = SYS_REG_READ32(CMIC_CMC1_CH0_DMA_CTRL);
            val &= ~PKTDMA_ENABLE; /* clearing enable will also clear CHAIN_DONE */
            SYS_REG_WRITE32(CMIC_CMC1_CH0_DMA_CTRL, val);

            val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT_CLR);
            SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, (val | (DS_DESCRD_CMPLT_CLR(TX_CH))));
            SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, val);

            tx_pkt_in_dma->status = SYS_OK;
        } else if (SAL_TIME_EXPIRED(tx_start_time, tx_timeout_ticks)) {
            /* Timeout, should abort this. */
            tx_pkt_in_dma->status = SYS_ERR_TIMEOUT;
            tx_pkt_in_dma->status = SYS_ERR;

            if (irqstat & DS_CMCx_DMA_ACTIVE(TX_CH)) {
                /* Abort TX DMA since it has something wrong */
                int i;
                uint32 ctrl;
                ctrl = SYS_REG_READ32(CMIC_CMC1_CH0_DMA_CTRL);
                ctrl |= PKTDMA_ENABLE;
                SYS_REG_WRITE32(CMIC_CMC1_CH0_DMA_CTRL, (ctrl | PKTDMA_ABORT));

                val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT);
                /* Check active */
                for (i = 10; i > 0; i--) {
                     if (val & DS_CMCx_DMA_ACTIVE(TX_CH)) {
                         sal_usleep(10);
                     } else {
                         break;
                     }
                     val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT);
                }
                /* Restore value */
                ctrl &= ~PKTDMA_ENABLE;
                SYS_REG_WRITE32(CMIC_CMC1_CH0_DMA_CTRL, ctrl);
                val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT_CLR);
                SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, (val | DS_DESCRD_CMPLT_CLR(TX_CH)));
                SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, val);
            }
        } else {
            /* Haven't completed */
            return;
        }

        /* Send next in queue (if any) */
        pkt = tx_pkt_in_dma;
        tx_pkt_in_dma = NULL;
        if (tx_pkts_list && rxtx_initialized) {
            soc_tx_packet_t *ppkt = tx_pkts_list;
            tx_pkts_list = ppkt->next;
            bcm5333x_tx_internal(ppkt);
        }

        /* Notify tx caller */
        (*(pkt->callback))(pkt);

    }
}

void
bcm5333x_rxtx_stop(void)
{
    uint32 val, ctrl;
    int i, j;

    rxtx_initialized = FALSE;

    bcm5333x_reg_get(0, R_EPC_LINK_BMAP_64, &val);
    val &= ~0x1;
    bcm5333x_reg_set(0, R_EPC_LINK_BMAP_64, val);

    SYS_REG_WRITE32(CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITS, 0);

    /* Poll for the channel 0-1 to become inactive */
    for (i = 0; i < 2; i++) {
        val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT);
        ctrl = SYS_REG_READ32(CMIC_CMC1_CHx_DMA_CTRL(i));
        /* Abort TX/RX DMA */
        if (val & DS_CMCx_DMA_ACTIVE(i)) {
            SYS_REG_WRITE32(CMIC_CMC1_CHx_DMA_CTRL(i), ctrl|PKTDMA_ABORT);
        }

        val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT);
        /* Check active */
        for (j = 10; j > 0; j--) {
            if (val & DS_CMCx_DMA_ACTIVE(i)) {
                sal_usleep(10);
            } else {
                break;
            }
            val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT);
        }

        ctrl &= ~PKTDMA_ENABLE; /* clearing enable will also clear CHAIN_DONE */
        SYS_REG_WRITE32(CMIC_CMC1_CHx_DMA_CTRL(i), ctrl);

        val = SYS_REG_READ32(CMIC_CMC1_DMA_STAT_CLR);
        SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, (val | _DD_MAKEMASK1(i)));
        SYS_REG_WRITE32(CMIC_CMC1_DMA_STAT_CLR, val);
    }
}

void
bcm5333x_rxtx_init(void)
{
    int i;
    uint32 val;

    /* Lazy initialization */
    if (rxtx_initialized) {
        return;
    }

    bcm5333x_rxtx_stop();

    rxtx_initialized = TRUE;

    /* Known good state, CH0 for tx and CH1 for rx. */
    SYS_REG_WRITE32(CMIC_CMC1_CH0_DMA_CTRL, 0x1);
    SYS_REG_WRITE32(CMIC_CMC1_CH1_DMA_CTRL, 0x0);

    /* Enable CMIC to release available credits to EP */
    SYS_REG_WRITE32(CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITS, 0);
    SYS_REG_WRITE32(CMIC_RXBUF_EPINTF_RELEASE_ALL_CREDITS, 1);

    /* Map all COS queues to default Rx DMA channel (1) */
    SYS_REG_WRITE32(CMIC_CMC1_CH1_COS_CTRL_RX_0, 0xff);

    /* Enable CPU port to receive packets. */
    bcm5333x_reg_get(0, R_EPC_LINK_BMAP_64, &val);
    val |= 0x1;
    bcm5333x_reg_set(0, R_EPC_LINK_BMAP_64, val);

    val = 0x1;
    bcm5333x_mem_set(0, M_EGR_ENABLE(0), &val, 1);

    /*
     * Enable IP to CMIC credit transfer:
     * TRANSFER_ENABLE =1, NUM_OF_CREDITS = 32
     */
    val = (0x1 << 6) | 32;
    bcm5333x_reg_set(0, R_IP_TO_CMICM_CREDIT_TRANSFER, val);

#if CFG_CONSOLE_ENABLED
    sal_printf("TX/RX support enabled.\n");
#endif /* CFG_CONSOLE_ENABLED */

    /* Initialize RX global variables */
    rx_handler = NULL;
    rx_handler_in_intr = FALSE;
    for(i=0; i<SOC_MAX_RX_BUF_POSTED; i++) {
        rx_pkt_valid[i] = FALSE;
    }
    rx_pkt_in_dma = NULL;

    /* Register a background task for RX handling */
    task_add(bcm5333x_rxtx_task, (void *)NULL);

    /* Initialize TX global variables */
    tx_pkt_in_dma = NULL;
    tx_pkts_list = NULL;
    tx_timeout_ticks = SAL_USEC_TO_TICKS(TX_WAIT_TIMEOUT);
}

APISTATIC sys_error_t
bcm5333x_tx_internal(soc_tx_packet_t *pkt)
{
    int i;
    uint32 port;
    enet_hdr_t  *th; /* Tagged header pointers */
    uint32 val;

    sal_memset(&tx_dcb, 0, sizeof(dcb_t));

    
    if (pkt->port_bitmap != 0) {
        /* Use SOBMH mode. */
        tx_dcb.w1 = (1 << 19);
        for (port = BCM5333X_LPORT_MIN; port <= BCM5333X_LPORT_MAX; port++) {
            if (pkt->port_bitmap & (1 << port)) {
                break;
            }
        }

        if (port > BCM5333X_LPORT_MAX) {
            return SYS_ERR_PARAMETER;
        }

        /* SOBMH_FROM_CPU */
        tx_dcb.w2 = 0x81000000;
        /* Assign LOCAL_DEST_PORT */
        tx_dcb.w3 = (port & 0x7F);
        /* Use cosq 3 and enable UNICAST */
        tx_dcb.w4 = 0x40000 | (0x3 << 8);

        /* remove tag for untag members */
        if (pkt->untag_bitmap != 0) {
            th = (enet_hdr_t *)pkt->buffer;
            if (ENET_TAGGED(th)) {
                pkt->pktlen -= 4;
                for (i = 12 ; i < pkt->pktlen ; i++) {
                    pkt->buffer[i] = pkt->buffer[i + 4];
                }
                sal_memset(&(pkt->buffer[pkt->pktlen]), 0, 4);
            }
        }
    }

    tx_dcb.mem_addr = PTR_TO_PCI(pkt->buffer);
    tx_dcb.w1 |= (V_DCB1_BYTE_COUNT(pkt->pktlen) | (1 << 20));
#if RXTX_DEBUG
    {
        int j;
        uint32 *p = (uint32 *)&tx_dcb;
        sal_printf("\nTX DCB: ");
        for (j = 0; j < 16; j++) {
            sal_printf("0x%08x ", *(p + j));
        }
        sal_printf("\n");
    }
    sal_printf("dcb:buf addr = 0x%08x:0x%08x\n", &tx_dcb, PTR_TO_PCI(pkt->buffer));
#endif /* RXTX_DEBUG */
    SYS_REG_WRITE32(CMIC_CMC1_DMA_DESC(0), PTR_TO_PCI(&tx_dcb));
    val = SYS_REG_READ32(CMIC_CMC1_CH0_DMA_CTRL);
    val |= PKTDMA_ENABLE;
    SYS_REG_WRITE32(CMIC_CMC1_CH0_DMA_CTRL, val);

    tx_start_time = sal_get_ticks();
    tx_pkt_in_dma = pkt;

    return SYS_OK;
}

sys_error_t
bcm5333x_tx(uint8 unit, soc_tx_packet_t *pkt)
{
    SAL_ASSERT(pkt != NULL && pkt->callback != NULL && pkt->buffer != NULL);
    if (pkt == NULL || pkt->callback == NULL || pkt->buffer == NULL) {
        return SYS_ERR_PARAMETER;
    }
    if (pkt->pktlen == 0 || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    pkt->unit = 0;

    if (tx_pkt_in_dma == NULL) {
        bcm5333x_tx_internal(pkt);
    } else {
        /* DMA in progress, queue this packet */
        pkt->next = tx_pkts_list;
        tx_pkts_list = pkt;
    }
    return SYS_OK;
}

sys_error_t
bcm5333x_rx_set_handler(uint8 unit, SOC_RX_HANDLER fn, BOOL intr)
{
    if (fn == NULL || unit > 0) {
        /* XXX: should allow to remove current handler (and do RX reset) */
        return SYS_ERR_PARAMETER;
    }
    if (rxtx_initialized == FALSE) {
        return SYS_ERR_STATE;
    }

    /* XXX: currently it can only be set once (and can't change) */
    SAL_ASSERT(rx_handler == NULL);
    if (rx_handler != NULL) {
        return SYS_ERR_STATE;
    }

    rx_handler = fn;
    rx_handler_in_intr = intr;

    return SYS_OK;
}

sys_error_t
bcm5333x_rx_fill_buffer(uint8 unit, soc_rx_packet_t *pkt)
{
    uint8 i;
    uint32 val;

    if (pkt == NULL || pkt->buffer == NULL || pkt->buflen == 0 || unit > 0) {
        return SYS_ERR_PARAMETER;
    }
    if (rxtx_initialized == FALSE) {
        return SYS_ERR_STATE;
    }

    if (rx_handler == NULL) {
        return SYS_ERR_STATE;
    }

    for(i=0; i<SOC_MAX_RX_BUF_POSTED; i++) {
        if (rx_pkt_valid[i] == FALSE) {
            pkt->unit = 0;
            pkt->flags = 0;
            rx_packets[i] = pkt;
            rx_pkt_valid[i] = TRUE;
            if (rx_pkt_in_dma == NULL) {
                pkt->flags |= RX_FLAG_STATE_DMA;
                rx_pkt_in_dma = pkt;
                sal_memset(&rx_dcb, 0, sizeof(dcb_t));
                rx_dcb.mem_addr = PTR_TO_PCI(pkt->buffer);
                rx_dcb.w1 = V_DCB1_BYTE_COUNT(pkt->buflen);
#if RXTX_DEBUG
                sal_printf("dcb:buf addr = 0x%08x:0x%08x\n", &rx_dcb, PTR_TO_PCI(pkt->buffer));
#endif
                SYS_REG_WRITE32(CMIC_CMC1_DMA_DESC(RX_CH1), PTR_TO_PCI(&rx_dcb));
                val = SYS_REG_READ32(CMIC_CMC1_CH1_DMA_CTRL);
                val |= PKTDMA_ENABLE;
                SYS_REG_WRITE32(CMIC_CMC1_CH1_DMA_CTRL, val);
            }
            return SYS_OK;
        }
    }

    return SYS_ERR_FULL;
}

#endif /* CFG_RXTX_SUPPORT_ENABLED */
