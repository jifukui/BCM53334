# $Id: tools_gnu.mk,v 1.2 Broadcom SDK $
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

#
# Addresses of things unless overridden
#

# Hurricane2: CFG_TEST_START and CFG_DATA_START will be overrided.

ifeq ($(strip ${CFG_RAMAPP}),1)
CFG_TEXT_START = 0x60100000
endif 
CFG_DATA_START ?= 0x80000000
CFG_ROM_START  ?= 0xFFFD0000

#
# BOOTRAM mode (runs from ROM vector assuming ROM is writable)
# implies no relocation.
#

ifeq ($(strip ${CFG_BOOTRAM}),1)
  CFG_RELOC = 0
endif


#
# Basic compiler options and preprocessor flags
#

gccincdir = $(shell $(GCC) -print-file-name=include)

# Need back here ?
sys-include = $(gccincdir)/../../../../../arm-linux/sys-include


ifeq ($(strip ${CFG_UCLIBC}),1)
TOOLS ?= /projects/ntsw-tools/toolchains/iproc/arm9uClibctoolchain/usr/bin/
export LD_LIBRARY_PATH=/projects/ntsw-tools/toolchains/iproc/arm9uClibctoolchain/usr/lib
else
TOOLS ?= /projects/ntsw-tools/toolchains/iproc/arm9tools/toolchain-built/sysroot.2011_06_15.17_59_18/le_cortex_a9_external/bin/
endif

# Add GCC lib
ifdef USE_PRIVATE_LIBGCC
  ifeq ("$(USE_PRIVATE_LIBGCC)", "yes")
    PLATFORM_LIBGCC = $(OBJTREE)/arch/$(ARCH)/lib/libgcc.o
  else
    PLATFORM_LIBGCC = -L $(USE_PRIVATE_LIBGCC) -lgcc
  endif
else
  PLATFORM_LIBGCC = -L $(shell dirname `$(GCC) $(CFLAGS) -print-libgcc-file-name`) -lgcc
endif

PLATFORM_LIBS += $(PLATFORM_LIBGCC)
export PLATFORM_LIBS

CFLAGS += -Wall -Werror
CFLAGS += -g -DSTANDALONE -O2 -ffunction-sections -fdata-sections -fno-builtin -ffreestanding -nostdinc -isystem $(gccincdir) -isystem $(sys-include)
CFLAGS += -Wall -Wstrict-prototypes -fno-stack-protector -Wno-strict-aliasing

#
# Tools locations
#
TOOLPREFIX ?= arm-linux-
GCC        ?= $(TOOLS)$(TOOLPREFIX)gcc
GCPP       ?= $(TOOLS)$(TOOLPREFIX)cpp
GCCAS        ?= $(TOOLS)$(TOOLPREFIX)gcc
GLD        ?= $(TOOLS)$(TOOLPREFIX)ld
GAR        ?= $(TOOLS)$(TOOLPREFIX)ar
GOBJDUMP    ?= $(TOOLS)$(TOOLPREFIX)objdump
GOBJCOPY    ?= $(TOOLS)$(TOOLPREFIX)objcopy
GRANLIB     ?= $(TOOLS)$(TOOLPREFIX)ranlib
GSTRIP      ?= $(TOOLS)$(TOOLPREFIX)strip

#
# Check for 64-bit mode
#

ifeq ($(strip ${CFG_MLONG64}),1)
  CFLAGS += -mlong64 -D__long64
endif

#
# Determine parameters for the linker script, which is generated
# using the C preprocessor.
#
# Supported combinations:
#
#  CFG_RAMAPP   CFG_UNCACHED   CFG_RELOC   Description
#    Yes        YesOrNo        MustBeNo    CFE as a separate "application"
#    No         YesOrNo        Yes         CFE relocates to RAM as firmware
#    No         YesOrNo        No          CFE runs from flash as firmware
#

# NorthStar: ./cfe.lds is dynamiclly generated

LDSCRIPT = $(BUILD_DIR)/um.lds
LDFLAGS += -g --script $(LDSCRIPT) -pie -Ttext ${CFG_TEXT_START} --stub-group-size=4
LDSCRIPT_TEMPLATE = ${BSP_SRC}/um_ldscript.template

# NorthStar: ?
ifeq ($(strip ${CFG_UNCACHED}),1)
#  GENLDFLAGS += -DCFG_RUNFROMKSEG0=0
else
#  GENLDFLAGS += -DCFG_RUNFROMKSEG0=1
endif

# NorthStar: ?
ifeq ($(strip ${CFG_RAMAPP}),1)
   GENLDFLAGS += -DCFG_RAMAPP=1
#   GENLDFLAGS += -DCFG_RUNFROMKSEG0=1
else 
 ifeq ($(strip ${CFG_RELOC}),0)
    ifeq ($(strip ${CFG_BOOTRAM}),1)
      GENLDFLAGS += -DCFG_BOOTRAM=1
    else
      GENLDFLAGS += -DCFG_BOOTRAM=0
    endif
  else
    CFLAGS += -membedded-pic -mlong-calls 
    GENLDFLAGS += -DCFG_EMBEDDED_PIC=1
    LDFLAGS +=  --embedded-relocs
  endif
endif

#
# Add GENLDFLAGS to CFLAGS (we need this stuff in the C code as well)
#

CFLAGS += ${GENLDFLAGS}

#
# Determine target endianness
#
# NorthStar ?
ifeq ($(strip ${CFG_LITTLE}),1)
#  ENDIAN = -EL
else
#  ENDIAN = -EB
#  CFLAGS += -EB
#  LDFLAGS += -EB
endif

#
# Add the text/data/ROM addresses to the GENLDFLAGS so they
# will make it into the linker script.
#

GENLDFLAGS += -DCONFIG_SYS_TEXT_BASE=${CFG_TEXT_START}
GENLDFLAGS += -DCFE_ROM_START=${CFG_ROM_START}
GENLDFLAGS += -DCFE_TEXT_START=${CFG_TEXT_START}
GENLDFLAGS += -DCFE_DATA_START=${CFG_DATA_START}

