/*
 * $Id: snaketest.c,v 1.15.6.1 Broadcom SDK $
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
#include "utils/ports.h"
#include "appl/snaketest.h"

#ifdef CFG_SWITCH_SNAKETEST_INCLUDED

#define SNAKETEST_PACKET_LEN    (64)

#define DA1_LAST_OCTET          0xBB
#define DA2_LAST_OCTET          0xBC

#define ETHERTYPE_OCTET_1       0x88
#define ETHERTYPE_OCTET_2       0x00

#define VLAN_ID_MIN               2

extern sys_error_t
board_snaketest_trap_to_cpu(uint16 vlanid, uint8 *uplist);

static const uint8 snaketest_da[6] = { 0x66, 0x77, 0x88, 0x99, 0xAA, DA1_LAST_OCTET };
static const uint8 snaketest_sa[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };
static uint8 snaketest_min_uport = 0;
static uint8 snaketest_max_uport = 0;
static uint8 snaketest_max_vlan = 0;
static uint8 r_len[2];
static uint8 r_data[2][SNAKETEST_PACKET_LEN*2];
static int reverse_pkt_flag, rpkt_count;

static void
_snake_test_cbk(sys_pkt_t *pkt, sys_error_t status)
{
    if (!reverse_pkt_flag) {
        /* Reuse packet */
        reverse_pkt_flag = 1;
        return;
    }
    if (pkt) {
        if (pkt->pkt_data) {
            sal_dma_free(pkt->pkt_data);
        }
        sal_free(pkt);
    }
}

static sys_rx_t
_snake_rx_handler(sys_pkt_t *pkt, void *cookie)
{
    /* Save data */
    if (rpkt_count < 2) {
        sal_memcpy(r_data[rpkt_count], pkt->pkt_data, pkt->pkt_len);
        r_len[rpkt_count] = pkt->pkt_len;
        rpkt_count++;
    }

    return SYS_RX_HANDLED;
}

/* Compared packets returned to CPU match tx packet */
static int
_snake_analysis(void)
{
    int i, j, k;

    sal_printf("\nSnake Test : %d packets received by CPU.\n", rpkt_count);

    if (rpkt_count != 2) {
        sal_printf("\nSnake Test : ERROR - Expected 2 packet received by CPU !\n");
        return -1;
    }

    /* Check packet content */
    for (i = 0; i < rpkt_count; i++) {
        if (sal_memcmp(r_data[i], snaketest_da, 5)) {
            goto err;
        }

        if (r_data[i][5] != DA1_LAST_OCTET && r_data[i][5] != DA2_LAST_OCTET) {
            goto err;
        }

        if (sal_memcmp(&r_data[i][6], snaketest_sa, 6)) {
            goto err;
        }

        for (j = 18, k = 0; j < SNAKETEST_PACKET_LEN + 4; j++, k++) {
            if (r_data[i][j] != (uint8)k) {
                goto err;
            }
        } 

    }
    return 0;
err:
    sal_printf("\nSnake Test : ERROR - Packet %d is not matched !\n", i+1);
    sal_printf("Data: ");
    for (j = 0; j < r_len[i]; j++) {
        sal_printf("%02x ", r_data[i][j]);
    }
    return -1;
}

/* Check error counters */
static int
_snaketest_stats(uint8 mode)
{
    port_stat_t stat[BOARD_MAX_NUM_OF_PORTS + 1];
    uint16 uport;
    int  perror = 0;

#if !defined(CFG_SWITCH_STAT_INCLUDED)
    return 0;
#endif
    sal_memset(stat, 0, (sizeof(port_stat_t) * BOARD_MAX_NUM_OF_PORTS));
    for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
        board_port_stat_get(uport, &(stat[uport]));

        if (stat[uport].CRCErrors_lo != 0) {
            sal_printf("\nSnake Test : GRFCS %d found at port %d !\n", stat[uport].CRCErrors_lo, uport);
            perror = 1;
        }
    }

    if (perror == 0) {
        sal_printf("\nSnake Test : Statistics for each port ");
        for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
            sal_printf("\nPort  %02d    TX %u pkt  %u byte,  RX ", uport, stat[uport].TxPkts_lo, stat[uport].TxOctets_lo);
            sal_printf(" %u pkt  %u byte", stat[uport].RxPkts_lo, stat[uport].RxOctets_lo);

            if ((stat[uport].TxPkts_lo == 0x0) ||
                (stat[uport].TxOctets_lo == 0x0) ||
                (stat[uport].RxPkts_lo == 0x0) ||
                (stat[uport].RxOctets_lo == 0x0)) {
                sal_printf("\nSnake Test : ERROR - TX/RX error found at port %d !\n", uport);
                perror = 1;
            }

            if (mode == SNAKETEST_TYPE_PORT_PAIR) {
                if ((uport == snaketest_min_uport) || 
                    (uport == snaketest_max_uport)) {
                    /* statistic may be not matched at port snaketest_min_uport and port snaketest_max_uport due to stop traffic */
                    continue;
                }

                /* Snake test with port pair loopback */
                if (uport % 2) {
                    if ((stat[uport].TxPkts_lo != stat[uport+1].RxPkts_lo) ||
                        (stat[uport].TxOctets_lo != stat[uport+1].RxOctets_lo)) {
                        sal_printf("\nSnake Test : ERROR - TX/RX mismatch error found at port %d and port %d!\n", uport, uport+1);
                        perror = 1;
                    }
                } else {
                    if ((stat[uport].TxPkts_lo != stat[uport-1].RxPkts_lo) ||
                        (stat[uport].TxOctets_lo != stat[uport-1].RxOctets_lo)) {
                        sal_printf("\nSnake Test : ERROR - TX/RX mismatch error found at port %d and port %d!\n", uport-1, uport);
                        perror = 1;
                    }
                }
            } else {
                if ((stat[uport].TxPkts_lo != stat[uport].RxPkts_lo) ||
                    (stat[uport].TxOctets_lo != stat[uport].RxOctets_lo)) {
                    sal_printf("\nSnake Test : ERROR - TX/RX mismatch error found at port %d !\n", uport);
                    perror = 1;
                }
            }
        }
        sal_printf("\n");
    }

    return perror;
}

