/*
 * $Id: board.c,v 1.23.2.2 Broadcom SDK $
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
#include "soc/bcm5346x.h"
#include "utils/ports.h"

extern soc_switch_t soc_switch_bcm5346x;
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

#ifdef CFG_SWITCH_MCAST_INCLUDED
typedef struct mcast_list_s {
    uint8 mac[6];
    uint16 vlan_id;
    uint16 index;
    pbmp_t  port_lpbmp; /* all ports' pbmp in this vlan_id */

    struct mcast_list_s *next;
} mcast_list_t;

static mcast_list_t *mlist = NULL;

/* address the minimal l2mc index can be used */
static pbmp_t mcindex[4];
#endif /* CFG_SWITCH_MCAST_INCLUDED */



#ifdef CFG_SWITCH_LAG_INCLUDED
/* variables for static trunk */
uint8 lag_enable = FALSE;

typedef struct lag_group_s {
    uint8 enable;
    pbmp_t lpbmp;
} lag_group_t;

lag_group_t lag_group[BOARD_MAX_NUM_OF_LAG];
#endif /* CFG_SWITCH_LAG_INCLUDED */

const char *
board_name(void)
{
    return um_boardname;
}

/**
 * Get the number of chip internal logical ports (lports)
 *
 * @return the number of chip internal logical  ports
 *
 */

