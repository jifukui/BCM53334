/*
* $Id: armpll.c,v 1.3 Broadcom SDK $
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


#include <system.h>
#include "socregs.h"
#define mb() __asm__ __volatile__ ("" : : : "memory")

#define ARM_FREQ_1500 1500
#define ARM_FREQ_1250 1250
#define ARM_FREQ_1000 1000
#define ARM_FREQ_625  625
#define ARM_FREQ_600  600
#define ARM_FREQ_400  400
#define ARM_FREQ_200  200
#define ARM_FREQ_125  125


#if 0
uint32_t iproc_config_armpll_v7(uint32_t mode)
{
    volatile int i_loop  ;
 //   uint32_t pdiv = 1;
    uint32_t data;
    uint32_t freq_id;
    uint32 run_fast = 1;
    uint32_t mdiv;
    
    // Reference CCU PLL Programming sequence
    switch (mode) {
        case ARM_FREQ_1250:
             mdiv = 2;
        break;
        case ARM_FREQ_625:
             mdiv = 4;
        break;
        case ARM_FREQ_125:
             mdiv = 20;
        break;
        default:
            return 1;

    }
    // 1. Access Enable
    WRITECSR(IHOST_PROC_CLK_WR_ACCESS, 0xA5A501);    // Write KPROC_CLK_MGR_REG_WR_ACCESS = 32'h00A5A501 to enable clk manager access. 

    WRITECSR(IHOST_PROC_CLK_POLICY_FREQ, 0x82020202);   // Select the frequency ID =2 for all policies    
    WRITECSR(IHOST_PROC_CLK_POLICY_CTL , (1 << IHOST_PROC_CLK_POLICY_CTL__GO) | ( 1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC));     // Set the GO and GO_AC bit     
    while ((READCSR(IHOST_PROC_CLK_POLICY_CTL) & (1 << IHOST_PROC_CLK_POLICY_CTL__GO) ) != 0);   // Wait for Go bit to get clear     
    
    // 2. PLL reset
    data = READCSR(IHOST_PROC_CLK_PLLARMA);
        
    // (Optional) override VCO frequency -- may need to adjust PLL parameters in PLLARMCTRL3 according to PLL spec
    // data[`KPROC_CLK_MGR_REG_FULL_TYPE_768_PLLARM_PDIV]        = 1;
    // data[`KPROC_CLK_MGR_REG_FULL_TYPE_768_PLLARM_NDIV_INT]    = freq / ctx_cfg.xtal_freq * 2; // VCO freq
    data &= ~(1 << IHOST_PROC_CLK_PLLARMA__pllarm_idle_pwrdwn_sw_ovrride);   // IHOST_PROC_CLK_PLLARMA.pllarm_idle_pwrdwn_sw_ovrride = 0;
    data |= (1 << IHOST_PROC_CLK_PLLARMA__pllarm_soft_resetb);
    WRITECSR(IHOST_PROC_CLK_PLLARMA, data);

    for (i_loop =0 ; i_loop < 1000 ; i_loop++) {
    }       
    
    // 3. Poll and wait for PLL lock signal
    do {
        for (i_loop =0 ; i_loop < 1000 ; i_loop++) {
        }             
  
        data = READCSR(IHOST_PROC_CLK_PLLARMA);
   } while(!(data & (1 << IHOST_PROC_CLK_PLLARMA__pllarm_lock)));
    

    
    // 4. PLL post-div Reset
    data |= (1 << IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb);
    WRITECSR(IHOST_PROC_CLK_PLLARMA, data);
                
    // 5. Select Frequency ID
    data = 0;
    freq_id = run_fast ? 7 : 6;
    data |= (freq_id << IHOST_PROC_CLK_POLICY_FREQ__policy0_freq_R);
    data |= (freq_id << IHOST_PROC_CLK_POLICY_FREQ__policy1_freq_R);
    data |= (freq_id << IHOST_PROC_CLK_POLICY_FREQ__policy2_freq_R);
    data |= (freq_id << IHOST_PROC_CLK_POLICY_FREQ__policy3_freq_R);    
    WRITECSR(IHOST_PROC_CLK_POLICY_FREQ, data);
    

  
    // 6. (If necessary) Adjust PLL post-divider (MDIV)
    // FreqID 7 (Fast)
    if (freq_id == 7) {
        data = READCSR(IHOST_PROC_CLK_PLLARMCTRL5);
//          data[`KPROC_CLK_MGR_REG_C_TYPE_770_PLLARM_MDIV] = 20;
        data &= 0xFFFFFF00;
        data |= (mdiv << IHOST_PROC_CLK_PLLARMCTRL5__pllarm_h_mdiv);
        WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5, data);
    } else {
        // FreqID 6 (Slow)
        // ReadReg(`PROC_CLK_REG_START + `KPROC_CLK_MGR_REG_PLLARMCTRL5, data);
        // data[`KPROC_CLK_MGR_REG_FULL_TYPE_776_PLLARM_H_MDIV] = 4;
        // WriteReg(`PROC_CLK_REG_START + `KPROC_CLK_MGR_REG_PLLARMCTRL5, data);
    }
    
    // 7. (If necessary) Adjust AXI_CLK DIV (FreqID 7 only)
    WRITECSR(IHOST_PROC_CLK_ARM_SWITCH_DIV, 0);
    WRITECSR(IHOST_PROC_CLK_ARM_SWITCH_TRIGGER, 1);
    // 7. Update CCU policy
    data = 0;
    data = ((1 << IHOST_PROC_CLK_POLICY_CTL__GO) | (1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC));
    WRITECSR(IHOST_PROC_CLK_POLICY_CTL, data);
    do {
        for (i_loop =0 ; i_loop < 1000 ; i_loop++) {
        }             
       data = READCSR(IHOST_PROC_CLK_POLICY_CTL);
    } while (data & (1 << IHOST_PROC_CLK_POLICY_CTL__GO));

    // (Optional) Enabling auto clock gating
    WRITECSR(IHOST_PROC_CLK_CORE0_CLKGATE ,     0x00000301);
    WRITECSR(IHOST_PROC_CLK_CORE1_CLKGATE ,     0x00000301);
    WRITECSR(IHOST_PROC_CLK_ARM_SWITCH_CLKGATE, 0x00000301);
    WRITECSR(IHOST_PROC_CLK_ARM_PERIPH_CLKGATE, 0x00000301);
    WRITECSR(IHOST_PROC_CLK_APB0_CLKGATE ,      0x00000301);    
    return 0;
}
#else

uint32_t iproc_config_armpll_v7(uint32_t mode) {
     volatile int i_loop;
     uint32_t data;     
     uint32_t pdiv = 1;    
   
     // Before PLL locking change the Frequency ID to 2 'default'     
     mb();    
     WRITECSR(IHOST_PROC_CLK_WR_ACCESS, 0xA5A501);    // Write KPROC_CLK_MGR_REG_WR_ACCESS = 32'h00A5A501 to enable clk manager access.     
     WRITECSR(IHOST_PROC_CLK_POLICY_FREQ, 0x82020202);   // Select the frequency ID =2 for all policies    
     WRITECSR(IHOST_PROC_CLK_POLICY_CTL , (1 << IHOST_PROC_CLK_POLICY_CTL__GO) | ( 1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC));     // Set the GO and GO_AC bit     
     while ((READCSR(IHOST_PROC_CLK_POLICY_CTL) & (1 << IHOST_PROC_CLK_POLICY_CTL__GO) ) != 0);   // Wait for Go bit to get clear     
     if (mode ==  ARM_FREQ_1250) {         // Reset the PLL and post-divider
         WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0006400 | (pdiv << 24));       
         for (i_loop =0 ; i_loop < 5 ; i_loop++); // crystal_clk=25 MHz , MDIV=4 , H_MDIV=2 , PDIV=1 therefore pll_h_clk= (((crystal_clk/pdiv)* ndiv)/h_mdiv) = 1250         
         WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x2);           
         WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x4);          
         WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0006401 | (pdiv << 24));       
     } else if (mode == ARM_FREQ_125) {
         WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0006400 | (pdiv << 24));       
         for (i_loop =0 ; i_loop < 5 ; i_loop++); // crystal_clk=25 MHz , MDIV=4 , H_MDIV=2 , PDIV=1 therefore pll_h_clk= (((crystal_clk/pdiv)* ndiv)/h_mdiv) = 1250         
         WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x14);           
         WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x4);          
         WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0006401 | (pdiv << 24));            
     } else if (mode == ARM_FREQ_1000) {         // Reset the PLL and post-divider
           //         WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x00005000 | (pdiv << 24)); 
           for (i_loop =0 ; i_loop < 5 ; i_loop++) ;  // Dummy loop for reste propagation         // crystal_clk=25 MHz , MDIV=4 , H_MDIV=2 , PDIV=1 therefore pll_h_clk= (((crystal_clk/pdiv)* ndiv)/h_mdiv) = 1000
           //         WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x2); 
           //         WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x4);          
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0005001 | (pdiv << 24));    
     } else if (mode == ARM_FREQ_600) {         
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0006000 | (pdiv << 24));         
           for (i_loop=0; i_loop < 5; i_loop++);
           WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x4);         
           WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x4);         
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0006001 | (pdiv << 24));    
     } else if (mode == ARM_FREQ_400)  { // 400 MHz         //VCO Frequency should be in between 1568.00/4080.00 MHz, set by vco_range = High          
           // Reset the PLL and post-divider         
           WRITECSR(IHOST_PROC_CLK_PLLARMA, 0x0002000 | (pdiv << 24));         
           for (i_loop =0 ; i_loop < 5 ; i_loop++) ;  // Dummy loop for reste propagation         
           // crystal_clk=25 MHz , MDIV=8 , H_MDIV=4 , PDIV=1 therefore pll_h_clk= (((crystal_clk/pdiv)* ndiv)/h_mdiv) = 400         
           // SBL config is not right - ndiv = 32, pdiv =1, hmdiv needs to be 2!!!         
           WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x2);           
           WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x8);           
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0004001| (pdiv << 24));    
      } else if (mode == ARM_FREQ_200) {          // 200 MHz        
           //VCO Frequency should be in between 1568.00/4080.00 MHz, set by vco_range = High         
           // Reset the PLL and post-divider         
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0002000 | (pdiv << 24));         
           for (i_loop =0 ; i_loop < 5 ; i_loop++) ;  // Dummy loop for reste propagation         
           // crystal_clk=25 MHz , MDIV=8 , H_MDIV=4 , PDIV=1 therefore pll_h_clk= (((crystal_clk/pdiv)* ndiv)/h_mdiv) = 200         
           // SBL config is not right - ndiv = 32, pdiv =1, hmdiv needs to be 4!!!         
           WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x4);           
           WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x10);           
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0004001 | (pdiv << 24));    
      } else if (mode == ARM_FREQ_1500) {         
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0007800 | (pdiv << 24));         
           for (i_loop=0; i_loop < 5; i_loop++) {         }         
           WRITECSR(IHOST_PROC_CLK_PLLARMCTRL5 , (READCSR(IHOST_PROC_CLK_PLLARMCTRL5) & 0xffffff00) | 0x2);         
           WRITECSR(IHOST_PROC_CLK_PLLARMC, (READCSR(IHOST_PROC_CLK_PLLARMC) & 0xffffff00) | 0x2);         
           WRITECSR(IHOST_PROC_CLK_PLLARMA , 0x0007801 | (pdiv << 24));    
       } else {         
           sal_printf("mode is not correct\n");          
           return(-1);    
       } 
       while ( !(READCSR(IHOST_PROC_CLK_PLLARMA) & (1 <<IHOST_PROC_CLK_PLLARMA__pllarm_lock)) ) {           
              // Wait for PLL lock to be set                
       }    
       data = READCSR(IHOST_PROC_CLK_PLLARMA) | (1 << IHOST_PROC_CLK_PLLARMA__pllarm_soft_post_resetb);
       WRITECSR(IHOST_PROC_CLK_PLLARMA, data);
       WRITECSR(IHOST_PROC_CLK_POLICY_FREQ, 0x87070707);  // Switch to frequency ID 7     
       WRITECSR(IHOST_PROC_CLK_POLICY_CTL , (1 << IHOST_PROC_CLK_POLICY_CTL__GO) | ( 1 << IHOST_PROC_CLK_POLICY_CTL__GO_AC));      // Set the GO and GO_AC bit    
       while ((READCSR(IHOST_PROC_CLK_POLICY_CTL) & (1 << IHOST_PROC_CLK_POLICY_CTL__GO) ) != 0) {          // Wait for Go bit to get clear     
       }
       WRITECSR(IHOST_PROC_CLK_CORE0_CLKGATE ,         0x00000301);    
       WRITECSR(IHOST_PROC_CLK_CORE1_CLKGATE ,         0x00000301);    
       WRITECSR(IHOST_PROC_CLK_ARM_SWITCH_CLKGATE, 0x00000301);    
       WRITECSR(IHOST_PROC_CLK_ARM_PERIPH_CLKGATE, 0x00000301);    
       WRITECSR(IHOST_PROC_CLK_APB0_CLKGATE ,          0x00000303);

       mb();    
       return 0;
}

#endif