static void
_snaketest_txrx(uint8 mode, int test_duration)
{
    int i, j, duration, rv;
    tick_t start;
    sys_pkt_t *tx_pkt;
    uint8 *txpkt_data;
    uint8 uplist[MAX_UPLIST_WIDTH];
    uint8 tag_uplist[MAX_UPLIST_WIDTH];

    rpkt_count = 0;
    reverse_pkt_flag = 0;

    sys_rx_register(
                _snake_rx_handler, 
                CFG_CLI_RX_MON_PRIORITY, 
                NULL,
                SYS_RX_REGISTER_FLAG_ACCEPT_PKT_ERRORS | 
                SYS_RX_REGISTER_FLAG_ACCEPT_TRUNCATED_PKT
                );
    

    tx_pkt = (sys_pkt_t *)sal_malloc(sizeof(sys_pkt_t));
    if (tx_pkt == NULL) {
        sal_printf("\nSnake Test : ERROR - Out of memory !\n");
    }
    sal_memset(tx_pkt, 0, sizeof(sys_pkt_t));

    txpkt_data = (uint8 *)sal_dma_malloc(SNAKETEST_PACKET_LEN+4);
    if (txpkt_data == NULL) {
        sal_printf("\nSnake Test :  ERROR - Out of memory !\n");
    }

    if (uplist_clear(tx_pkt->tx_uplist) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_clear failed !\n");
    }
    if (uplist_clear(tx_pkt->tx_untag_uplist) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_clear failed !\n");
    }
    if (uplist_port_add(tx_pkt->tx_uplist, snaketest_min_uport) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_port_add failed !\n");
    }
    if (uplist_port_add(tx_pkt->tx_untag_uplist, snaketest_min_uport) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_port_add failed !\n");
    }

    tx_pkt->pkt_data = txpkt_data;
    tx_pkt->pkt_len = SNAKETEST_PACKET_LEN + 4;
    tx_pkt->internal1 = (uint32) _snake_test_cbk;

    /* DA and SA */    
    sal_memcpy(txpkt_data, snaketest_da, 6);
    sal_memcpy(txpkt_data + 6, snaketest_sa, 6);

    /* Untagged packet, use ethertype 0x8800 to invalidate 802.3 length field for XLMAC */
    txpkt_data[12] = ETHERTYPE_OCTET_1;
    txpkt_data[13] = ETHERTYPE_OCTET_2;

    for (i = 14, j = 0; i < SNAKETEST_PACKET_LEN; i++, j++) {
        *(txpkt_data + i) = (uint8)j;
    }
 
    if (sys_tx(tx_pkt, NULL)) {
        sal_printf("\nSnake Test :  ERROR - sys_tx failed with snaketest_min_uport in _snaketest_txrx !\n");
    }

    sal_sleep(1000);

    if (uplist_clear(tx_pkt->tx_uplist) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_clear failed !\n");
    }
    if (uplist_clear(tx_pkt->tx_untag_uplist) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_clear failed !\n");
    }
    if (uplist_port_add(tx_pkt->tx_uplist, snaketest_max_uport) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_port_add failed !\n");
    }
    if (uplist_port_add(tx_pkt->tx_untag_uplist, snaketest_max_uport) != SYS_OK) {
        sal_printf("\nSnake Test :  ERROR - uplist_port_add failed !\n");
    }

    *(txpkt_data+5) = DA2_LAST_OCTET;

    if (sys_tx(tx_pkt, NULL)) {
        sal_printf("\nSnake Test :  ERROR - sys_tx failed with snaketest_max_uport in _snaketest_txrx !\n");
    }
#ifndef CFG_EMULATION
    sal_sleep(1000);
#else
    sal_sleep(5000);
#endif

    start = sal_get_ticks();

    sal_printf("\nSnake Test : Start circular test on all testing ports for %d seconds.\n", test_duration);

    duration = 0;

    while (duration < test_duration) {
        while(!SAL_TIME_EXPIRED(start, 10000)) {
            POLL();
        }
        duration += 10;
        sal_printf("\nSnake Test : Time elapsed %d seconds.\n", duration);
        start = sal_get_ticks();
    }

    /* Trap packet to CPU */
    uplist_clear(uplist);
    uplist_port_add(uplist, snaketest_max_uport);
    uplist_port_add(uplist, snaketest_min_uport);
    board_snaketest_trap_to_cpu(snaketest_max_vlan, uplist);
    
    /* Wait for receiving packets by CPU */
#ifndef CFG_EMULATION
    sal_sleep(2000);
#else
    sal_sleep(50000);
#endif
    /* Stop traffic*/
    uplist_clear(tag_uplist);

    board_vlan_create(snaketest_max_vlan+1);
    uplist_clear(uplist);
    uplist_port_add(uplist, snaketest_min_uport);
    board_qvlan_port_set(snaketest_max_vlan+1, uplist, tag_uplist);
    board_untagged_vlan_set(snaketest_min_uport, snaketest_max_vlan+1);

    board_vlan_create(snaketest_max_vlan+2);
    uplist_clear(uplist);
    uplist_port_add(uplist, snaketest_max_uport);
    board_qvlan_port_set(snaketest_max_vlan+2, uplist, tag_uplist);
    board_untagged_vlan_set(snaketest_max_uport, snaketest_max_vlan+2);

#ifndef CFG_EMULATION
    sal_sleep(2000);
#else
    sal_sleep(50000);
#endif

    rv = _snake_analysis();
    
    if ((rv == 0) && (_snaketest_stats(mode) == 0)) {
        sal_printf("\nSnake Test : Passed.\n");
    } else {
        if (rv != 0) {
            _snaketest_stats(mode);
        }
        sal_printf("\nSnake Test : Failed.\n");
    }
}