uint8
board_lport_count(void)
{
	return bcm5346x_port_count(0);
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
	return bcm5346x_port_count(0);
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

    if (SAL_UPORT_IS_NOT_VALID(uport) || unit == NULL || lport == NULL) {
        return SYS_ERR_PARAMETER;
    }

    count = SAL_UPORT_BASE;
    SOC_LPORT_ITER(port) {
        if (count == uport) {
            break;
        }
        count++;
    }

    *unit = 0;
    *lport = port;

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
    uint8 count, port;
    if (uport == NULL || lport > BCM5346X_LPORT_MAX || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    count = SAL_UPORT_BASE;
    SOC_LPORT_ITER(port) {
        if (port == lport) {
            break;
        }
        count++;
    }

    *uport = count;

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
    uint8 lport;
    uint16 uport;
    if (uplist == NULL || lpbmp == NULL || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    *lpbmp = 0;

    SAL_UPORT_ITER(uport) {
            if (uplist_port_matched(uplist, uport) == SYS_OK) {
                if (board_uport_to_lport(uport, &unit, &lport) == SYS_OK) {
                     *lpbmp |= (1 << lport);
                }
            }
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
    uint16 uport;/* uport is front port index from 0 to board_uport_count()-1 */
    uint8 lport;

    if (uplist == NULL || unit > 0) {
        return SYS_ERR_PARAMETER;
    }

    uplist_clear(uplist);

    SOC_LPORT_ITER(lport) {
        if ((lpbmp >> lport ) & 0x1) {
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
    return &soc_switch_bcm5346x;
}
#if !CONFIG_HURRICANE3_ROMCODE
sys_error_t
board_get_port_link_status(uint16 uport, BOOL *link)
{
    uint8 unit, lport;
    sys_error_t r;
    r = board_uport_to_lport(uport, &unit, &lport);
    if (r != SYS_OK) {
        return r;
    }

    return (*soc_switch_bcm5346x.link_status)(unit, lport, link);
}
#endif /* !CONFIG_HURRICANE3_ROMCODE */
void
board_reset(void *param)
{
    if (param) {
        if (*(BOOL *)param) {
            /* Hard reset */
        }
    }
    /* [Bit 0]: switch reset, [Bit 1]: iproc only reset */
    SYS_REG_WRITE32(R_DMU_CRU_RESET, 0x0);
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
    bcm5346x_rxtx_stop();
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
 *  Output: the egress mask of this port. 1: can forward to. 0: cannot.
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
    uint32 port_entry[FP_PORT_T_SIZE];
    uint32 val32 = vlan_id;

    sys_error_t rv = SYS_OK;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));


    /* PORT_TABL.PORT_VID[32:21] */
    bcm5346x_mem_get(0, M_PORT(lport), port_entry, FP_PORT_T_SIZE);
    port_entry[0] = (port_entry[0] & 0x001fffff) | (val32 << 21);
    port_entry[1] = (port_entry[1] & 0xfffffffe) | ((val32 >> 11) & 0x1);
    bcm5346x_mem_set(0, M_PORT(lport), port_entry, FP_PORT_T_SIZE);

  return rv;
}

sys_error_t
board_untagged_vlan_get(uint16 uport, uint16 *vlan_id)
{
    uint32 port_entry[FP_PORT_T_SIZE];
    uint8 unit, lport;
    sys_error_t rv = SYS_OK;
    uint32 vid1, vid2;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    bcm5346x_mem_get(0, M_PORT(lport), port_entry, FP_PORT_T_SIZE);
    vid1 = (port_entry[0] >> 21);
    vid2 = port_entry[1] & 0x1;
    *vlan_id = (vid2 << 11) | vid1;
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

    *mode = PM_LINKDOWN;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    if (!SOC_PORT_LINK_STATUS(lport)) {
       *mode = PM_LINKDOWN;
        return rv;
    }

    if(SOC_PORT_SPEED_STATUS(lport) == 10) {
        *mode = SOC_PORT_DUPLEX_STATUS(lport) ? PM_10MB_FD : PM_10MB_HD;
    } else if (SOC_PORT_SPEED_STATUS(lport) == 100) {
        *mode = SOC_PORT_DUPLEX_STATUS(lport) ? PM_100MB_FD : PM_100MB_HD;
    } else if (SOC_PORT_SPEED_STATUS(lport) == 1000) {
        *mode = PM_1000MB;
    } else if (SOC_PORT_SPEED_STATUS(lport) == 2500) {
        *mode = PM_2500MB;
    } else if (SOC_PORT_SPEED_STATUS(lport) == 10000) {
        *mode = PM_10000MB;
    } else {
        *mode = PM_AUTO;
    }

    return rv;
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

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

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
    uint8 i, unit;
    pbmp_t hw_lpbmp;
    uint8  lport;
    BOOL   link;
    uint16 uport;

    if (lag_enable != enable) {
        for(i = 1 ; i <= BOARD_MAX_NUM_OF_LAG ; i++) {
            /* add lag setting to HW */
            if (enable == TRUE) {
                hw_lpbmp = lag_group[i-1].lpbmp;

                /* remove link down port from hw_pbmp */
                SAL_UPORT_ITER(uport) {
                    board_uport_to_lport(uport, &unit, &lport);
                    if (lag_group[i-1].lpbmp & (0x1 << lport)) {
                        SOC_IF_ERROR_RETURN(
                            (*soc_switch_bcm5346x.link_status)(0, lport, &link));
                        if (!link) {
                            hw_lpbmp &= ~(0x1 << lport);
                        }
                    }
                }

                SOC_IF_ERROR_RETURN(
                    bcm5346x_lag_group_set(0, (i - 1), hw_lpbmp));
            } else {
                /* remove lag setting from HW */
                SOC_IF_ERROR_RETURN(bcm5346x_lag_group_set(0, (i - 1), 0x0));
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
    pbmp_t lpbmp;
    pbmp_t hw_lpbmp;
    uint8 lport, count = 0;
    BOOL  link;

    if ((lagid == 0) || (lagid > BOARD_MAX_NUM_OF_LAG)) {
        return SYS_ERR_PARAMETER;
    }

    if (uplist != NULL) {
        SOC_IF_ERROR_RETURN(board_uplist_to_lpbmp(uplist, 0, &lpbmp));
    } else {
        return SYS_ERR_PARAMETER;
    }

    /* Check the number of trunk member.
     * - Minimum number of ports in each trunk: 2
     * - maximum number of ports in each trunk: 8
     */
    SOC_LPORT_ITER(lport) {
        if(lpbmp & (0x1 << lport)) {
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
        hw_lpbmp = lpbmp;
        /* remove link down port from hw_pbmp */
        SOC_LPORT_ITER(lport) {
            if (lpbmp & (0x1 << lport)) {
                SOC_IF_ERROR_RETURN(
                    (*soc_switch_bcm5346x.link_status)(0, lport, &link));
                if (!link) {
                    hw_lpbmp &= ~(0x1 << lport);
                }
            }
        }
    } else {
        hw_lpbmp = 0;
    }

    /* HW setting */
    SOC_IF_ERROR_RETURN(bcm5346x_lag_group_set(0, (lagid - 1), hw_lpbmp));

    /* lag SW database setting */
    lag_group[lagid-1].enable = enable;
    lag_group[lagid-1].lpbmp = lpbmp;
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
    pbmp_t hw_lpbmp;
    uint8 i;

    if (lag_enable == TRUE) {
        board_uport_to_lport(uport, &unit, &lport);
        for (i = 1 ; i <= BOARD_MAX_NUM_OF_LAG ; i++) {
            if ((lag_group[i-1].enable == TRUE) &&
                (lag_group[i-1].lpbmp & (0x1 << lport))){
                /* add lag setting to HW */
                bcm5346x_lag_group_get(0, (i - 1), &hw_lpbmp);
                if (link) {
                    hw_lpbmp |= (0x1 << lport);
                } else {
                    hw_lpbmp &= ~(0x1 << lport);
                }
                bcm5346x_lag_group_set(0, (i - 1), hw_lpbmp);
            }
        }
    }
}
#endif /* CFG_SWITCH_LAG_INCLUDED */

#ifdef CFG_SWITCH_MIRROR_INCLUDED
sys_error_t
board_mirror_to_set(uint16 uport)
{
    uint32 entry[3];
    uint8 unit, lport = 0;
    sys_error_t rv = SYS_OK;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    entry[0] = lport;
    entry[1] = 0;
    entry[2] = 0;
    
    rv = bcm5346x_mem_set(0, M_IM_MTP_INDEX(0), entry, 3);

    rv = bcm5346x_mem_set(0, M_EM_MTP_INDEX(1), entry, 2);

    return rv;
}

sys_error_t
board_mirror_to_get(uint16 *uport)
{
    uint8 lport;
    uint32 entry[2];
    sys_error_t rv;

    rv = bcm5346x_mem_get(0, M_EM_MTP_INDEX(1), entry, 2);
    if (rv != SYS_OK) {
        return rv;
    }
    lport = entry[0] & 0x03f;
    return  board_lport_to_uport(0, lport, uport);
}

sys_error_t
board_mirror_port_set(uint16 uport, uint8 enable)
{

    uint32 val;
    int i;
    uint8 unit, lport;
    uint32 entry[15];
    sys_error_t rv = SYS_OK;
    
    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    /* enable == 0 */
    if (enable == 0) {
        
        /* check MIRROR in PORT_TAB */
        bcm5346x_mem_get(0, M_PORT(lport), entry, 15);
        entry[0] &= 0xfffffff7;
        bcm5346x_mem_set(0, M_PORT(lport), entry, 15);
        
        SOC_LPORT_ITER(i) {
            rv = bcm5346x_mem_get(0, M_EMIRROR_CONTROL1(i), entry, 2);
            entry[0] &= ~(1 << lport);
            if (entry[0]) {
                enable = 1;
            }
            rv = bcm5346x_mem_set(0, M_EMIRROR_CONTROL1(i), entry, 2);
        }
        SOC_LPORT_ITER(i) {
            if (enable) {
                rv = bcm5346x_mem_get(0, M_MIRROR_CONTROL(i), &val, 1);
                val = val | 0x0001;
                rv = bcm5346x_mem_set(0, M_MIRROR_CONTROL(i), &val, 1);
            }
        }
    } else {
        /* enable != 0 */
        bcm5346x_reg_set(0, R_MIRROR_SELECT, 0x2);   
        bcm5346x_reg_set(0, R_EGR_MIRROR_SELECT, 0x2);   
        
        /* ingress enable */
        bcm5346x_mem_get(0, M_PORT(lport), entry, 15);
        /* Only set Mirror 0 */
        entry[0] |= 0x0008;
        bcm5346x_mem_set(0, M_PORT(lport), entry, 15);
        
        
        SOC_LPORT_ITER(i) {
            rv = bcm5346x_mem_get(0, M_MIRROR_CONTROL(i), entry, 1);
            /* M_ENABLE */
            entry[0] = 0x0001c9c9;
            rv = bcm5346x_mem_set(0, M_MIRROR_CONTROL(i), entry, 1);
            rv = bcm5346x_mem_get(0, M_EMIRROR_CONTROL1(i), entry, 1);
            entry[0] |= 1 << lport;
            rv = bcm5346x_mem_set(0, M_EMIRROR_CONTROL1(i), entry, 1);
        }

    }

    return rv;
}

sys_error_t
board_mirror_port_get(uint16 uport, uint8 *enable)
{
    uint32 port_entry[FP_PORT_T_SIZE];
    uint8 unit, lport = 0;
    sys_error_t rv = SYS_OK;


    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));
    /* check MIRROR in PORT_TAB */
    bcm5346x_mem_get(0, M_PORT(lport), port_entry, FP_PORT_T_SIZE);
    *enable = 0;
    if ((port_entry[0] & 0x0008) == 0x0008) {
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
    uint32 port_field_sel_entry[FP_PORT_FIELD_SEL_T_SIZE];
    uint32 tcam_entry[FP_TCAM_T_SIZE], dm_entry[FP_TCAM_T_SIZE], xy_entry[FP_TCAM_T_SIZE];
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };
    int i;

    if (type == qos_info) {
        return SYS_OK;
    }

    if (type == QT_DOT1P_PRIORITY) {
        /*
         * 802.1P QoS use entries 24~31.
         */
        for (i = 0; i <= 7; i++) {
            bcm5346x_mem_get(0, M_FP_TCAM(DOT1P_BASE_IDX + i), dm_entry, FP_TCAM_T_SIZE);
            bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

            tcam_entry[0] |= 0x3;

            bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
            bcm5346x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, FP_TCAM_T_SIZE);
            bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
								global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

        }

        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
            bcm5346x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5346X_PORT_MIN)), dm_entry, FP_TCAM_T_SIZE);
            bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

            tcam_entry[0] &= 0xfffffffc;

            bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
            bcm5346x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5346X_PORT_MIN)), xy_entry, FP_TCAM_T_SIZE);
            bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + (i - BCM5346X_PORT_MIN)),
								global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

        }


        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
            /* Set SLICE2_F3 = 3, clear source port qualifier Slice2_F1=11 */
            bcm5346x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
            
            port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE2_F1_MINBIT/32] &= FP_PORT_FIELD_SEL__SLICE2_F1_UNSET_LOWER;
            port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE2_F1_MAXBIT/32] &= 
                    (	FP_PORT_FIELD_SEL__SLICE2_F1_UNSET_HIGHER &
                    	FP_PORT_FIELD_SEL__SLICE2_F2_UNSET & 
                    	FP_PORT_FIELD_SEL__SLICE2_F3_UNSET);
									
            port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE2_F3_MINBIT/32] |= (0x3 << (FP_PORT_FIELD_SEL__SLICE2_F3_MINBIT%32) );
            
            bcm5346x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
	    }
        bcm5346x_dscp_map_enable(1);
    } else if (type == QT_PORT_BASED) {
        /*
         * Port based QoS use entries 0~23.
         * It'll be created in board_untagged_priority_set later.
         */
        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
            bcm5346x_mem_get(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5346X_PORT_MIN)), dm_entry, FP_TCAM_T_SIZE);
            bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

            tcam_entry[0] |= 0x3;

            bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
            bcm5346x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + (i - BCM5346X_PORT_MIN)), xy_entry, FP_TCAM_T_SIZE);
            bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + (i - BCM5346X_PORT_MIN)),
								global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

        }

        for (i = 0; i <= 7; i++) {
            bcm5346x_mem_get(0, M_FP_TCAM(DOT1P_BASE_IDX + i), dm_entry, FP_TCAM_T_SIZE);
            bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

            tcam_entry[0] &= 0xfffffffc;

            bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
            bcm5346x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, FP_TCAM_T_SIZE);
            bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
								global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

        }

        for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
            bcm5346x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
            
            /* Set SLICE2_F3 = 11(0xb), clear VLAN qualifier F3=3*/
            port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE2_F1_MINBIT/32] &= FP_PORT_FIELD_SEL__SLICE2_F1_UNSET_LOWER;
            port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE2_F1_MAXBIT/32] &= 
            		(	FP_PORT_FIELD_SEL__SLICE2_F1_UNSET_HIGHER &
									FP_PORT_FIELD_SEL__SLICE2_F2_UNSET & 
									FP_PORT_FIELD_SEL__SLICE2_F3_UNSET);
									
						port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE2_F3_MINBIT/32] |= (0xb << (FP_PORT_FIELD_SEL__SLICE2_F3_MINBIT%32) );
						
	        bcm5346x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
	    }

        /* disable dscp while Qos in port_based */
        bcm5346x_dscp_map_enable(0);
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
#ifdef CFG_SWITCH_LAG_INCLUDED
    /* We use real lagid(1~4)(the same as lagid on web page) instead of (0~3) to set the HW.
    *   lag_pbmp[0] is not not used so we use lag_pbmp[1]~lag_pbmp[BOARD_MAX_NUM_OF_LAG+1]
    */
    uint32 lag_pbmp[BOARD_MAX_NUM_OF_LAG+1][2];
    int k;
