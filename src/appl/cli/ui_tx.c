/*
 * $Id: ui_tx.c,v 1.5 Broadcom SDK $
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
 
#ifdef __C51__
#ifdef CODE_USERCLASS
#pragma userclass (code = uitx)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "appl/cli.h"
#include "utils/ui.h"
#include "utils/ports.h"

#if (CFG_CLI_ENABLED && CFG_RXTX_SUPPORT_ENABLED && CFG_CLI_TX_CMD_ENABLED)

/* TX packet configurations */
typedef struct tx_async_s {
    uint32      count;
    uint32      current;
    BOOL        timer;
    BOOL        done;
    BOOL        verbose;
} tx_async_t;
STATIC sys_pkt_t *tx_pkts[CFG_CLI_TX_MAX_PKTCFGS];
STATIC CODE uint8 tx_default_sa[6] = { 0x00, 0x10, 0x18, 0x55, 0x44, 0x4B };
STATIC CODE uint8 tx_bcast_da[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
STATIC uint8 tx_src_mac[6];

/* Forwards */
void APIFUNC(ui_tx_init)(void) REENTRANT;
APISTATIC void cli_cmd_switch_tx(CLI_CMD_OP op) REENTRANT;
APISTATIC void cli_switch_tx_edit_pktcfg(sys_pkt_t *pkt) REENTRANT;
APISTATIC void cli_switch_tx_list_pktcfg(int8 idx, sys_pkt_t *pkt) REENTRANT;
APISTATIC void cli_switch_tx_send_pktcfg(sys_pkt_t *pkt) REENTRANT;
APISTATIC void cli_switch_tx_async_send(tx_async_t *async, sys_pkt_t *pkt) REENTRANT;
APISTATIC void cli_switch_tx_async_timer(void *arg) REENTRANT;
APISTATIC void cli_switch_tx_async_cbk(sys_pkt_t *pkt, sys_error_t status) REENTRANT;

STATICFN int8
cli_switch_tx_select_config(void) REENTRANT
{
    uint32 v;
    sal_printf("Select packet config (0 ~ %bu): ", 
        (uint8)CFG_CLI_TX_MAX_PKTCFGS - 1);
    if (ui_get_decimal(&v, NULL) == UI_RET_OK) {
        if (v >= CFG_CLI_TX_MAX_PKTCFGS) {
            sal_printf("Invalid choice.\n");
            return -1;
        }
        return (int8)v;
    }
    return -1;
}

APISTATIC void
APIFUNC(cli_switch_tx_edit_pktcfg)(sys_pkt_t *pkt) REENTRANT
{
    uint32 val32;
    uint16 val16;
    uint8 val8;
    uint8 *pval8;
    uint8 uplist[MAX_UPLIST_WIDTH];
    char c;
    ui_ret_t r;


    sal_printf("Port bitmap to send (0 means follow switch rules):\n");
    sal_memcpy(uplist, pkt->tx_uplist, MAX_UPLIST_WIDTH);
    if (ui_get_bytes(uplist, MAX_UPLIST_WIDTH, "Port list", TRUE) == UI_RET_OK) {
        sal_memcpy(pkt->tx_uplist, uplist, MAX_UPLIST_WIDTH);
    }
    
    if (uplist_is_empty(pkt->tx_uplist) != SYS_OK) {
        val8 = ((pkt->flags & SYS_TX_FLAG_USE_UNTAG_PORT_LIST)? 1 : 0);
        sal_printf("VLAN tagging: \n"
                   " 0: Follow switch tagging rules\n"
                   " 1: Use untag port bitmap\n"
                   "Enter your choice: [%bu] ",
                   val8);
        r = ui_get_decimal(&val32, NULL);
        if (r == UI_RET_EMPTY || r == UI_RET_OK) {
            if (r == UI_RET_EMPTY) {
                val32 = val8;
            }
            
            if (val32 > 1) {
                sal_printf("Invalid choice.\n");
            } else if (val32 == 0) {
                pkt->flags &= ~SYS_TX_FLAG_USE_UNTAG_PORT_LIST;
            } else {
                sal_printf("Untag port bitmap:\n");
                sal_memcpy(uplist, pkt->tx_untag_uplist, MAX_UPLIST_WIDTH);
                r = ui_get_bytes(uplist, MAX_UPLIST_WIDTH, "Port list", TRUE);
                if (r == UI_RET_OK) {
                    pkt->flags |= SYS_TX_FLAG_USE_UNTAG_PORT_LIST;
                    sal_memcpy(pkt->tx_untag_uplist, uplist, MAX_UPLIST_WIDTH);
                }
                    
            }
        }

    } else {
        uplist_clear(pkt->tx_uplist);
        sal_printf("VLAN tagging: \n"
                   " 0: Follow switch tagging rules\n"
                   " 1: Untag all \n"
                   " 2: Tag all\n");
        r = ui_get_decimal(&val32, "Enter your choice: [0] ");
        if (r == UI_RET_EMPTY || r == UI_RET_OK) {
            if (val32 == 0 || r == UI_RET_EMPTY) {
                pkt->tx_tag_mode = SYS_TX_TAG_MODE_FOLLOW_SWITCH_RULES;
            } else if (val32 == 1) {
                pkt->tx_tag_mode = SYS_TX_TAG_MODE_UNTAG_ALL;
            } else if (val32 == 2) {
                pkt->tx_tag_mode = SYS_TX_TAG_MODE_TAG_ALL;
            }
        }
    }

    sal_printf("CoS: [%u] ", pkt->cos);
    if (ui_get_decimal(&val32, NULL) == UI_RET_OK) {
        pkt->cos = (uint16)val32;
    }

    if (ui_yes_or_no("Request timestamp?", 
                ((pkt->flags & SYS_TX_FLAG_TIMESTAMP_REQUEST) ? 1 : 0))) {
        pkt->flags |= SYS_TX_FLAG_TIMESTAMP_REQUEST;
    } else {
        pkt->flags &= ~SYS_TX_FLAG_TIMESTAMP_REQUEST;
    }
    
    if (ui_yes_or_no("Fill packet content with pattern?", 0)) {
        sal_printf("  0 - Fill with fixed byte\n"
                   "  1 - Fill with increment bytes\n"
                   "Enter your choice: ");
        c = sal_getchar();
        sal_putchar('\n');
        if (c == '0') {
            if (ui_get_byte(&val8, "Pattern byte: ") == UI_RET_OK) {
                pval8 = pkt->pkt_data + 12;
                for(val16=12; val16<pkt->pkt_len; val16++, pval8++) {
                    *pval8 = val8;
                }
            }
        } else if (c == '1') {
            pval8 = pkt->pkt_data + 12;
            val8 = 0;
            for(val16=12; val16<pkt->pkt_len; val16++, pval8++, val8++) {
                *pval8 = val8;
            }
        }
    }
    
    if (ui_yes_or_no("Modify DA?", 0)) {
        if (ui_get_bytes(pkt->pkt_data, 6, "DA", TRUE) != UI_RET_OK) {
            sal_printf("Cancelled.\n");
        }
    }
    
    if (ui_yes_or_no("Modify SA?", 0)) {
        if (ui_get_bytes(pkt->pkt_data + 6, 6, "SA", TRUE) != UI_RET_OK) {
            sal_printf("Cancelled.\n");
        }
    }
    
    val32 = 0; /* To mark if VLAN ID is set or not */
    if (ui_yes_or_no("Set VLAN tag?", 1)) {
        r = ui_get_decimal(&val32, " VLAN ID (1 ~ 4094): [1] ");
        if (r == UI_RET_OK) {
            if (val32 < 1 || val32 > 4094) {
                sal_printf("Invalid VLAN ID!\n");
                val32 = 0;
            }
        } else if (r == UI_RET_EMPTY) {
            val32 = 1;
        } else {
            val32 = 0;
        }

        if (val32 != 0) {
            pval8 = &pkt->pkt_data[12];
            *pval8++ = 0x81;
            *pval8++ = 0x00;
            *pval8++ = (uint8)((val32 >> 8) & 0xff);
            *pval8++ = (uint8)(val32 & 0xff);
        }
    }
    
    if (ui_yes_or_no("Set ethertype?", 0)) {
        if (ui_get_word(&val16, " Ethertype: ") == UI_RET_OK) {
            pval8 = &pkt->pkt_data[12 + (val32? 4 : 0)];
            *pval8++ = (uint8)((val16 >> 8) & 0xff);
            *pval8++ = (uint8)(val16 & 0xff);
        }
    }
#ifdef __C51__    
    sal_printf("\nPacket size: %u (0x%X)\n"
               "Buffer address: 0x%04X ~ 0x%04X\n"
               "You can use 'e' command to edit packet content.\n", 
               pkt->pkt_len, pkt->pkt_len, (uint16)pkt->pkt_data, 
               (uint16)pkt->pkt_data + pkt->pkt_len);
#else
    sal_printf("\nPacket size: %u (0x%X)\n"
               "Buffer address: 0x%08X ~ 0x%08X\n"
               "You can use 'e' command to edit packet content.\n", 
               pkt->pkt_len, pkt->pkt_len, (uint32)pkt->pkt_data, 
               (uint32)pkt->pkt_data + pkt->pkt_len);
#endif
}

APISTATIC void
APIFUNC(cli_switch_tx_list_pktcfg)(int8 idx, sys_pkt_t *pkt) REENTRANT
{
    uint8 i;
    sal_printf(" [%bd]: ", idx);
    if (pkt == NULL) {
        sal_printf("(empty)\n");
    } else {
#ifdef __C51__
        sal_printf("BUF:0x%04X ", (uint16)pkt->pkt_data);
#else
        sal_printf("BUF:0x%08X ", (uint32)pkt->pkt_data);
#endif
        sal_printf("LEN:%u ", pkt->pkt_len);
        sal_printf("DA:");
        for(i=0; i<6; i++) {
            sal_printf("%02bX", (pkt->pkt_data)[i]);
        }
        sal_printf(" PBMP:0x");
        for(i=0; i<MAX_UPLIST_WIDTH; i++) {
            sal_printf("%02bX", pkt->tx_uplist[i]);
        }
        if (uplist_is_empty(pkt->tx_uplist) != SYS_OK) {
            sal_printf(" UNTAG:");
            if (pkt->flags & SYS_TX_FLAG_USE_UNTAG_PORT_LIST) {
                sal_printf("0x");
                for(i=0; i<MAX_UPLIST_WIDTH; i++) {
                    sal_printf("%02bX", pkt->tx_untag_uplist[i]);
                }
            } else {
                sal_printf("N/A");
            }
        } else {
            sal_printf(" TAGMODE:");
            switch(pkt->tx_tag_mode) {
            case SYS_TX_TAG_MODE_UNTAG_ALL:
                sal_printf("untag");
                break;
            case SYS_TX_TAG_MODE_TAG_ALL:
                sal_printf("tag");
                break;
            default:
                sal_printf("N/A");
                break;
            }
        }
        
        sal_printf(" COS:%u", pkt->cos);
        
        if (pkt->flags & SYS_TX_FLAG_TIMESTAMP_REQUEST) {
            sal_printf(" TS");
        }
        
        sal_putchar('\n');
    }
}

void
APIFUNC(cli_switch_tx_async_send)(tx_async_t *async, sys_pkt_t *pkt) REENTRANT
{
    sys_error_t r;
    if (async->verbose) {
        sal_printf("Sending packet %lu ..... ", async->current + 1);
    }
    
    async->done = FALSE;
    r = sys_tx(pkt, cli_switch_tx_async_cbk);
    if (r != SYS_OK) {
        sal_printf("\n TX ERROR(%d) when sending %luth packet!\n", 
            (int16)r, async->current + 1);
    }
}

void 
APIFUNC(cli_switch_tx_async_timer)(void *arg) REENTRANT
{
    sys_pkt_t *pkt = (sys_pkt_t *)arg;
    tx_async_t *async = (tx_async_t *)pkt->cookie;
    
    if (!async->done) {
        return;
    }
    
    cli_switch_tx_async_send(async, pkt);
}

APISTATIC void 
APIFUNC(cli_switch_tx_async_cbk)(sys_pkt_t *pkt, sys_error_t status) REENTRANT
{
    tx_async_t *async = (tx_async_t *)pkt->cookie;
    
    async->done = TRUE;
    if (status != SYS_OK) {
        if (async->timer) {
            timer_remove(cli_switch_tx_async_timer);
        }
        sal_printf("\n TX ERROR(%d) when sending %luth packet!\n", 
            (int16)status, async->current + 1);
        return;
    }
        
    if (async->verbose) {
        sal_printf("Done.\n");
    }
    
    async->current++;
    if (async->current == async->count) {
        if (async->timer) {
            timer_remove(cli_switch_tx_async_timer);
        }
        sal_printf("TX: %lu packet(s) sent.\n", async->count);
        return;
    }
    
    if (!async->timer) {
        cli_switch_tx_async_send(async, pkt);
    }
}

APISTATIC void
APIFUNC(cli_switch_tx_send_pktcfg)(sys_pkt_t *pkt) REENTRANT
{
    uint32 count;
    uint32 gap;
    BOOL blocking;
    uint8 verbose;
    sys_error_t r;
    tick_t ticks = 0;
    ui_ret_t uir;

    
    if (pkt == NULL) {
        sal_printf("Empty packet configuration!\n");
        return;
    }
    
    uir = ui_get_decimal(&count, "Count: [1] ");
    if (uir == UI_RET_OK) {
        if (count == 0) {
            return;
        }
    } else if (uir == UI_RET_EMPTY) {
        count = 1;
    } else {
        return;
    }
    
    if (count > 1) {
        uir = ui_get_decimal(&gap, "Gap in micro seconds: [0] ");
        if (uir == UI_RET_EMPTY) {
            gap = 0;
        } else if (uir != UI_RET_OK) {
            return;
        }
    }
    
    if (gap > 0) {
        ticks = SAL_USEC_TO_TICKS(gap);
    }
    
    blocking = ui_yes_or_no("Blocking mode?", 0);
    
    verbose = ui_yes_or_no("Verbose?", 0);
    
    if (blocking) {

        uint32 c;
        for(c=0; c<count; c++) {
            tick_t start = sal_get_ticks();

            r = sys_tx(pkt, NULL);
            if (verbose) {
                sal_printf("Sending packet %lu ..... ", c + 1);
            }
            if (r != SYS_OK) {
                sal_printf("ERROR(%d)", (int16)r);
                if (verbose) {
                    sal_printf("!\n");
                } else {
                    sal_printf(" when sending %luth packet!\n", c + 1);
                }
                if (c != count - 1) {
                    sal_printf("Aborted.\n");
                }
                return;
            }
            if (verbose) {
                sal_printf("Done.\n");
            }
            
            if (ticks > 0) {
                for(;;) {
                    if (SAL_TIME_EXPIRED(start, ticks)) {
                        break;
                    }
                    POLL();
                }
            }
        }
        sal_printf("TX: %lu packet(s) sent.\n", count);

    } else {
        tx_async_t *async = (tx_async_t *)pkt->cookie;
        async->count = count;
        async->current = 0;
        async->timer = (ticks > 0)? TRUE : FALSE;
        async->verbose = verbose;
        
        if (ticks > 0) {
            timer_add(cli_switch_tx_async_timer, pkt, gap);
        }
        cli_switch_tx_async_send(async, pkt);
    }
}

APISTATIC void
APIFUNC(cli_cmd_switch_tx)(CLI_CMD_OP op) REENTRANT
{
    sys_pkt_t *pkt = NULL;
    int8 idx = 0;
    uint32 val32;
    ui_ret_t r;


    if (op == CLI_CMD_OP_HELP) {
        sal_printf("Command to transmit one or more packets.\n"
                   "You can configure packet attributes and content\n"
                   "and sent them at a later time.\n"
                   "Note: Be sure to clear the packet configuration after\n"
                   "you're done with it since it occupies system memory.\n"
                   );
    } else if (op == CLI_CMD_OP_DESC) {
        sal_printf("Transmit packet(s)");
    } else {
        char c;
        sal_printf("  l - List packet configurations\n"
                   "  e - Edit packet configuration\n"
                   "  t - Transmit packet(s) using packet configuration\n"
                   "  c - Clear packet configuration\n"
                   "  s - Change default source MAC\n"
                   "Enter your choice: ");
        c = sal_getchar();
        sal_putchar('\n');
        if (c == 'e' || c == 't' || c == 'c') {
            idx = cli_switch_tx_select_config();
            if (idx < 0) {
                return;
            }
            pkt = tx_pkts[idx];
        }
        if (c == 'e') {
            if (pkt == NULL) {
                sal_printf("Packet length (%u ~ %u) in decimal: [%u] ",
                    (uint16)MIN_PACKET_LENGTH, 
                    (uint16)MAX_PACKET_LENGTH,
                    (uint16)MIN_PACKET_LENGTH);
                r = ui_get_decimal(&val32, " ");
                if (r == UI_RET_EMPTY) {
                    val32 = MIN_PACKET_LENGTH;
                } else if (r != UI_RET_OK) {
                    return;
                }
                if (val32 < MIN_PACKET_LENGTH || val32 > MAX_PACKET_LENGTH) {
                    sal_printf("Invalid packet length!\n");
                    return;
                }
                pkt = (sys_pkt_t *)sal_malloc(sizeof(sys_pkt_t));
                if (pkt == NULL) {
                    sal_printf("Out of memory!\n");
                    return;
                }
                sal_memset(pkt, 0, sizeof(sys_pkt_t));
                pkt->pkt_data = (uint8 *)sal_dma_malloc(val32 + 4); /* CRC */
                if (pkt->pkt_data == NULL) {
                    sal_printf("Out of memory!\n");
                    sal_free(pkt);
                    return;
                }
                pkt->cookie = (void *)sal_malloc(sizeof(tx_async_t));
                if (pkt->cookie == NULL) {
                    sal_printf("Out of memory!\n");
                    sal_dma_free(pkt->pkt_data);
                    sal_free(pkt);
                    return;
                }
                
                pkt->buf_len = val32 + 4;
                pkt->pkt_len = val32;

                /* Set default DA and SA */                
                sal_memcpy(pkt->pkt_data, tx_bcast_da, 6);
                sal_memcpy(pkt->pkt_data + 6, tx_src_mac, 6);
                
                tx_pkts[idx] = pkt;
            }
            
            cli_switch_tx_edit_pktcfg(pkt);

        } else if (c == 's') {
            if (ui_get_bytes(tx_src_mac, 6, "SA", TRUE) != UI_RET_OK) {
                sal_printf("Cancelled.\n");
            }
        } else if (c == 'l') {
            
            for(idx=0; idx<CFG_CLI_TX_MAX_PKTCFGS; idx++) {
                cli_switch_tx_list_pktcfg(idx, tx_pkts[idx]);
            }

        } else if (c == 't') {
            
            cli_switch_tx_send_pktcfg(pkt);
            
        } else if (c == 'c') {
            if (pkt != NULL) {
                if (pkt->cookie != NULL) {
                    sal_free(pkt->cookie);
                }
                if (pkt->pkt_data != NULL) {
                    sal_dma_free(pkt->pkt_data);
                }
                sal_free(pkt);
                tx_pkts[idx] = NULL;
            }
            sal_printf("Done.\n");
        } else {
            sal_printf("Invalid choice.\n");
        }
    }
}

void
APIFUNC(ui_tx_init)(void) REENTRANT
{
    uint8 i;
    
    /* TX */
    for(i=0; i<CFG_CLI_TX_MAX_PKTCFGS; i++) {
        tx_pkts[i] = NULL;
    }
    cli_add_cmd('t', cli_cmd_switch_tx);
    sal_memcpy(tx_src_mac, tx_default_sa, 6);
}

#endif
