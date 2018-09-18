/*
 * $Id: board.c,v 1.52 Broadcom SDK $
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
#include "bsp_config.h"
#include "brdimpl.h"
#include "soc/bcm5333x.h"
#include "utils/ports.h"
#include "utils/ui.h"

extern soc_switch_t soc_switch_bcm5333x;
#if CFG_FLASH_SUPPORT_ENABLED
/* Flash device driver. */
extern flash_dev_t n25q256_dev;
#endif /* CFG_FLASH_SUPPORT_ENABLED */

static const char *um_boardname = CFG_BOARDNAME;

#ifdef CFG_SWITCH_QOS_INCLUDED
static qos_type_t  qos_info = QT_COUNT;
#endif

#ifdef CFG_SWITCH_RATE_INCLUDED
static uint8 storm_info = STORM_RATE_NONE;
#endif

#if defined(CFG_SWITCH_MCAST_INCLUDED)
typedef struct mcast_list_s {
    uint8 mac[6];
    uint16 vlan_id;
    uint16 index;
    pbmp_t  port_lpbmp; /* all ports' pbmp in this vlan_id */
	uint16 umc_index;
	uint16 uc_index;
    struct mcast_list_s *next;
} mcast_list_t;

static mcast_list_t *mlist = NULL;

/* address the minimal l2mc index can be used */
static pbmp_t mcindex[4];
#endif /*  defined(CFG_SWITCH_MCAST_INCLUDED) */

#define QOS_BASE_IDX               (2 * ENTRIES_PER_SLICE)
/* 802.1p higher than DSCP fp entries index */
#define DOT1P_BASE_IDX             (2 * ENTRIES_PER_SLICE + BCM5333X_LPORT_MAX + BCM5333X_LPORT_MIN + 1)
#define RATE_IGR_IDX               (1 * ENTRIES_PER_SLICE + 3)

#ifdef CFG_SWITCH_LAG_INCLUDED
/* variables for static trunk */
uint8 lag_enable = FALSE;

typedef struct lag_group_s {
    uint8 enable;
    pbmp_t lpbmp;
} lag_group_t;

lag_group_t lag_group[BOARD_MAX_NUM_OF_LAG];
#endif /* CFG_SWITCH_LAG_INCLUDED */

extern void um_launch(hsaddr_t entry);

const char *
board_name(void)
{
    return um_boardname;
}
/**
 * Get the number of user ports (uports)
 * 
 * @return the number of user ports
 *    
 */

uint8
board_uport_count(void)
{
    return bcm5333x_port_count(0);
}

/**
 * Map a user port to a chip-internal logical port.
 * 
 * @param uport (IN)- The user port
 * @param unit (OUT) - the chip unit number of the lport
 * @param lport (OUT) - the chip-internal logical port
 * @return sys_error_t
 *     SYS_OK : there is no error
 *     SYS_ERR_PARAMETER : fail, because parameter is invalid
 *   
 */
sys_error_t
board_uport_to_lport(uint16 uport, uint8 *unit, uint8 *lport)
{
    uint8 count, port;
    int temp_lport;

    if (SAL_UPORT_IS_NOT_VALID(uport)|| unit == NULL || lport == NULL) {
        return SYS_ERR_PARAMETER;
    }

    count = SAL_UPORT_BASE;
    
    SOC_PPORT_ITER(port) {
        temp_lport = SOC_PORT_P2L_MAPPING(port);
        if (!lport_active[temp_lport]) {
            continue;
        }
        
        if (count == uport) {
            break;
        }
        count++;
    }

    if (port == (BCM5333X_PORT_MAX+1)) {
        return SYS_ERR_NOT_FOUND;
    }

    *unit = 0;

    if (SOC_IS_DEERHOUND(*unit)) {
        *lport = SOC_PORT_P2L_MAPPING(port);
    } else {

        /* In Wolfhound ref platform, TSC ports are Even-Odd swapped (TSC physical port start from 26)*/ 
        /*  To simply HR2 QGPHY(2~9) port swapping  we use physical port instead of logical port*/
        if (port >= 10) {
            *lport = SOC_PORT_P2L_MAPPING(port);
        } else {
            *lport = port;
        }
    }

    if (*lport == -1) {
        return SYS_ERR_NOT_FOUND;
    }
    return SYS_OK;
}


/**
 * Map a chip-internal logical port to a user port.
 *
 * @param lport (IN)- The chip-internal logical port
 * @param unit (IN) - the chip unit number of the lport
 * @param uport (OUT) - the user port
 * @return sys_error_t
 *     SYS_OK : there is no error
 *     SYS_ERR_PARAMETER : fail, because parameter is invalid
 *   
 */

sys_error_t
board_lport_to_uport(uint8 unit, uint8 lport, uint16 *uport)
{

    uint8 count;
    int idx, find;
    int temp_lport;


    if (uport == NULL || lport > BCM5333X_PORT_MAX || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    count = SAL_UPORT_BASE;
    find = 0;

    
    SOC_PPORT_ITER(idx) {
        temp_lport = SOC_PORT_P2L_MAPPING(idx);
        if (!lport_active[temp_lport]) {
            continue;
        }

        /* In Wolfhound ref platform, TSC ports are Even-Odd swapped (TSC physical port start from 26)*/ 
        /*  To simply HR2 QGPHY(2~9) port swapping  we use physical port instead of logical port*/
         if (!SOC_IS_DEERHOUND(unit) && idx < 10) {
            if (lport == idx) {
                 find = 1;
                 break;
            }
         } else {
            if (lport == SOC_PORT_P2L_MAPPING(idx)) {
                 find = 1;
                 break;
            }
         }
         count++;
     }
     if (find == 1) {
         *uport = count;
     } else {
         *uport = -1;
         return SYS_ERR_NOT_FOUND;
     }
     return SYS_OK;

}

/**
 * Map the user port bitmap arrary (uplist)  to the chip-internal logical port bitmap (lpbmp) on selected chip unit .
 *
 * @param uplist (IN)- The user port bit map array  which may cover many chips
 * @param unit (IN) - the selected chip unit number
 * @param lpbmp (OUT) - the chip-internal logical port bit map 
 * @return  sys_error_t
 *     SYS_OK : there is no error
 *     SYS_ERR_PARAMETER : fail, because parameter is invalid
 *   
 */

sys_error_t
board_uplist_to_lpbmp(uint8 *uplist, uint8 unit, pbmp_t *lpbmp)
{

    uint8 val, lport;
    uint16 uport;
    
    if (uplist == NULL || lpbmp == NULL || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    *lpbmp = 0;

    SAL_UPORT_ITER(uport) {
        if (uplist_port_matched(uplist, uport) == SYS_OK) {
            if (board_uport_to_lport(uport, &val, &lport) == SYS_OK) {
                *lpbmp |= (1 << lport);
            }
        }
    }

    return SYS_OK;
}

sys_error_t
board_uport_to_lpbmp(uint8 uport, uint8 unit, pbmp_t *lpbmp)
{
    uint8 lport;

    if (lpbmp == NULL || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    BCM_PBMP_CLEAR(*lpbmp);
	if (board_uport_to_lport(uport, &unit, &lport) == SYS_OK) {
	    *lpbmp |= (1 << lport);
	}
    return SYS_OK;
}


/**
 * Map the chip-internal logical port bit map (lpbmp) to a user port port bit map array (uplist) on selected chip unit.
 *
 *
 * @param unit (IN)- the selected chip unit number
 * @param lpbmp (IN) - The chip-internal logical port bit map 
 * @param uplist (OUT) - The user port bit map array which may cover many chips
 * @return sys_error_t 
 *             SYS_OK : there is no error
 *             SYS_ERR_PARAMETER : fail, because parameter is invalid
 *   
 */

sys_error_t
board_lpbmp_to_uplist(uint8 unit, pbmp_t lpbmp, uint8 *uplist)
{

    uint8 pport, lport;
    uint16 uport;/* uport is front port index from 0 to board_uport_count()-1 */


    if (uplist == NULL || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    uplist_clear(uplist);
    
    SOC_PPORT_ITER(pport) {
        /* In Wolfhound ref platform, TSC ports are Even-Odd swapped (TSC physical port start from 26)*/ 
        /*  To simply HR2 QGPHY(2~9) port swapping  we use physical port instead of logical port*/        
        if (!SOC_IS_DEERHOUND(unit) && pport < 10) {
            lport = pport; 
        } else {
            lport = SOC_PORT_P2L_MAPPING(pport); // port swap 2~9          
        }
        if ((lpbmp >> lport) & 0x1) {
            if (board_lport_to_uport(unit, lport, &uport) == SYS_OK) {
                uplist_port_add(uplist, uport);
            }
        }
    }
    return SYS_OK;
}

soc_switch_t *
board_get_soc_by_unit(uint8 unit)
{
    if (unit > 0) {
        return NULL;
    }
    return &soc_switch_bcm5333x;
}

sys_error_t
board_port_enable_get(uint16 uport, BOOL *enable)
{
    uint8 unit, lport, pport;
    sys_error_t r;
    uint32 en;
    
    r = board_uport_to_lport(uport, &unit, &lport);

    if (r != SYS_OK) {
        return r;
    }    

    pport = SOC_PORT_L2P_MAPPING(lport);

    if (!SOC_IS_DEERHOUND(unit) && pport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        r = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, pport), 
                       PhyConfig_Enable, &en, NULL);
    } else {   
    
        r = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport), 
                       PhyConfig_Enable, &en, NULL);
    }

    *enable = en;

    return r;
}

sys_error_t
board_port_enable_set(uint16 uport, BOOL enable)
{
    uint8 unit;
    uint8 lport, pport;
    sys_error_t r;
    uint16 phy_reg;
    int link;

    r = board_uport_to_lport(uport, &unit, &lport);

    if (r != SYS_OK) {
        return r;
    }
    /* 
     * In board_uport_to_lport, 
     * it returns chip logical port for DH and WH
     * and returns chip phyical port for FH
     */
    pport = SOC_PORT_L2P_MAPPING(lport);
    
    if (!SOC_IS_DEERHOUND(unit) && pport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }    

    if (enable) {
        phy_reg_read(lport, 0x1, &phy_reg);
        link = 0;
        if (phy_reg & (1 << 2)) {
            link = 1;
        }

        r = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport), 
                            PhyConfig_Enable, 1, NULL);

        if (link) {
            MAC_ENABLE_SET(hr2_sw_info.p_mac[lport], unit, lport, enable);
        }
    } else {
        MAC_ENABLE_SET(hr2_sw_info.p_mac[lport], unit, lport, enable);
        r = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport), 
                                             PhyConfig_Enable, 0, NULL);
    }

    return r;
}