#endif /* CFG_SWITCH_LAG_INCLUDED */
    sys_error_t rv;
    uint8 unit, lport;
    int i;
    uint32 tcam_entry[FP_TCAM_T_SIZE], dm_entry[FP_TCAM_T_SIZE], xy_entry[FP_TCAM_T_SIZE];
    uint32 policy_entry[FP_POLICY_T_SIZE] = 
			{   0x00000000, 0x00000000, 0x00000000, 0x00000000,
			    0x00000088, 0x0000a000, 0x0a000005, 0x00000000,
			    0x00000000 };
	
    uint32 tcam_entry_port_based_qos[15] = 
    		{	0x00000003, 0x00000000, 0x00000000, 0x00000000, 
				0x00000000, 0x00000000, 0x00000000, 0x00000000, 
				0x0023ffc0, 0x00000000, 0x00000000, 0x00000000, 
				0x00000000, 0x00000000, 0x00000000 };

    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };
    uint32 priority_val = priority;
    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    /* While enable QoS priority in FP, need to disable "1p priority higher than DSCP,
       using set invalid bit in FP_TCAM */
    for (i = 0; i < 8; i++) {
        bcm5346x_mem_get(0, M_FP_TCAM(DOT1P_BASE_IDX + i), dm_entry, FP_TCAM_T_SIZE);
        bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        tcam_entry[0] &= 0xfffffffc;

        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
        bcm5346x_mem_set(0, M_FP_TCAM(DOT1P_BASE_IDX + i), xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(DOT1P_BASE_IDX + i),
						global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

    }
               
    priority_val &= 0x7;
    policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT/32] &= 
    		(FP_POLICY_TABLE__R_COS_INT_PRI_UNSET_LOWER);		
    policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MAXBIT/32] &= 
    		(	FP_POLICY_TABLE__R_COS_INT_PRI_UNSET_HIGHER & 
    			FP_POLICY_TABLE__Y_COS_INT_PRI_UNSET & 
    			FP_POLICY_TABLE__G_COS_INT_PRI_UNSET);
    			
    policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT/32] |= 
            (priority_val << FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT);
    policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MAXBIT/32] |= 
			(   (priority_val >> (32-FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT) ) | 
			    (priority_val << (FP_POLICY_TABLE__Y_COS_INT_PRI_MINBIT-32) ) | 
			    (priority_val << (FP_POLICY_TABLE__G_COS_INT_PRI_MINBIT-32) ) );
                                   
    rv = bcm5346x_mem_set(0, M_FP_POLICY_TABLE(QOS_BASE_IDX +
            (lport - BCM5346X_PORT_MIN)), policy_entry, FP_POLICY_T_SIZE);

    
    /* Using FP slice 2, entry 0~ for port based qos */
    tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_LOWER;
    tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] |= (lport << (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) );
    
    tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_HIGHER;
    tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= (lport >> (32 - (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32)) );

