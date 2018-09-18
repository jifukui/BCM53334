/*
 * $Id: ui_utils.c,v 1.12 Broadcom SDK $
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
#pragma userclass (code = uutl)
#endif /* CODE_USERCLASS */
#endif /* __C51__ */

#include "system.h"
#include "utils/ui.h"
#include "sal.h"


extern uint32 counter_val[BCM5333X_LPORT_MAX][R_MAX * 2];





#if CFG_CONSOLE_ENABLED

void
APIFUNC(ui_backspace)(void) REENTRANT
{
    sal_putchar(UI_KB_BS);
}

void
APIFUNC(ui_dump_memory)(uint8 *addr, uint16 len) REENTRANT
{
    uint8 p;
    uint8 c, buf[17];
    
    p = 0;
    for(;;) {
        
        if (len == 0) {
            break;
        }
        
        if (p == 0) {
            sal_printf("\n %08lX: ", (uint32)DATAPTR2MSADDR(addr));
        }
        
        c = *addr;
        if (p == 8) {
            sal_putchar(' ');
        }
        sal_printf("%02bX ", c);
        if (c < ' ' || c > '~') {
            c = '.';
        }
        
        buf[p] = c;
        
        p++;
        if (p == 16 || len == 1) {
            buf[p] = 0;
            sal_printf("   %s", buf);
            p = 0;
        }
        
        addr++;
        len--;

        /* Cancel per user request */        
        while (sal_char_avail()) {
            char ch = sal_getchar();
            ui_backspace();
            if (ch == UI_KB_ESC || ch == UI_KB_CTRL_C) {
                sal_printf("\n");
                return;
            }
        }
    }
    
    sal_printf("\n");
}

#endif /* CFG_CONSOLE_ENABLED */

#if CFG_CLI_ENABLED

ui_ret_t
APIFUNC(ui_get_hex)(uint32 *value, uint8 size) REENTRANT
{
    uint8 len;
    char ch;
    
    if (value == NULL) {
        return UI_RET_ERROR;
    }

    len = 0;
    *value = 0;
    sal_printf("0x");
    while(len <= size) {
        ch = sal_getchar();
        if (ch == UI_KB_LF || ch == UI_KB_CR) {
            break;
        } else if (ch == UI_KB_ESC || ch == UI_KB_CTRL_C) {
            /* Cancelled */
            ui_backspace();
            if (ch == UI_KB_ESC) {
                sal_putchar(UI_KB_CR);
            }
            sal_putchar('\n');
            return UI_RET_CANCEL;
        } else if (ch == UI_KB_BS || ch == UI_KB_DEL) {
            if (len > 0) {
                len--;
                *value >>= 4;
#ifdef __C51__
                sal_putchar(' ');
#endif
                ui_backspace();

            } else {
                sal_putchar('x');
            }
        } else if (len == size) {
            /* Can only accept Backspace or ENTER */
            ui_backspace();
            sal_putchar(' ');
            ui_backspace();
        } else if (ch >= '0' && ch <= '9') {
            len++;
            ch -= '0';
            *value <<= 4;
            *value += ch;
        } else if (ch >= 'A' && ch <= 'F') {
            len++;
            ch -= 'A';
            *value <<= 4;
            *value += ch + 10;
        } else if (ch >= 'a' && ch <= 'f') {
            len++;
            ch -= 'a';
            *value <<= 4;
            *value += ch + 10;
        } else {
            ui_backspace();
            sal_putchar(' ');
            ui_backspace();
        }
    }
    
    if (len == 0) {
        return UI_RET_EMPTY;
    }
    
    return UI_RET_OK;
}

ui_ret_t
APIFUNC(ui_get_byte)(uint8 *val, const char *str) REENTRANT
{
    uint32 value;
    ui_ret_t r;
    
    if (val == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
    }
    r = ui_get_hex(&value, 2);
    *val = (uint8)(value & 0xFF);
    return r;
}

ui_ret_t
APIFUNC(ui_get_word)(uint16 *val, const char *str) REENTRANT
{
    uint32 value;
    ui_ret_t r;

    if (val == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
    }
    r = ui_get_hex(&value, 4);
    *val = (uint16)(value & 0xFFFF);
    return r;
}

ui_ret_t
APIFUNC(ui_get_dword)(uint32 *val, const char *str) REENTRANT
{
    uint32 value;
    ui_ret_t r;

    if (val == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
    }
    r = ui_get_hex(&value, 8);
    *val = value;
    return r;
}

ui_ret_t
APIFUNC(ui_get_address)(uint8 **paddr, const char *str) REENTRANT
{
    uint32 value;
    uint8 len = sizeof(msaddr_t) * 2;  /* 2 digits per byte */    
    ui_ret_t r;

    if (paddr == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
    }
    r = ui_get_hex(&value, len);
    *paddr = MSADDR2DATAPTR(value);
    sal_putchar(' ');
    return r;
}

#if CFG_SAL_LIB_SUPPORT_ENABLED
ui_ret_t
APIFUNC(ui_get_decimal)(uint32 *pvalue, const char *str) REENTRANT
{
    char ch, last;
    uint32 value = 0;
    uint8 len;
    
    if (pvalue == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
        last = str[sal_strlen(str) - 1];
    } else {
        last = ' ';
    }
    len = 0;
    while(len <= 10) {
        ch = sal_getchar();
        if (ch == UI_KB_LF || ch == UI_KB_CR) {
            break;
        } else if (ch == UI_KB_ESC || ch == UI_KB_CTRL_C) {
            /* Cancelled */
            ui_backspace();
            if (ch == UI_KB_ESC) {
                sal_putchar(UI_KB_CR);
            }
            sal_putchar('\n');
            return UI_RET_CANCEL;
        } else if (ch == UI_KB_BS || ch == UI_KB_DEL) {
            if (len > 0) {
                len--;
                value /= 10;
#ifdef __C51__
                sal_putchar(' ');
#endif
                ui_backspace();
            } else {
                sal_putchar(last);
            }
        } else if (len == 10) {
            /* Can only accept Backspace or ENTER */
            ui_backspace();
            sal_putchar(' ');
            ui_backspace();
        } else if (ch >= '0' && ch <= '9') {
            len++;
            ch -= '0';
            value *= 10;
            value += ch;
        } else {
            ui_backspace();
            sal_putchar(' ');
            ui_backspace();
        }
    }

    if (len == 0) {
        return UI_RET_EMPTY;
    }    
    
    *pvalue = value;
    return UI_RET_OK;
}

BOOL
APIFUNC(ui_yes_or_no)(const char *str, uint8 suggested) REENTRANT
{
    char ch;
    
    for(;;) {
        if (str != NULL) {
            sal_printf(str);
        }
        if (suggested == 0) {
            sal_printf(" [N/y] ");
        } else if (suggested == 1) {
            sal_printf(" [Y/n] ");
        } else if (suggested == 2) {
            sal_printf(" [y/n] ");
        }
    
        ch = sal_getchar();
        if (ch == UI_KB_LF || ch == UI_KB_CR) {
            if (suggested == 0) {
                return FALSE;
            }
            if (suggested == 1) {
                return TRUE;
            }
            continue;
        } 
        if (ch == UI_KB_ESC) {
            ui_backspace();
            sal_putchar(UI_KB_CR);
        }
        sal_putchar('\n');
        if (ch == 'Y' || ch == 'y') {
            return TRUE;
        } else if (ch == 'N' || ch == 'n') {
            return FALSE;
        }
    }
}
#endif /* CFG_SAL_LIB_SUPPORT_ENABLED */

ui_ret_t
APIFUNC(ui_get_bytes)(uint8 *pbytes, uint8 len, const char *str, BOOL show_org) REENTRANT
{
    uint8 i;
    uint32 value;
    ui_ret_t r;
    
    if (pbytes == NULL) {
        return UI_RET_ERROR;
    }

    for(i=0; i<len; i++) {
        if (str != NULL && str[0] != 0) {
            sal_printf(" %s", str);
        }
        sal_printf(" byte %bu: ", i);
        if (show_org) {
            sal_printf("[0x%02bX] ", pbytes[i]);
        }

        r = ui_get_hex(&value, 2);
        if (r == UI_RET_OK) {
            pbytes[i] = (uint8)(value & 0xFF);
        } else if (r == UI_RET_EMPTY) {
            continue;
        } else {
            return r;
        }
    }
    return UI_RET_OK;    
}

#if CFG_SAL_LIB_SUPPORT_ENABLED
ui_ret_t
APIFUNC(ui_get_string)(char *pbytes, uint8 len, const char *str) REENTRANT
{
    char ch, last;
    uint8 ch_len = 0;
    if (pbytes == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
        last = str[sal_strlen(str) - 1];
    } else {
        last = ' ';
    }

    for(;;) {
        ch = sal_getchar();
        if (ch == UI_KB_LF || ch == UI_KB_CR) {
            if (!ch_len) {
                return UI_RET_EMPTY;
            }
            *(pbytes+ch_len) = '\0';
            break;
        } else if (ch == UI_KB_BS || ch == UI_KB_DEL) {
            if (ch_len > 0) {
                ch_len--;
                ui_backspace();
                sal_putchar(' ');
                ui_backspace();
            } else {
                ui_backspace();
                sal_putchar(last);
            }
            continue;
        } else if (ch_len == len) {
            /* Can only accept Backspace or ENTER */
            ui_backspace();
            sal_putchar(' ');
            ui_backspace();
            continue;
        }
        *(pbytes+ch_len) = ch;
        ch_len++;
    }
    return UI_RET_OK;
}