sys_error_t
board_get_port_link_status(uint16 uport, BOOL *link)
{
    uint8 unit, lport;
    sys_error_t r;
    *link = 0;
    r = board_uport_to_lport(uport, &unit, &lport);
    if (r != SYS_OK) {
        return r;
    }

    /* 
     * In board_uport_to_lport, 
     * it returns chip logical port for DH and WH
     * and returns chip phyical port for FH
     */
    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }

    return (*soc_switch_bcm5333x.link_status)(unit, lport, link);
}

void
board_reset(void *param)
{
    if (param) {
        if (*(BOOL *)param) {
            /* Hard reset */
        }
    }
    /* [Bit 0]: switch reset, [Bit 1]: iproc only reset */
    SYS_REG_WRITE32(DMU_CRU_RESET, 0x0);
}

void
board_firmware_version_get(uint8 *major, uint8 *minor, uint8 *eco, uint8 *misc)
{
    *major = CFE_VER_MAJOR;
    *minor = CFE_VER_MINOR;
    *eco = CFE_VER_BUILD;
    *misc = 0;
}

#if CFG_RXTX_SUPPORT_ENABLED
sys_error_t
board_rx_set_handler(BOARD_RX_HANDLER fn)
{
    return brdimpl_rx_set_handler(fn);
}

sys_error_t
board_rx_fill_buffer(sys_pkt_t *pkt)
{
    return brdimpl_rx_fill_buffer(pkt);
}

sys_error_t
board_tx(sys_pkt_t *pkt, BOARD_TX_CALLBACK cbk)
{
    return brdimpl_tx(pkt, cbk);
}

void
board_rxtx_stop(void)
{
    bcm5333x_rxtx_stop();
    return;
}

#endif /* CFG_RXTX_SUPPORT_EN ABLED */

#if CFG_FLASH_SUPPORT_ENABLED
/* Get flash device object */
flash_dev_t *
board_get_flash_dev()
{
    /* to use auto-probe result */
    return flash_dev_get();

}
#endif /* CFG_FLASH_SUPPORT_ENABLED */

/* Check integrity of firmware image */
BOOL
board_check_image(hsaddr_t address, hsaddr_t *outaddr)
{
    uint16 hdrchksum, chksum = 0;
    uint32 i, size;
    flash_imghdr_t *hdr = (flash_imghdr_t *)address;
    uint8 *ptr = HSADDR2DATAPTR(address);
    char buf[64];

    if (sal_memcmp(hdr->seal, UM_IMAGE_HEADER_SEAL, sizeof(hdr->seal)) != 0) {
#if CFG_CONSOLE_ENABLED && !defined(CFG_DUAL_IMAGE_INCLUDED)
        sal_printf("Invalid header seal.  This is not a valid image.\n");
#endif /* CFG_CONSOLE_ENABLED */
        return FALSE;
    }

    sal_memcpy(buf, hdr->size, 8);
    buf[8] = 0;
    size = (uint32)sal_xtoi((const char*)buf);

    sal_memcpy(buf, hdr->chksum, 4);
    buf[4] = 0;
    hdrchksum = (uint16)sal_xtoi((const char*)buf);
#if CFG_CONSOLE_ENABLED
    sal_printf("Flash image is %d bytes, chksum %04X, version %c.%c.%c for board %s\n",
                size, hdrchksum, hdr->majver, hdr->minver, hdr->ecover, hdr->boardname);
#endif /* CFG_CONSOLE_ENABLED */
    if (sal_strcmp(board_name(), (const char*)hdr->boardname) != 0) {
#if CFG_CONSOLE_ENABLED
        sal_printf("This image is not appropriate for board type '%s'\n",board_name());
#endif /* CFG_CONSOLE_ENABLED */
        return FALSE;
    }

    ptr += sizeof(flash_imghdr_t);
    for (i = 0; i < size; i = i + 50) {
        chksum += *(ptr+i);
    }

    if (chksum != hdrchksum) {
#if CFG_CONSOLE_ENABLED
        sal_printf("Checksum incorrect. Calculated chksum is %04X\n", chksum);
#endif /* CFG_CONSOLE_ENABLED */

        return FALSE;
    }

    *outaddr = DATAPTR2HSADDR(ptr);
    return TRUE;
}

BOOL
board_check_imageheader(msaddr_t address)
{
    flash_imghdr_t *hdr = (flash_imghdr_t *)address;

    if (sal_memcmp(hdr->seal, UM_IMAGE_HEADER_SEAL, sizeof(hdr->seal)) != 0) {
#if CFG_CONSOLE_ENABLED && !defined(CFG_DUAL_IMAGE_INCLUDED)
        sal_printf("Invalid header seal.  This is not a valid image.\n");
#endif /* CFG_CONSOLE_ENABLED */
        return FALSE;
    }

#if CFG_CONSOLE_ENABLED
    sal_printf("Flash image is version %c.%c.%c for board %s\n",
                hdr->majver, hdr->minver, hdr->ecover, hdr->boardname);
#endif /* CFG_CONSOLE_ENABLED */
    if (sal_strcmp(board_name(), (const char*)hdr->boardname) != 0) {
#if CFG_CONSOLE_ENABLED
        sal_printf("This image is not appropriate for board type '%s'\n",board_name());
#endif /* CFG_CONSOLE_ENABLED */
        return FALSE;
    }

    return TRUE;
}

/* Launch program at entry address */
void
board_load_program(hsaddr_t entry)
{
    void (*funcptr)(void) = (void (*)(void))entry;
    (*funcptr)();

}

loader_mode_t
board_loader_mode_get(bookkeeping_t *data, BOOL reset)
{
    bookkeeping_t *pdata = (bookkeeping_t *)BOARD_BOOKKEEPING_ADDR;

    if (pdata->magic == UM_BOOKKEEPING_SEAL) {
        /* Firmware notify looader to do firmware upgrade */
        if (data) {
            sal_memcpy(data, pdata, sizeof(bookkeeping_t));
        }
        if (reset) {
            pdata->magic = 0x0;
        }
        return LM_UPGRADE_FIRMWARE;
    }
    return LM_NORMAL;
}

void
board_loader_mode_set(loader_mode_t mode, bookkeeping_t *data)
{
    bookkeeping_t *pdata = (bookkeeping_t *)BOARD_BOOKKEEPING_ADDR;
#ifdef CFG_DUAL_IMAGE_INCLUDED
    uint32 active_image = pdata->active_image;
#endif
    if (mode == LM_UPGRADE_FIRMWARE) {
        sal_memcpy(pdata, data, sizeof(bookkeeping_t));
#ifdef CFG_DUAL_IMAGE_INCLUDED
        pdata->active_image = active_image;
#endif
    }
}

#ifdef CFG_DUAL_IMAGE_INCLUDED
static int
calculate_score(hsaddr_t address)
{
    flash_imghdr_t *hdr = (flash_imghdr_t *)address;
    hsaddr_t addr;
    
    /* Check image (header and checksum) first */
    if (!board_check_imageheader(address) || !board_check_image(address, &addr)) {
        return -1;
    }

    /* Header OK, now check if there is any valid timestamp */
    if (hdr->timestamp[0] != TIMESTAMP_MAGIC_START ||
        hdr->timestamp[3] != TIMESTAMP_MAGIC_END) {
        /* No timestamp found, but the image is OK */
        return 0;
    }

    return (int)(((uint16) hdr->timestamp[1] << 8) + (uint16)hdr->timestamp[2]);
}
/* Should be invoked in boot loader only */
BOOL
board_select_boot_image(hsaddr_t *outaddr)
{
    int score1, score2;
    uint8 *ptr;
    /* [31:16] = timestamp, [15:0] = booting partition */
    uint32 booting_partition ;

    score1 = calculate_score((hsaddr_t)BOARD_FIRMWARE_ADDR);
    score2 = calculate_score((hsaddr_t)BOARD_SECONDARY_FIRMWARE_ADDR);

    if (score1 == -1 && score2 == -1) {
#if CFG_CONSOLE_ENABLED
        sal_printf("There is no valid image \n");
#endif /* CFG_CONSOLE_ENABLED */
        return FALSE;
    }

    if (score1 == 0xffff || score2 == 0xffff) {
        /* Deal with overflow condition */
        if (score1 == 1) {
            score2 = 0;
        } else if (score2 == 1) {
            score1 = 0;
        }
    }

    if (score1 == -1) {
        booting_partition = 2 | ((uint32)(score2) << 16);
    } else if (score2 == -1) {
        booting_partition = 1 | ((uint32)(score1) << 16);
    } else {
        /*
         * Partition 1 has priority over partition 2
         */
        if (score1 >= score2) {
            booting_partition = 0x201 | ((uint32)(score1) << 16);
        } else {
            booting_partition = 0x102 | ((uint32)(score2) << 16);
        }
    }

    if (ACTIVE_IMAGE_GET(booting_partition) == 1) {
        ptr = HSADDR2DATAPTR(BOARD_FIRMWARE_ADDR);
    } else {
        ptr = HSADDR2DATAPTR(BOARD_SECONDARY_FIRMWARE_ADDR);
    }
   
    ptr += sizeof(flash_imghdr_t);
    *outaddr = DATAPTR2HSADDR(ptr);

    board_active_image_set(booting_partition);

    return TRUE;
}

void
board_active_image_set(uint32 partition)
{
    bookkeeping_t *pdata = (bookkeeping_t *)BOARD_BOOKKEEPING_ADDR;
    pdata->active_image = partition;
}

uint32
board_active_image_get(void)
{
    bookkeeping_t *pdata = (bookkeeping_t *)BOARD_BOOKKEEPING_ADDR;
    return pdata->active_image;
}
#endif /* CFG_DUAL_IMAGE_INCLUDED */