#ifdef CFG_SWITCH_LAG_INCLUDED
    for (i = 1; i < (BOARD_MAX_NUM_OF_LAG+1); i++) {
        lag_pbmp[i][0] = 0;
        lag_pbmp[i][1] = 0;
        bcm5346x_mem_get(0, M_TRUNK_BITMAP(i), lag_pbmp[i], 2);
    }
    
    /*  Revise the source tgid qualify if the port is trunk port */
    for (k = 1; k < (BOARD_MAX_NUM_OF_LAG+1); k++) {
		if (lag_pbmp[k][0] & (1 << (lport))) {
            tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_LOWER;
            tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] &= IFP_SINGLE_WIDE_F3_11__SGLP_UNSET_HIGHER;	    	
            tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT/32] |= (k << (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) );
            tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= (k >> (32 - (IFP_SINGLE_WIDE_F3_11__SGLP_MINBIT%32) ) );
            tcam_entry_port_based_qos[IFP_SINGLE_WIDE_F3_11__SGLP_MAXBIT/32] |= 0x200;
            
            tcam_entry_port_based_qos[8] = 0x00203fc0;     //update the SGLP mask for trunk
        }
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */


    bcm5346x_dm_to_xy(tcam_entry_port_based_qos, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
    bcm5346x_mem_set(0, M_FP_TCAM(QOS_BASE_IDX + (lport - BCM5346X_PORT_MIN)),
                                                xy_entry, FP_TCAM_T_SIZE);
    bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(QOS_BASE_IDX + (lport - BCM5346X_PORT_MIN)),
                                                global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);
    return rv;
}

sys_error_t
board_untagged_priority_get(uint16 uport, uint8 *priority)
{
    sys_error_t rv;
    uint8 unit, lport;
    uint32 policy_entry[FP_POLICY_T_SIZE];

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    rv = bcm5346x_mem_get(0, M_FP_POLICY_TABLE(QOS_BASE_IDX
                        + (lport - BCM5346X_PORT_MIN)), policy_entry, FP_POLICY_T_SIZE);
		// R_COS_INT_PRI field located in two words, but we only use low bytes.
    *priority = (policy_entry[FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT/32] >> FP_POLICY_TABLE__R_COS_INT_PRI_MINBIT) & 0x7;

    return rv;
}
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_RATE_INCLUDED