#ifdef CFG_XCOMMAND_INCLUDED
ui_ret_t
APIFUNC(ui_get_secure_string)(char *pbytes, uint8 len, const char *str) REENTRANT
{
    char ch, last;
    uint8 ch_len = 0;
    if (pbytes == NULL) {
        return UI_RET_ERROR;
    }

    if (str != NULL) {
        sal_printf(str);
        last = str[sal_strlen(str) - 1];
    } else {
        last = ' ';
    }

    for(;;) {
        ch = get_char();
        if (ch == UI_KB_LF || ch == UI_KB_CR) {
            sal_printf("\r\n");
            if (!ch_len) {
                return UI_RET_EMPTY;
            }
            
            *(pbytes+ch_len) = '\0';
            break;
        } else if (ch == UI_KB_BS || ch == UI_KB_DEL) {
            if (ch_len > 0) {
                ch_len--;
                ui_backspace();
                sal_putchar(' ');
                ui_backspace();
            } else {
                ui_backspace();
                sal_putchar(last);
            }
            continue;
        } else if (ch_len == len) {
            /* Can only accept Backspace or ENTER */
            ui_backspace();
            sal_putchar(' ');
            ui_backspace();
            continue;
        }
        put_char('*');
        *(pbytes+ch_len) = ch;
        ch_len++;
    }
    return UI_RET_OK;
}
#endif
#endif /* CFG_SAL_LIB_SUPPORT_ENABLED */

/* l2x */
char *soc_L2X_M_views[] = {
    "ASSOCIATED_DATAf",
    "CLASS_IDf",
    "CPUf",
    "DESTINATIONf",
    "DESTINATION_1f",
    "DEST_TYPEf",
    "DST_DISCARDf",
    "DUMMY_INDEXf",
    "EVEN_PARITYf",
    "HITDAf",
    "HITSAf",   
    "KEY_TYPEf",    
    "L2MC_PTRf",
    "L3f",
    "LIMIT_COUNTEDf",
    "LOCAL_SAf",
    "MAC_ADDRf",
    "MAC_BLOCK_INDEXf",
    "MIRRORf",
    "MIRROR0f",
    "MODULE_IDf",    
    "OVIDf",
    "PENDINGf",
    "PORT_NUMf",
    "PRIf",
    "REMOTEf",
    "REMOTE_TRUNKf",
    "RPEf",
    "SCPf",
    "SRC_DISCARDf",
    "STATIC_BITf",
    "Tf",
    "TGIDf",
    "VALIDf", 
    "VPGf",
};

soc_field_info_t soc_L2X_M_fields[] = {
    { ASSOCIATED_DATAf, 37, 62, SOCF_LE },
    { CLASS_IDf, 6, 78, SOCF_LE },
    { CPUf, 1, 91, 0 },
    { DESTINATIONf, 13, 62, SOCF_LE },
    { DESTINATION_1f, 13, 47, SOCF_LE },
    { DEST_TYPEf, 2, 75, SOCF_LE },
    { DST_DISCARDf, 1, 92, 0 },
    { DUMMY_INDEXf, 1, 83, SOCF_RES },
    { L2X_EVEN_PARITYf, 1, 100, 0 | SOCF_GLOBAL },
    { HITDAf, 1, 101, 0 | SOCF_GLOBAL },
    { HITSAf, 1, 102, 0 | SOCF_GLOBAL },   
    { KEY_TYPEf, 2, 0, SOCF_LE | SOCF_GLOBAL },    
    { L2MC_PTRf, 10, 62, SOCF_LE },
    { L3f, 1, 77, 0 },
    { LIMIT_COUNTEDf, 1, 95, 0 },
    { LOCAL_SAf, 1, 103, 0 | SOCF_GLOBAL },
    { MAC_ADDRf, 48, 14, SOCF_LE },
    { MAC_BLOCK_INDEXf, 5, 78, SOCF_LE },
    { L2X_MIRRORf, 1, 86, 0 },
    { L2X_MIRROR0f, 1, 86, 0 },
    { MODULE_IDf, 7, 68, SOCF_LE },    
    { L2X_OVIDf, 12, 2, SOCF_LE },
    { PENDINGf, 1, 98, 0 },
    { PORT_NUMf, 6, 62, SOCF_LE },
    { PRIf, 3, 88, SOCF_LE },
    { REMOTEf, 1, 96, 0 },
    { REMOTE_TRUNKf, 1, 74, 0 },
    { RPEf, 1, 85, 0 },
    { SCPf, 1, 94, 0 },
    { SRC_DISCARDf, 1, 93, 0 },
    { STATIC_BITf, 1, 97, 0 },
    { Tf, 1, 75, 0 },
    { TGIDf, 7, 62, SOCF_LE },
    { L2X_VALIDf, 1, 99, 0 | SOCF_GLOBAL }, 
    { VPGf, 13, 62, SOCF_LE|SOCF_RES },
};

soc_mem_info_t SOC_L2X_M = { 
	"L2_ENTRY",
	SOC_MEM_FLAG_VALID,
	0,
	16383,
	10,
	0x1c000000,
	1,
	13,
	L2X_FIELD_MAX,
	soc_L2X_M_fields,	  
	soc_L2X_M_views,	
  };

/* port table */
char *soc_port_tab_m_views[] = {	
    "ALLOW_SRC_MODf",
    "CFI_0_MAPPINGf",
    "CFI_1_MAPPINGf",
    "CFI_AS_CNGf",
    "CLASS_BASED_SM_ENABLEf",
    "CML_FLAGS_MOVEf",
    "CML_FLAGS_NEWf",
    "CTRL_PROFILE_INDEX_1588f",
    "DATA_0f",
    "DATA_1f",
    "DISABLE_STATIC_MOVE_DROPf",
    "DROP_BPDUf",
    "DUAL_MODID_ENABLEf",
    "EN_IFILTERf",
    "FILTER_ENABLEf",
    "FP_PORT_SELECT_TYPEf",
    "HIGIG2f",
    "HIGIG_TRUNKf",
    "HIGIG_TRUNK_IDf",
    "IEEE_802_1AS_ENABLEf",
    "IGNORE_IPMC_L2_BITMAPf",
    "IGNORE_IPMC_L3_BITMAPf",
    "INNER_TPID_ENABLEf",
    "IPMC_DO_VLANf",
    "IVIDf",
    "MAC_BASED_VID_ENABLEf",
    "MAC_IP_BIND_LOOKUP_MISS_DROPf",
    "MAP_TAG_PKT_PRIORITYf",
    "MDL_BITMAPf",
    "MH_INGRESS_TAGGED_SELf",
    "MIRRORf",
    "MIRROR0f",
    "MY_MODIDf",
    "OAM_ENABLEf",
    "OUTER_TPID_ENABLEf",
    "OUTER_TPID_VERIFYf",
    "OVIDf",
    "PASS_CONTROL_FRAMESf",
    "PORT_BRIDGEf",
    "PORT_DIS_TAGf",
    "PORT_DIS_UNTAGf",
    "PORT_PRIf",
    "PORT_TYPEf",
    "PORT_VIDf",
    "PRI_MAPPINGf",
    "PROTOCOL_PKT_INDEXf",
    "PVLAN_ENABLEf",
    "REMOTE_CPU_ENf",
    "REMOVE_HG_HDR_SRC_PORTf",
    "RESERVED_0f",
    "RESERVED_1f",
    "RESERVED_2f",
    "SUBNET_BASED_VID_ENABLEf",
    "TAG_ACTION_PROFILE_PTRf",
    "TRUST_DOT1P_PTRf",
    "TRUST_DSCP_V4f",
    "TRUST_DSCP_V6f",
    "TRUST_INCOMING_VIDf",
    "TRUST_OUTER_DOT1Pf",
    "USE_INCOMING_DOT1Pf",
    "USE_INNER_PRIf",
    "USE_IVID_AS_OVIDf",
    "USE_PORT_TABLE_GROUP_IDf",
    "V4IPMC_ENABLEf",
    "V4IPMC_L2_ENABLEf",
    "V4L3_ENABLEf",
    "V6IPMC_ENABLEf",
    "V6IPMC_L2_ENABLEf",
    "V6L3_ENABLEf",
    "VFP_ENABLEf",
    "VFP_PORT_GROUP_IDf",
    "VLAN_PRECEDENCEf",
    "VLAN_PROTOCOL_DATA_INDEXf",
    "VT_ENABLEf",
    "VT_KEY_TYPEf",
    "VT_KEY_TYPE_2f",
    "VT_KEY_TYPE_2_USE_GLPf",
    "VT_KEY_TYPE_USE_GLPf",
    "VT_MISS_DROPf",
};