extern void uip_task(void *param);

void
snaketest(uint8 mode, uint8 min_uport, uint8 max_uport, int duration)
{
    uint8 uplist[MAX_UPLIST_WIDTH], tag_uplist[MAX_UPLIST_WIDTH];
    uint16 uport, vlan;
    int lb_mode;
    BOOL link;

    snaketest_min_uport = min_uport;
    snaketest_max_uport = max_uport;

#if (CFG_RXTX_SUPPORT_ENABLED && CFG_UIP_STACK_ENABLED)
    task_suspend(uip_task);
#endif /* CFG_RXTX_SUPPORT_ENABLED && CFG_UIP_STACK_ENABLED */

    board_vlan_type_set(VT_DOT1Q);

    uplist_clear(tag_uplist);
    if ((mode == SNAKETEST_TYPE_PORT_PAIR) || (mode == SNAKETEST_TYPE_PKT_GEN)) {
        for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
            board_port_enable_set(uport, FALSE);
        }
        /* wait for all packets drain out */
        sal_sleep(2000);
        
        if (mode == SNAKETEST_TYPE_PORT_PAIR) {
            /* Snake test setting for port pair loopback */
            for (uport = snaketest_min_uport, vlan = VLAN_ID_MIN ; uport < snaketest_max_uport ; uport+=2, vlan++) {
                snaketest_max_vlan = vlan;
                board_vlan_create(vlan);
                uplist_clear(uplist);
                uplist_port_add(uplist, uport+1);
                if ((uport + 2) > snaketest_max_uport) {
                    uplist_port_add(uplist, snaketest_min_uport);
                } else {
                    uplist_port_add(uplist, uport+2);
                }
                board_qvlan_port_set(vlan, uplist, tag_uplist);
                board_untagged_vlan_set(uport+1, vlan);
                if ((uport + 2) > snaketest_max_uport) {
                    board_untagged_vlan_set(snaketest_min_uport, vlan);
                } else {
                    board_untagged_vlan_set(uport+2, vlan);
                }
            }
        } else {        
            /* Snake test setting for packet generator device */
            for (uport = snaketest_min_uport, vlan = VLAN_ID_MIN ; uport < snaketest_max_uport ; uport+=2, vlan++) {
                snaketest_max_vlan = vlan;
                board_vlan_create(vlan);
                uplist_clear(uplist);
                uplist_port_add(uplist, uport);
                uplist_port_add(uplist, uport+1);
                board_qvlan_port_set(vlan, uplist, tag_uplist);
                board_untagged_vlan_set(uport, vlan);
                board_untagged_vlan_set(uport+1, vlan);
            }
        }

        for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
            board_port_enable_set(uport, TRUE);
        }

        /* Check link status */
        while (1) {
            /* wait for all testing port are link up */
            sal_sleep(2000);
            for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                board_get_port_link_status (uport, &link);
                if (link == FALSE) {
                    sal_printf("\nSnake Test : Port %d is link down, please connect cable to it !\n", uport);
                    break;
                }
            }

            if (uport > snaketest_max_uport) {
                break;
            }
        }
        
        if (mode == SNAKETEST_TYPE_PORT_PAIR) {
            /* Snake test with port pair loopback */
#if defined(CFG_SWITCH_STAT_INCLUDED)
            /* Clear statistics */
            board_port_stat_clear_all();
#endif
            _snaketest_txrx(mode, duration);
        } else {
            sal_printf("\nUser can inject packet to port %d and %d from packet generator device now !\n", snaketest_min_uport, snaketest_max_uport);
        }
    } else if (mode == SNAKETEST_TYPE_INT_MAC || mode == SNAKETEST_TYPE_INT_PHY || mode == SNAKETEST_TYPE_EXT) {
        /* Snake test with MAC/PHY/EXT loopback */

        /* Check link status */
#ifndef CFG_EMULATION
        if (mode == SNAKETEST_TYPE_INT_MAC || mode == SNAKETEST_TYPE_INT_PHY) {
            while (1) {
                /* wait for all testing port are link dwon */
                sal_sleep(2000);
                for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                    board_get_port_link_status (uport, &link);
                    if (link == TRUE) {
                        sal_printf("\nSnake Test : Port %d is link up, please disconnect cable from it !\n", uport);
                        break;
                    }
                }

                if (uport > snaketest_max_uport) {
                    break;
                }
            }
        } else {
            for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                board_port_enable_set(uport, FALSE);
            }
            
            /* wait for all packets drain out */
            sal_sleep(2000);
        }