sys_error_t
board_port_rate_ingress_set(uint16 uport, uint32 bits_sec)
{
    sys_error_t rv = 0;
    uint8 unit, lport;
    uint32 fp_meter_table[FP_METER_T_SIZE];
    uint32 kbits_sec = bits_sec/1000;
    uint32 tcam_entry[FP_TCAM_T_SIZE], dm_entry[FP_TCAM_T_SIZE], xy_entry[FP_TCAM_T_SIZE];
    uint32 policy_entry[FP_POLICY_T_SIZE];
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    if (0 == bits_sec) {
        /* no limit */
        bcm5346x_mem_get(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                            dm_entry, FP_TCAM_T_SIZE);
        bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        tcam_entry[0] &= 0xfffffffc;

        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
        bcm5346x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                            xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

    } else {
        bcm5346x_mem_get(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                            dm_entry, FP_TCAM_T_SIZE);
        bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

        tcam_entry[0] |= 0x3;

        bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
        bcm5346x_mem_set(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                            xy_entry, FP_TCAM_T_SIZE);
        bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

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

    bcm5346x_mem_get(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                                + lport - BCM5346X_PORT_MIN), policy_entry, FP_POLICY_T_SIZE);                                
    bcm5346x_mem_set(0, M_FP_METER_TABLE(2 * ((policy_entry[FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT/32] >> (FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT%32)) & 0x3ff)
                         + ((policy_entry[FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT/32] >> (FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT%32) ) & 0x1)), 
                         fp_meter_table, FP_METER_T_SIZE);

    return rv;
}

sys_error_t
board_port_rate_ingress_get(uint16 uport, uint32 *bits_sec)
{
    uint8 unit, lport;
    uint32 fp_meter_table[FP_METER_T_SIZE];
    uint32 tcam_entry[FP_TCAM_T_SIZE];
    uint32 policy_entry[FP_POLICY_T_SIZE], dm_entry[FP_TCAM_T_SIZE];

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    bcm5346x_mem_get(0, M_FP_TCAM(RATE_IGR_IDX + lport - BCM5346X_PORT_MIN),
                                                                dm_entry, FP_TCAM_T_SIZE);
    bcm5346x_xy_to_dm(dm_entry, tcam_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

    if ((tcam_entry[0] & 0x3) == 0)
    {
        *bits_sec = 0;
        return SYS_OK;
    }

    SOC_IF_ERROR_RETURN(
        bcm5346x_mem_get(0, M_FP_POLICY_TABLE(RATE_IGR_IDX
                         + lport - BCM5346X_PORT_MIN), policy_entry, FP_POLICY_T_SIZE));

    SOC_IF_ERROR_RETURN(
        bcm5346x_mem_get(0, M_FP_METER_TABLE(2 * ((policy_entry[FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT/32] >> (FP_POLICY_TABLE__METER_PAIR_INDEX_MINBIT%32)) & 0x3ff)
                         + ((policy_entry[FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT/32] >> (FP_POLICY_TABLE__METER_PAIR_MODE_MODIFIER_BIT%32) ) & 0x1)), 
                         fp_meter_table, FP_METER_T_SIZE) );
                         
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

    return SYS_OK;
}

sys_error_t
board_port_rate_egress_set(uint16 uport, uint32 bits_sec)
{
    uint32 val;
    sys_error_t rv;
    uint8 unit, lport;
    uint32 kbits_sec = bits_sec/1000;
    uint32 port_shaper_config= 0x0;
    uint32 port_shaper_bucket= 0x00400000;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

		/* Shaper update  */
		rv = bcm5346x_mem_set(0, M_LLS_PORT_SHAPER_CONFIG_C(lport), &port_shaper_config, 1);
		if (rv < 0) {
        return rv;
    }
		rv = bcm5346x_mem_set(0, M_LLS_PORT_SHAPER_BUCKET_C(lport), &port_shaper_bucket, 1);
		if (rv < 0) {
        return rv;
    }
    
    switch(kbits_sec) {
        case 512:
            val = 0x13e94100;
            break;
        case 1024:
            val = 0x15e98200;
            break;
        case 2048:
            val = 0x17e9c400;
            break;
        case 4096:
            val = 0x19e9c800;
            break;
        case 8192:
            val = 0x1be9cc00;
            break;
        case 16384:
            val = 0x1de99000;
            break;
        case 32768:
            val = 0x1dfd5400;
            break;
        case 65536:
            val = 0x1dfd1800;
            break;
        case 131072:
            val = 0x1dfcdc00;
            break;
        case 262144:
            val = 0x1dfca000;
            break;
        case 524288:
            val = 0x1dfc6400;
            break;
       default:
            val = 0;
            break;
    }
    rv = bcm5346x_mem_set(0, M_LLS_PORT_SHAPER_CONFIG_C(lport), &val, 1);
		if (rv < 0) {
        return rv;
    }

    return SYS_OK;
}
sys_error_t
board_port_rate_egress_get(uint16 uport, uint32 *bits_sec)
{
    uint32 val;
    sys_error_t rv;
    uint8 unit, lport;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));
		
		rv = bcm5346x_mem_get(0, M_LLS_PORT_SHAPER_CONFIG_C(lport), &val, 1);
		if (rv < 0) {
        return rv;
    }
    switch(val) {
        case 0x13e94100:
            *bits_sec = 512 * 1000;
            break;
        case 0x15e98200:
            *bits_sec = 1024 * 1000;
            break;
        case 0x17e9c400:
            *bits_sec = 2048 * 1000;
            break;
        case 0x19e9c800:
            *bits_sec = 4096 * 1000;
            break;
        case 0x1be9cc00:
            *bits_sec = 8192 * 1000;
            break;
        case 0x1de99000:
            *bits_sec = 16384 * 1000;
            break;
        case 0x1dfd5400:
            *bits_sec = 32768 * 1000;
            break;
        case 0x1dfd1800:
            *bits_sec = 65536 * 1000;
            break;
        case 0x1dfcdc00:
            *bits_sec = 131072 * 1000;
            break;
        case 0x1dfca000:
            *bits_sec = 262144 * 1000;
            break;
        case 0x1dfc6400:
            *bits_sec = 524288 * 1000;
            break;
       default:
            *bits_sec = 0;
            break;
    }

    return SYS_OK;
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
    sys_error_t rv;
    uint8 unit, lport;
    uint32 entry[2];
    uint32 kbits_sec = bits_sec/1000;
    int i;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));


    if (!bits_sec) {
        /* metering is disabled on port - program tables to max values */
        /* Need to set bit-mode */
        rv = bcm5346x_reg_set(0, R_STORM_CONTROL_METER_CONFIG(lport), (0x1 << 12));

        entry[0] = 0;
        entry[1] = 0x7FFFF <<1;
        for (i = 0; i < 4; i++) {
            rv = bcm5346x_mem_set(0, M_FP_STORM_CONTROL_METERS((lport*4) + i), entry, 2);
        }
        /* no limit */
        return SYS_OK;
    }

    switch(kbits_sec) {
        case 512:
            /* 512K */
            entry[0] = 0x80100000;
            entry[1] = 0x00200008;
            break;
        case 1024:
            entry[0] = 0x80100000;
            entry[1] = 0x00200010;
            break;
        case 2048:
            entry[0] = 0xc0100000;
            entry[1] = 0x00200020;
            break;
        case 4096:
            entry[0] = 0xc0400000;
            entry[1] = 0x0c400040;
            break;
        case 8192:
            entry[0] = 0x01000000;
            entry[1] = 0x0c400081;
            break;
        case 16384:
            entry[0] = 0x01000000;
            entry[1] = 0x0c400101;
            break;
        case 32768:
            entry[0] = 0x44000000;
            entry[1] = 0x0c400201;
            break;
        case 65536:
            entry[0] = 0x44000000;
            entry[1] = 0x0c400401;
            break;
        case 131072:
            entry[0] = 0x88000000;
            entry[1] = 0x0c400801;
            break;
        case 262144:
            entry[0] = 0xd0000000;
            entry[1] = 0x0c401001;
            break;
        case 524288:
            entry[0] = 0xd0000000;
            entry[1] = 0x0c402001;
            break;
       default:
            entry[0] = 0;
            entry[1] = 0;
            break;
    }

    /* enable bcast in STORM_CONTROL_METER_CONFIG*/
    rv = bcm5346x_reg_set(0, R_STORM_CONTROL_METER_CONFIG(lport), (0x7f << 12));
    for (i = 0; i < 4; i++) {
        rv = bcm5346x_mem_set(0, M_FP_STORM_CONTROL_METERS((lport * 4) + i), entry, 2);
    }

    return rv;
}