#ifdef CFG_SWITCH_VLAN_INCLUDED
sys_error_t
board_vlan_type_set(vlan_type_t type)
{
    return brdimpl_vlan_type_set(type);
}


sys_error_t
board_vlan_type_get(vlan_type_t *type)
{
    return brdimpl_vlan_type_get(type);
}


sys_error_t
board_vlan_create(uint16 vlan_id)
{
    return brdimpl_vlan_create(vlan_id);
}


sys_error_t
board_vlan_destroy(uint16 vlan_id)
{
    return brdimpl_vlan_destroy(vlan_id);
}

sys_error_t
board_pvlan_port_set(uint16  vlan_id, uint8 *uplist)
{
    return brdimpl_pvlan_port_set(vlan_id, uplist);
}

sys_error_t
board_pvlan_port_get(uint16  vlan_id, uint8 *uplist)
{
    return brdimpl_pvlan_port_get(vlan_id, uplist);
}

/* Input: lport
 * Output: the egress mask of this port. 1: can forward to. 0: cannot.
 */
sys_error_t
board_pvlan_egress_get(uint16 uport, uint8 *uplist)
{
    return brdimpl_pvlan_egress_get(uport, uplist);
}

sys_error_t
board_untagged_vlan_set(uint16 uport, uint16 vlan_id)
{
    uint8 unit, lport;
    uint32 port_entry[8];
    sys_error_t rv = SYS_OK;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    bcm5333x_mem_get(0, M_PORT(lport), port_entry, 8);
    port_entry[0] = (port_entry[0] & 0x00ffffff) | (vlan_id << 24);
    port_entry[1] = (port_entry[1] & 0xfffffff0) | (vlan_id >> 8);
    bcm5333x_mem_set(0, M_PORT(lport), port_entry, 8);

  return rv;
}

sys_error_t
board_untagged_vlan_get(uint16 uport, uint16 *vlan_id)
{
    uint32 port_entry[8];
    uint8 unit, lport;
    sys_error_t rv = SYS_OK;
    uint32 vid1, vid2;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    bcm5333x_mem_get(0, M_PORT(lport), port_entry, 8);
    vid1 = (port_entry[0] >> 24) & 0x000000ff;
    vid2 = port_entry[1] & 0xf;
    *vlan_id = (vid2 << 8) | vid1;
    return rv;
}

sys_error_t
board_qvlan_port_set(uint16  vlan_id, uint8 *uplist, uint8 *tag_uplist)
{
    return brdimpl_qvlan_port_set(vlan_id, uplist, tag_uplist);
}

sys_error_t
board_qvlan_port_get(uint16  vlan_id, uint8 *uplist, uint8 *tag_uplist)
{
    return brdimpl_qvlan_port_get(vlan_id, uplist, tag_uplist);
}

uint16
board_vlan_count(void)
{
    return brdimpl_vlan_count();
}

sys_error_t
board_qvlan_get_by_index(uint16  index, uint16 *vlan_id, uint8 *uplist, uint8 *tag_uplist)
{
    return brdimpl_qvlan_get_by_index(index, vlan_id, uplist, tag_uplist);
}
#endif /* CFG_SWITCH_VLAN_INCLUDED */

sys_error_t
board_port_mode_get(uint16 uport, port_mode_t *mode)
{
    int rv = SYS_OK;
    uint8 unit, lport;
    int link, ad, duplex;
    uint32 speed;

    *mode = PM_LINKDOWN;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    /* 
     * In board_uport_to_lport, 
     * it returns chip logical port for DH and WH
     * and returns chip phyical port for FH
     */
    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }

    SOC_IF_ERROR_RETURN(PHY_LINK_GET(BMD_PORT_PHY_CTRL(unit, lport), &link, &ad));

    if (!link) {
       *mode = PM_LINKDOWN;
        return rv;
    }

    SOC_IF_ERROR_RETURN(PHY_SPEED_GET(BMD_PORT_PHY_CTRL(unit, lport), &speed));

    SOC_IF_ERROR_RETURN(PHY_DUPLEX_GET(BMD_PORT_PHY_CTRL(unit, lport), &duplex));

    if(speed == 10) {
        *mode = duplex ? PM_10MB_FD : PM_10MB_HD;
    } else if (speed == 100) {
        *mode = duplex ? PM_100MB_FD : PM_100MB_HD;
    } else if (speed == 1000) {
        *mode = PM_1000MB;
    } else if (speed == 10000) {
        *mode = PM_10000MB;
    } else {
        *mode = PM_AUTO;
    }

    return rv;
}

sys_error_t
board_port_pause_get(uint16 uport, BOOL *tx, BOOL *rx)
{
    uint8 unit, lport;
    sys_error_t r;

    r = board_uport_to_lport(uport, &unit, &lport);

    if (r != SYS_OK) {
        return r;
    }

    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }

    *tx = SOC_PORT_TX_PAUSE_STATUS(lport);
    *rx = SOC_PORT_RX_PAUSE_STATUS(lport);    
    return r;
}

sys_error_t
board_port_an_get(uint16 uport, BOOL *an)
{
    uint8 unit, lport;
    sys_error_t r;
    int get_an;
    
    r = board_uport_to_lport(uport, &unit, &lport);

    if (r != SYS_OK) {
        return r;
    }

    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }
    
    r = PHY_AUTONEG_GET(BMD_PORT_PHY_CTRL(unit, lport), &get_an);
    *an = get_an;
    
    return r;
}

sys_error_t
board_port_cable_diag(uint16 uport, port_cable_diag_t *status)
{
    return brdimpl_port_cable_diag(uport, status);
}

sys_error_t
board_get_cable_diag_support_by_port(uint16 uport, BOOL *support)
{
    uint8 unit, lport;
    sys_error_t r;

    r = board_uport_to_lport(uport, &unit, &lport);

    if (r != SYS_OK) {
        return r;
    }

    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }

    r = PHY_CABLE_DIAG_SUPPORT(BMD_PORT_PHY_CTRL(unit, lport));
    if (r == SYS_OK) {
        *support = TRUE;
    } else {
        *support = FALSE;
    }
    return r;
}

uint16
board_cable_diag_port_count(void)
{
    uint8 cnt = 0;
    BOOL support;
    uint16 uport;
    
    SAL_UPORT_ITER(uport){
        board_get_cable_diag_support_by_port(uport, &support);
        if (support) {
            cnt++;
        }
    }
    /* cnt = 0: no port can support cable count
       cnt is the number of how many port can support cable diag
     */
    return cnt;
}

#ifdef CFG_SWITCH_LAG_INCLUDED
sys_error_t
board_lag_set(uint8 enable)
{
    uint8 i;
    pbmp_t hw_pbmp;
    uint8  lport;
    BOOL   link;
    uint8  pport;

    if (lag_enable != enable) {
        for(i = 1 ; i <= BOARD_MAX_NUM_OF_LAG ; i++) {
            /* add lag setting to HW */
            if (enable == TRUE) {
                hw_pbmp = lag_group[i-1].lpbmp;
                if (SOC_IS_DEERHOUND(0)) {
                    /* remove link down port from hw_pbmp */
                    SOC_LPORT_ITER(lport) { 
                        if (lag_group[i-1].lpbmp & (0x1 << lport)) {
                            SOC_IF_ERROR_RETURN(
                                (*soc_switch_bcm5333x.link_status)(0, lport, &link));
                            if (!link) {
                                hw_pbmp &= ~(0x1 << lport);
                            }
                        }
                    }
                } else {
                    /* remove link down port from hw_pbmp */
                    SOC_LPORT_ITER(lport) {
                        pport = SOC_PORT_L2P_MAPPING(lport);
                        if (lag_group[i-1].lpbmp & (0x1 << pport)) {
                            SOC_IF_ERROR_RETURN(
                                (*soc_switch_bcm5333x.link_status)(0, lport, &link));
                            if (!link) {
                                hw_pbmp &= ~(0x1 << pport);
                            }
                        }
                    }
                }

                SOC_IF_ERROR_RETURN(
                    bcm5333x_lag_group_set(0, (i - 1), hw_pbmp));
            } else {
                /* remove lag setting from HW */
                SOC_IF_ERROR_RETURN(bcm5333x_lag_group_set(0, (i - 1), 0x0));
            }
        }
        lag_enable = enable;
    }
    return SYS_OK;
}

void
board_lag_get(uint8 *enable)
{
    *enable = lag_enable;
}

sys_error_t
board_lag_group_set(uint8 lagid, uint8 enable, uint8 *uplist)
{
    pbmp_t pbmp;
    pbmp_t hw_pbmp;
    uint8 i, j, count = 0;
    BOOL  link;

    if ((lagid == 0) || (lagid > BOARD_MAX_NUM_OF_LAG)) {
        return SYS_ERR_PARAMETER;
    }

    if (uplist != NULL) {
        SOC_IF_ERROR_RETURN(board_uplist_to_lpbmp(uplist, 0, &pbmp));
    } else {
        return SYS_ERR_PARAMETER;
    }

    /* Check the number of trunk member.
     * - Minimum number of ports in each trunk: 2
     * - maximum number of ports in each trunk: 8
     */
    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        if(pbmp & (0x1 << i)) {
            count ++;
            if (count > BOARD_MAX_PORT_PER_LAG) {
                return SYS_ERR_PARAMETER;
            }
        }
    }

    if (count == 0x1) {
        return SYS_ERR_PARAMETER;
    }

    if ((enable == TRUE) && (lag_enable == TRUE)) {
        hw_pbmp = pbmp;
        if (SOC_IS_DEERHOUND(0)) {
            /* remove link down port from hw_pbmp */
            for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
                if (pbmp & (0x1 << i)) {
                    SOC_IF_ERROR_RETURN(
                        (*soc_switch_bcm5333x.link_status)(0, i, &link));
                    if (!link) {
                        hw_pbmp &= ~(0x1 << i);
                    }
                }
            }
        } else {
           /* remove link down port from hw_pbmp */
            for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
                j = SOC_PORT_L2P_MAPPING(i);
                if (pbmp & (0x1 << j)) {
                    SOC_IF_ERROR_RETURN(
                        (*soc_switch_bcm5333x.link_status)(0, i, &link));
                    if (!link) {
                        hw_pbmp &= ~(0x1 << j);
                    }
                }
            }
        }
    } else {
        hw_pbmp = 0;
    }
    /* HW setting */
    SOC_IF_ERROR_RETURN(bcm5333x_lag_group_set(0, (lagid - 1), hw_pbmp));
    /* lag SW database setting */
    lag_group[lagid-1].enable = enable;
    lag_group[lagid-1].lpbmp = pbmp;
    return SYS_OK;
}

