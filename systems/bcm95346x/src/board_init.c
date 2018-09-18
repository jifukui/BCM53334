/*
 * $Id: board_init.c,v 1.16 Broadcom SDK $
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
#include "brdimpl.h"
#include "soc/bcm5346x.h"
#include "bsp_config.h"
#include "brdimpl.h"
#include "utils/nvram.h"
#include "utils/system.h"
#include "ns16550.h"
/* __BOOT_UART_TRAP: For debug only !!! please define it as zero at final product
                                     In 50ms since power on,  If there is any character comes from uart,
                                     system will not perform normal booting flow and stay at bootcode recovery stage. */
#ifdef __BOOTLOADER__
#define __BOOTLOADER_UART_TRAP__ 0
#endif

#if CFG_FLASH_SUPPORT_ENABLED
/* Flash device driver. */
extern flash_dev_t n25q256_dev;
#endif /* CFG_FLASH_SUPPORT_ENABLED */

#if !CFG_TIMER_USE_INTERRUPT
extern void sal_timer_task(void *param);
#endif /* !CFG_TIMER_USE_INTERRUPT */

extern void enable_arm_cyclecount(void);

#if CFG_CONSOLE_ENABLED
static const char *um_fwname = target;
#endif /* CFG_CONSOLE_ENABLED */

#ifdef CFG_RESET_BUTTON_INCLUDED
uint8 reset_button_enable = 0;
#endif /* CFG_RESET_BUTTON_INCLUDED */

/* Access to shadowed registers at offset 0x1c */
#define REG_1C_SEL(_s)                  ((_s) << 10)
#define REG_1C_WR(_s,_v)                (REG_1C_SEL(_s) | (_v) | 0x8000)

/* Access expansion registers at offset 0x15 */
#define MII_EXP_MAP_REG(_r)             ((_r) | 0x0f00)
#define MII_EXP_UNMAP                   (0)

/*
 * Non-standard MII Registers
 */
#define MII_ECR_REG             0x10 /* MII Extended Control Register */
#define MII_EXP_REG             0x15 /* MII Expansion registers */
#define MII_EXP_SEL             0x17 /* MII Expansion register select */
#define MII_TEST1_REG           0x1e /* MII Test Register 1 */
#define RDB_LED_MATRIX          0x1f /* LED matrix mode */

int board_phy_init_callback(phy_ctrl_t *pc) {
    return SYS_OK;
}

#if CFG_CONSOLE_ENABLED
#define UART_READREG(r)    SYS_REG_READ32((CFG_UART_BASE+(r)))
#define UART_WRITEREG(r,v) SYS_REG_WRITE32((CFG_UART_BASE+(r)), v)

void
board_console_init(uint32 baudrate, uint32 clk_hz)
{
    uint32 brtc;
    brtc = BRTC(clk_hz, baudrate);

    UART_WRITEREG(R_UART_IER, 0x0);

    UART_WRITEREG(R_UART_CFCR, CFCR_DLAB | CFCR_8BITS);
    UART_WRITEREG(R_UART_DATA, 0x0);
    UART_WRITEREG(R_UART_IER, 0x0);
    UART_WRITEREG(R_UART_CFCR, CFCR_8BITS);

    UART_WRITEREG(R_UART_MCR, MCR_DTR | MCR_RTS);

    UART_WRITEREG(R_UART_FIFO, FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST);

    UART_WRITEREG(R_UART_CFCR, CFCR_DLAB | CFCR_8BITS);
    UART_WRITEREG(R_UART_DATA, brtc & 0xFF);
    UART_WRITEREG(R_UART_IER, brtc >> 8);
    UART_WRITEREG(R_UART_CFCR, CFCR_8BITS);
}
#endif /* CFG_CONSOLE_ENABLED */

/* Function:
 *   board_early_init
 * Description:
 *   Perform initialization of on-board devices that are required for SAL.
 *   This will be called from main startup task.
 * Parameters:
 *   None
 * Returns:
 *   None
 */
