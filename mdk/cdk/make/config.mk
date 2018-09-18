# $Id: config.mk,v 1.1 Broadcom SDK $
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
# CDK make rules and definitions
#

#
# Provide reasonable defaults for configuration variables
#

# Default build directory
ifndef CDK_BLDDIR
CDK_BLDDIR = $(CDK)/build
endif

# Location to build objects in
CDK_OBJDIR = $(CDK_BLDDIR)/obj
override BLDDIR := $(CDK_OBJDIR)

# Location to place libraries
CDK_LIBDIR = $(CDK_BLDDIR)
LIBDIR := $(CDK_LIBDIR)

# Option to retrieve compiler version
ifndef CDK_CC_VERFLAGS
CDK_CC_VERFLAGS := -v
endif
CC_VERFLAGS = $(CDK_CC_VERFLAGS); 

# Default suffix for object files
ifndef CDK_OBJSUFFIX
CDK_OBJSUFFIX = o
endif
OBJSUFFIX = $(CDK_OBJSUFFIX)

# Default suffix for library files
ifndef CDK_LIBSUFFIX
CDK_LIBSUFFIX = a
endif
LIBSUFFIX = $(CDK_LIBSUFFIX)

#
# Set up compiler options, etc.
#

# Default include path
CDK_INCLUDE_PATH = -I$(CDK)/include

# Import preprocessor flags avoiding include duplicates
TMP_CDK_CPPFLAGS := $(filter-out $(CDK_INCLUDE_PATH),$(CDK_CPPFLAGS))

# Convenience Makefile flags for building specific chips
ifdef CDK_CHIPS
CDK_DSYM_CPPFLAGS := -DCDK_CONFIG_INCLUDE_CHIP_DEFAULT=0 
CDK_DSYM_CPPFLAGS += $(foreach chip,$(CDK_CHIPS),-DCDK_CONFIG_INCLUDE_${chip}=1) 
endif
ifdef CDK_NCHIPS
CDK_DSYM_CPPFLAGS += $(foreach chip,$(CDK_NCHIPS),-DCDK_CONFIG_INCLUDE_${chip}=0)
endif

TMP_CDK_CPPFLAGS += $(CDK_DSYM_CPPFLAGS)
export CDK_DSYM_CPPFLAGS

ifdef DSYMS
TMP_CDK_CPPFLAGS += -DCDK_CONFIG_CHIP_SYMBOLS_USE_DSYMS=1
endif

override CPPFLAGS = $(TMP_CDK_CPPFLAGS) $(CDK_INCLUDE_PATH)


# Import compiler flags
override CFLAGS = $(CDK_CFLAGS)




#
# Define standard targets, etc.
#

ifdef LOCALDIR
override BLDDIR := $(BLDDIR)/$(LOCALDIR)
endif

ifndef LSRCS
LSRCS = $(wildcard *.c)
endif
ifndef LOBJS
LOBJS = $(addsuffix .$(OBJSUFFIX), $(basename $(LSRCS)))
endif
ifndef BOBJS
BOBJS = $(addprefix $(BLDDIR)/,$(LOBJS))
endif

# Use CDK_QUIET=1 to control printing of compilation lines.
ifdef CDK_QUIET
Q = @
endif

#
# Define rules for creating object directories
#

.PRECIOUS: $(BLDDIR)/.tree

%/.tree:
	@$(ECHO) 'Creating build directory $(dir $@)'
	$(Q)$(MKDIR) $(dir $@)
	@$(ECHO) "Build Directory for $(LOCALDIR) created" > $@

#
# Configure build tools
#
include $(CDK)/make/maketools.mk