sys_error_t
board_lag_group_get(uint8 lagid, uint8 *enable, uint8 *uplist)
{

    if ((lagid == 0) || (lagid > BOARD_MAX_NUM_OF_LAG)) {
        return SYS_ERR_PARAMETER;
    }

    *enable = lag_group[lagid-1].enable;
    SOC_IF_ERROR_RETURN(board_lpbmp_to_uplist(0, lag_group[lagid-1].lpbmp, uplist));

    return SYS_OK;
}

void
board_lag_group_max_num(uint8 *num)
{
    *num = BOARD_MAX_NUM_OF_LAG;
}

void
board_lag_linkchange(uint16 uport, BOOL link, void *arg)
{
    uint8 unit, lport;
    pbmp_t hw_pbmp;
    uint8 i;
    sys_error_t rv = SYS_OK;

    if (lag_enable == TRUE) {
        rv = board_uport_to_lport(uport, &unit, &lport);
        if (rv != SYS_OK) {
            return;
        }
        if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
            lport =SOC_PORT_L2P_MAPPING(lport);
        }
        for (i = 1 ; i <= BOARD_MAX_NUM_OF_LAG ; i++) {
            if ((lag_group[i-1].enable == TRUE) &&
                (lag_group[i-1].lpbmp & (0x1 << lport))){
                /* add lag setting to HW */
                bcm5333x_lag_group_get(0, (i - 1), &hw_pbmp);
                if (link) {
                    hw_pbmp |= (0x1 << lport);
                } else {
                    hw_pbmp &= ~(0x1 << lport);
                }
                bcm5333x_lag_group_set(0, (i - 1), hw_pbmp);
            }
        }
    }
}
#endif /* CFG_SWITCH_LAG_INCLUDED */

#ifdef CFG_SWITCH_MIRROR_INCLUDED
sys_error_t
board_mirror_to_set(uint16 uport)
{
    uint32 entry;
    uint8 unit, lport = 0;
    sys_error_t rv = SYS_OK;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    entry = lport;
    rv = bcm5333x_mem_set(0, M_IM_MTP_INDEX(0), &entry, 1);

    entry = lport;
    rv = bcm5333x_mem_set(0, M_EM_MTP_INDEX(0), &entry, 1);

    return rv;
}

sys_error_t
board_mirror_to_get(uint16 *uport)
{
    uint8 lport;
    uint32 entry;
    sys_error_t rv = SYS_OK;

    rv = bcm5333x_mem_get(0, M_EM_MTP_INDEX(0), &entry, 1);
    if (rv)
    {
       return rv;
    }
    lport = entry & 0x03f;

    return  board_lport_to_uport(0, lport, uport);
}

sys_error_t
board_mirror_port_set(uint16 uport, uint8 enable)
{
    uint32 val, e_val;
    int i;
    uint8 unit, lport;
    uint32 port_entry[8];
    sys_error_t rv = SYS_OK;

    /* check M_ENABLE in PORT_TAB */
    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    /* enable == 0 */
    if (enable == 0) {
        bcm5333x_mem_get(0, M_PORT(lport), port_entry, 8);
        port_entry[0] &= 0xffffffbf;
        bcm5333x_mem_set(0, M_PORT(lport), port_entry, 8);

        rv = bcm5333x_reg_get(0, R_MIRROR_CONTROL(lport), &val);
        val &= 0xfffffffe;
        rv = bcm5333x_reg_set(0, R_MIRROR_CONTROL(lport), val);
        /* exclude the disable port to all ports' EMIRROR_CONTROL */
        for (i = BCM5333X_LPORT_MIN ; i <= BCM5333X_LPORT_MAX; i++) {
            rv = bcm5333x_reg_get(0, R_EMIRROR_CONTROL_64(i), &val);
            val &= ~(1 << lport);
            rv = bcm5333x_reg_set(0, R_EMIRROR_CONTROL_64(i), val);
        }
     } else {
        /* enable != 0 */
        /* ingress enable */
        bcm5333x_mem_get(0, M_PORT(lport), port_entry, 8);
        /* Only set Mirror 0 */
        port_entry[0] |= 0x0040;
        bcm5333x_mem_set(0, M_PORT(lport), port_entry, 8);

        rv = bcm5333x_reg_get(0, R_MIRROR_CONTROL(lport), &val);
        val |= 0x0001;
        rv = bcm5333x_reg_set(0, R_MIRROR_CONTROL(lport), val);

        rv = bcm5333x_reg_get(0, R_EMIRROR_CONTROL_64(lport), &val);
        val |= 1 << lport;
        rv = bcm5333x_reg_set(0, R_EMIRROR_CONTROL_64(lport), val);
    }

    /* handle all MIRROR_CONTROL if any of one is enabled */
    if (SOC_PORT_L2P_MAPPING(lport) <= BCM5333X_PORT_MAX) {
        enable = 0;
        e_val = 0x0;
        for (i = BCM5333X_LPORT_MIN ; i <= BCM5333X_LPORT_MAX; i++) {
            if (SOC_PORT_L2P_MAPPING(i) == -1) {
                continue;
            }
            rv = bcm5333x_reg_get(0, R_MIRROR_CONTROL(i), &val);
            enable |= val;
            rv = bcm5333x_reg_get(0, R_EMIRROR_CONTROL_64(i), &val);
            e_val |= val;
        }
        if (enable) {
            for (i = BCM5333X_LPORT_MIN ; i <= BCM5333X_LPORT_MAX; i++) {
                /* set all ports MIRROR_CONTROL is enable */
                val = 0x0001;
                rv = bcm5333x_reg_set(0, R_MIRROR_CONTROL(i), val);
                rv = bcm5333x_reg_set(0, R_EMIRROR_CONTROL_64(i), e_val);
            }
        }
    }

  return rv;
}

sys_error_t
board_mirror_port_get(uint16 uport, uint8 *enable)
{
    uint32 port_entry[8];
    uint8 unit, lport = 0;
    sys_error_t rv = SYS_OK;

    /* check M_ENABLE in PORT_TAB */
    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    bcm5333x_mem_get(0, M_PORT(lport), port_entry, 8);
    *enable = 0;
    if ((port_entry[0] & 0x0040) == 0x0040) {
        *enable = 1;
    }

    return rv;
}
#endif /* CFG_SWITCH_MIRROR_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
sys_error_t
board_qos_type_set(qos_type_t type)
{
    sys_error_t rv = SYS_OK;
    uint32 port_field_sel_entry[5];
    uint32 tcam_entry[15], dm_entry[15], xy_entry[15];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffffc0, 0x7ff };
    int i;

    if (type == qos_info) {
        return SYS_OK;
    }

    if (type == QT_DOT1P_PRIORITY) {
        /*
         * 802.1P QoS use entries 24~31.
         */
        for (i = 0; i <= 7; i++) {
            bcm5333x_mem_get(0, M_FP_TCAM(DOT1P_BASE_IDX + i), dm_entry, 15);
            bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

            tcam_entry[0] |= 0x3;

            bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
            bcm5333x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, 15);
            bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

        }

        for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
            bcm5333x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + i), dm_entry, 15);
            bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

            tcam_entry[0] &= 0xfffffffc;

            bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
            bcm5333x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + i), xy_entry, 15);
            bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

        }


        for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
            /* Set SLICE2_F1 = 0x6, clear source port qualifier Slice2_F3=0x2 */
            bcm5333x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, 5);

            port_field_sel_entry[1] = 0xc0;

            bcm5333x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, 5);
        }
        bcm5333x_dscp_map_enable(1);
    } else if (type == QT_PORT_BASED) {
        /*
         * Port based QoS use entries 0~23.
         * It'll be created in board_untagged_priority_set later.
         */
        for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
            bcm5333x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + i), dm_entry, 15);
            bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

            tcam_entry[0] |= 0x3;

            bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
            bcm5333x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + i), xy_entry, 15);
            bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

        }

        for (i = 0; i <= 7; i++) {
            bcm5333x_mem_get(0, M_FP_TCAM(DOT1P_BASE_IDX + i), dm_entry, 15);
            bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

            tcam_entry[0] &= 0xfffffffc;

            bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
            bcm5333x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, 15);
            bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

        }

        for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
            bcm5333x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, 5);
            /* SLICE2_F3 = 0x2, clear VLAN qualifier */
            port_field_sel_entry[0] &= 0x3fffffff;
            port_field_sel_entry[1] &= 0xffffe000;
            port_field_sel_entry[1] |= (0x2 << 6);
            bcm5333x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, 5);
        }

        /* disable dscp while Qos in port_based */
        bcm5333x_dscp_map_enable(0);
    }

    qos_info = type;
    return rv;
}

sys_error_t
board_qos_type_get(qos_type_t *type)
{
    *type = qos_info;
    return SYS_OK;
}

sys_error_t
board_untagged_priority_set(uint16 uport, uint8 priority)
{
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
    int i;
    uint32 tcam_entry[15], dm_entry[15], xy_entry[15];
    uint32 policy_entry[8] = { 0x00000000, 0x00000000, 0x00000000, 0x00000110,
                               0x02800000, 0x00000500, 0x00000005, 0x00000000 };

    uint32 tcam_entry_port_based_qos[15] = { 0x00000003, 0x00000000, 0x00000000,
                                 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                 0x00000000, 0xffc00000, 0x0000000b, 0x00000000,
                                 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffffc0, 0x7ff };
    uint32 priority_val = 0x0;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    /* While enable QoS priority in FP, need to disable "1p priority higher than DSCP,
       using set invalid bit in FP_TCAM */
    for (i = 0; i < 8; i++) {
        bcm5333x_mem_get(0, M_FP_TCAM(DOT1P_BASE_IDX + i), dm_entry, 15);
        bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

        tcam_entry[0] &= 0xfffffffc;

        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
        bcm5333x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
                                                global_tcam_mask_entry, 3);

    }

    priority_val = (priority << 10) | (priority << 5) | (priority << 0);
    policy_entry[0] = (policy_entry[0] & 0x001fffff)
                    | ((priority_val & 0x7ff) << 21);
    policy_entry[1] = (policy_entry[1] & 0xfffffff0)
                    | ((priority_val & 0xfffff800) >> 11);
    rv = bcm5333x_mem_set(0, M_FP_POLICY_TABLE(QOS_BASE_IDX + lport), policy_entry, 8);

    /* Using FP slice 2, entry 0~23 for port based qos */
    tcam_entry_port_based_qos[1] = lport << 12;

    bcm5333x_dm_to_xy(tcam_entry_port_based_qos, xy_entry, 15, 234);
    bcm5333x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + lport), xy_entry, 15);
    bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + lport),
                                                global_tcam_mask_entry, 3);
    return rv;
}