void
board_early_init(void)
{
   void (*funcptr)(void);
#if CFG_CONSOLE_ENABLED
   /* Get chip revision */
   bcm5346x_chip_revision(0, &ml_sw_info.devid, &ml_sw_info.revid); 

   /* Initialize UART using default clock */
   board_console_init(CFG_UART_BAUDRATE, BOARD_CCA_UART_CLOCK);

#endif /* CFG_CONSOLE_ENABLED */
   /* Initialize timer using default clock */
   funcptr = (void (*)(void))enable_arm_cyclecount;
   (*funcptr)();
   sal_timer_init(BOARD_CPU_CLOCK, TRUE);

#if CFG_CONSOLE_ENABLED
    sal_printf("\nMETROLITE %s-%d.%d.%d\n", um_fwname, CFE_VER_MAJOR, CFE_VER_MINOR, CFE_VER_BUILD);
    sal_printf("Build Date: %s\n", __DATE__);
#endif /* CFG_CONSOLE_ENABLED */

#if CFG_FLASH_SUPPORT_ENABLED
   /* Flash driver init */
   flash_init(NULL);

#ifdef CFG_NVRAM_SUPPORT_INCLUDED
   nvram_init();
#endif /* CFG_NVRAM_SUPPORT_INCLUDED */

#endif /* CFG_FLASH_SUPPORT_ENABLED */

}

#if CFG_RXTX_SUPPORT_ENABLED
static void
bcm5346x_fp_init(void)
{
    int i;
    uint32 port_entry[FP_PORT_T_SIZE];
    /* 
     * Slice 0: Trap Layer 4 protocol packets.
     * SLICE0_F2 = 0 (L4_DST and L4_SRC)
     *
     * Slice 1: 
     * Ingress rate limit (SLICE1_F3 = 2)
     * Match system_mac (SLICE1_F2 = 5)
     *
     * Slice 2: 
     *  (1)Port-based QoS  (SLICE2_F3 = 2)
     *  (2)Make 1P priority have precedence over DSCP(SLICE2_F1 = 6)
     *  Note (1) and (2) are mutually exclusive.
     *
     * Slice 3: Loop detect
     * Slice3_F3 = 2, Slice3_F2 = 8, Slice3_F1 = 4;
     *
     */
    uint32 port_field_sel_entry[FP_PORT_FIELD_SEL_T_SIZE];

    uint32 slice_map_entry[2] = { 0x88fac688, 0x0000fac6 };

    
#if (CFG_UIP_STACK_ENABLED)
    uint32 tcam_entry[FP_TCAM_T_SIZE] = 
    		{ 0x00000003, 0x00000000, 0x00000000, 0x00000000,
          0x00000000, 0x00000000, 0x00000000, 0x00000000,
          0x00000000, 0x00000000, 0x00000000, 0x00000000,
          0x00000000, 0x00000000, 0x00000000};
          
    uint32 xy_entry[FP_TCAM_T_SIZE];
    
    uint32 global_tcam_mask_entry[FP_GLOBAL_TCAM_MASK_T_SIZE] = { 0x1, 0, 0, 0 };
    /* 
     * FP_POLICY :
     * COPY_TO_CPU is [133:132]
     * YP_COPY_TO_CPU is [106:105]
     */
    uint32 policy_entry[FP_POLICY_T_SIZE] = 
    		{	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	        0x00000088, 0x00400011, 0x00080000, 0x00000000,
	        0x00000000 };

    uint32 redirection_entry[2] = {BCM5346X_ALL_PORTS_MASK, 0x0};    
	
#endif /* CFG_UIP_STACK_ENABLED */

#if CFG_UIP_STACK_ENABLED
    uint8 mac_addr[6];
#endif /* CFG_UIP_STACK_ENABLED */

    /* Enable FILTER_ENABLE[bit 0] for all ports, include CPU. */
    for (i = 0; i <= BCM5346X_LPORT_MAX; i++) {
    	/*
        if (1 == i) {
            continue;
        }
      */
        bcm5346x_mem_get(0, M_PORT(i), port_entry, FP_PORT_T_SIZE);

        port_entry[PORT_TABLE__FILTER_ENABLE_BIT/32] |= 
        		(	1 << (PORT_TABLE__FILTER_ENABLE_BIT % 32) );

				port_entry[PORT_TABLE__FP_PORT_FIELD_SEL_INDEX/32] &= PORT_TABLE__FP_PORT_FIELD_SEL_INDEX_UNSET;
				port_entry[PORT_TABLE__FP_PORT_FIELD_SEL_INDEX/32] |= 
        		(	i << (PORT_TABLE__FP_PORT_FIELD_SEL_INDEX % 32) );
   		    
				port_entry[PORT_TABLE__STORM_CONTROL_PTR/32] &= PORT_TABLE__STORM_CONTROL_PTR_UNSET;
				port_entry[PORT_TABLE__STORM_CONTROL_PTR/32] |= 
        		(	i << (PORT_TABLE__STORM_CONTROL_PTR % 32) );

				port_entry[PORT_TABLE__TRUST_DSCP_PTR/32] &= PORT_TABLE__TRUST_DSCP_PTR_UNSET;
				port_entry[PORT_TABLE__TRUST_DSCP_PTR/32] |= 
        		(	i << (PORT_TABLE__TRUST_DSCP_PTR % 32) );
        bcm5346x_mem_set(0, M_PORT(i), port_entry, FP_PORT_T_SIZE);
    }

    bcm5346x_reg_set(0, R_FP_SLICE_ENABLE, 0xffff);

    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        bcm5346x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
        
        /* Slice 0: F2 = 0 L4DstPort; F3=11 SrcPort */
        port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE0_F1_MINBIT/32] &= 
        		(	FP_PORT_FIELD_SEL__SLICE0_F2_UNSET &
							FP_PORT_FIELD_SEL__SLICE0_F3_UNSET);
        port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE0_F1_MINBIT/32] |= 
        		(	0xb << FP_PORT_FIELD_SEL__SLICE0_F3_MINBIT);
        		
        bcm5346x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
    }

    bcm5346x_mem_set(0, M_FP_SLICE_MAP, slice_map_entry, 2);
    
