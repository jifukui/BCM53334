/*
 * $Id: sal_config.h,v 1.18 Broadcom SDK $
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
#ifdef CFG_VENDOR_CONFIG_SUPPORT_INCLUDED

extern sys_error_t sal_config_init(void);
extern void sal_config_show(void);
/**
 * sal_config_get: get ascii value associated with name
 * 
 * @param name (IN)- name of config item
 * @return  address of 
 *     NULL : config get fail
 *     else config get success
 *   
 */
extern const char *sal_config_get(const char *name);

/**
 * sal_config_pbmp_get: get pbmp_t associated with name
 * 
 * @param name (IN)- name of config item
 * @param p (IN)- address of ouput port bit map 
 * @return 
 *     SYS_ERR_FALSE : config get fail
 *     SYS_OK: config get success
 *   
 */

extern sys_error_t sal_config_pbmp_get(const char *name, pbmp_t *p);

/**
 * sal_config_bytes_get: get bytes buffer associated with name
 * 
 * @param name (IN)- name of config item
 * @param buf (IN) - output word buffer address
 * @param len (IN) - output word buffer length
 * @return byte count of output result
 *    0 : means no output 
 *    
 *   
 */

extern int sal_config_bytes_get(const char*name, uint8* buf, int len);

/**
 * sal_config_uint8_get: get a uint8 variable associated with name
 * 
 * @param name (IN)- name of config item
 * @param byte (IN) - output byte address
 * @param len (IN) - output word buffer length
 * @return word count of output result
 *          0 : means no output 
 *    
 *   
 */

sys_error_t sal_config_uint8_get(const char*name, uint8* byte);

/**
 * sal_config_uint16_get: get a uint16 variable associated with name
 * 
 * @param name (IN)- name of config item
 * @param hword (IN) - output hword address
 * @param len (IN) - output word buffer length
 * @return word count of output result
 *          0 : means no output 
 *    
 *   
 */

sys_error_t sal_config_uint16_get(const char*name, uint16* hword);


/**
 * sal_config_uint32_get: get a uint32 variable associated with name
 * 
 * @param name (IN)- name of config item
 * @param word (IN) - output word buffer address
 * @param len (IN) - output word buffer length
 * @return word count of output result
 *          0 : means no output 
 *    
 *   
 */

sys_error_t sal_config_uint32_get(const char*name, uint32* word);



/* 

     To enumerate all possible config items

*/
#define SAL_CONFIG_SKU_DEVID                   "sku_devid"
#define SAL_CONFIG_SKU_OPTION                  "sku_option"

#define SAL_CONFIG_LED_OPTION                  "led_option"
#define SAL_CONFIG_LED_PROGRAM                 "led_program"

#define SAL_CONFIG_RESET_BUTTON_ENABLE         "reset_button_enable"
#define SAL_CONFIG_RESET_BUTTON_GPIO_BIT       "reset_button_gpio_bit"
#define SAL_CONFIG_RESET_BUTTON_POLARITY       "reset_button_polarity"

#define SAL_CONFIG_PHY_LED1_MODE               "phy_led1_mode"
#define SAL_CONFIG_PHY_LED2_MODE               "phy_led2_mode"
#define SAL_CONFIG_PHY_LED3_MODE               "phy_led3_mode"
#define SAL_CONFIG_PHY_LED4_MODE               "phy_led4_mode"
#define SAL_CONFIG_PHY_LED_CTRL                "phy_led_ctrl"
#define SAL_CONFIG_PHY_LED_SELECT              "phy_led_select"

#define SAL_CONFIG_VALID_PORTS                 "valid_logical_ports"

#define SAL_CONFIG_SPEED_1000_PORTS            "speed_1000_logical_ports"
#define SAL_CONFIG_SPEED_2500_PORTS            "speed_2500_logical_ports"
#define SAL_CONFIG_SPEED_5000_PORTS            "speed_5000_logical_ports"
#define SAL_CONFIG_SPEED_10000_PORTS           "speed_10000_logical_ports"

#define SAL_CONFIG_PHY_AN_PORTS                "phy_an_logical_ports"
#define SAL_CONFIG_PHY_CL73_PORTS              "phy_cl73_logical_ports"
#define SAL_CONFIG_PHY_CL37_PORTS              "phy_cl37_logical_ports"

#define SAL_CONFIG_QTC_INTERFACE               "qtc_interface"
#define SAL_CONFIG_TSCE_INTERFACE              "tsce_interface"
#define SAL_CONFIG_VIPER_INTERFACE             "viper_interface"

#define SAL_CONFIG_UIP_IFCONFIG                "ifconfig"
#endif /* CFG_VENDOR_CONFIG_SUPPORT_INCLUDED */