sys_error_t
board_untagged_priority_get(uint16 uport, uint8 *priority)
{
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
    uint32 policy_entry[8];

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    rv = bcm5333x_mem_get(0, M_FP_POLICY_TABLE(QOS_BASE_IDX + lport), policy_entry, 8);
    *priority = (policy_entry[0] >> 21) & 0x7;

    return rv;
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_RATE_INCLUDED

sys_error_t
board_port_rate_ingress_set(uint16 uport, uint32 bits_sec)
{
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
    uint32 fp_meter_table[3];
    uint32 kbits_sec = bits_sec/1000;
    uint32 tcam_entry[15], dm_entry[15], xy_entry[15];
    uint32 policy_entry[8];
    uint32 global_tcam_mask_entry[3] = { 0x1, 0xffffffc0, 0x7ff };

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    if (0 == bits_sec) {
        /* no limit */
        bcm5333x_mem_get(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                            dm_entry, 15);
        bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

        tcam_entry[0] &= 0xfffffffc;

        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
        bcm5333x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                            xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                global_tcam_mask_entry, 3);

    } else {
        bcm5333x_mem_get(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                            dm_entry, 15);
        bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

        tcam_entry[0] |= 0x3;

        bcm5333x_dm_to_xy(tcam_entry, xy_entry, 15, 234);
        bcm5333x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                            xy_entry, 15);
        bcm5333x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                global_tcam_mask_entry, 3);

    }

    switch(kbits_sec) {
        case 512:
            fp_meter_table[0] = 0x407cffff;
            fp_meter_table[1] = 0x0001001f;
            fp_meter_table[2] = 0;
            break;
        case 1024:
            fp_meter_table[0] = 0x80f9ffff;
            fp_meter_table[1] = 0x0002003e;
            fp_meter_table[2] = 0;
            break;
        case 2048:
            fp_meter_table[0] = 0x01f3ffff;
            fp_meter_table[1] = 0x0004007d;
            fp_meter_table[2] = 0;
            break;
        case 4096:
            fp_meter_table[0] = 0x03e7ffff;
            fp_meter_table[1] = 0x000800fa;
            fp_meter_table[2] = 0;
            break;
        case 8192:
            fp_meter_table[0] = 0x07cfffff;
            fp_meter_table[1] = 0x001001f4;
            fp_meter_table[2] = 0;
            break;
        case 16384:
            fp_meter_table[0] = 0x0f9fffff;
            fp_meter_table[1] = 0x002003e8;
            fp_meter_table[2] = 0;
            break;
        case 32768:
            fp_meter_table[0] = 0x0f9fffff;
            fp_meter_table[1] = 0x802003e8;
            fp_meter_table[2] = 0;
            break;
        case 65536:
            fp_meter_table[0] = 0x0f9fffff;
            fp_meter_table[1] = 0x002003e8;
            fp_meter_table[2] = 0x1;
            break;
        case 131072:
            fp_meter_table[0] = 0x0f9fffff;
            fp_meter_table[1] = 0x802003e8;
            fp_meter_table[2] = 0x1;
            break;
        case 262144:
            fp_meter_table[0] = 0x0f9fffff;
            fp_meter_table[1] = 0x002003e8;
            fp_meter_table[2] = 0x2;
            break;
        case 524288:
            fp_meter_table[0] = 0x0f9fffff;
            fp_meter_table[1] = 0x802003e8;
            fp_meter_table[2] = 0xa;
            break;
        default:
            fp_meter_table[0] = 0;
            fp_meter_table[1] = 0;
            fp_meter_table[2] = 0;
            break;
    }

    bcm5333x_mem_get(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                                + lport - BCM5333X_LPORT_MIN), policy_entry, 8);
    bcm5333x_mem_set(0, M_FP_METER_TABLE(2 * ((policy_entry[3] >> 19) & 0x3ff)
                         + ((policy_entry[3] >> 8) & 0x1)), fp_meter_table, 3);

    return rv;
}

sys_error_t
board_port_rate_ingress_get(uint16 uport, uint32 *bits_sec)
{
    uint8 unit, lport;
    sys_error_t rv = SYS_OK;
    uint32 fp_meter_table[3];
    uint32 tcam_entry[15];
    uint32 policy_entry[8], dm_entry[15];

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    bcm5333x_mem_get(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5333X_LPORT_MIN),
                                                                dm_entry, 15);
    bcm5333x_xy_to_dm(dm_entry, tcam_entry, 15, 234);

    if ((tcam_entry[0] & 0x3) == 0)
    {
        *bits_sec = 0;
        return SYS_OK;
    }
    bcm5333x_mem_get(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                                + lport - BCM5333X_LPORT_MIN), policy_entry, 8);
    bcm5333x_mem_get(0, M_FP_METER_TABLE(2 * ((policy_entry[3] >> 19) & 0x3ff)
                         + ((policy_entry[3] >> 8) & 0x1)), fp_meter_table, 3);
    switch(fp_meter_table[1]) {
        case 0x0001001f:
            *bits_sec = 512 * 1000;
            break;
        case 0x0002003e:
            *bits_sec = 1024 * 1000;
            break;
        case 0x0004007d:
            *bits_sec = 2048 * 1000;
            break;
        case 0x000800fa:
            *bits_sec = 4096 * 1000;
            break;
        case 0x001001f4:
            *bits_sec = 8192 * 1000;
            break;
        case 0x002003e8:
            if (fp_meter_table[2] == 0) {
                *bits_sec = 16384 * 1000;
            } else if (fp_meter_table[2] == 1) {
                *bits_sec = 65536 * 1000;
            } else {
                *bits_sec = 262144 * 1000;
            }
            break;
        case 0x802003e8:
            if (fp_meter_table[2] == 0) {
                *bits_sec = 32768 * 1000;
            } else if (fp_meter_table[2] == 0x1) {
                *bits_sec = 131072 * 1000;
            } else {
                *bits_sec = 524288 * 1000;
            }
            break;
        default:
            *bits_sec = 0x0;
            break;
    }

    return rv;
}

sys_error_t
board_port_rate_egress_set(uint16 uport, uint32 bits_sec)
{
    uint32 val;
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
//    uint32 kbits_burst;
    uint32 kbits_sec = bits_sec/1000;

    /* Not assign kbits_bust, set kbits_bust = kbits_sec */
//    kbits_burst = kbits_sec;
    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    rv = bcm5333x_reg_set(0, R_EGRMETERINGCONFIG(lport), 0x0);
    if (rv < 0) {
        return rv;
    }

    switch(kbits_sec) {
        case 512:
            val = 0x8010;
            break;
        case 1024:
            val = 0x10020;
            break;
        case 2048:
            val = 0x2003f;
            break;
        case 4096:
            val = 0x4007d;
            break;
        case 8192:
            val = 0x800fa;
            break;
        case 16384:
            val = 0x1001f4;
            break;
        case 32768:
            val = 0x2003e8;
            break;
        case 65536:
            val = 0x4007d0;
            break;
        case 131072:
            val = 0x800fa0;
            break;
        case 262144:
            val = 0x1000fff;
            break;
        case 524288:
            val = 0x2000fff;
            break;
       default:
            val = 0;
            break;
    }
    rv = bcm5333x_reg_set(0, R_EGRMETERINGCONFIG(lport), val);
    if (rv < 0) {
        return rv;
    }

    return rv;
}
sys_error_t
board_port_rate_egress_get(uint16 uport, uint32 *bits_sec)
{
    uint32 val;
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    rv = bcm5333x_reg_get(0, R_EGRMETERINGCONFIG(lport), &val);
    switch(val) {
        case 0x8010:
            *bits_sec = 512 * 1000;
            break;
        case 0x10020:
            *bits_sec = 1024 * 1000;
            break;
        case 0x2003f:
            *bits_sec = 2048 * 1000;
            break;
        case 0x4007d:
            *bits_sec = 4096 * 1000;
            break;
        case 0x800fa:
            *bits_sec = 8192 * 1000;
            break;
        case 0x1001f4:
            *bits_sec = 16384 * 1000;
            break;
        case 0x2003e8:
            *bits_sec = 32768 * 1000;
            break;
        case 0x4007d0:
            *bits_sec = 65536 * 1000;
            break;
        case 0x800fa0:
            *bits_sec = 131072 * 1000;
            break;
        case 0x1000fff:
            *bits_sec = 262144 * 1000;
            break;
        case 0x2000fff:
            *bits_sec = 524288 * 1000;
            break;
       default:
            *bits_sec = 0;
            break;
    }

    return rv;
}

sys_error_t
board_rate_type_set(uint8 flags)
{
    uint16 uport;

    if (flags == STORM_RATE_NONE && storm_info != STORM_RATE_NONE) {
        /* Clear all current settings */
        SAL_UPORT_ITER(uport) {
            board_rate_set(uport, 0);
        }
    }
    storm_info = flags;
    return SYS_OK;
}

sys_error_t
board_rate_type_get(uint8 *flags)
{
    *flags = storm_info;
    return SYS_OK;
}