sys_error_t
board_rate_get(uint16 uport, uint32 *bits_sec)
{
    sys_error_t rv;
    uint8 unit, lport;
    uint32 val;
    uint32 entry[2];

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    rv = bcm5346x_reg_get(0, R_STORM_CONTROL_METER_CONFIG(lport), &val);
    if (!(val & 0x7e000)) {
    	/*disable, no limit */
    	*bits_sec = 0;
    	return rv;
    }

    rv = bcm5346x_mem_get(0, M_FP_STORM_CONTROL_METERS(lport * 4), entry, 2);
    *bits_sec = ((entry[1] & 0xffffe) << 6) * 1000;
    return rv;
}
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_STAT_INCLUDED
sys_error_t
board_port_stat_get(uint16 uport, port_stat_t *stat)
{
    sys_error_t rv = SYS_OK;
    uint8 unit, lport;
    uint32 entry[2];

    board_uport_to_lport(uport, &unit, &lport);

    sal_memset(stat, 0, sizeof(port_stat_t));

    if (IS_XL_PORT(lport)) {
        /* Byte Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxOctets_lo = entry[0];
        stat->TxOctets_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxOctets_lo = entry[0];
        stat->RxOctets_hi = entry[1];
        /* Frame counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxPkts_lo = entry[0];
        stat->TxPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxPkts_lo = entry[0];
        stat->RxPkts_hi = entry[1];
        /* Rx FCS Error Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->CRCErrors_lo = entry[0];
        stat->CRCErrors_hi = entry[1];
        /* EEE LPI counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxLPIPkts_lo = entry[0];
        stat->RxLPIPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxLPIDuration_lo = entry[0];
        stat->RxLPIDuration_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxLPIPkts_lo = entry[0];
        stat->TxLPIPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxLPIDuration_lo = entry[0];
        stat->TxLPIDuration_hi = entry[1];
        /* Unicast Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxUnicastPkts_lo = entry[0];
        stat->RxUnicastPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxUnicastPkts_lo = entry[0];
        stat->TxUnicastPkts_hi = entry[1];
        /* Multicast Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxMulticastPkts_lo = entry[0];
        stat->RxMulticastPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxMulticastPkts_lo = entry[0];
        stat->TxMulticastPkts_hi = entry[1];
        /* Broadcast Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxBroadcastPkts_lo = entry[0];
        stat->RxBroadcastPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxBroadcastPkts_lo = entry[0];
        stat->TxBroadcastPkts_hi = entry[1];
        /* Pause Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_RXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxPauseFramePkts_lo = entry[0];
        stat->RxPauseFramePkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxPauseFramePkts_lo = entry[0];
        stat->TxPauseFramePkts_hi = entry[1];
        /* Oversized Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxOversizePkts_lo = entry[0];
        stat->RxOversizePkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxOversizePkts_lo = entry[0];
        stat->TxOversizePkts_hi = entry[1];
    } else {
        /* Byte Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxOctets_lo = entry[0];
        stat->TxOctets_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxOctets_lo = entry[0];
        stat->RxOctets_hi = entry[1];
        /* Frame counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxPkts_lo = entry[0];
        stat->TxPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxPkts_lo = entry[0];
        stat->RxPkts_hi = entry[1];
        /* Rx FCS Error Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->CRCErrors_lo = entry[0];
        stat->CRCErrors_hi = entry[1];
        /* EEE LPI counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxLPIPkts_lo = entry[0];
        stat->RxLPIPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxLPIDuration_lo = entry[0];
        stat->RxLPIDuration_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxLPIPkts_lo = entry[0];
        stat->TxLPIPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxLPIDuration_lo = entry[0];
        stat->TxLPIDuration_hi = entry[1];
        /* Unicast Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxUnicastPkts_lo = entry[0];
        stat->RxUnicastPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxUnicastPkts_lo = entry[0];
        stat->TxUnicastPkts_hi = entry[1];
        /* Multicast Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxMulticastPkts_lo = entry[0];
        stat->RxMulticastPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxMulticastPkts_lo = entry[0];
        stat->TxMulticastPkts_hi = entry[1];
        /* Broadcast Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxBroadcastPkts_lo = entry[0];
        stat->RxBroadcastPkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxBroadcastPkts_lo = entry[0];
        stat->TxBroadcastPkts_hi = entry[1];
        /* Pause Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_RXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxPauseFramePkts_lo = entry[0];
        stat->RxPauseFramePkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TXPF(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxPauseFramePkts_lo = entry[0];
        stat->TxPauseFramePkts_hi = entry[1];
        /* Oversized Frame Counter */
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->RxOversizePkts_lo = entry[0];
        stat->RxOversizePkts_hi = entry[1];
        rv |= bcm5346x_reg64_get(0, SOC_PORT_BLOCK(lport), R_MXQ_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        stat->TxOversizePkts_lo = entry[0];
        stat->TxOversizePkts_hi = entry[1];
    }
       
    

    return rv;
}

sys_error_t
board_port_stat_clear(uint16 uport)
{
    sys_error_t rv = SYS_OK;
    uint32 entry[2] = {0, 0};
    uint8 unit, lport;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    if (IS_XL_PORT(lport)) {    
        /* Byte Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Frame counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Rx FCS Error Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* EEE LPI counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Unicast Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Multicast Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Broadcast Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Pause Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Oversized Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);   
    } else {
        /* Byte Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RBYT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Frame counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RPKT(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Rx FCS Error Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RFCS(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* EEE LPI counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TX_EEE_LPI_EVENT_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TX_EEE_LPI_DURATION_COUNTER(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Unicast Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TUCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Multicast Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TMCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Broadcast Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Pause Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_RBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TBCA(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        /* Oversized Frame Counter */
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_ROVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
        rv |= bcm5346x_reg64_set(0, SOC_PORT_BLOCK(lport), R_MXQ_TOVR(SOC_PORT_BLOCK_INDEX(lport)), entry, 2);
    }

    

    return rv;
}

sys_error_t
board_port_stat_clear_all(void)
{
    uint16 uport;
    sys_error_t rv = SYS_OK;

    SAL_UPORT_ITER(uport) {
        rv |= board_port_stat_clear(uport);
    }
    return rv;
}
#endif /* CFG_SWITCH_STAT_INCLUDED */

#ifdef CFG_SWITCH_MCAST_INCLUDED
sys_error_t
board_mcast_addr_add(uint8 *mac_addr, uint16 vlan_id, uint8 *uplist)
{
    int rv = SYS_OK;
    int i,j;
    uint32 entry[2];
    pbmp_t lpbmp;
    mcast_list_t *mcast = NULL;
    l2x_entry_t l2x;

    if (uplist != NULL) {
        SOC_IF_ERROR_RETURN(board_uplist_to_lpbmp(uplist, 0, &lpbmp));
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
    rv = bcm5346x_l2_op(0, &l2x, SC_OP_L2_INS_CMD);

    /* after get mcindex from l2_entry */
    bcm5346x_mem_get(0, M_L2MC(mcast->index), entry, 2);
    entry[0] &= 0x00000000;
    entry[0] |= (lpbmp);
    entry[1] |= (0x1 << 2);
    bcm5346x_mem_set(0, M_L2MC(mcast->index), entry, 2);

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
    rv = bcm5346x_l2_op(0, &l2x, SC_OP_L2_DEL_CMD);

    /* after get mcindex from l2_entry */
    bcm5346x_mem_get(0, M_L2MC(l2_index), entry, 2);
    entry[0] &= 0x00000000;  /* clear valid bit and pbmp */
    entry[1] &= ~(1<<2);
    bcm5346x_mem_set(0, M_L2MC(l2_index), entry, 2);



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

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));
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
    bcm5346x_mem_get(0, M_L2MC(mcast->index), entry, 2);
    entry[0] |= (1 << (lport));    
    bcm5346x_mem_set(0, M_L2MC(mcast->index), entry, 2);


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

    /* check if exists */
    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));
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
    bcm5346x_mem_get(0, M_L2MC(mcast->index), entry, 2);
    entry[0] &= ~(1 << (lport));    
    bcm5346x_mem_set(0, M_L2MC(mcast->index), entry, 2);

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
    rv = bcm5346x_reg_get(0, R_IGMP_MLD_PKT_CONTROL(0), &val);
    if (enable == 1) {
        /* set bit 26-18 = 011011011 */
        val = (val & 0xf803ffff) | 0x036c0000;
    } else {
        /* set bit 26-18 = 100100100 */
        val = (val & 0xf803ffff) | 0x04900000;
    }
    rv = bcm5346x_reg_set(0, R_IGMP_MLD_PKT_CONTROL(0), val);
    rv = bcm5346x_reg_get(0, R_IGMP_MLD_PKT_CONTROL(0), &val);

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
    rv = bcm5346x_reg_get(0, R_IGMP_MLD_PKT_CONTROL(0), &val);
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
    uint32 entry[2];
    int i;
    int rv;

    rv = SYS_OK;

    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        rv = bcm5346x_mem_get(0, M_UNKNOWN_MCAST_BLOCK_MASK(i), entry, 2);
        if (enable == 1) {
            entry[0] |= 0xffffffff; /* block all */
        } else {
            entry[0] &= 0x0;
        }
        rv = bcm5346x_mem_set(0, M_UNKNOWN_MCAST_BLOCK_MASK(i), entry, 2);
    }
    return rv;
}