#if CFG_UIP_STACK_ENABLED
    /* Reset tcam_entry */
    tcam_entry[2] = 0x0;
    tcam_entry[3] = 0x0;
    tcam_entry[10] = 0x0;
    tcam_entry[11] = 0x0;

    /* Program for DUT'S MAC  : entry 0 of slice 1 */
    get_system_mac(mac_addr);
    tcam_entry[4] = ((mac_addr[5] << 18) & 0x03fc0000) |
                    ((mac_addr[4] << 26) & 0xfc000000);
    tcam_entry[5] = ((mac_addr[4] >> 6) & 0x00000003) |
                    ((mac_addr[3] <<  2) & 0x000003fc) |
                    ((mac_addr[2] << 10) & 0x0003fc00) |
                    ((mac_addr[1] << 18) & 0x03fc0000) |
                    ((mac_addr[0] << 26) & 0xfc000000);
    tcam_entry[6] = (mac_addr[0] >> 6) & 0x00000003;
    tcam_entry[11] = 0xc0000000;
    tcam_entry[12] = 0xffffffff;
    tcam_entry[13] = 0x00003fff;

    bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));
    
    bcm5346x_mem_set(0, M_FP_TCAM(SYS_MAC_IDX), xy_entry, FP_TCAM_T_SIZE);
    bcm5346x_mem_set(0, M_FP_POLICY_TABLE(SYS_MAC_IDX), policy_entry, FP_POLICY_T_SIZE);
    bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(SYS_MAC_IDX), global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

    for (i = BCM5346X_LPORT_MIN; i <= BCM5346X_LPORT_MAX; i++) {
        bcm5346x_mem_get(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
        
        /* Slice 1: F2 = 5 DstMAC; F3=11 SrcPort */
        port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE1_F1_MINBIT/32] &= 
        		(	FP_PORT_FIELD_SEL__SLICE1_F1_UNSET 
							& FP_PORT_FIELD_SEL__SLICE1_F2_UNSET 
							& FP_PORT_FIELD_SEL__SLICE1_F3_UNSET);
				port_field_sel_entry[FP_PORT_FIELD_SEL__SLICE1_F1_MINBIT/32] |= 0x05a80000;
				
        bcm5346x_mem_set(0, M_FP_PORT_FIELD_SEL(i), port_field_sel_entry, FP_PORT_FIELD_SEL_T_SIZE);
    }

#endif /* CFG_UIP_STACK_ENABLED */