sys_error_t
board_rate_set(uint16 uport, uint32 bits_sec)
{
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
    uint32 entry[2];
    uint32 kbits_sec = bits_sec/1000;
    int i;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    if (!bits_sec) {
        /* metering is disabled on port - program tables to max values */
        /* Need to set bit-mode */
        rv = bcm5333x_reg_set(0, R_STORM_CONTROL_METER_CONFIG(lport), (0x1 << 12));

        entry[0] = 0;
        entry[1] = 0x7ffff;
        for (i = 0; i < 4; i++) {
            rv = bcm5333x_mem_set(0, M_FP_STORM_CONTROL_METERS((lport*4) + i), entry, 2);
        }
        /* no limit */
        return SYS_OK;
    }

    switch(kbits_sec) {
        case 512:
            /* 512K */
            entry[0] = 0x80000000;
            entry[1] = 0x300010;
            break;
        case 1024:
            entry[0] = 0xc0000000;
            entry[1] = 0x00100020;
            break;
        case 2048:
            entry[0] = 0xc0000000;
            entry[1] = 0x00100040;
            break;
        case 4096:
            entry[0] = 0x00000000;
            entry[1] = 0x300081;
            break;
        case 8192:
            entry[0] = 0x00000000;
            entry[1] = 0x300101;
            break;
        case 16384:
            entry[0] = 0x40000000;
            entry[1] = 0x100201;
            break;
        case 32768:
            entry[0] = 0x40000000;
            entry[1] = 0x100401;
            break;
        case 65536:
            entry[0] = 0x80000000;
            entry[1] = 0x100801;
            break;
        case 131072:
            entry[0] = 0xc0000000;
            entry[1] = 0x301001;
            break;
        case 262144:
            entry[0] = 0xc0000000;
            entry[1] = 0x302001;
            break;
        case 524288:
            entry[0] = 0xc0000000;
            entry[1] = 0x304001;
            break;
       default:
            entry[0] = 0;
            entry[1] = 0;
            break;
    }

    /* enable bcast in STORM_CONTROL_METER_CONFIG*/
    rv = bcm5333x_reg_set(0, R_STORM_CONTROL_METER_CONFIG(lport), (0x7f << 12));
    for (i = 0; i < 4; i++) {
        rv = bcm5333x_mem_set(0, M_FP_STORM_CONTROL_METERS((lport * 4) + i), entry, 2);
    }

    return rv;
}

sys_error_t
board_rate_get(uint16 uport, uint32 *bits_sec)
{
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
    uint32 val;
    uint32 entry[2];

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    rv = bcm5333x_reg_get(0, R_STORM_CONTROL_METER_CONFIG(lport), &val);
    if (!(val & 0x7e000)) {
        /*disable, no limit */
        *bits_sec = 0;
        return rv;
    }

    rv = bcm5333x_mem_get(0, M_FP_STORM_CONTROL_METERS(lport * 4), entry, 2);
    *bits_sec = ((entry[1] & 0xffffe) << 5) * 1000;
    return rv;
}
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_STAT_INCLUDED
sys_error_t
board_port_stat_get(uint16 uport, port_stat_t *stat)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2];
    uint8 unit, lport, lport_before_swap;


    rv |= board_uport_to_lport(uport, &unit, &lport);

    lport_before_swap = lport;

    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        lport = SOC_PORT_L2P_MAPPING(lport);
    }
    sal_memset(stat, 0, sizeof(port_stat_t));
    if (IS_XL_PORT(lport)) {
        /* Byte Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxOctets_lo = entry[0];
        stat->TxOctets_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxOctets_lo = entry[0];
        stat->RxOctets_hi = entry[1];
        /* Frame counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxPkts_lo = entry[0];
        stat->TxPkts_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxPkts_lo = entry[0];
        stat->RxPkts_hi = entry[1];
        /* Rx FCS Error Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->CRCErrors_lo = entry[0];
        stat->CRCErrors_hi = entry[1];
        /* Unicast Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxUnicastPkts_lo = entry[0];
        stat->RxUnicastPkts_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxUnicastPkts_lo = entry[0];
        stat->TxUnicastPkts_hi = entry[1];        
        /* Multicast Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxMulticastPkts_lo = entry[0];
        stat->RxMulticastPkts_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxMulticastPkts_lo = entry[0];
        stat->TxMulticastPkts_hi = entry[1]; 
        /* Broadcast Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxBroadcastPkts_lo = entry[0];
        stat->RxBroadcastPkts_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxBroadcastPkts_lo = entry[0];
        stat->TxBroadcastPkts_hi = entry[1]; 
        /* Pause Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxPauseFramePkts_lo = entry[0];
        stat->RxPauseFramePkts_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxPauseFramePkts_lo = entry[0];
        stat->TxPauseFramePkts_hi = entry[1]; 
        /* Oversized Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxOversizePkts_lo = entry[0];
        stat->RxOversizePkts_hi = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxOversizePkts_lo = entry[0];
        stat->TxOversizePkts_hi = entry[1];         
        
    } else {
        /* Byte Counter */
	sal_printf("lport %d The block id is %d\n",lport,SOC_PORT_BLOCK(lport));
        sal_printf("the port block is%d\n",R_GTBYT(SOC_PORT_BLOCK_INDEX(lport)));
	rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTBYT(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxOctets_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRBYT(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxOctets_lo);
        /* Frame counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTPKT(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxPkts_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRPKT(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxPkts_lo);
        /* Rx FCS Error Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRFCS(SOC_PORT_BLOCK_INDEX(lport)), &stat->CRCErrors_lo);
        /* EEE LPI counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxLPIPkts_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxLPIDuration_lo);        
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxLPIPkts_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxLPIDuration_lo);        
        /* Unicast Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRUC(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxUnicastPkts_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTUC(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxUnicastPkts_lo);
        /* Multicast Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRMCA(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxMulticastPkts_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTMCA(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxMulticastPkts_lo);
        /* Broadcast Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRBCA(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxBroadcastPkts_lo);
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTBCA(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxBroadcastPkts_lo);
        /* Pause Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRXPF(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxPauseFramePkts_lo);    
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTXPF(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxPauseFramePkts_lo);   
        /* Oversized Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GROVR(SOC_PORT_BLOCK_INDEX(lport)), &stat->RxOversizePkts_lo);    
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTOVR(SOC_PORT_BLOCK_INDEX(lport)), &stat->TxOversizePkts_lo);   
    }

    /* IPIPE HOLD Drop Counter */
    if (SOC_IS_DEERHOUND(unit)) {
        rv |= bcm5333x_reg_get(0, R_HOLD_COS(lport), &stat->EgressDropPkts_lo);
    } else {
        rv |= bcm5333x_reg_get(0, R_HOLD_COS(lport_before_swap), &stat->EgressDropPkts_lo);
    }

    return rv;
}

sys_error_t
board_port_stat_clear(uint16 uport)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2] = {0, 0};
    uint8 unit, lport;

    rv = board_uport_to_lport(uport, &unit, &lport);

    if (IS_XL_PORT(lport)) {
        /* Byte Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Frame counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Rx FCS Error Frame Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Unicast Frame Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Multicast Frame Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Broadcast Frame Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Pause Frame Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Oversized Frame Counter */
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5333x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    } else {
        /* Byte Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTBYT(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRBYT(SOC_PORT_BLOCK_INDEX(lport)), 0);
        /* Frame counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTPKT(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRPKT(SOC_PORT_BLOCK_INDEX(lport)), 0);
        /* Rx FCS Error Frame Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRFCS(SOC_PORT_BLOCK_INDEX(lport)), 0);
        /* EEE LPI counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), 0);        
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), 0);        
        /* Unicast Frame Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRUC(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTUC(SOC_PORT_BLOCK_INDEX(lport)), 0);
        /* Multicast Frame Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRMCA(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTMCA(SOC_PORT_BLOCK_INDEX(lport)), 0);
        /* Broadcast Frame Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRBCA(SOC_PORT_BLOCK_INDEX(lport)), 0);
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTBCA(SOC_PORT_BLOCK_INDEX(lport)), 0);
        /* Pause Frame Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GRXPF(SOC_PORT_BLOCK_INDEX(lport)), 0);    
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTXPF(SOC_PORT_BLOCK_INDEX(lport)), 0);   
        /* Oversized Frame Counter */
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GROVR(SOC_PORT_BLOCK_INDEX(lport)), 0);    
        rv |= bcm5333x_reg_set(0, SOC_PORT_BLOCK(lport), R_GTOVR(SOC_PORT_BLOCK_INDEX(lport)), 0);   
    }
    
    /* IPIPE HOLD Drop Counter */
    rv |= bcm5333x_reg_set(0, R_HOLD_COS(lport), 0);

    return rv;
}

sys_error_t
board_port_stat_clear_all(void)
{
    uint16 uport;
    sys_error_t rv = SYS_OK;

    SAL_UPORT_ITER(uport) {
        rv = board_port_stat_clear(uport);
    }
    return rv;
}

#endif /* CFG_SWITCH_STAT_INCLUDED */