sys_error_t
board_block_unknown_mcast_get(uint8 *enable)
{
    uint32 entry[2];
    int rv;

    rv = SYS_OK;

    /* select port 1 to check */
    *enable = 0;
    rv = bcm5346x_mem_get(0, M_UNKNOWN_MCAST_BLOCK_MASK(BCM5346X_LPORT_MIN), entry, 2);
    if (entry[0] & 0xffffffff) {
        *enable = 1;
    }

    return rv;
}
#endif /* CFG_SWITCH_MCAST_INCLUDED */

#ifdef CFG_SWITCH_LOOPDETECT_INCLUDED
void
board_loop_detect_enable(BOOL enable)
{
    bcm5346x_loop_detect_enable(enable);
}

uint8
board_loop_detect_status_get(void)
{
    return bcm5346x_loop_detect_status_get();
}
#endif /* CFG_SWITCH_LOOPDETECT_INCLUDED */

sys_error_t
board_port_loopback_enable_set(uint16 uport, int loopback_mode)
{
    uint8 unit, lport;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    bcm5346x_loopback_enable(unit, lport, loopback_mode);

    return SYS_OK;
}

sys_error_t
board_port_enable_get(uint16 uport, BOOL *enable)
{
    uint8 unit, lport;
    sys_error_t r;
    uint32 en;
#if CONFIG_METROLITE_EMULATION
    *enable = 1;
    return SYS_OK;
#endif
    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    r = PHY_CONFIG_GET(BMD_PORT_PHY_CTRL(unit, lport),
                       PhyConfig_Enable, &en, NULL);

    *enable = en;

    return r;
}