#endif

        for (uport = snaketest_min_uport, vlan = VLAN_ID_MIN ; uport <= snaketest_max_uport ; uport++, vlan++) {
            snaketest_max_vlan = vlan;

            board_untagged_vlan_set(uport, vlan);

            board_vlan_create(vlan);
            uplist_clear(uplist);
            uplist_port_add(uplist, uport);
            if (uport  == snaketest_max_uport) {
                uplist_port_add(uplist, snaketest_min_uport);
            } else {
                uplist_port_add(uplist, uport + 1);
            }
            board_qvlan_port_set(vlan, uplist, tag_uplist);
        }

        if (mode == SNAKETEST_TYPE_INT_MAC || mode == SNAKETEST_TYPE_INT_PHY) {
            lb_mode = (mode == 0) ? PORT_LOOPBACK_MAC : PORT_LOOPBACK_PHY;

            for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                board_port_loopback_enable_set(uport, lb_mode);
            }
        } else {
            /* Check link status */
            while (1) {
                /* wait for non testing port are link down */
                sal_sleep(2000);

                SAL_UPORT_ITER(uport) {
                    if ((uport >= snaketest_min_uport) && (uport <= snaketest_max_uport)) {
                        continue;
                    }
                    
                    board_get_port_link_status (uport, &link);
                    if (link == TRUE) {
                        sal_printf("\nSnake Test : Port %d is link up, please disconnect cable to it !\n", uport);
                        break;
                    }
                }

                if (uport >= (board_uport_count() + SAL_UPORT_BASE)) {
                    break;
                }
            }

            for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                board_port_enable_set(uport, TRUE);
            }

            while (1) {
                /* wait for all testing port are link up */
                sal_sleep(2000);

                for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                    board_get_port_link_status (uport, &link);
                    if (link == FALSE) {
                        sal_printf("\nSnake Test : Port %d is link down, please connect cable to it !\n", uport);
                        break;
                    }
                }

                if (uport > snaketest_max_uport) {
                    break;
                }
            }
        }

        sal_sleep(6000);
#if defined(CFG_SWITCH_STAT_INCLUDED)
        /* Clear statistics */
        board_port_stat_clear_all();
#endif

        _snaketest_txrx(mode, duration);

        if (mode == SNAKETEST_TYPE_INT_MAC || mode == SNAKETEST_TYPE_INT_PHY) {
            /* Disable loopback mode */
            for (uport = snaketest_min_uport ; uport <= snaketest_max_uport ; uport++) {
                board_port_loopback_enable_set(uport, PORT_LOOPBACK_NONE);
            }
        }
    }
}
#endif /* CFG_SWITCH_SNAKETEST_INCLUDED */