#if CFG_UIP_IPV6_ENABLED
      /* Program IPV6 all notes multicast MAC  : entry 1 of slice 1
       * FP_TCAM :
       * DA is start from bit 106
       * MASK of DA is start from bit 316
       */
     mac_addr[0] = 0x33;
     mac_addr[1] = 0x33;
     mac_addr[2] = 0x00;
     mac_addr[3] = 0x00;
     mac_addr[4] = 0x00;
     mac_addr[5] = 0x01;
    tcam_entry[4] = ((mac_addr[5] << 18) & 0x03fc0000) |
                    ((mac_addr[4] << 26) & 0xfc000000);
    tcam_entry[5] = ((mac_addr[4] >> 6) & 0x00000003) |
                    ((mac_addr[3] <<  2) & 0x000003fc) |
                    ((mac_addr[2] << 10) & 0x0003fc00) |
                    ((mac_addr[1] << 18) & 0x03fc0000) |
                    ((mac_addr[0] << 26) & 0xfc000000);
    tcam_entry[6] = (mac_addr[0] >> 6) & 0x00000003;

     bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

     bcm5346x_mem_set(0, M_FP_TCAM(SYS_MAC_IDX+1), xy_entry, FP_TCAM_T_SIZE);
     bcm5346x_mem_set(0, M_FP_POLICY_TABLE(SYS_MAC_IDX+1), policy_entry, FP_POLICY_T_SIZE);
     bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(SYS_MAC_IDX+1), global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);

     /* Program for IPV6 multicast MAC  : entry 2 of slice 1
      * FP_TCAM :
      * DA is start from bit 106
      * MASK of DA is start from bit 316
      */
    get_system_mac(mac_addr);
    mac_addr[0] = 0x33;
    mac_addr[1] = 0x33;
    mac_addr[2] = 0xff;
    tcam_entry[4] = ((mac_addr[5] << 18) & 0x03fc0000) |
                    ((mac_addr[4] << 26) & 0xfc000000);
    tcam_entry[5] = ((mac_addr[4] >> 6) & 0x00000003) |
                    ((mac_addr[3] <<  2) & 0x000003fc) |
                    ((mac_addr[2] << 10) & 0x0003fc00) |
                    ((mac_addr[1] << 18) & 0x03fc0000) |
                    ((mac_addr[0] << 26) & 0xfc000000);
    tcam_entry[6] = (mac_addr[0] >> 6) & 0x00000003;

    bcm5346x_dm_to_xy(tcam_entry, xy_entry, FP_TCAM_T_SIZE, (32*FP_TCAM_T_SIZE));

    bcm5346x_mem_set(0, M_FP_TCAM(SYS_MAC_IDX+2), xy_entry, FP_TCAM_T_SIZE);
    bcm5346x_mem_set(0, M_FP_POLICY_TABLE(SYS_MAC_IDX+2), policy_entry, FP_POLICY_T_SIZE);
    bcm5346x_mem_set(0, M_FP_GLOBAL_MASK_TCAM(SYS_MAC_IDX+2), global_tcam_mask_entry, FP_GLOBAL_TCAM_MASK_T_SIZE);
#endif /* CFG_UIP_IPV6_ENABLED */


#if (CFG_UIP_STACK_ENABLED)
    /* Set each redireciton entry per each port
     * No need to exclude source port nor trunk ports but due to pvlan need to use
     * cpu and each front port needs each entry.
     */
    for (i = 0; i <= BCM5346X_LPORT_MAX; i++) {
        bcm5346x_mem_set(0, M_IFP_REDIRECTION_PROFILE(i), redirection_entry, 2);
    }
#endif /* (CFG_UIP_STACK_ENABLED) */
}
#endif /* CFG_RXTX_SUPPORT_ENABLED */

/* Function:
 *   board_init
 * Description:
 *   Perform board (chip and devices) initialization.
 *   This will be called from main startup task.
 * Parameters:
 *   None
 * Returns:
 *   SYS_OK or SYS_XXX
 */