sys_error_t
board_port_enable_set(uint16 uport, BOOL enable)
{
    uint8 unit;
    uint8 lport;
    sys_error_t r;
    uint16 phy_reg;
    int link;
#if CONFIG_METROLITE_EMULATION
	return SYS_OK;
#endif

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    if (enable) {
        phy_reg_read(lport, 0x1, &phy_reg);
        link = 0;
        if (phy_reg & (1 << 2)) {
            link = 1;
        }
        r = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport),
                            PhyConfig_Enable, 1, NULL);
        if (link) {
            MAC_ENABLE_SET(ml_sw_info.p_mac[lport], unit, lport, enable);
        }
    } else {
        MAC_ENABLE_SET(ml_sw_info.p_mac[lport], unit, lport, enable);
        r = PHY_CONFIG_SET(BMD_PORT_PHY_CTRL(unit, lport),
                                             PhyConfig_Enable, 0, NULL);
    }

    return r;
}


#ifdef CFG_ZEROCONF_MDNS_INCLUDED
sys_error_t
board_mdns_enable_set(BOOL enable)
{
    return bcm5346x_mdns_enable_set(0, enable);
}
sys_error_t
board_mdns_enable_get(BOOL *enable)
{
    return bcm5346x_mdns_enable_get(0, enable);
}
#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

sys_error_t
board_port_pause_get(uint16 uport, BOOL *tx, BOOL *rx)
{
    uint8 unit, lport;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    *tx = SOC_PORT_TX_PAUSE_STATUS(lport);
    *rx = SOC_PORT_RX_PAUSE_STATUS(lport);

    return SYS_OK;

}

sys_error_t
board_port_an_get(uint16 uport, BOOL *an)
{
    uint8 unit, lport;

    SOC_IF_ERROR_RETURN(board_uport_to_lport(uport, &unit, &lport));

    *an = SOC_PORT_AN_STATUS(lport);

    return SYS_OK;
}
#if defined(CFG_SWITCH_SNAKETEST_INCLUDED) && defined(CFG_SWITCH_VLAN_INCLUDED)
sys_error_t
board_snaketest_trap_to_cpu(uint16 vlanid, uint8 *uplist) {
    pbmp_t      lpbmp, tag_lpbmp;

    board_uplist_to_lpbmp(uplist, 0, &lpbmp);
    lpbmp |= 0x1;
    tag_lpbmp = 0x1;

    return bcm5346x_qvlan_port_set(0, vlanid, lpbmp, tag_lpbmp);
}
#endif /* CFG_SWITCH_SNAKETEST_INCLUDED && CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_RESET_BUTTON_INCLUDED
uint8 sw_simulate_press_reset_button_duration = 0;
uint8 reset_button_active_high = 1;
uint8 reset_button_gpio_bit = 4;

BOOL
board_reset_button_get(void)
{
    uint32 val;
    /*
     * GPIO 0 ~ 7 : from IPROC 0 ~ 7
     */
    val = READCSR(R_CHIPCOMMONG_GP_DATA_IN) & 0x00ff;

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