soc_field_info_t soc_PORT_TAB_M_fields[] = {
    { ALLOW_SRC_MODf, 1, 56, 0 | SOCF_GLOBAL },
    { CFI_0_MAPPINGf, 1, 153, 0 | SOCF_GLOBAL },
    { CFI_1_MAPPINGf, 1, 154, 0 | SOCF_GLOBAL },
    { PORT_TAB_CFI_AS_CNGf, 4, 80, SOCF_LE | SOCF_GLOBAL },
    { CLASS_BASED_SM_ENABLEf, 1, 120, 0 | SOCF_GLOBAL },
    { CML_FLAGS_MOVEf, 4, 125, SOCF_LE | SOCF_GLOBAL },
    { CML_FLAGS_NEWf, 4, 121, SOCF_LE | SOCF_GLOBAL },
    { CTRL_PROFILE_INDEX_1588f, 6, 209, SOCF_LE | SOCF_GLOBAL },
    { DATA_0f, 114, 0, SOCF_LE | SOCF_GLOBAL },
    { DATA_1f, 114, 114, SOCF_LE | SOCF_GLOBAL },
    { DISABLE_STATIC_MOVE_DROPf, 1, 161, 0 | SOCF_GLOBAL },
    { DROP_BPDUf, 1, 18, 0 | SOCF_GLOBAL },
    { DUAL_MODID_ENABLEf, 1, 38, 0 | SOCF_GLOBAL },
    { EN_IFILTERf, 1, 5, 0 | SOCF_GLOBAL },
    { FILTER_ENABLEf, 1, 0, 0 | SOCF_GLOBAL },
    { FP_PORT_SELECT_TYPEf, 1, 67, 0 | SOCF_GLOBAL },
    { HIGIG2f, 1, 55, 0 | SOCF_GLOBAL },
    { HIGIG_TRUNKf, 1, 51, 0 | SOCF_GLOBAL },
    { HIGIG_TRUNK_IDf, 1, 52, 0 | SOCF_GLOBAL },
    { IEEE_802_1AS_ENABLEf, 1, 181, 0 | SOCF_GLOBAL },
    { IGNORE_IPMC_L2_BITMAPf, 1, 49, 0 | SOCF_GLOBAL },
    { IGNORE_IPMC_L3_BITMAPf, 1, 50, 0 | SOCF_GLOBAL },
    { INNER_TPID_ENABLEf, 1, 215, 0 | SOCF_GLOBAL },
    { IPMC_DO_VLANf, 1, 11, 0 | SOCF_GLOBAL },
    { IVIDf, 12, 98, SOCF_LE | SOCF_GLOBAL },
    { MAC_BASED_VID_ENABLEf, 1, 23, 0 | SOCF_GLOBAL },
    { MAC_IP_BIND_LOOKUP_MISS_DROPf, 1, 159, 0 | SOCF_GLOBAL },
    { MAP_TAG_PKT_PRIORITYf, 1, 39, 0 | SOCF_GLOBAL },
    { MDL_BITMAPf, 8, 201, SOCF_LE | SOCF_GLOBAL },
    { MH_INGRESS_TAGGED_SELf, 1, 119, 0 | SOCF_GLOBAL },
    { PORT_MIRRORf, 1, 6, 0 | SOCF_GLOBAL },
    { PORT_MIRROR0f, 1, 6, 0 | SOCF_GLOBAL },
    { MY_MODIDf, 7, 40, SOCF_LE | SOCF_GLOBAL },
    { OAM_ENABLEf, 1, 195, 0 | SOCF_GLOBAL },
    { OUTER_TPID_ENABLEf, 4, 85, SOCF_LE | SOCF_GLOBAL },
    { OUTER_TPID_VERIFYf, 1, 89, 0 | SOCF_GLOBAL },
    { PORT_OVIDf, 12, 24, SOCF_LE | SOCF_GLOBAL },
    { PASS_CONTROL_FRAMESf, 1, 21, 0 | SOCF_GLOBAL },
    { PORT_BRIDGEf, 1, 48, 0 | SOCF_GLOBAL },
    { PORT_DIS_TAGf, 1, 19, 0 | SOCF_GLOBAL },
    { PORT_DIS_UNTAGf, 1, 20, 0 | SOCF_GLOBAL },
    { PORT_PRIf, 3, 8, SOCF_LE | SOCF_GLOBAL },
    { PORT_TYPEf, 2, 36, SOCF_LE | SOCF_GLOBAL },
    { PORT_VIDf, 12, 24, SOCF_LE | SOCF_GLOBAL },
    { PRI_MAPPINGf, 24, 129, SOCF_LE | SOCF_GLOBAL },
    { PROTOCOL_PKT_INDEXf, 5, 196, SOCF_LE | SOCF_GLOBAL },
    { PVLAN_ENABLEf, 1, 91, 0 | SOCF_GLOBAL },
    { REMOTE_CPU_ENf, 1, 160, 0 | SOCF_GLOBAL },
    { REMOVE_HG_HDR_SRC_PORTf, 1, 66, 0 | SOCF_GLOBAL },
    { PORT_TAB_RESERVED_0f, 2, 53, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { PORT_TAB_RESERVED_1f, 6, 169, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { PORT_TAB_RESERVED_2f, 12, 216, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { SUBNET_BASED_VID_ENABLEf, 1, 22, 0 | SOCF_GLOBAL },
    { TAG_ACTION_PROFILE_PTRf, 6, 92, SOCF_LE | SOCF_GLOBAL },
    { TRUST_DOT1P_PTRf, 6, 175, SOCF_LE | SOCF_GLOBAL },
    { TRUST_DSCP_V4f, 1, 3, 0 | SOCF_GLOBAL },
    { TRUST_DSCP_V6f, 1, 4, 0 | SOCF_GLOBAL },
    { TRUST_INCOMING_VIDf, 1, 110, 0 | SOCF_GLOBAL },
    { TRUST_OUTER_DOT1Pf, 1, 57, 0 | SOCF_GLOBAL },
    { USE_INCOMING_DOT1Pf, 1, 90, 0 | SOCF_GLOBAL },
    { USE_INNER_PRIf, 1, 84, 0 | SOCF_GLOBAL },
    { USE_IVID_AS_OVIDf, 1, 162, 0 | SOCF_GLOBAL },
    { USE_PORT_TABLE_GROUP_IDf, 1, 182, 0 | SOCF_GLOBAL },
    { V4IPMC_ENABLEf, 1, 13, 0 | SOCF_GLOBAL },
    { V4IPMC_L2_ENABLEf, 1, 15, 0 | SOCF_GLOBAL },
    { V4L3_ENABLEf, 1, 17, 0 | SOCF_GLOBAL },
    { V6IPMC_ENABLEf, 1, 12, 0 | SOCF_GLOBAL },
    { V6IPMC_L2_ENABLEf, 1, 14, 0 | SOCF_GLOBAL },
    { V6L3_ENABLEf, 1, 16, 0 | SOCF_GLOBAL },
    { VFP_ENABLEf, 1, 68, 0 | SOCF_GLOBAL },
    { VFP_PORT_GROUP_IDf, 8, 69, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PRECEDENCEf, 1, 47, 0 | SOCF_GLOBAL },
    { VLAN_PROTOCOL_DATA_INDEXf, 5, 190, SOCF_LE | SOCF_GLOBAL },
    { VT_ENABLEf, 1, 2, 0 | SOCF_GLOBAL },
    { VT_KEY_TYPEf, 3, 111, SOCF_LE | SOCF_GLOBAL },
    { VT_KEY_TYPE_2f, 3, 115, SOCF_LE | SOCF_GLOBAL },
    { VT_KEY_TYPE_2_USE_GLPf, 1, 118, 0 | SOCF_GLOBAL },
    { VT_KEY_TYPE_USE_GLPf, 1, 114, 0 | SOCF_GLOBAL },
    { VT_MISS_DROPf, 1, 1, 0 | SOCF_GLOBAL }
  };

soc_mem_info_t SOC_PORT_TAB_M = {
    "PORT",
    0,  
    0,
    29,
    10,
    0x04000000,
    1,
    29,
    79,
    soc_PORT_TAB_M_fields,
    soc_port_tab_m_views,
};

/* vlan tab*/
char *soc_vlan_tab_m_views[] = {
    "VIRTUAL_PORT_ENf",
    "BC_IDXf",
    "EVEN_PARITYf",
    "FID_IDf",
    "HIGIG_TRUNK_OVERRIDEf",
    "L2_ENTRY_KEY_TYPEf",
    "L3_IIFf",
    "PORT_BITMAPf",
    "PORT_BITMAP_LOf",
    "RESERVED0f",
    "RESERVED1f",
    "STGf",
    "UMC_IDXf",
    "UUC_IDXf",
    "VALIDf",
    "VLAN_CLASS_IDf",
    "VLAN_PROFILE_PTRf",
};


soc_field_info_t soc_VLAN_TAB_M_fields[] = {
	{ VIRTUAL_PORT_ENf, 1, 125, SOCF_LE | SOCF_GLOBAL },
    { BC_IDXf, 12, 89, SOCF_LE | SOCF_GLOBAL },
    { VLAN_EVEN_PARITYf, 1, 126, 0 | SOCF_GLOBAL },
    { FID_IDf, 12, 48, SOCF_LE | SOCF_GLOBAL },
    { HIGIG_TRUNK_OVERRIDEf, 2, 30, SOCF_LE | SOCF_GLOBAL },
    { L2_ENTRY_KEY_TYPEf, 2, 87, SOCF_LE | SOCF_GLOBAL },
    { L3_IIFf, 13, 66, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PORT_BITMAPf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PORT_BITMAP_LOf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { RESERVED0f, 6, 32, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED1f, 2, 64, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { VLAN_STGf, 9, 39, SOCF_LE | SOCF_GLOBAL },
    { UMC_IDXf, 12, 113, SOCF_LE | SOCF_GLOBAL },
    { UUC_IDXf, 12, 101, SOCF_LE | SOCF_GLOBAL },
    { VLAN_VALIDf, 1, 38, 0 | SOCF_GLOBAL },
    { VLAN_CLASS_IDf, 8, 79, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PROFILE_PTRf, 4, 60, SOCF_LE | SOCF_GLOBAL }
};

soc_mem_info_t SOC_VLAN_TAB_M = { 
	"VLAN",
	SOC_MEM_FLAG_VALID,
	0,
	4095,
	10,
	0x14000000,
	1,
	16,
	17,
	soc_VLAN_TAB_M_fields,
	soc_vlan_tab_m_views,

};

/*egr vlan tab*/
char *soc_egr_vlan_tab_m_views[] = {	
    "DOT1P_MAPPING_PTRf",
    "EVEN_PARITYf",
    "OUTER_TPID_INDEXf",
    "PORT_BITMAPf",
    "PORT_BITMAP_HIf",
    "PORT_BITMAP_LOf",
    "REMARK_DOT1Pf",
    "STGf",
    "UT_BITMAPf",
    "UT_BITMAP_HIf",
    "UT_BITMAP_LOf",
    "UT_PORT_BITMAPf",
    "UT_PORT_BITMAP_HIf",
    "UT_PORT_BITMAP_LOf",
    "VALIDf",
};

soc_field_info_t soc_EGR_VLAN_M_fields[] = {
    { DOT1P_MAPPING_PTRf, 4, 72, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_EVEN_PARITYf, 1, 76, 0 | SOCF_GLOBAL },
    { OUTER_TPID_INDEXf, 2, 69, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_PORT_BITMAPf, 30, 30, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_PORT_BITMAP_HIf, 30, 30, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_PORT_BITMAP_LOf, 30, 30, SOCF_LE | SOCF_GLOBAL },
    { REMARK_DOT1Pf, 1, 71, 0 | SOCF_GLOBAL },
    { EGR_VLAN_STGf, 8, 60, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_UT_BITMAPf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_UT_BITMAP_HIf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_UT_BITMAP_LOf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { UT_PORT_BITMAPf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { UT_PORT_BITMAP_HIf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { UT_PORT_BITMAP_LOf, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { EGR_VLAN_VALIDf, 1, 68, 0 | SOCF_GLOBAL }
};

soc_mem_info_t SOC_EGR_VLAN_TAB_M = {
    "EGR_VLAN",
    SOC_MEM_FLAG_VALID,
    0,
    4095,
    11,
    0x100c0000,
    1,
    10,
    15,
    soc_EGR_VLAN_M_fields,
    soc_egr_vlan_tab_m_views,
};

/* vlan stg */
char *soc_stg_tab_m_views[] = {    
    "SP_TREE_PORT0f",
    "SP_TREE_PORT1f",
    "SP_TREE_PORT2f",
    "SP_TREE_PORT3f",
    "SP_TREE_PORT4f",
    "SP_TREE_PORT5f",
    "SP_TREE_PORT6f",
    "SP_TREE_PORT7f",
    "SP_TREE_PORT8f",
    "SP_TREE_PORT9f",
    "SP_TREE_PORT10f",
    "SP_TREE_PORT11f",
    "SP_TREE_PORT12f",
    "SP_TREE_PORT13f",
    "SP_TREE_PORT14f",
    "SP_TREE_PORT15f",
    "SP_TREE_PORT16f",
    "SP_TREE_PORT17f",
    "SP_TREE_PORT18f",
    "SP_TREE_PORT19f",    
    "SP_TREE_PORT20f",
    "SP_TREE_PORT21f",
    "SP_TREE_PORT22f",
    "SP_TREE_PORT23f",
    "SP_TREE_PORT24f",
    "SP_TREE_PORT25f",
    "SP_TREE_PORT26f",
    "SP_TREE_PORT27f",
    "SP_TREE_PORT28f",
    "SP_TREE_PORT29f",
    "EVEN_PARITYf",  
};

soc_field_info_t soc_STG_TAB_M_fields[] = {    
    { SP_TREE_PORT0f, 2, 0, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT1f, 2, 2, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT2f, 2, 4, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT3f, 2, 6, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT4f, 2, 8, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT5f, 2, 10, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT6f, 2, 12, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT7f, 2, 14, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT8f, 2, 16, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT9f, 2, 18, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT10f, 2, 20, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT11f, 2, 22, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT12f, 2, 24, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT13f, 2, 26, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT14f, 2, 28, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT15f, 2, 30, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT16f, 2, 32, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT17f, 2, 34, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT18f, 2, 36, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT19f, 2, 38, SOCF_LE | SOCF_GLOBAL },    
    { SP_TREE_PORT20f, 2, 40, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT21f, 2, 42, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT22f, 2, 44, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT23f, 2, 46, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT24f, 2, 48, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT25f, 2, 50, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT26f, 2, 52, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT27f, 2, 54, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT28f, 2, 56, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT29f, 2, 58, SOCF_LE | SOCF_GLOBAL },
    { SP_EVEN_PARITYf, 1, 60, 0 | SOCF_GLOBAL },
};

soc_mem_info_t SOC_VLAN_STG_TAB_M = { 
    "VLAN_STG",
    0,
    0,
    255,
    10,
    0x14040000,
    1,
    8,
    31,
    soc_STG_TAB_M_fields,
    soc_stg_tab_m_views,
};

soc_mem_info_t SOC_EGR_VLAN_STG_TAB_M = {
    "EGR_VLAN_STG",
    0,        
    0,
    255,    
    11,
    0x10100000,
    1,
    8,
   31,
   soc_STG_TAB_M_fields,
   soc_stg_tab_m_views,    
};

char *soc_vlan_profile_m_views[] = {
    "ICMP_REDIRECT_TOCPUf",
    "IPMCV4_ENABLEf",
    "IPMCV4_L2_ENABLEf",
    "IPMCV6_ENABLEf",
    "IPMCV6_L2_ENABLEf",
    "IPV4L3_ENABLEf",
    "IPV6L3_ENABLEf",
    "IPV6_ROUTING_HEADER_TYPE_0_DROPf",
    "L2_MISS_DROPf",
    "L2_MISS_TOCPUf",
    "L2_NON_UCAST_DROPf",
    "L2_NON_UCAST_TOCPUf",
    "L2_PFMf",
    "L3_IPV4_PFMf",
    "L3_IPV6_PFMf",
    "LEARN_DISABLEf",
    "OUTER_TPID_INDEXf",
    "RESERVED_0f",
    "RESERVED_1f",
    "RESERVED_2f",
    "RESERVED_3f",
    "UNKNOWN_IPV4_MC_TOCPUf",
    "UNKNOWN_IPV6_MC_TOCPUf",
}; 
    
soc_field_info_t soc_VLAN_PROFILE_TAB_m_fields[] = {
    { ICMP_REDIRECT_TOCPUf, 1, 11, 0 | SOCF_GLOBAL },
    { IPMCV4_ENABLEf, 1, 13, 0 | SOCF_GLOBAL },
    { IPMCV4_L2_ENABLEf, 1, 15, 0 | SOCF_GLOBAL },
    { IPMCV6_ENABLEf, 1, 12, 0 | SOCF_GLOBAL },
    { IPMCV6_L2_ENABLEf, 1, 14, 0 | SOCF_GLOBAL },
    { IPV4L3_ENABLEf, 1, 17, 0 | SOCF_GLOBAL },
    { IPV6L3_ENABLEf, 1, 16, 0 | SOCF_GLOBAL },
    { IPV6_ROUTING_HEADER_TYPE_0_DROPf, 1, 22, 0 | SOCF_GLOBAL },
    { L2_MISS_DROPf, 1, 19, 0 | SOCF_GLOBAL },
    { L2_MISS_TOCPUf, 1, 18, 0 | SOCF_GLOBAL },
    { L2_NON_UCAST_DROPf, 1, 21, 0 | SOCF_GLOBAL },
    { L2_NON_UCAST_TOCPUf, 1, 20, 0 | SOCF_GLOBAL },
    { L2_PFMf, 2, 0, SOCF_LE | SOCF_GLOBAL },
    { L3_IPV4_PFMf, 2, 2, SOCF_LE | SOCF_GLOBAL },
    { L3_IPV6_PFMf, 2, 4, SOCF_LE | SOCF_GLOBAL },
    { LEARN_DISABLEf, 1, 8, 0 | SOCF_GLOBAL },
    { VLAN_PROFILE_OUTER_TPID_INDEXf, 2, 9, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PROFILE_RESERVED_0f, 1, 29, SOCF_RES | SOCF_GLOBAL },
    { VLAN_PROFILE_RESERVED_1f, 1, 28, SOCF_RES | SOCF_GLOBAL },
    { VLAN_PROFILE_RESERVED_2f, 1, 27, SOCF_RES | SOCF_GLOBAL },
    { VLAN_PROFILE_RESERVED_3f, 4, 23, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { UNKNOWN_IPV4_MC_TOCPUf, 1, 6, 0 | SOCF_GLOBAL },
    { UNKNOWN_IPV6_MC_TOCPUf, 1, 7, 0 | SOCF_GLOBAL }
};    
    
soc_mem_info_t SOC_VLAN_PROFILE_M = {
    "VLAN_PROFILE",
    0,
    0,
    15,       
    10,
    0x14080000,
    1,
    4,
    23,
    soc_VLAN_PROFILE_TAB_m_fields,
    soc_vlan_profile_m_views,  
};


char *soc_vlan_pfofile_2_m_views[] = {
    "BCAST_MASK_SELf",
    "BLOCK_MASK_Af",
    "BLOCK_MASK_Bf",
    "KNOWN_MCAST_MASK_SELf",
    "UNKNOWN_MCAST_MASK_SELf",
    "UNKNOWN_UCAST_MASK_SELf",
};    

soc_field_info_t soc_VLAN_PROFILE_2_M_fields[] = {
    { BCAST_MASK_SELf, 2, 60, SOCF_LE | SOCF_GLOBAL },
    { BLOCK_MASK_Af, 30, 0, SOCF_LE | SOCF_GLOBAL },
    { BLOCK_MASK_Bf, 30, 30, SOCF_LE | SOCF_GLOBAL },
    { KNOWN_MCAST_MASK_SELf, 2, 66, SOCF_LE | SOCF_GLOBAL },
    { UNKNOWN_MCAST_MASK_SELf, 2, 64, SOCF_LE | SOCF_GLOBAL },
    { UNKNOWN_UCAST_MASK_SELf, 2, 62, SOCF_LE | SOCF_GLOBAL }
};
    
soc_mem_info_t SOC_VLAN_PROFILE_2_M = { 
    "VLAN_PROFILE_2",
     0,
     0,
     15,     
     10,
     0x44400000,
     1,
     9,
     6,
     soc_VLAN_PROFILE_2_M_fields,
     soc_vlan_pfofile_2_m_views, 
};


char *soc_L2MC_m_views[] = {
    "EVEN_PARITYf",
    "HIGIG_TRUNK_OVERRIDEf",
    "PORT_BITMAPf",
    "PORT_BITMAP_LOf",
    "RESERVED0f",
    "VALIDf",
};

soc_field_info_t soc_L2MC_m_fields[] = {
    { L2_MC_EVEN_PARITYf, 1, 39, 0 | SOCF_GLOBAL },
    { L2_MC_HIGIG_TRUNK_OVERRIDEf, 2, 0, SOCF_LE | SOCF_GLOBAL },
    { L2_MC_PORT_BITMAPf, 30, 8, SOCF_LE | SOCF_GLOBAL },
    { L2_MC_PORT_BITMAP_LOf, 30, 8, SOCF_LE | SOCF_GLOBAL },
    { L2_MC_RESERVED0f, 6, 2, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { L2_MC_VALIDf, 1, 38, 0 | SOCF_GLOBAL }
};


soc_mem_info_t SOC_L2_MC_M = { 
    "L2MCm",
     0,
     0,
     1023,
     10,
     0x24000000,
     1,
     5,
     6,
     soc_L2MC_m_fields,
     soc_L2MC_m_views,       
};

soc_mem_info_t *mem_arr[] = {
    &SOC_L2X_M,
    &SOC_PORT_TAB_M,
    &SOC_VLAN_TAB_M,
    &SOC_EGR_VLAN_TAB_M,
    &SOC_VLAN_STG_TAB_M,
    &SOC_EGR_VLAN_STG_TAB_M,    
    &SOC_VLAN_PROFILE_M,
    &SOC_VLAN_PROFILE_2_M,
    &SOC_L2_MC_M,    
};

uint32 mem_valid_field[] = {
    L2X_VALIDf,
    1024,
    VLAN_VALIDf,
    EGR_VLAN_VALIDf,
    1024,
    1024,
    1024,
    1024,
    L2_MC_VALIDf,    
};


uint32 *
soc_memacc_field_get(soc_field_info_t *fieldinfo, const uint32 *entbuf,
                     uint32 *fldbuf)
{    
    int i, wp, bp, len;

    bp = fieldinfo->bp;
    len = fieldinfo->len;
    if (fieldinfo == NULL || entbuf == NULL || fldbuf == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    if (bp >= 1024) {
        fldbuf[0] = 1;
        return NULL;
    }

    if (len == 1) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        if (entbuf[wp] & (1<<bp)) {
            fldbuf[0] = 1;
        } else {
            fldbuf[0] = 0;
        }
        return fldbuf;
    }

    wp = bp / 32;
    bp = bp & (32 - 1);
    i = 0;

    for (; len > 0; len -= 32) {
        if (bp) {
            fldbuf[i] =
                entbuf[wp++] >> bp &
                ((1 << (32 - bp)) - 1);
            if ( len > (32 - bp) ) {
                fldbuf[i] |= entbuf[wp] <<
                    (32 - bp);
            }
        } else {
            fldbuf[i] = entbuf[wp++];
        }

        if (len < 32) {
            fldbuf[i] &= ((1 << len) - 1);
        }
        i++;
    }
    return fldbuf;
}  

void
soc_memacc_mac_addr_get(soc_field_info_t *fieldinfo,
                          void *entry, sal_mac_addr_t mac)
{
    uint32              mac_field[2];
    if (fieldinfo == NULL || entry == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
    
    soc_memacc_field_get(fieldinfo, entry, &mac_field[0]);

    SAL_MAC_ADDR_FROM_UINT32(mac, mac_field);
}

uint32
soc_memacc_field32_get(soc_field_info_t *fieldinfo, void *entry)
{
    uint32 value;

    if (fieldinfo == NULL || entry == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return 0;
    }
    soc_memacc_field_get(fieldinfo, entry, &value);

    return value;
}



uint32 *
soc_mem_field_get(soc_field_info_t *fieldinfo, const uint32 *entbuf,
                     uint32 *fldbuf)
{
    if(fieldinfo == NULL || entbuf == NULL || fldbuf == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    return soc_memacc_field_get(fieldinfo, entbuf, fldbuf);
}


 soc_field_info_t *
 soc_mem_valid_field_info(mem_tab_t tab_index) { 
    uint32 field;

    if (tab_index >= MEM_MAX_COUNT) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return NULL; 
    }
    field = mem_valid_field[tab_index];

    return &mem_arr[tab_index]->fields[field];
} 


void
_shr_format_integer(char *buf, unsigned int n, int min_digits, int base)
{
    static char		*digit_char = "0123456789abcdef";
    unsigned int	tmp;
    int			digit, needed_digits = 0;

    for (tmp = n, needed_digits = 0; tmp; needed_digits++) {
	    tmp /= base;
    }

    if (needed_digits > min_digits) {
	    min_digits = needed_digits;
    }

    buf[min_digits] = 0;

    for (digit = min_digits - 1; digit >= 0; digit--) {
	    buf[digit] = digit_char[n % base];
	    n /= base;
    }
}



void
_shr_format_long_integer(char *buf, uint32 *val, int nval)
{
    int i = BYTES2WORDS(nval) - 1;  
  
    if (i == 0 && val[i] < 10) {
	    buf[0] = '0' + val[i];
	    buf[1] = 0;
    } else {
	    buf[0] = '0';
	    buf[1] = 'x';

      
        if (i == 0) {
	        _shr_format_integer(buf + 2, val[i], 1, 16);
        }
        else
        {
            if ((nval % 4) == 0) {
                _shr_format_integer(buf + 2, val[i], 8, 16);
            }
            else
            {
                _shr_format_integer(buf + 2, val[i], (2 * (nval % 4)), 16);
            }
        }

        while (--i >= 0) {
            while (*buf) {
                buf++;
            }
            _shr_format_integer(buf, val[i], 8, 16);
	    }
    }
}

void soc_mem_dump(int tab_index, int change)
{
    soc_mem_info_t * tab_prt;
    uint32 index;
    soc_field_info_t *fieldinfo;
    uint32 val;
    int  first_print_flag = 0;
    int f;
    char tmp[512];
    uint32 entry_data[SOC_MAX_MEM_FIELD_WORDS];
    uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
    uint32 nfval[SOC_MAX_MEM_FIELD_WORDS] = {0};
    soc_field_info_t *fieldp;

 
    if (tab_index >= MEM_MAX_COUNT) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
    tab_prt = mem_arr[tab_index];
    if (tab_prt == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }
	
    for (index = tab_prt->index_min; index <= tab_prt->index_max; index++) {
        sal_memset(entry_data, 0, SOC_MAX_MEM_FIELD_WORDS);		
		bcm5333x_mem_get(0, tab_prt->block_id,tab_prt->base + index, entry_data,
		                    soc_mem_entry_words(tab_index));

        if (tab_prt->flags & SOC_MEM_FLAG_VALID) {
            fieldinfo = soc_mem_valid_field_info(tab_index);
            val = soc_memacc_field32_get(fieldinfo, entry_data);
            if (0 == val) {
                continue;
            }
        }

        for (f = tab_prt->nFields - 1; f >= 0; f--) {       
            sal_memset(fval, 0, sizeof (fval));
            fieldp = &tab_prt->fields[f];
            if (fieldp == NULL) {
                continue;
            }
            if (change) {
                soc_mem_field_get(fieldp, entry_data, fval);        
                if (sal_memcmp(fval, nfval, BITS2BYTES(fieldp->len) *
                       sizeof (uint32)) == 0) {
                    continue;
                }
            }
   
            if (first_print_flag == 0) {
                sal_printf("%s.[%d]:%s ", tab_prt->name, index,  "<");
                first_print_flag = 1;
            }
	    sal_memset(tmp, 0, 512);
            _shr_format_long_integer(tmp, fval, BITS2BYTES(fieldp->len));
            sal_printf("%s=%s%s", tab_prt->views[f],  tmp, f > 0 ? "," : "");        

        }
        if (first_print_flag == 1) {
            sal_printf("%s\n",  ">");
            first_print_flag = 0;
        }		
        
    }
}



char *soc_reg_ing_config_64_views[] = {	
    "APPLY_EGR_MASK_ON_L2f",
    "APPLY_EGR_MASK_ON_L3f",
    "APPLY_MTU_CHECK_ON_HIGIG_IPMCf",
    "ARP_RARP_TO_FPf",
    "ARP_VALIDATION_ENf",
    "CFI_AS_CNGf",
    "CVLAN_CFI_AS_CNGf",
    "DISABLE_E2E_HOL_CHECKf",
    "ENABLE_MAC_IP_BINDING_FOR_ARP_PKTSf",
    "FB_A0_COMPATIBLEf",
    "IGMP_PKTS_UNICAST_IGNOREf",
    "IGNORE_HG_HDR_DONOT_LEARNf",
    "IGNORE_HG_HDR_HDR_EXT_LENf",
    "IGNORE_HG_HDR_LAG_FAILOVERf",
    "IGNORE_MY_MODIDf",
    "IGNORE_PPD0_PRESERVE_QOSf",
    "IGNORE_PPD2_PRESERVE_QOSf",
    "IPHDR_ERROR_L3_LOOKUP_ENABLEf",
    "IPV4_MC_MACDA_CHECK_ENABLEf",
    "IPV4_RESERVED_MC_ADDR_IGMP_ENABLEf",
    "IPV6_MC_MACDA_CHECK_ENABLEf",
    "IPV6_RESERVED_MC_ADDR_MLD_ENABLEf",
    "L2DH_ENf",
    "L2DST_HIT_ENABLEf",
    "L3SH_ENf",
    "L3SRC_HIT_ENABLEf",
    "LBID_RTAGf",
    "LOOKUP_L2MC_WITH_FID_IDf",
    "MAP_FID_ID_TO_INNER_TAGf",
    "MAP_FID_ID_TO_OUTER_TAGf",
    "MLD_CHECKS_ENABLEf",
    "MLD_PKTS_UNICAST_IGNOREf",
    "RESERVED_0f",
    "RESERVED_40f",
    "SNAP_OTHER_DECODE_ENABLEf",
    "STACK_MODEf",
    "STNMOVE_ON_L2SRC_DISCf",
    "SVL_ENABLEf",
    "TREAT_PKTPRI_AS_DOT1Pf",
    "TRUNKS128f",
    "USE_PPD_SOURCEf",
    "VFP_PRI_ACTION_FB2_MODEf",
};


typedef enum {
    APPLY_EGR_MASK_ON_L2f,
    APPLY_EGR_MASK_ON_L3f,
    APPLY_MTU_CHECK_ON_HIGIG_IPMCf,
    ARP_RARP_TO_FPf,
    ARP_VALIDATION_ENf,
    ING_CONFIG_64_CFI_AS_CNGf,
    CVLAN_CFI_AS_CNGf,
    DISABLE_E2E_HOL_CHECKf,
    ENABLE_MAC_IP_BINDING_FOR_ARP_PKTSf,
    FB_A0_COMPATIBLEf,
    IGMP_PKTS_UNICAST_IGNOREf,
    IGNORE_HG_HDR_DONOT_LEARNf,
    IGNORE_HG_HDR_HDR_EXT_LENf,
    IGNORE_HG_HDR_LAG_FAILOVERf,
    IGNORE_MY_MODIDf,
    IGNORE_PPD0_PRESERVE_QOSf,
    IGNORE_PPD2_PRESERVE_QOSf,
    IPHDR_ERROR_L3_LOOKUP_ENABLEf,
    IPV4_MC_MACDA_CHECK_ENABLEf,
    IPV4_RESERVED_MC_ADDR_IGMP_ENABLEf,
    IPV6_MC_MACDA_CHECK_ENABLEf,
    IPV6_RESERVED_MC_ADDR_MLD_ENABLEf,
    L2DH_ENf,
    L2DST_HIT_ENABLEf,
    L3SH_ENf,
    L3SRC_HIT_ENABLEf,
    LBID_RTAGf,
    LOOKUP_L2MC_WITH_FID_IDf,
    MAP_FID_ID_TO_INNER_TAGf,
    MAP_FID_ID_TO_OUTER_TAGf,
    MLD_CHECKS_ENABLEf,
    MLD_PKTS_UNICAST_IGNOREf,
    ING_CONFIG_64_RESERVED_0f,
    ING_CONFIG_64_RESERVED_40f,
    SNAP_OTHER_DECODE_ENABLEf,
    STACK_MODEf,
    STNMOVE_ON_L2SRC_DISCf,
    SVL_ENABLEf,
    TREAT_PKTPRI_AS_DOT1Pf,
    TRUNKS128f,
    USE_PPD_SOURCEf,
    VFP_PRI_ACTION_FB2_MODEf,
} soc_reg_ing_config_64_t;

soc_field_info_t soc_ING_CONFIG_64_fields[] = {
    { APPLY_EGR_MASK_ON_L2f, 1, 12, 0 },
    { APPLY_EGR_MASK_ON_L3f, 1, 13, 0 },
    { APPLY_MTU_CHECK_ON_HIGIG_IPMCf, 1, 22, 0 },
    { ARP_RARP_TO_FPf, 2, 38, 0 },
    { ARP_VALIDATION_ENf, 1, 29, 0 },
    { ING_CONFIG_64_CFI_AS_CNGf, 1, 5, 0 },
    { CVLAN_CFI_AS_CNGf, 1, 11, 0 },
    { DISABLE_E2E_HOL_CHECKf, 1, 18, 0 },
    { ENABLE_MAC_IP_BINDING_FOR_ARP_PKTSf, 1, 49, 0 },
    { FB_A0_COMPATIBLEf, 1, 8, 0 },
    { IGMP_PKTS_UNICAST_IGNOREf, 1, 27, 0 },
    { IGNORE_HG_HDR_DONOT_LEARNf, 1, 24, 0 },
    { IGNORE_HG_HDR_HDR_EXT_LENf, 1, 25, 0 },
    { IGNORE_HG_HDR_LAG_FAILOVERf, 1, 23, 0 },
    { IGNORE_MY_MODIDf, 1, 26, 0 },
    { IGNORE_PPD0_PRESERVE_QOSf, 1, 43, 0 },
    { IGNORE_PPD2_PRESERVE_QOSf, 1, 44, 0 },
    { IPHDR_ERROR_L3_LOOKUP_ENABLEf, 1, 48, 0 },
    { IPV4_MC_MACDA_CHECK_ENABLEf, 1, 33, 0 },
    { IPV4_RESERVED_MC_ADDR_IGMP_ENABLEf, 1, 31, 0 },
    { IPV6_MC_MACDA_CHECK_ENABLEf, 1, 34, 0 },
    { IPV6_RESERVED_MC_ADDR_MLD_ENABLEf, 1, 32, 0 },
    { L2DH_ENf, 1, 2, 0 },
    { L2DST_HIT_ENABLEf, 1, 2, 0 },
    { L3SH_ENf, 1, 3, 0 },
    { L3SRC_HIT_ENABLEf, 1, 3, 0 },
    { LBID_RTAGf, 3, 19, SOCF_LE },
    { LOOKUP_L2MC_WITH_FID_IDf, 1, 15, 0 },
    { MAP_FID_ID_TO_INNER_TAGf, 1, 17, 0 },
    { MAP_FID_ID_TO_OUTER_TAGf, 1, 16, 0 },
    { MLD_CHECKS_ENABLEf, 1, 30, 0 },
    { MLD_PKTS_UNICAST_IGNOREf, 1, 28, 0 },
    { ING_CONFIG_64_RESERVED_0f, 1, 0, 0 },
    { ING_CONFIG_64_RESERVED_40f, 1, 40, 0 },
    { SNAP_OTHER_DECODE_ENABLEf, 1, 6, 0 },
    { STACK_MODEf, 2, 9, 0 },
    { STNMOVE_ON_L2SRC_DISCf, 1, 7, 0 },
    { SVL_ENABLEf, 1, 14, 0 },
    { TREAT_PKTPRI_AS_DOT1Pf, 1, 46, 0 },
    { TRUNKS128f, 1, 1, 0 },
    { USE_PPD_SOURCEf, 1, 45, 0 },
    { VFP_PRI_ACTION_FB2_MODEf, 1, 41, 0 }
};

soc_reg_info_t SOC_REG_ING_CONFIG_64 = 
{ 
    "ING_CONFIG_64",    
    0x6000000,
    10,
    SOC_REG_FLAG_64_BITS,
    42,
    soc_ING_CONFIG_64_fields,
    soc_reg_ing_config_64_views,
    7,
};

char *soc_reg_igmp_mld_pkt_control_views[] = {    
    "IGMP_QUERY_FWD_ACTIONf",
    "IGMP_QUERY_TO_CPUf",
    "IGMP_REP_LEAVE_FWD_ACTIONf",
    "IGMP_REP_LEAVE_TO_CPUf",
    "IGMP_UNKNOWN_MSG_FWD_ACTIONf",
    "IGMP_UNKNOWN_MSG_TO_CPUf",
    "IPV4_MC_ROUTER_ADV_PKT_FWD_ACTIONf",
    "IPV4_MC_ROUTER_ADV_PKT_TO_CPUf",
    "IPV4_RESVD_MC_PKT_FWD_ACTIONf",
    "IPV4_RESVD_MC_PKT_TO_CPUf",
    "IPV6_MC_ROUTER_ADV_PKT_FWD_ACTIONf",
    "IPV6_MC_ROUTER_ADV_PKT_TO_CPUf",
    "IPV6_RESVD_MC_PKT_FWD_ACTIONf",
    "IPV6_RESVD_MC_PKT_TO_CPUf",
    "MLD_QUERY_FWD_ACTIONf",
    "MLD_QUERY_TO_CPUf",
    "MLD_REP_DONE_FWD_ACTIONf",
    "MLD_REP_DONE_TO_CPUf",
    "PFM_RULE_APPLYf",
};

typedef enum {
    IGMP_QUERY_FWD_ACTIONf,
    IGMP_QUERY_TO_CPUf,
    IGMP_REP_LEAVE_FWD_ACTIONf,
    IGMP_REP_LEAVE_TO_CPUf,
    IGMP_UNKNOWN_MSG_FWD_ACTIONf,
    IGMP_UNKNOWN_MSG_TO_CPUf,
    IPV4_MC_ROUTER_ADV_PKT_FWD_ACTIONf,
    IPV4_MC_ROUTER_ADV_PKT_TO_CPUf,
    IPV4_RESVD_MC_PKT_FWD_ACTIONf,
    IPV4_RESVD_MC_PKT_TO_CPUf,
    IPV6_MC_ROUTER_ADV_PKT_FWD_ACTIONf,
    IPV6_MC_ROUTER_ADV_PKT_TO_CPUf,
    IPV6_RESVD_MC_PKT_FWD_ACTIONf,
    IPV6_RESVD_MC_PKT_TO_CPUf,
    MLD_QUERY_FWD_ACTIONf,
    MLD_QUERY_TO_CPUf,
    MLD_REP_DONE_FWD_ACTIONf,
    MLD_REP_DONE_TO_CPUf,
    PFM_RULE_APPLYf,
} soc_reg_igmp_mld_pkt_control_t;

soc_field_info_t soc_IGMP_MLD_PKT_CONTROL_fields[] = {
    { IGMP_QUERY_FWD_ACTIONf, 2, 22, 0 },
    { IGMP_QUERY_TO_CPUf, 1, 21, 0 },
    { IGMP_REP_LEAVE_FWD_ACTIONf, 2, 25, 0 },
    { IGMP_REP_LEAVE_TO_CPUf, 1, 24, 0 },
    { IGMP_UNKNOWN_MSG_FWD_ACTIONf, 2, 19, 0 },
    { IGMP_UNKNOWN_MSG_TO_CPUf, 1, 18, 0 },
    { IPV4_MC_ROUTER_ADV_PKT_FWD_ACTIONf, 2, 4, 0 },
    { IPV4_MC_ROUTER_ADV_PKT_TO_CPUf, 1, 3, 0 },
    { IPV4_RESVD_MC_PKT_FWD_ACTIONf, 2, 10, 0},
    { IPV4_RESVD_MC_PKT_TO_CPUf, 1, 9, 0 },
    { IPV6_MC_ROUTER_ADV_PKT_FWD_ACTIONf, 2, 1, 0 },
    { IPV6_MC_ROUTER_ADV_PKT_TO_CPUf, 1, 0, 0 },
    { IPV6_RESVD_MC_PKT_FWD_ACTIONf, 2, 7, 0 },
    { IPV6_RESVD_MC_PKT_TO_CPUf, 1, 6, 0 },
    { MLD_QUERY_FWD_ACTIONf, 2, 13, 0 },
    { MLD_QUERY_TO_CPUf, 1, 12, 0 },
    { MLD_REP_DONE_FWD_ACTIONf, 2, 16, 0 },
    { MLD_REP_DONE_TO_CPUf, 1, 15, 0 },
    { PFM_RULE_APPLYf, 1, 27, 0 }
};

soc_reg_info_t SOC_IGMP_MLD_PKT_CONTROL = 
{ 
    "IGMP_MLD_PKT_CONTROL",   
    0x1c000500,
    10,
    1,
    19,
    soc_IGMP_MLD_PKT_CONTROL_fields,
    soc_reg_igmp_mld_pkt_control_views,
    4,
};

char *soc_reg_l2_timer_views[] = { 
     "AGE_ENAf",
     "AGE_VALf",
};

typedef enum {
    AGE_ENAf,
    AGE_VALf,
} soc_reg_l2_timer_t;


soc_field_info_t soc_L2_AGE_TIMER_fields[] = {
    { AGE_ENAf, 1, 20, 0 },
    { AGE_VALf, 20, 0, SOCF_LE }
};


soc_reg_info_t SOC_L2_AGE_TIMER = {
    "L2_AGE_TIMER",
    0x2000400,
    10,
    0,
    2,
    soc_L2_AGE_TIMER_fields,
    soc_reg_l2_timer_views,
    3,
};

char *soc_reg_protocol_pkt_control_views[] = { 
   "ARP_REPLY_DROPf",
   "ARP_REPLY_TO_CPUf",
   "ARP_REQUEST_DROPf",
   "ARP_REQUEST_TO_CPUf",
   "DHCP_PKT_DROPf",
   "DHCP_PKT_TO_CPUf",
   "ND_PKT_DROPf",
   "ND_PKT_TO_CPUf",
   "RESERVED_1f",
   "RESERVED_2f",
   "RESERVED_3f",
   "RESERVED_4f",
};


typedef enum {
    PRO_PKT_CTRL_ARP_REPLY_DROPf,
    PRO_PKT_CTRL_ARP_REPLY_TO_CPUf,
    PRO_PKT_CTRL_ARP_REQUEST_DROPf,
    PRO_PKT_CTRL_ARP_REQUEST_TO_CPUf,
    PRO_PKT_CTRL_DHCP_PKT_DROPf,
    PRO_PKT_CTRL_DHCP_PKT_TO_CPUf,
    PRO_PKT_CTRL_ND_PKT_DROPf,
    PRO_PKT_CTRL_ND_PKT_TO_CPUf,
    PRO_PKT_CTRL_RESERVED_1f,
    PRO_PKT_CTRL_RESERVED_2f,
    PRO_PKT_CTRL_RESERVED_3f,
    PRO_PKT_CTRL_RESERVED_4f,
} soc_reg_protocol_pkt_control_t;



soc_field_info_t soc_PROTOCOL_PKT_CONTROL_fields[] = {
    { PRO_PKT_CTRL_ARP_REPLY_DROPf, 1, 7, 0 },
    { PRO_PKT_CTRL_ARP_REPLY_TO_CPUf, 1, 6, 0 },
    { PRO_PKT_CTRL_ARP_REQUEST_DROPf, 1, 5, 0 },
    { PRO_PKT_CTRL_ARP_REQUEST_TO_CPUf, 1, 4, 0 },
    { PRO_PKT_CTRL_DHCP_PKT_DROPf, 1, 1, 0 },
    { PRO_PKT_CTRL_DHCP_PKT_TO_CPUf, 1, 0, 0 },
    { PRO_PKT_CTRL_ND_PKT_DROPf, 1, 3, 0 },
    { PRO_PKT_CTRL_ND_PKT_TO_CPUf, 1, 2, 0 },
    { PRO_PKT_CTRL_RESERVED_1f, 1, 8, SOCF_RES },
    { PRO_PKT_CTRL_RESERVED_2f, 2, 9, SOCF_LE|SOCF_RES },
    { PRO_PKT_CTRL_RESERVED_3f, 1, 11, SOCF_RES },
    { PRO_PKT_CTRL_RESERVED_4f, 2, 12, SOCF_LE|SOCF_RES }
};

soc_reg_info_t SOC_PROTOCOL_PKT_CONTRO = {
	"PROTOCOL_PKT_CONTROL", 
    0x1c000400,
    10,
    0,
    12,
    soc_PROTOCOL_PKT_CONTROL_fields,
    soc_reg_protocol_pkt_control_views,
    2,        
};

char *soc_reg_unknow_mcast_block_mask_64_views[] = { 
   "BLK_BITMAPf",
   "BLK_BITMAPf",
};

typedef enum {
    BLK_BITMAPf,
    BLK_BITMAP_0f,
} soc_reg_unknow_mcast_block_mask_64_t;

soc_field_info_t SOC_UNKNOWN_MCAST_BLOCK_MASK_64_fields[] = {
    { BLK_BITMAPf, 30, 0, SOCF_LE},
    { BLK_BITMAP_0f, 30, 0, SOCF_LE}
};


soc_reg_info_t SOC_UNKNOWN_MCAST_BLOCK_MASK_64 = {
	"UNKNOWN_MCAST_BLOCK_MASK_64",
    0x44000200,
    10,
    0,
    2,
    SOC_UNKNOWN_MCAST_BLOCK_MASK_64_fields,
    soc_reg_unknow_mcast_block_mask_64_views,
    4
};

soc_reg_info_t SOC_IUNKNOWN_MCAST_BLOCK_MASK_64 = {
	"IUNKNOWN_MCAST_BLOCK_MASK_64",
    0x44000300,
    10,
    0,
    2,
    SOC_UNKNOWN_MCAST_BLOCK_MASK_64_fields,
    soc_reg_unknow_mcast_block_mask_64_views,
    4
};


soc_reg_info_t *reg_arr[] = {
    &SOC_REG_ING_CONFIG_64,
    &SOC_IGMP_MLD_PKT_CONTROL,
    &SOC_L2_AGE_TIMER,
    &SOC_PROTOCOL_PKT_CONTRO,
    &SOC_UNKNOWN_MCAST_BLOCK_MASK_64,
    &SOC_IUNKNOWN_MCAST_BLOCK_MASK_64
};


void
format_long_integer(char *buf, uint32 *val, int nval)
{
    int i;	

    for (i = nval - 1; i > 0; i--) {
        if (val[i]) {
            break;
        }
    }

    if (i == 0 && val[i] < 10) {
        sal_sprintf(buf, "%d", val[i]);
    } else {
        sal_sprintf(buf, "0x%x", val[i]);
    }

    while (--i >= 0) {
        sal_sprintf(buf + sal_strlen(buf), "%08x", val[i]);
    }
}


void
soc_reg_dump(uint32 index, uint32 id)
{
    int         f;    
    char        line_buf[1024];
    int		    nprint;
	int		    len;
    uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
    soc_reg_info_t *reginfo; 
    uint32 val[SOC_MAX_MEM_FIELD_WORDS];
    uint8 size  = soc_reg_entry_bytes(index);
    
	if (index >= REG_MAX_COUNT) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
	}

    reginfo = reg_arr[index]; 
    if (reginfo == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }


    bcm5333x_reg64_get(0, reginfo->block_id, reginfo->base + id, val, size); 

    sal_memset(line_buf, 0 , 1024);
    sal_sprintf(line_buf, "%s[%d][0x%x] = ", reginfo->name, id, reginfo->base + id);
	
   len = sal_strlen(line_buf);
   format_long_integer(line_buf + len, val, size);

   
    sal_printf("%s%s: <", line_buf, val);       
 
    nprint = 0;
    for (f = reginfo->nFields - 1; f >= 0; f--) {
	    soc_field_info_t *fieldp = &reginfo->fields[f];

	    sal_memset(fval, 0, sizeof (fval));

        if (fieldp == NULL) {
			sal_printf("%s:%d ---continue\n", __FUNCTION__, __LINE__);
            continue;
        }
	    soc_mem_field_get(fieldp, val, fval);        
	    if (nprint > 0) {			
	        sal_printf("%s", ",");			
          
	    }
	    sal_printf("%s=", reginfo->views[f]);

		sal_memset(line_buf, 0, 1024);
		format_long_integer(line_buf, fval, BITS2BYTES(fieldp->len));
		sal_printf("%s", line_buf);  
	    nprint += 1;
    }
    sal_printf(">\n");
}



char * reg_str[] = {
    "TBYT",
    "RBYT",
    "TPKT",
    "RPKT",
    "RFCS",
    "RUCA",
    "TUCA",
    "RMCA",
    "TMCA",
    "RBCA",
    "TBCA",
    "RXPF",
    "TXPF",
    "ROVR",
    "TOVR",
    "TDrp",
};

char * xl_reg_str[] = {
    "GTBYT",
    "GRBYT",
    "GTPKT",
    "GRPKT",
    "GRFCS",
    "GRUC",
    "GTUC",
    "GRMCA",
    "GTMCA",
    "GRBCA",
    "GTBCA",
    "GRXPF",
    "GTXPF",
    "GROVR",
    "GTOVR",
    "TDrop",
};

void
counter_val_set(int port, int reg, uint32 *dst)
{
    if (dst == NULL) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
    }

	
	if (port > BCM5333X_LPORT_MAX || reg > R_MAX) {
		sal_printf("%s:%d ---return\n", __FUNCTION__, __LINE__);
        return;
	}
	
   
    counter_val[port][reg] = dst[reg];
    counter_val[port][reg + R_MAX] = dst[reg + R_MAX];	
}

void
counter_val_diff(     int port, int reg, uint32 *dst, uint32 *diff)
{
    uint32 val = 0;
	uint8 pport;

    if (dst == NULL || diff == NULL) {
        return;
    }
	pport = SOC_PORT_L2P_MAPPING(port);

    if (IS_XL_PORT(pport)) {
        if (dst[reg + R_MAX] < counter_val[port][reg + R_MAX]) {
            val = 4294967295 - counter_val[port][reg + R_MAX];
            val += dst[reg + R_MAX];
		    diff[0] = dst[reg];
            diff[0] -= 1;
        } else {
	
            diff[0] = dst[reg];
            diff[1] = val;
        }
    } else {
        if (dst[reg] < counter_val[port][reg]) {
			diff[0] = dst[reg];
			counter_val[port][reg] = 0;
        } else {           
            diff[0] = dst[reg] - counter_val[port][reg];           
        }

    }
}

void
soc_port_status_dump(int port)
{
    char buf[80];    
	uint8 lport;
	uint8 pport;
	int index;
	uint32 stat[2 * R_MAX];
	uint32 diff[2];

	for (pport = 2; pport < 25; pport ++) {
		lport = SOC_PORT_P2L_MAPPING(pport);
            if (port != 0) {
                if (pport - 1 != port) {
                    continue;
                }
	    }
		sal_memset(stat, 0, 2 * R_MAX * sizeof(uint32));
		board_lport_stat_get(lport, stat);		
		for (index = TBYT; index < R_MAX; index++) {
			counter_val_diff(lport, index, stat, diff);			
			sal_memset(buf, 0, 80);
		    if (IS_XL_PORT(pport)) {
				sal_sprintf(buf, "%s%s%d", xl_reg_str[index], ".port", pport - 1);
		        sal_printf("%-22s: 0x%08x%08x + 0x%08x%08x\n",
			    buf, stat[index], stat[R_MAX + index], diff[0], diff[1]);
			    counter_val_set(lport, index, stat);
			} else {
				sal_sprintf(buf, "%s%s%d", reg_str[index], ".port", pport - 1);
		        sal_printf("%s:     0x%08x + 0x%08x\n",	buf, counter_val[port][index], diff[0]);
				counter_val_set(lport, index, stat);
		    }
		}

	}
}


int
resource_alloc( id_resource_t resource, uint32 *id)
{
    int  idx;

    /* Input parameters check. */
    if (NULL == id) {
        return (SYS_ERR);
    }

    /* Find unused id. */
    for (idx = 0; idx < resource.total_id; idx++) {
        if (0 == RESOURCE_BMP_TEST(resource.resource_bmp, idx)) {
            RESOURCE_BMP_ADD(resource.resource_bmp, idx);
            *id =  idx;
            break;
        }
    }

    if (idx == resource.total_id) {
        return (SYS_ERR_FULL);
    }
    return (SYS_OK);
}


int
resource_free( id_resource_t resource, int id)
{

    /* Input parameters check. */
    if (id <= 0 || id >= resource.total_id) {
        return (SYS_ERR);
    }

    /* Free the used id. */
    RESOURCE_BMP_REMOVE(resource.resource_bmp, id);
    return (SYS_OK);
}


#define L3MC_ID_TOTAL		 256
#define L2MC_ID_TOTAL		 1024

static uint32    l3_ipmc_id;
static uint32    l2mc_id;

id_resource_t _l3_ipmc_id;
id_resource_t _l2mc_id;


#define L3_IPMC_ID_GET    RESOURCE_ID_GET(_l3_ipmc_id, l3_ipmc_id)
#define L2_MC_ID_GET      RESOURCE_ID_GET(_l2mc_id, l2mc_id)


#define L3_IPMC_ID_ALLOC    (l3_ipmc_id)
#define L2_MC_ID_ALLOC    (l2mc_id)
#define L3_IPMC_ID_FREE(id)   RESOURCE_ID_FREE(_l3_ipmc_id, (id))
#define L2_MC_ID_FREE(id)     RESOURCE_ID_FREE(_l2mc_id, (id))


int
igmp_resource_init(void)
{
    int size = 0;

    l3_ipmc_id = 0;
    l2mc_id = 0;

    _l3_ipmc_id.total_id = L3MC_ID_TOTAL;
    size = BITALLOCSIZE(_l3_ipmc_id.total_id);
    _l3_ipmc_id.resource_bmp.w = sal_malloc(size);
    if (NULL == _l3_ipmc_id.resource_bmp.w) {
        return SYS_ERR;
    }
    sal_memset(_l3_ipmc_id.resource_bmp.w, 0, size);

    _l2mc_id.total_id = L2MC_ID_TOTAL;
    size = BITALLOCSIZE(_l2mc_id.total_id);
    _l2mc_id.resource_bmp.w = sal_malloc(size);
    if (NULL == _l2mc_id.resource_bmp.w) {
        return SYS_ERR;
    }
    sal_memset(_l2mc_id.resource_bmp.w, 0, size);	
    return SYS_OK;
}

int l3_ipmc_id_get(     uint16 *id)
{    
    if (id == NULL) {
        return SYS_ERR;
    }
    L3_IPMC_ID_GET;
	*id = L3_IPMC_ID_ALLOC;
    return SYS_OK;
}

int l3_ipmc_id_free(     uint16 id)
{    
    L3_IPMC_ID_FREE(id);
    return SYS_OK;
}

#endif /* CFG_CLI_ENABLED */