#if defined(CFG_SWITCH_MCAST_INCLUDED) 
sys_error_t
board_mcast_addr_add(uint8 *mac_addr, uint16 vlan_id, uint8 *uplist)
{
    int rv = SYS_OK;
    int i,j;
    uint32 entry[2];
    pbmp_t lpbmp;
    mcast_list_t *mcast = NULL;
    l2x_entry_t l2x;
	uint16 uc_idx;
	uint16 umc_idx;

    if (uplist != NULL) {
        board_uplist_to_lpbmp(uplist, 0, &lpbmp);
    } else {
        lpbmp = 0;
    }

    mcast = (mcast_list_t *)sal_malloc(sizeof(mcast_list_t));
    if (mcast == NULL) {
        return SYS_ERR_OUT_OF_RESOURCE;
    }

    if (vlan_id == 0xFFF) {
        /* LEARN_VID as '1' in VLAN_CTRL
           set in soc->vlan_type_set */
        vlan_id = 1;
    }

    mcast->vlan_id = vlan_id;
    mcast->next = NULL;
    mcast->port_lpbmp = lpbmp;
	mcast->uc_index = 0xFFFF;
    sal_memcpy(mcast->mac, mac_addr, 6);
    if (mlist == NULL) {
        mlist = mcast;
        mlist->index=1;
        mcindex[0]=0x1;
        mcindex[1]=0;
        mcindex[2]=0;
        mcindex[3]=0;
    } else {
       mcast->next = mlist;
       mcast->index = 0;
       for (i=0; i<4; i++) {
           for (j=0; j<32; j++) {
                if ((mcindex[i] & (1<<j)) == 0) {
                    mcast->index = i*32 + j + 1;
                    mcindex[i] |= (1<<j);
                    break;
                }
           }
           if(mcast->index != 0) {
               break;
           }
       }
       /* get next mc index it can be used */
       mlist = mcast;
    }

    l2x.vlan_id = vlan_id;
    l2x.port = mcast->index;
    sal_memcpy(l2x.mac_addr, mac_addr, 6);
    rv = bcm5333x_l2_op(0, &l2x, SC_OP_L2_INS_CMD);

    /* after get mcindex from l2_entry */
    bcm5333x_mem_get(0, M_L2MC(mcast->index), entry, 2);
    entry[0] &= 0x000000ff;
    entry[0] |= ((lpbmp & 0xffffff)<< 8);
    entry[1] &= 0xffffff80;
    entry[1] |= 0x40 | ((lpbmp & 0xff000000) >> 24);
    bcm5333x_mem_set(0, M_L2MC(mcast->index), entry, 2);
	
	rv = bcm5333x_vlan_l3_ipmc_get(0, vlan_id, &uc_idx, &umc_idx);
	if (rv != SYS_OK) {
        return rv;
	}

	if (uc_idx == 0xFFFF || umc_idx == 0xFFFF) {
		rv = l3_ipmc_id_get(&mcast->uc_index);
	    if (rv != SYS_OK) {
            return rv;
	    }		
	}
	uc_idx = mcast->uc_index;
	
	
	rv = bcm5333x_l3_ipmc_set(0, uc_idx, lpbmp);
	if (rv != SYS_OK) {
		l3_ipmc_id_free(uc_idx);
        return rv;
	}
	rv = bcm5333x_vlan_l3_ipmc_set(0, vlan_id, uc_idx, 0);
	if (rv != SYS_OK) {
        return rv;
	}	

    return rv;
}

sys_error_t
board_mcast_addr_remove(uint8 *mac_addr, uint16 vlan_id)
{
    int rv = SYS_OK;
    uint32 entry[2];
    mcast_list_t *mcast = NULL;
    int exists=0;
    mcast_list_t *prev_mcast, *tmp_mcast;
    int l2_index=0;
    l2x_entry_t l2x;
	int l3_index = 0;

    if (vlan_id == 0xFFF) {
        /* LEARN_VID as '1' in VLAN_CTRL
           set in soc->vlan_type_set */
        vlan_id = 1;
    }

     /* check if exists */
    mcast = mlist;
    while(mcast != NULL) {
        if((mcast->vlan_id == vlan_id) && (!sal_memcmp(mcast->mac, mac_addr, 6))) {
            exists = 1;
            break;
        }
        mcast = mcast->next;
    }
    if (exists == 0) {
        return SYS_OK;
    }

    /* existed, destroy mcast */
    prev_mcast = mlist;
    if((prev_mcast->vlan_id == vlan_id) && (!sal_memcmp(prev_mcast->mac, mac_addr, 6))) {
        /* first node is vlan need to destroy */
        l2_index = prev_mcast->index;
		l3_index = prev_mcast->uc_index;
        if ((prev_mcast->index)<=32) {
            mcindex[0] &= ~(1<<((prev_mcast->index - 1)%32));
        } else if ((prev_mcast->index)<=64) {
            mcindex[1] &= ~(1<<((prev_mcast->index - 1)%32));
        } else if ((prev_mcast->index)<=96) {
            mcindex[2] &= ~(1<<((prev_mcast->index - 1)%32));
        } else {
            mcindex[3] &= ~(1<<((prev_mcast->index - 1)%32));
        }
        mlist = mlist->next;
        sal_free(prev_mcast);
    } else {
        mcast= prev_mcast->next;
        while((prev_mcast->next) != NULL) {
            if((mcast->vlan_id == vlan_id) && (!sal_memcmp(mcast->mac, mac_addr, 6))) {
                l2_index = mcast->index;
				l3_index = prev_mcast->uc_index;
                if ((mcast->index)<=32) {
                    mcindex[0] &= ~(1<<((mcast->index - 1)%32));
                } else if ((mcast->index)<=64) {
                    mcindex[1] &= ~(1<<((mcast->index - 1)%32));
                } else if ((mcast->index)<=96) {
                    mcindex[2] &= ~(1<<((mcast->index - 1)%32));
                } else {
                    mcindex[3] &= ~(1<<((mcast->index - 1)%32));
                }
                tmp_mcast = mcast->next;
                prev_mcast->next = tmp_mcast;
                sal_free(mcast);
                mcast = mlist;
            } else {
                prev_mcast = mcast;
                mcast = mcast->next;
            }
        }
    }
    if (l2_index == 0) {
        return SYS_OK;
    }

    l2x.vlan_id = vlan_id;
    l2x.port = l2_index;
    sal_memcpy(l2x.mac_addr, mac_addr, 6);
    rv = bcm5333x_l2_op(0, &l2x, SC_OP_L2_DEL_CMD);

    /* after get mcindex from l2_entry */
    bcm5333x_mem_get(0, M_L2MC(l2_index), entry, 2);
    entry[0] &= 0x000000ff;  /* clear valid bit and pbmp from bit 8 -bit 38 */
    entry[1] &= 0xffffff80;
    bcm5333x_mem_set(0, M_L2MC(l2_index), entry, 2);
	
	rv = bcm5333x_l3_ipmc_del(0, l3_index);
	l3_ipmc_id_free(l3_index);

    return rv;
}

sys_error_t
board_mcast_port_add(uint8 *mac_addr, uint16 vlan_id, uint16 uport)
{
    int rv = SYS_OK;
    uint8 unit;
    uint32 entry[2];
    uint8 lport;
    mcast_list_t *mcast = NULL;
    int exists = 0;
	pbmp_t lpbmp;

    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    /* check if exists */
    mcast = mlist;

    if (vlan_id == 0xFFF) {
        /* LEARN_VID as '1' in VLAN_CTRL
           set in soc->vlan_type_set */
        vlan_id = 1;
    }

    while(mcast != NULL) {
        if((mcast->vlan_id == vlan_id) && (!sal_memcmp(mcast->mac, mac_addr, 6))) {
            mcast->port_lpbmp |= (1<<lport);
            exists = 1;
            break;
        }
        mcast = mcast->next;
    }
    if (exists == 0) {
        /* no exists */
        return SYS_ERR_NOT_FOUND;
    }

    /* after get mcindex from l2_entry */
    bcm5333x_mem_get(0, M_L2MC(mcast->index), entry, 2);
    if (lport <= 23) {
        entry[0] |= (1 << (lport + 8));
    } else {
        entry[1] |= (1 << (lport - 24));
    }
    bcm5333x_mem_set(0, M_L2MC(mcast->index), entry, 2);
	board_uport_to_lpbmp(uport, 0, &lpbmp);

	if (mcast->uc_index != 0xFFFF) {		
		
        rv = bcm5333x_l3_ipmc_set(0, mcast->uc_index, lpbmp);
	    if (rv != SYS_OK) {
            return rv;
	    }
	} else {
        rv = l3_ipmc_id_get(&mcast->uc_index);
	    if (rv != SYS_OK) {
            return rv;
	    }		
		
		rv = bcm5333x_l3_ipmc_set(0, mcast->uc_index, lpbmp);
	    if (rv != SYS_OK) {
		    l3_ipmc_id_free(mcast->uc_index);
            return rv;
	    }
	}
    return rv;
}

sys_error_t
board_mcast_port_remove(uint8 *mac_addr, uint16 vlan_id, uint16 uport)
{
    int rv = SYS_OK;
    uint8 unit;
    uint32 entry[2];
    uint8 lport;
    mcast_list_t *mcast = NULL;
    int exists = 0;
	pbmp_t pbmp;
	pbmp_t lpbmp;

    /* check if exists */
    rv = board_uport_to_lport(uport, &unit, &lport);
    if (rv != SYS_OK) {
        return rv;
    }

    mcast = mlist;

    if (vlan_id == 0xFFF) {
        /* LEARN_VID as '1' in VLAN_CTRL
           set in soc->vlan_type_set */
        vlan_id = 1;
    }

    while(mcast != NULL) {
        if((mcast->vlan_id == vlan_id) && (!sal_memcmp(mcast->mac, mac_addr, 6))) {
            mcast->port_lpbmp &= ~(1<<lport);
            exists = 1;
            break;
        }
        mcast = mcast->next;
    }
    if (exists == 0) {
        /* no exists, not need to remove */
        return SYS_OK;
    }

    /* after get mcindex from l2_entry */
    bcm5333x_mem_get(0, M_L2MC(mcast->index), entry, 2);
    if (lport <= 23) {
        entry[0] &= ~(1 << (lport + 8));
    } else {
        entry[1] &= ~(1 << (lport - 24));
    }
    bcm5333x_mem_set(0, M_L2MC(mcast->index), entry, 2);
	
	board_uport_to_lpbmp(uport, 0, &lpbmp);	
	if (mcast->uc_index != 0xFFFF) {
		BCM_PBMP_CLEAR(pbmp);
		bcm5333x_l3_ipmc_get(0, mcast->uc_index, &pbmp);
	    BCM_PBMP_REMOVE(pbmp, lpbmp);
		
		if (BCM_PBMP_IS_NULL(pbmp)) {
            rv = bcm5333x_l3_ipmc_del(0, mcast->uc_index);
	        if (rv != SYS_OK) {
                return rv;
	        }
			l3_ipmc_id_free(mcast->uc_index);
			mcast->uc_index = 0xFFFF;
		}
		else {			
            rv = bcm5333x_l3_ipmc_set(0, mcast->uc_index, pbmp);
	        if (rv != SYS_OK) {		        
                return rv;
	        }
		}
	}
    return rv;
}

sys_error_t
board_mcast_port_get(uint8 *mac_addr, uint16 vlan_id, uint8 *uplist)
{
    int rv = SYS_OK;
    mcast_list_t *mcast = NULL;
    int exists = 0;

    /* check if exists */
    mcast = mlist;

    if (vlan_id == 0xFFF) {
        /* LEARN_VID as '1' in VLAN_CTRL set in soc->vlan_type_set */
        vlan_id = 1;
    }

    while(mcast != NULL) {
        if((mcast->vlan_id == vlan_id) && (!sal_memcmp(mcast->mac, mac_addr, 6))) {
            exists = 1;
            break;
        }
        mcast = mcast->next;
    }
    if (exists == 0) {
        /* no exists */
        return SYS_ERR_NOT_FOUND;
    }

    board_lpbmp_to_uplist(0, mcast->port_lpbmp, uplist);

    return rv;
}

