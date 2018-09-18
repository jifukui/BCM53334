# $Id: um_gen_phylibs.mk,v 1.6 Broadcom SDK $
# $Copyright: Copyright 2016 Broadcom Corporation.
# This program is the proprietary software of Broadcom Corporation
# and/or its licensors, and may only be used, duplicated, modified
# or distributed pursuant to the terms and conditions of a separate,
# written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized
# License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software
# and all intellectual property rights therein.  IF YOU HAVE
# NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
# IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
# ALL USE OF THE SOFTWARE.  
#  
# Except as expressly set forth in the Authorized License,
#  
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of
# Broadcom integrated circuit products.
#  
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
# PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
# OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
# INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
# ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
# TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
# THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
# ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$

# CFLAGS 
ifeq ($(strip ${CFG_MDKSYM}),1)
CFLAGS += -DCFG_MDKSYM
else
CFG_MDKSYM := 0 
endif

# Patch mdk source code
UM_MDK_DIR = $(TOP)/src/driver/mdk
UM_MDK_PATCH = $(patsubst $(UM_MDK_DIR)/%,%, $(shell find  $(UM_MDK_DIR) -name "*.c") $(shell find  $(UM_MDK_DIR) -name "*.h"))
UM_MDK_DRV_TARGETS = $(addsuffix .MDKCOPY, $(UM_MDK_PATCH)) 

# MDK Build parameters
TOOLCHAIN_BIN_DIR = $(TOOLS)

MDKSYM_INCLUDE += -I$(TOP)/mdk/diag/bmddiag/include -I$(TOP)/mdk/phy/include -I$(TOP)/mdk/cdk/include -I$(TOP)/mdk/bmd/include -I$(TOP)/mdk/appl/sys/include -I$(TOP)/mdk/board/include -I$(TOP)/mdk/appl/bmdshell

override PATH := $(TOOLCHAIN_BIN_DIR):$(PATH)
MDK_CFLAGS =  $(filter-out -D%,$(filter-out -I%,$(filter-out -Werror,$(CFLAGS)))) $(MDKSYM_INCLUDE) -DCDK_CONFIG_INCLUDE_BCM56270_A0=1 -DPHY_SYS_USLEEP=sal_usleep  -DBMD_CONFIG_INCLUDE_DMA=0 -DPHY_SYS_USLEEP=sal_usleep -DBMD_SYS_USLEEP=sal_usleep -DPHY_CONFIG_INCLUDE_CHIP_SYMBOLS=$(CFG_MDKSYM) -DCDK_CONFIG_INCLUDE_FIELD_INFO=$(CFG_MDKSYM) -DPHY_DEBUG_ENABLE -DBMD_SYS_DMA_ALLOC_COHERENT=sal_dma_malloc -DBMD_SYS_DMA_FREE_COHERENT=sal_dma_free

export TOOLCHAIN_BIN_DIR LD_LIBRARY_PATH
export PHY_PKG_OPTIONS = -c bcmi_tsce_xgxs,bcmi_viper_xgxs -b ,
export PHY_CPPFLAGS = $(MDK_CFLAGS) 

# MDK Build tools
export CC = $(GCC) 
export LD = $(GLD)
export AR = $(GAR)

#phylibs : cleanpkgs instpkgs $(UM_MDK_DRV_TARGETS)
phylibs : cleanpkgs instpkgs
	@echo "Bulding MDK phy library."
	@make -C $(TOP)/mdk/phy phylibs MDK=$(TOP)/mdk
	@echo "Copying MDK phy library."
	cp $(TOP)/mdk/phy/build/libphygeneric.a lib/.
	cp $(TOP)/mdk/phy/build/libphypkgsrc.a lib/.
	cp $(TOP)/mdk/phy/build/libphyutil.a lib/.

phylibs_clean :
	@echo "Cleaning MDK phy library."
	make -C $(TOP)/mdk/phy clean MDK=$(TOP)/mdk
	make -C $(TOP)/mdk/phy cleanpkgs MDK=$(TOP)/mdk

cleanpkgs : 
	make -C $(TOP)/mdk/phy clean MDK=$(TOP)/mdk
	make -C $(TOP)/mdk/phy cleanpkgs MDK=$(TOP)/mdk

%.MDKCOPY :  
	echo "Patch MDK :"  $(TOP)/mdk/$(patsubst %.MDKCOPY,%,$@)  
	-cp -v $(UM_MDK_DIR)/$(patsubst %.MDKCOPY,%,$@) $(TOP)/mdk/$(patsubst %.MDKCOPY,%,$@) 

instpkgs:
	make -C $(TOP)/mdk/phy instpkgs MDK=$(TOP)/mdk