sys_error_t
board_init(void)
{
    sys_error_t rv = SYS_OK;

#ifdef __BOOTLOADER__
#if __BOOTLOADER_UART_TRAP__
        sal_usleep(500000);
        if (sal_char_avail() == FALSE)
#endif /* __BOOTLODER_UART_TRAP__ */

        {
            hsaddr_t loadaddr;
            if (board_loader_mode_get(NULL, FALSE) != LM_UPGRADE_FIRMWARE) {
#ifdef CFG_DUAL_IMAGE_INCLUDED
                /* Determine which image to boot */
                if (board_select_boot_image(&loadaddr)) {
#else
                /* Validate firmware image if not requested to upgrade firmware */
                if (board_check_image(BOARD_FIRMWARE_ADDR, &loadaddr)) {
#endif /* CFG_DUAL_IMAGE_INCLUDED */
                    /* launch firmware */
#if CFG_CONSOLE_ENABLED
                    sal_printf("Load program at 0x%08lX...\n", loadaddr);
#endif /* CFG_CONSOLE_ENABLED */
                    board_load_program(loadaddr);
                }
                /* Stay in loader in case of invalid firmware */
            }
        }

#endif /* __BOOTLOADER__ */

#if CFG_FLASH_SUPPORT_ENABLED
#if defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)
   nvram_init();
#endif /* defined(CFG_NVRAM_SUPPORT_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED) */
#endif /* CFG_FLASH_SUPPORT_ENABLED */

#if CONFIG_HURRICANE3_EMULATION
    sal_printf("EMULATION usage only!\n");
#endif

#if !CFG_TIMER_USE_INTERRUPT
    task_add(sal_timer_task, (void *)NULL);
#endif /* CFG_TIMER_USE_INTERRUPT */
    
    bmd_phy_init_cb_register(board_phy_init_callback);
    
    rv = bcm5346x_sw_init();

    if (rv != SYS_OK) {
        return rv;
    }

#ifdef CFG_SWITCH_VLAN_INCLUDED
    _brdimpl_vlan_init();
#endif /* CFG_SWITCH_VLAN_INCLUDED */

#ifdef CFG_SWITCH_QOS_INCLUDED
    bcm5346x_qos_init();
#endif /* CFG_SWITCH_QOS_INCLUDED */

#ifdef CFG_SWITCH_RATE_INCLUDED
    bcm5346x_rate_init();
#endif /* CFG_SWITCH_RATE_INCLUDED */

#ifdef CFG_SWITCH_LAG_INCLUDED
    {
        uint32 val32;

        /* Enables factoring src/dest MAC or src/dest IP into non-unicast trunk block mask hashing */
        bcm5346x_reg_get(0, R_HASH_CONTROL, &val32);
        val32 |= 0x6;
        bcm5346x_reg_set(0, R_HASH_CONTROL, val32);
        sys_register_linkchange(board_lag_linkchange, NULL);
    }
#endif /* CFG_SWITCH_LAG_INCLUDED */

#if defined(CFG_SWITCH_EEE_INCLUDED)
    {
        bcm5346x_eee_init();
    }
#endif /* CFG_SWITCH_EEE_INCLUDED */

#if CFG_RXTX_SUPPORT_ENABLED
    bcm5346x_rxtx_init();
    brdimpl_rxtx_init();
    bcm5346x_fp_init();
#endif /* CFG_RXTX_SUPPORT_ENABLED */

#ifdef CFG_RESET_BUTTON_INCLUDED
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED
    
    rv = sal_config_uint8_get(SAL_CONFIG_RESET_BUTTON_ENABLE, &reset_button_enable);
    if ((rv == SYS_OK) && reset_button_enable) {
        sal_printf("Vendor Config : Change to enable the reset buton feature by configuration.\n");
    }

    rv = sal_config_uint8_get(SAL_CONFIG_RESET_BUTTON_GPIO_BIT, &reset_button_gpio_bit);
    if (rv == SYS_OK) {
        sal_printf("Vendor Config : Change to use GPIO bit %d for reset button by configuration.\n", reset_button_gpio_bit);
    }

    rv = sal_config_uint8_get(SAL_CONFIG_RESET_BUTTON_POLARITY, &reset_button_active_high);
    if (rv == SYS_OK) {
        sal_printf("Vendor Config : Change the reset button polarity to %d\n", reset_button_active_high);
        sal_printf("                (0:active low/1:active high) by configuration.\n");
    }
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
    if (reset_button_enable) {
        /* Detect reset button per 1 second */
        timer_add(brdimpl_reset_button_detect, NULL, (1 * 1000000));
    }
#endif /* CFG_RESET_BUTTON_INCLUDED */

    return SYS_OK;
}


/* Function:
 *   board_late_init
 * Description:
 *   Perform final stage of platform dependent initialization
 *   This will be called from main startup task.
 * Parameters:
 *   None
 * Returns:
 *   None
 */
void
board_late_init(void) {
#if !(defined(__BOOTLOADER__) || CONFIG_HURRICANE3_ROMCODE || !defined(CFG_SWITCH_STAT_INCLUDED))
    board_port_stat_clear_all();
#endif
}