sys_error_t
board_igmp_snoop_enable_set(uint8 enable)
{
    uint32 val;
    sys_error_t rv = SYS_OK;

    /* in M_PORT.PROTOCOL_PKT_INDEX set the index to IGMP_MLD_PKT_CONTROL
     * Per port set in this bit is '0', thus set IGMP_MLD_PKT_CONTROL(0)
     */
    rv = bcm5333x_reg_get(0, R_IGMP_MLD_PKT_CONTROL(0), &val);
    if (enable == 1) {
        /* set bit 26-18 = 011011011 */
        val = (val & 0xf803ffff) | 0x036c0000;
    } else {
        /* set bit 26-18 = 100100100 */
        val = (val & 0xf803ffff) | 0x04900000;
    }
    rv = bcm5333x_reg_set(0, R_IGMP_MLD_PKT_CONTROL(0), val);
    rv = bcm5333x_reg_get(0, R_IGMP_MLD_PKT_CONTROL(0), &val);

    return rv;
}

sys_error_t
board_igmp_snoop_enable_get(uint8 *enable)
{
    uint32 val;
    sys_error_t rv = SYS_OK;

    /* select (0) for check due to all port in M_PORT.PROTOCOL_PKT_INDEX
       set to '0'
     */
    rv = bcm5333x_reg_get(0, R_IGMP_MLD_PKT_CONTROL(0), &val);
    if (val & 0x036c0600) {
        *enable = 1;
    } else {
        *enable = 0;
    }
    return rv;
}
sys_error_t
board_block_unknown_mcast_set(uint8 enable)
{
    uint32 val;
    int i;
    int rv;

    rv = SYS_OK;

    for (i = BCM5333X_LPORT_MIN; i <= BCM5333X_LPORT_MAX; i++) {
        if (i == 1) {
            continue;
        }
        rv = bcm5333x_reg_get(0, R_UNKNOWN_MCAST_BLOCK_MASK_64(i), &val);
        if (enable == 1) {
            val |= 0xffffffff; /* block all */
        } else {
            val &= 0x0;
        }
        rv = bcm5333x_reg_set(0, R_UNKNOWN_MCAST_BLOCK_MASK_64(i), val);

        rv = bcm5333x_reg_get(0, R_IUNKNOWN_MCAST_BLOCK_MASK_64(i), &val);
        if (enable == 1) {
            val |= 0xffffffff; /* block all */
        } else {
            val &= 0x0;
        }
        rv = bcm5333x_reg_set(0, R_IUNKNOWN_MCAST_BLOCK_MASK_64(i), val);
    }
    return rv;
}

sys_error_t
board_block_unknown_mcast_get(uint8 *enable)
{
    uint32 val;
    int rv;

    rv = SYS_OK;

    /* select port 1 to check */
    *enable = 0;
    rv = bcm5333x_reg_get(0, R_UNKNOWN_MCAST_BLOCK_MASK_64(BCM5333X_LPORT_MIN), &val);
    if (val & 0xffffffff) {
        *enable = 1;
    }

    return rv;
}
#endif /*  defined(CFG_SWITCH_MCAST_INCLUDED)  */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
void
board_loop_detect_enable(BOOL enable)
{
    bcm5333x_loop_detect_enable(enable);
}

uint8
board_loop_detect_status_get(void)
{
    return bcm5333x_loop_detect_status_get();
}
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

sys_error_t
board_port_loopback_enable_set(uint16 uport, int loopback_mode)
{
    uint8 unit, lport;
    sys_error_t r = SYS_OK;
    
    r = board_uport_to_lport(uport, &unit, &lport);

    if (r != SYS_OK) {
        return r;
    }

    if (!SOC_IS_DEERHOUND(unit) && lport < PHY_SECOND_QGPHY_PORT0) {
        /* Get chip local port for BMD_PORT_PHY_CTRL for FH */
        lport = SOC_PORT_P2L_MAPPING(lport);
    }

    bcm5333x_loopback_enable(unit, lport, loopback_mode);

    return r;
}

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
sys_error_t
board_mdns_enable_set(BOOL enable)
{
    return bcm5333x_mdns_enable_set(0, enable);
}
sys_error_t
board_mdns_enable_get(BOOL *enable){
    return bcm5333x_mdns_enable_get(0, enable);
}
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

#ifdef CFG_SWITCH_SNAKETEST_INCLUDED
sys_error_t
board_snaketest_trap_to_cpu(uint16 vlanid, uint8 *uplist) {
    pbmp_t      lpbmp, tag_lpbmp;

    board_uplist_to_lpbmp(uplist, 0, &lpbmp);
    lpbmp |= 0x1;
    tag_lpbmp = 0x1;

    return bcm5333x_qvlan_port_set(0, vlanid, lpbmp, tag_lpbmp);
}
#endif /* CFG_SWITCH_SNAKETEST_INCLUDED */

#ifdef CFG_RESET_BUTTON_INCLUDED
uint8 sw_simulate_press_reset_button_duration = 0;
uint8 reset_button_active_high = 1;
uint8 reset_button_gpio_bit = 4;

BOOL
board_reset_button_get(void)
{
    uint32 val;
    /*
     * PAD_GPIO 0 ~ 3 : from CMICD
     * PAD_GPIO 4 ~ 7 : from IPROC
     */
    val = (READCSR(CCA_GPIOINPUT) & 0x00f0) | (READCSR(CMIC_GP_DATA_IN) & 0x000f);

    if (sw_simulate_press_reset_button_duration) {
        sw_simulate_press_reset_button_duration--;
        return TRUE;
    }

    if  (reset_button_active_high) {
        if (val & (1 << (reset_button_gpio_bit))) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if (val & (1 << (reset_button_gpio_bit))) {
            return FALSE;
        } else {
            return TRUE;
        }
    }
}

#endif /* CFG_RESET_BUTTON_INCLUDED */

sys_error_t
board_lport_stat_get(uint16 lport, uint32 *stat)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2];    
	uint8 lport_before_swap;

    lport_before_swap = lport;

    if (!SOC_IS_DEERHOUND(0) && lport < PHY_SECOND_QGPHY_PORT0) {
        lport = SOC_PORT_L2P_MAPPING(lport);
    }
    sal_memset(stat, 0, 2 * R_MAX * sizeof(uint32));
    if (IS_XL_PORT(lport)) {
        /* Byte Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat[TBYT] = entry[0];
        stat[TBYT + R_MAX] = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RBYT] = entry[0];
        stat[RBYT + R_MAX] = entry[1];
        /* Frame counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[TPKT] = entry[0];
        stat[TPKT + R_MAX] = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RPKT] = entry[0];
        stat[RPKT + R_MAX] = entry[1];
        /* Rx FCS Error Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RFCS] = entry[0];
        stat[RFCS + R_MAX] = entry[1];
        /* Unicast Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RUC] = entry[0];
        stat[RUC + R_MAX] = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[TUC] = entry[0];
        stat[TUC + R_MAX] = entry[1];        
        /* Multicast Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RMCA] = entry[0];
        stat[RMCA + R_MAX] = entry[0];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[TMCA] = entry[0];
        stat[TMCA + R_MAX] = entry[0]; 
        /* Broadcast Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RBCA] = entry[0];
        stat[RBCA + R_MAX] = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[TBCA] = entry[0];
        stat[TBCA + R_MAX] = entry[1]; 
        /* Pause Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[RXPF] = entry[0];
        stat[RXPF + R_MAX] = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[TXPF] = entry[0];
        stat[TXPF + R_MAX] = entry[1]; 
        /* Oversized Frame Counter */
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[ROVR] = entry[0];
        stat[ROVR + R_MAX] = entry[1];
        rv |= bcm5333x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
		
        stat[TOVR] = entry[0];
        stat[TOVR + R_MAX] = entry[1];         
        
    } else {
        /* Byte Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTBYT(SOC_PORT_BLOCK_INDEX(lport)), &stat[TBYT]);		
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRBYT(SOC_PORT_BLOCK_INDEX(lport)), &stat[RBYT]);

        /* Frame counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTPKT(SOC_PORT_BLOCK_INDEX(lport)), &stat[TPKT]);
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRPKT(SOC_PORT_BLOCK_INDEX(lport)), &stat[RPKT]);
        /* Rx FCS Error Frame Counter */

        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRFCS(SOC_PORT_BLOCK_INDEX(lport)), &stat[RFCS]);

        /* Unicast Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRUC(SOC_PORT_BLOCK_INDEX(lport)), &stat[RUC]);
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTUC(SOC_PORT_BLOCK_INDEX(lport)), &stat[TUC]);

        /* Multicast Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRMCA(SOC_PORT_BLOCK_INDEX(lport)), &stat[RMCA]);
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTMCA(SOC_PORT_BLOCK_INDEX(lport)), &stat[TMCA]);

        /* Broadcast Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRBCA(SOC_PORT_BLOCK_INDEX(lport)), &stat[RBCA]);
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTBCA(SOC_PORT_BLOCK_INDEX(lport)), &stat[TBCA]);

        /* Pause Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GRXPF(SOC_PORT_BLOCK_INDEX(lport)), &stat[RXPF]);
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTXPF(SOC_PORT_BLOCK_INDEX(lport)), &stat[TXPF]);

        /* Oversized Frame Counter */
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GROVR(SOC_PORT_BLOCK_INDEX(lport)), &stat[ROVR]);
		
        rv |= bcm5333x_reg_get(0, SOC_PORT_BLOCK(lport), R_GTOVR(SOC_PORT_BLOCK_INDEX(lport)), &stat[TOVR]);
    }

    /* IPIPE HOLD Drop Counter */
    if (SOC_IS_DEERHOUND(0)) {
        rv |= bcm5333x_reg_get(0, R_HOLD_COS(lport), &stat[EgressDrop]);
    } else {
        rv |= bcm5333x_reg_get(0, R_HOLD_COS(lport_before_swap), &stat[EgressDrop]);
    }

    return rv;
}


