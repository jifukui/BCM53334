#!/usr/bin/perl -w

#
# $Id: parse_context.pl,v 1.2 Broadcom SDK $
# 
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

use XML::Parser;
use File::Basename;
use Cwd;

######################################################
#
# Copyright
# 
######################################################
$copy_right = "/*\n * \$Id\$\n *\n * \$Copyright: (c) 2014 Broadcom Corp.\n * All Rights Reserved.\$\n *\n */\n\n";

######################################################
#
# Constants
# 
######################################################

# XML tags for context
$TAG_CONTEXT        = 'CONTEXT';
$TAG_SELECT         = 'SELECT';
$TAG_OPTIONAL       = 'OPTIONAL';
$TAG_CMD_KEYWORD    = 'COMMAND-KEYWORD';
$TAG_CMD_EOL        = 'COMMAND-EOL';
$TAG_PARAM_KEYWORD  = 'PARAM-KEYWORD';
$TAG_PARAM_WORD     = 'PARAM-WORD';
$TAG_PARAM_LINE     = 'PARAM-LINE';
$TAG_PARAM_INTEGER  = 'PARAM-INTEGER';
$TAG_PARAM_CUSTOM   = 'PARAM-CUSTOM';
$TAG_PARAM_OPTIONS  = 'PARAM-OPTIONS';
$TAG_OPTION         = 'OPTION';

# XML attributes for context

$ATTR_NAME          = 'name';
$ATTR_DESC          = 'desc';
$ATTR_ID            = 'id';
$ATTR_CMDTYPE       = 'cmdtype';
$ATTR_CMDTYPE_DEF   = 'defcmdtype';
$ATTR_ACCESS        = 'access';
$ATTR_CONTEXT       = 'context';
$ATTR_MINLEN        = 'minlen';
$ATTR_MAXLEN        = 'maxlen';
$ATTR_MEMTYPE       = 'memtype';
$ATTR_MIN           = 'min';
$ATTR_MAX           = 'max';
$ATTR_SIGNED        = 'signed';
$ATTR_RADIX         = 'radix';
$ATTR_TYPE          = 'type';
$ATTR_MARKED        = 'marked';

# Tags vs. Attributes map for context
use constant ATTR_OPTIONAL => scalar 0;
use constant ATTR_REQUIRED => scalar 1;
%g_context_tag_attribute_map = (

    $TAG_CONTEXT => {
            $ATTR_DESC      => ATTR_OPTIONAL,
            $ATTR_ID        => ATTR_REQUIRED,
            $ATTR_CMDTYPE   => ATTR_REQUIRED,
            $ATTR_ACCESS    => ATTR_OPTIONAL,       
        },

    $TAG_SELECT => {
        },

    $TAG_OPTIONAL => {
        },

    $TAG_CMD_KEYWORD => {
            $ATTR_NAME      => ATTR_REQUIRED,
            $ATTR_DESC      => ATTR_OPTIONAL,       
            $ATTR_CMDTYPE   => ATTR_OPTIONAL,
            $ATTR_ACCESS    => ATTR_OPTIONAL,
            $ATTR_CONTEXT   => ATTR_OPTIONAL,
            $ATTR_MARKED    => ATTR_OPTIONAL,
        },
    
    $TAG_CMD_EOL => {
            $ATTR_CMDTYPE   => ATTR_OPTIONAL,
            $ATTR_CONTEXT   => ATTR_OPTIONAL,
            $ATTR_ACCESS    => ATTR_OPTIONAL,
            $ATTR_MARKED    => ATTR_OPTIONAL,
        },
    
    $TAG_PARAM_KEYWORD => {
            $ATTR_NAME      => ATTR_REQUIRED,
            $ATTR_DESC      => ATTR_OPTIONAL,       
        },
    
    $TAG_PARAM_WORD => {
            $ATTR_NAME      => ATTR_OPTIONAL,
            $ATTR_DESC      => ATTR_OPTIONAL,       
            $ATTR_ID        => ATTR_REQUIRED,
            $ATTR_MINLEN    => ATTR_OPTIONAL,
            $ATTR_MAXLEN    => ATTR_OPTIONAL,
            $ATTR_MEMTYPE   => ATTR_OPTIONAL,
        },
    
    $TAG_PARAM_LINE => {
            $ATTR_NAME      => ATTR_OPTIONAL,
            $ATTR_DESC      => ATTR_OPTIONAL,       
            $ATTR_ID        => ATTR_REQUIRED,
            $ATTR_MINLEN    => ATTR_OPTIONAL,
            $ATTR_MAXLEN    => ATTR_OPTIONAL,
            $ATTR_MEMTYPE   => ATTR_OPTIONAL,
        },
    
    $TAG_PARAM_INTEGER => {
            $ATTR_NAME      => ATTR_OPTIONAL,
            $ATTR_DESC      => ATTR_OPTIONAL,       
            $ATTR_ID        => ATTR_REQUIRED,
            $ATTR_MIN       => ATTR_REQUIRED,       
            $ATTR_MAX       => ATTR_REQUIRED,       
            $ATTR_SIGNED    => ATTR_OPTIONAL,
            $ATTR_RADIX     => ATTR_OPTIONAL,
        },
       
    $TAG_PARAM_CUSTOM => {
            $ATTR_NAME      => ATTR_OPTIONAL,
            $ATTR_DESC      => ATTR_OPTIONAL,       
            $ATTR_ID        => ATTR_REQUIRED,
            $ATTR_TYPE      => ATTR_REQUIRED,
        },
    
    $TAG_PARAM_OPTIONS => {
            $ATTR_ID        => ATTR_REQUIRED,
        },
    
    $TAG_OPTION => {
            $ATTR_NAME      => ATTR_REQUIRED,
            $ATTR_DESC      => ATTR_OPTIONAL,       
        },
);

# Attribute validators
%g_attr_validators = (
    $ATTR_NAME      => \&validate_attr_name,
    $ATTR_DESC      => \&validate_attr_any,
    $ATTR_ID        => \&validate_attr_id,
    $ATTR_CMDTYPE   => \&validate_attr_cmdtype,
    $ATTR_ACCESS    => \&validate_attr_access,
    $ATTR_CONTEXT   => \&validate_attr_id,
    $ATTR_MINLEN    => \&validate_attr_strlen,
    $ATTR_MAXLEN    => \&validate_attr_strlen,
    $ATTR_MEMTYPE   => \&validate_attr_memtype,
    $ATTR_MIN       => \&validate_attr_integer,
    $ATTR_MAX       => \&validate_attr_integer,
    $ATTR_SIGNED    => \&validate_attr_yes,
    $ATTR_RADIX     => \&validate_attr_radix,
    $ATTR_TYPE      => \&validate_attr_id,
    $ATTR_MARKED    => \&validate_attr_yes,
);

# Context tag processors
%g_context_processor_map = (
    $TAG_CONTEXT        => \&process_context_tag_context,
    $TAG_SELECT         => \&process_context_tag_select,
    $TAG_OPTIONAL       => \&process_context_tag_optional,
    $TAG_CMD_KEYWORD    => \&process_context_tag_cmd_keyword,
    $TAG_CMD_EOL        => \&process_context_tag_cmd_eol,
    $TAG_PARAM_KEYWORD  => \&process_context_tag_param_keyword,
    $TAG_PARAM_WORD     => \&process_context_tag_param_word,
    $TAG_PARAM_LINE     => \&process_context_tag_param_line,
    $TAG_PARAM_INTEGER  => \&process_context_tag_param_integer,
    $TAG_PARAM_CUSTOM   => \&process_context_tag_param_custom,
    $TAG_PARAM_OPTIONS  => \&process_context_tag_param_options,
    $TAG_OPTION         => \&process_context_tag_option,
);

# Context tag type
use constant TAG_TYPE_OTHER => scalar 0;
use constant TAG_TYPE_COMMAND => scalar 1;
use constant TAG_TYPE_PARAM => scalar 2;
%g_context_tag_type = (
    $TAG_CONTEXT        => TAG_TYPE_COMMAND,
    $TAG_SELECT         => TAG_TYPE_PARAM,
    $TAG_OPTIONAL       => TAG_TYPE_PARAM,
    $TAG_CMD_KEYWORD    => TAG_TYPE_COMMAND,
    $TAG_CMD_EOL        => TAG_TYPE_COMMAND,
    $TAG_PARAM_KEYWORD  => TAG_TYPE_PARAM,
    $TAG_PARAM_WORD     => TAG_TYPE_PARAM,
    $TAG_PARAM_LINE     => TAG_TYPE_PARAM,
    $TAG_PARAM_INTEGER  => TAG_TYPE_PARAM,
    $TAG_PARAM_CUSTOM   => TAG_TYPE_PARAM,
    $TAG_PARAM_OPTIONS  => TAG_TYPE_PARAM,
    $TAG_OPTION         => TAG_TYPE_OTHER,
);

# Context node traverse map
%g_context_traverse_map = (
    $TAG_CONTEXT        => \&traverse_tag_context,
    $TAG_SELECT         => \&traverse_tag_select,
    $TAG_OPTIONAL       => \&traverse_tag_optional,
    $TAG_CMD_KEYWORD    => \&traverse_tag_cmd_keyword,
    $TAG_CMD_EOL        => \&traverse_tag_cmd_eol,
    $TAG_PARAM_KEYWORD  => \&traverse_tag_param_keyword,
    $TAG_PARAM_WORD     => \&traverse_tag_param_word,
    $TAG_PARAM_LINE     => \&traverse_tag_param_line,
    $TAG_PARAM_INTEGER  => \&traverse_tag_param_integer,
    $TAG_PARAM_CUSTOM   => \&traverse_tag_param_custom,
    $TAG_PARAM_OPTIONS  => \&traverse_tag_param_options,
    $TAG_OPTION         => \&traverse_tag_option,
);

# Context tag to C code type map
%g_tag_handler_c_type_map = (
    $TAG_PARAM_WORD     => "char *",
    $TAG_PARAM_LINE     => "char *",
    $TAG_PARAM_INTEGER  => "int",
    $TAG_PARAM_OPTIONS  => "int",
);

# Context tag to C code type map
%g_tag_handler_param_type_map = (
    $TAG_PARAM_WORD     => "word",
    $TAG_PARAM_LINE     => "line",
    $TAG_PARAM_INTEGER  => "integer",
    $TAG_PARAM_OPTIONS  => "options",
);

# Node fields
$NODE_TAG       = 'tag_element';
$NODE_ATTRS     = 'attributes';
$NODE_NEXT      = 'next';
$NODE_CHILDREN  = 'children';
$NODE_OPTIONS   = 'options';
$NODE_SRCLINE   = 'srcline';

# Additional node fields (traverse phase)
$NODE_ID        = 'ID';
$NODE_NAME      = 'name';

# Command flags
$CMDFLAG_ACCESS_GUEST       = 0x00000001;
$CMDFLAG_ACCESS_NORMAL      = 0x00000002;
$CMDFLAG_ACCESS_PRIVILEGED  = 0x00000004;
$CMDFLAG_ACCESS_MASK        = 0xFFFFFFF8;
$CMDFLAG_CMDTYPE_CONFIG     = 0x00000010;
$CMDFLAG_CMDTYPE_SHOW       = 0x00000020;
$CMDFLAG_CMDTYPE_UPDATE     = 0x00000040;
$CMDFLAG_CMDTYPE_OPERATE    = 0x00000080;
$CMDFLAG_CMDTYPE_MASK       = 0xFFFFFF0F;
$CMDFLAG_BEHAVIOR_MARKED    = 0x00010000;

# Access value to flags map
%g_access_value_to_flags = (
    'guest'         => $CMDFLAG_ACCESS_GUEST,
    'normal'        => $CMDFLAG_ACCESS_NORMAL,
    'privileged'    => $CMDFLAG_ACCESS_PRIVILEGED,
);


# cmdtype value to flags map
%g_cmdtype_value_to_flags = (
    'config'        => $CMDFLAG_CMDTYPE_CONFIG,
    'show'          => $CMDFLAG_CMDTYPE_SHOW,
    'update'        => $CMDFLAG_CMDTYPE_UPDATE,
    'operate'       => $CMDFLAG_CMDTYPE_OPERATE,
);

# flag bits to C flag bits
%g_flags_value_to_cflags = (
    $CMDFLAG_ACCESS_GUEST       => 'XCFLAG_ACCESS_GUEST',
    $CMDFLAG_ACCESS_NORMAL      => 'XCFLAG_ACCESS_NORMAL',
    $CMDFLAG_ACCESS_PRIVILEGED  => 'XCFLAG_ACCESS_PRIVILEGED',
    $CMDFLAG_CMDTYPE_CONFIG     => 'XCFLAG_CMDTYPE_CONFIG',
    $CMDFLAG_CMDTYPE_SHOW       => 'XCFLAG_CMDTYPE_SHOW',
    $CMDFLAG_CMDTYPE_UPDATE     => 'XCFLAG_CMDTYPE_UPDATE',
    $CMDFLAG_CMDTYPE_OPERATE    => 'XCFLAG_CMDTYPE_OPERATE',
    $CMDFLAG_BEHAVIOR_MARKED    => 'XCFLAG_BEHAVIOR_MARKED',
);

# XML tags for definitions
$TAG_DEFINITIONS    = 'DEFINITIONS';
$TAG_DEF_TYPE       = 'DEFINE-TYPE';

# XML attributes for definitions
$ATTR_DEF_ID        = 'id';
$ATTR_DEF_CTYPE     = 'ctype';
$ATTR_DEF_ALT_CTYPE = 'alt-ctype';
$ATTR_DEF_FORMAT    = 'format';

# Tags vs. Attributes map for definitios
%g_defines_tag_attribute_map = (

    $TAG_DEFINITIONS => {

        },

    $TAG_DEF_TYPE => {
            $ATTR_DEF_ID        => ATTR_REQUIRED,
            $ATTR_DEF_CTYPE     => ATTR_REQUIRED,
            $ATTR_DEF_ALT_CTYPE => ATTR_OPTIONAL,
            $ATTR_DEF_FORMAT    => ATTR_OPTIONAL,
        },

);

# Attribute validators (for definitions)
%g_defines_attr_validators = (
    $ATTR_DEF_ID            => \&validate_attr_id,
    $ATTR_DEF_CTYPE         => \&validate_attr_any,
    $ATTR_DEF_ALT_CTYPE     => \&validate_attr_any,
    $ATTR_DEF_FORMAT        => \&validate_def_attr_format,
);

# Defines tag processors
%g_defines_processor_map = (
    $TAG_DEFINITIONS    => \&process_defines_tag_definitions,
    $TAG_DEF_TYPE       => \&process_defines_tag_deftype,
);

# File inclusion support
$TAG_INC_INCLUDE    = "INCLUDE";
$TAG_INC_INCLUDED   = "INCLUDED";
$ATTR_INC_FILE      = "file";
$ATTR_INC_COMMENT   = "comment";

# Debugging levels
use constant DBG_NONE => scalar 0;
use constant DBG_INFO => scalar 1;
use constant DBG_VERBOSE => scalar 2;
use constant DBG_DETAIL => scalar 3;

# Warning levels
use constant WARN_NONE => scalar 0;
use constant WARN_INFO => scalar 1;
use constant WARN_VERBOSE => scalar 2;
use constant WARN_DETAIL => scalar 3;


######################################################
#
# Global variables
# 
######################################################

%g_defined_types = ();

$g_output_dir = "";


######################################################
#
# Configurations
# 
######################################################

# Debugging mode
$g_debug_level = DBG_NONE;

# Warning mode
$g_warning_level = WARN_DETAIL;


######################################################
#
# Common utilities
# 
######################################################

sub error
{
    die "***** ERROR: $_[0]\n";
}

sub warning
{
    if ($g_warning_level >= $_[0]) {
        print STDERR "*** WARNING: $_[1]\n";
    }
}

sub debug
{
    if ($g_debug_level >= $_[0]) {
        print "$_[1]\n";
    }
}

 
######################################################
#
# Subroutines
# 
######################################################

sub validate_attr_name
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    if ($el eq $TAG_CMD_KEYWORD || 
        $el eq $TAG_PARAM_KEYWORD || 
        $el eq $TAG_OPTION) {
        if ($val =~ /^(\w|-)+$/) {
            return 1;
        }
    } else {
        if ($val =~ /^\S+$/) {
            return 1;
        }
    }
    return 0;
}

sub validate_attr_id
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    if ($val =~ /^\w+$/) {
        return 1;
    }
    return 0;
}

sub validate_attr_integer
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    if ($val =~ /^0x/) {
        $val =~ s/^0x//;
        if ($val =~ /^[0-9a-f]+$/) {
            return 1;
        }
    } else {
        if ($val =~ /^-?\d+$/) {
            return 1;
        }
    }
    return 0;
}

sub validate_attr_strlen
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    unless ($val =~ /^\d+$/) {
        return 0;
    }
    if ($val < 0 || $val > 65535) {
        return 0;
    }
    return 1;
}

sub validate_attr_memtype
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    if (lc($val) eq "allocated" ||
        lc($val) eq "stack") {
        return 1;
    }
    return 0;
}

sub validate_attr_cmdtype
{
    my $el = shift;
    my $at = shift;
    my $val = lc(shift);

    if (defined($g_cmdtype_value_to_flags{$val})) {
        return 1;
    }
    return 0;
}

sub validate_attr_access
{
    my $el = shift;
    my $at = shift;
    my $val = lc(shift);

    if (defined($g_access_value_to_flags{$val})) {
        return 1;
    }
    return 0;
}

sub validate_attr_yes
{
    my $el = shift;
    my $at = shift;
    my $val = lc(shift);
    if ($val eq "yes" || $val eq "no") {
        return 1;
    }
    return 0;
}

sub validate_attr_radix
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    if (lc($val) eq "hex" || 
        lc($val) eq "dec") {

        return 1;
    }
    return 0;
}

sub validate_attr_any
{
    return 1;
}

sub check_tag_position
{
    my $tag = shift;
    my $tp = $g_context_tag_type{$tag};
    my $partp = TAG_TYPE_PARAM;
    if ($tp == TAG_TYPE_PARAM) {
        $partp = TAG_TYPE_COMMAND;
    }
    
    my $pparent = $node_stack[$#node_stack];
    if ($g_context_tag_type{$pparent->{$NODE_TAG}} != $partp) {
        error "$tag appears under a non-command-type tag".
              " at $context_file:".$expat->current_line;
    }
}

sub check_cmd_tag_position
{
    my $tag = shift;
    my $pparent = $node_stack[$#node_stack];
    if (($pparent->{$NODE_TAG} ne $TAG_SELECT) && ($pparent->{$NODE_TAG} ne $TAG_OPTIONAL)) {
        error "$tag appears under a tag not '$TAG_SELECT' or '$TAG_OPTIONAL'".
              " at $context_file:".$expat->current_line;
    }
}

sub follow_parent
{
    my $pnode = shift;
    my $pparent = $node_stack[$#node_stack];

    # Check for duplicated command
    my @children = @{ $pparent->{$NODE_CHILDREN} };
    if ($pnode->{$NODE_TAG} eq $TAG_CMD_EOL) {
        foreach $ref (@children) {
            if ($ref->{$NODE_TAG} eq $TAG_CMD_EOL) {
                error "Duplicated $TAG_CMD_EOL under one $TAG_SELECT or $TAG_OPTIONAL".
                      " at $context_file:".$expat->current_line;
            }
        }
    } else {
        my $name = lc($pnode->{$NODE_ATTRS}->{$ATTR_NAME});
        foreach $ref (@children) {
            if ($ref->{$NODE_TAG} eq $TAG_CMD_KEYWORD) {
                if (lc($ref->{$NODE_ATTRS}->{$ATTR_NAME}) eq $name) {
                    error "Duplicated name '$name' for tag '$TAG_CMD_KEYWORD'".
                          " at $context_file:".$expat->current_line;
                }
            }
        }

        # Set ATTR_CMDTYPE default value
        $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF} = $pparent->{$NODE_ATTRS}->{$ATTR_CMDTYPE};
        if (!defined($pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE})) {
            $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE} = $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF};        	  
        } elsif ($g_cmdtype_value_to_flags{$pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF}} & $CMDFLAG_CMDTYPE_SHOW) {
            $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE} = 'show';        	          
        }
              
        # Starting point of another chain of parameters
        $prev_node = $pnode;
        
        
    }

    push @{ $pparent->{$NODE_CHILDREN} }, $pnode;
}

sub follow_previous
{
    my $pnode = shift;
    if (defined($prev_node)) {
        
        # Nothing should appear AFTER a SELECT tag
        if ($prev_node->{$NODE_TAG} eq $TAG_SELECT) {
            my $tag = $pnode->{$NODE_TAG};
            error "'$tag' following $TAG_SELECT".
                  " at $context_file:".$expat->current_line;
        }
        
        # Nothing should appear AFTER a LINE tag
        if ($prev_node->{$NODE_TAG} eq $TAG_PARAM_LINE) {
            my $tag = $pnode->{$NODE_TAG};
            error "'$tag' following $TAG_PARAM_LINE".
                  " at $context_file:".$expat->current_line;
        }
        if ($prev_node->{$NODE_TAG} eq $TAG_PARAM_CUSTOM) {
            my $tag = $pnode->{$NODE_TAG};
            my $idtype = $prev_node->{$NODE_ATTRS}->{$ATTR_TYPE};
            my $ptype = $g_defined_types{$idtype};
            my $tpfm = $ptype->{$NODE_ATTRS}->{$ATTR_DEF_FORMAT};
            if (defined($tpfm) && lc($tpfm) eq 'line') {
                error "'$tag' following line-based custom type '$idtype'".
                      " at $context_file:".$expat->current_line;
            }
        }
        
        $prev_node->{$NODE_NEXT} = $pnode;
        # Set ATTR_CMDTYPE default value
        $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF} = $prev_node->{$NODE_ATTRS}->{$ATTR_CMDTYPE};
        if (!defined($pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE})) {
            $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE} = $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF};
        } elsif ($g_cmdtype_value_to_flags{$pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF}} & $CMDFLAG_CMDTYPE_SHOW) {
            $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE} = 'show';        	          
        }
 
    } else {
        my $ppar = $node_stack[$#node_stack];
        
        # CONTEXT tag should only contain one SELECT tag (for now)
        if (@node_stack == 1) {
            if ($pnode->{$NODE_TAG} ne $TAG_SELECT) {
                error "Only $TAG_SELECT can go under $TAG_CONTEXT tag".
                      " at $context_file:".$expat->current_line;
            }
        }
        
        # Nothing should go under <CR> tag
        if ($ppar->{$NODE_TAG} eq $TAG_CMD_EOL) {
            my $tag = $pnode->{$NODE_TAG};
            error "'$tag' following $TAG_CMD_EOL".
                  " at $context_file:".$expat->current_line;
        }
        
        $ppar->{$NODE_NEXT} = $pnode;
        # Set ATTR_CMDTYPE default value
        $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF} = $ppar->{$NODE_ATTRS}->{$ATTR_CMDTYPE};
        if (!defined($pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE})) {
            $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE} = $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF};
        } elsif ($g_cmdtype_value_to_flags{$pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE_DEF}} & $CMDFLAG_CMDTYPE_SHOW) {
            $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE} = 'show';        	          
        }
 
    }
}

sub process_context_tag_context
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};
    
    if (defined($context_node)) {
        error "Invalid tag '$tag' found".
              " at $context_file:".$expat->current_line;
    }
    $context_node = $pnode;
}

sub process_context_tag_select
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_optional
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_cmd_keyword
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_cmd_tag_position($tag);
    follow_parent($pnode);
}

sub process_context_tag_cmd_eol
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_cmd_tag_position($tag);
    follow_parent($pnode);
}

sub process_context_tag_param_keyword
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_param_word
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_param_line
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_param_integer
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_param_custom
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_param_options
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    check_tag_position($tag);
    follow_previous($pnode);
}

sub process_context_tag_option
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    my $pparent = $node_stack[$#node_stack];
    if ($pparent->{$NODE_TAG} ne $TAG_PARAM_OPTIONS) {
        error "$tag appears under a tag not '$TAG_PARAM_OPTIONS'".
              " at $context_file:".$expat->current_line;
    }
    
    # Check for duplicated option
    my @options = @{ $pparent->{$NODE_OPTIONS} };
    my %my_attrs = %{ $pnode->{$NODE_ATTRS} };
    my $name = lc $my_attrs{$ATTR_NAME};
    foreach my $ref (@options) {
        my %attrs = % { $ref->{$NODE_ATTRS} };
        if (lc($attrs{$ATTR_NAME}) eq $name) {
            error "Duplicated name '$name' for tag '$TAG_OPTION'".
                  " at $context_file:".$expat->current_line;
        }
    }
    
    push @{ $pparent->{$NODE_OPTIONS} }, $pnode;
}

sub start_handler
{
    local $expat = shift;
    my $tag = uc shift;
    my @args = @_;

    # File inclusion support
    if ($tag eq $TAG_INC_INCLUDE) {
        
        if (@args < 2 || @args > 4) {
            error "Incorrect number of attribues for $tag".
                  " at $context_file:".$expat->current_line;
        }
        
        my $fname = "";
        while(@args) {
            my $attr = lc shift(@args);
            my $value = shift(@args);
            
            if ($attr eq $ATTR_INC_FILE) {
                if ($fname ne "") {
                    error "Multiple '$attr' for $tag".
                          " at $context_file:".$expat->current_line;
                }
                $fname = $value;

            } elsif ($attr eq $ATTR_INC_COMMENT) {
                next;
            } else {
                error "Invalid attribue '$attr' for $tag".
                      " at $context_file:".$expat->current_line;
            }
        }

        # Handle relative pathname
        my $file = dirname($context_file)."/".$fname;
        unless (-f $file) {
            error "Can't open included file \"$fname\"".
                  " at $context_file:".$expat->current_line;
        }
        
        my $parser = new XML::Parser(ErrorContext => 2);
        $parser->setHandlers(Start => \&start_handler, 
                             Char => \&char_handler,
                             End => \&end_handler);
        local $context_file = dirname($context_file)."/".$fname;
        $parser->parsefile($file);
        return;

    } elsif ($tag eq $TAG_INC_INCLUDED) {
        # Ignored
        return;
    }
    
    # Check tag name
    if (!defined($g_context_tag_attribute_map{$tag})) {
        error "Invalid tag '$tag' found".
              " at $context_file:".$expat->current_line;
    }

    # Create a new node
    my %node = (
        $NODE_TAG => $tag,
        $NODE_ATTRS => {},
        $NODE_NEXT => undef,
        $NODE_CHILDREN => [],
        $NODE_OPTIONS => [],
        $NODE_SRCLINE => $expat->current_line,
    );
    
    # Get attributes
    my %attmap = %{ $g_context_tag_attribute_map{$tag} };
    while(@args) {
        my $attr = shift(@args);
        my $value = shift(@args);
        
        # Check attribute
        if (!defined($attmap{$attr})) {
            error "Invalid attribute '$attr' found".
                  " at $context_file:".$expat->current_line;
        }

        # Check attribute value
        my $func = $g_attr_validators{$attr};
        if (defined($func)) {
            if (!$func->($tag, $attr, $value)) {
                error "Invalid value '$value' for '$attr'".
                      " at $context_file:".$expat->current_line;
            }
        } else {
            error "Invalid attribute '$attr' found".
                  " at $context_file:".$expat->current_line;
        }
        
        # Add this attribute to the node
        $node{$NODE_ATTRS}->{$attr} = $value;
    }
    my %attrs = %{ $node{$NODE_ATTRS} };
    
    # Check required attributes are present
    foreach my $attr (keys %attmap) {
        if ($attmap{$attr} == ATTR_REQUIRED) {
            if (!defined($attrs{$attr})) {
                error "Required attribute '$attr' for tag '$tag' not found".
                      " at $context_file:".$expat->current_line;
            }
        }
    }
    
    # Check attributes for some special cases
    if ($tag eq $TAG_PARAM_WORD || $tag eq $TAG_PARAM_LINE) {
        my $minlen = $attrs{$ATTR_MINLEN};
        my $maxlen = $attrs{$ATTR_MAXLEN};
        my $memtype = $attrs{$ATTR_MEMTYPE};
        if (defined($minlen) && defined($maxlen)) {
            if ($minlen > $maxlen) {
                error "Incorrect min/max length for tag $tag".
                      " at $context_file:".$expat->current_line;
            }
        }
        
        # minlen must at least be 1 for WORD parameter
        if (defined($minlen)) {
            if ($tag eq $TAG_PARAM_WORD && $minlen == 0) {
                error "Incorrect minimal length for tag $tag".
                      " at $context_file:".$expat->current_line;
            }
        }
        
        if (defined($memtype) && lc($memtype) eq 'stack') {
            if (!defined($maxlen) || $maxlen > 256) {
                error "Incorrect or missing maxlen for stack-based $tag".
                      " at $context_file:".$expat->current_line;
            }
        }
        
    } elsif ($tag eq $TAG_PARAM_INTEGER) {
        my $min = $attrs{$ATTR_MIN};
        my $max = $attrs{$ATTR_MAX};
        my $signed = $attrs{$ATTR_SIGNED};
        if (defined($signed)) {
            $signed = lc $signed;
        } else {
            $signed = "no";
        }
        $node{$NODE_ATTRS}->{$ATTR_SIGNED} = $signed;
        my $radix = $attrs{$ATTR_RADIX};
        if (defined($radix)) {
            $radix = lc $radix;
        } else {
            $radix = "dec";
        }
        $node{$NODE_ATTRS}->{$ATTR_RADIX} = $radix;
        if (defined($min)) { 
            if ($min =~ /^0x/) {
                if ($radix ne 'hex') {
                    error "Decimal number starts with '0x' in tag $tag".
                          " at $context_file:".$expat->current_line;
                }
                $min = hex($min);
            } else {
                if ($radix eq 'hex') {
                    error "Missing '0x' prefix for hexadecimal in tag $tag".
                          " at $context_file:".$expat->current_line;
                }
            }
            if ($signed eq "no" && $min < 0) {
                error "Negative min value for unsigned number of tag $tag".
                      " at $context_file:".$expat->current_line;
            }
        }
        if (defined($max)) { 
            if ($max =~ /^0x/) {
                if ($radix ne 'hex') {
                    error "Decimal number starts with '0x' in tag $tag".
                          " at $context_file:".$expat->current_line;
                }
                $max = hex($max);
            } else {
                if ($radix eq 'hex') {
                    error "Missing '0x' prefix for hexadecimal in tag $tag".
                          " at $context_file:".$expat->current_line;
                }
            }
            if ($signed eq "no" && $max < 0) {
                error "Negative max value for unsigned number of tag $tag".
                      " at $context_file:".$expat->current_line;
            }
        }
        if (defined($min) && defined($max)) {
            if ($min > $max) {
                error "Incorrect min/max value for tag $tag".
                      " at $context_file:".$expat->current_line;
            }
        }
    } elsif ($tag eq $TAG_PARAM_CUSTOM) {
        my $type = lc $attrs{$ATTR_TYPE};
        if (!defined($g_defined_types{$type})) {
            error "Undefined type '$type' found".
                  " at $context_file:".$expat->current_line;
        }
    }
    
    # Tag-level checking and processing
    my $tagf = $g_context_processor_map{$tag};
    if (defined($tagf)) {
        $tagf->(\%node);
    } else {
        error "Invalid tag '$tag' found".
              " at $context_file:".$expat->current_line;
    }
    
    # Everything OK, push node the stack
    push @node_stack, \%node;

}

sub char_handler
{
    my $expat = shift;
    my $body = shift;
    $body =~ s/\s*//;
    if ($body) {
        error "Garbage body content present".
              " at $context_file:".$expat->current_line;
    }
}

sub end_handler
{
    local $expat = shift;
    my $tag = uc shift;

    # File inclusion support
    if ($tag eq $TAG_INC_INCLUDE || $tag eq $TAG_INC_INCLUDED) {
        # Ignored
        return;
    }

    my $pnode = pop @node_stack;
  
    if ($g_context_tag_type{$tag} == TAG_TYPE_COMMAND) {
        $prev_node = undef;
    } elsif ($g_context_tag_type{$tag} == TAG_TYPE_PARAM) {
        $prev_node = $pnode;
    }
    
    # Check for empty children/options
    if ($pnode->{$NODE_TAG} eq $TAG_CONTEXT) {
        if (!defined($pnode->{$NODE_NEXT})) {
            error "$TAG_CONTEXT has nothing in it".
                  " at $context_file:".$expat->current_line;
        }
    }
    if ($pnode->{$NODE_TAG} eq $TAG_SELECT) {
        my @children = @{ $pnode->{$NODE_CHILDREN} };
        if (@children < 2) {
            error "$TAG_SELECT has less than 2 commands".
                  " at $context_file:".$expat->current_line;
        }
    }
    if ($pnode->{$NODE_TAG} eq $TAG_OPTIONAL) {
        my @children = @{ $pnode->{$NODE_CHILDREN} };
        if (@children < 2) {
            error "$TAG_OPTIONAL has less than 2 commands".
                  " at $context_file:".$expat->current_line;
        }
    }
    if ($pnode->{$NODE_TAG} eq $TAG_PARAM_OPTIONS) {
        my @children = @{ $pnode->{$NODE_OPTIONS} };
        if (@children < 2) {
            error "$TAG_PARAM_OPTIONS has less than 2 options".
                  " at $context_file:".$expat->current_line;
        }
    }
}

sub name_to_id
{
    my $token = shift;
    $token =~ s/\W+/_/;
    return uc $token;
}

sub decode_cmd_flags
{
    my $flags = shift;
    my $clist = "";
    foreach my $fbit (keys %g_flags_value_to_cflags) {
        if (($flags & $fbit) == $fbit) {
            if ($clist ne "") {
                $clist .= "|";
            }
            $clist .= $g_flags_value_to_cflags{$fbit};
        }
    }
    
    return $clist;
}

sub add_new_id
{
    my $pnode = shift;
    my $mid = $pnode->{$NODE_ID};
    
    # Check duplicated id
    foreach my $id (@cur_ids) {
        if ($mid eq $id) {
            error "Duplicated ID '$id' at the same command path".
                  " at $context_file:".$pnode->{$NODE_SRCLINE};
        }
    }
    
    # Add to the pool
    my $idname = "XCID_".uc($context_id)."_".uc(name_to_id($cur_cmds[0]))."_".$mid;
    my $top = $all_ids{$cur_cmds[0]};
    if (!defined($top)) {
        my @idlist = ();
        $top = \@idlist;
        $all_ids{$cur_cmds[0]} = $top;
    }
    
    # Check if id is already defined
    foreach my $ex_id ( @{ $top } ) {
        if ($ex_id eq $idname) {
            return $idname;
        }
    }
    
    # Add it if not yet defined
    push @{ $top }, $idname;
    
    return $idname;
}

sub generate_path_code_template
{
    my $path = shift;
    my $remark = shift;
    my $build = 0;
    
    # Build command line only if cmdtype is 'config'
    if (($cur_flags & $CMDFLAG_CMDTYPE_CONFIG) == $CMDFLAG_CMDTYPE_CONFIG) {
        $build = 1;
    }
    
    # Generate handler/builder code template
    my $lines = "    /*\n";
    $lines   .= "     * COMMAND: $remark\n";
    $lines   .= "     */\n";
    $lines   .= "    case $path: {\n\n";
    print OUT_HANDLER $lines;
    if ($build) {
        print OUT_BUILDER $lines; 
    }
    
    foreach my $id (@cur_ids) {
        my $pnode = $cur_vars{$id};
        my $tag = $pnode->{$NODE_TAG};
        my $ctype = $g_tag_handler_c_type_map{$tag};
        my $alt_ctype = $ctype;
        my $ktype = $g_tag_handler_param_type_map{$tag};
        my $postfix = "";
        
        if ($tag eq $TAG_PARAM_CUSTOM) {
            $idtype = $pnode->{$NODE_ATTRS}->{$ATTR_TYPE};
            $ptype = $g_defined_types{$idtype};
            $ctype = $ptype->{$NODE_ATTRS}->{$ATTR_DEF_CTYPE};
            $alt_ctype = $ctype;
            if (defined($ptype->{$NODE_ATTRS}->{$ATTR_DEF_ALT_CTYPE})) {
                $alt_ctype = $ptype->{$NODE_ATTRS}->{$ATTR_DEF_ALT_CTYPE};
            }
            $ktype = $idtype;
        }
        if ($tag eq $TAG_PARAM_INTEGER) {
            my $signed = $pnode->{$NODE_ATTRS}->{$ATTR_SIGNED};
            if (!defined($signed) || lc($signed) eq "no") {
                $ctype = "unsigned ".$ctype;
            }
        }
        if ($tag eq $TAG_PARAM_WORD || $tag eq $TAG_PARAM_LINE) {
            my $memtype = $pnode->{$NODE_ATTRS}->{$ATTR_MEMTYPE};
            if (defined($memtype) && lc($memtype) eq "stack") {
                my $maxlen = $pnode->{$NODE_ATTRS}->{$ATTR_MAXLEN};
                $ctype = "char";
                $alt_ctype = $ctype;
                $postfix = "[".($maxlen + 1)."]";
            }
        }
        $lines = "        $alt_ctype ".lc($id).$postfix."; /* $ktype */\n";
        if ($build) {
            print OUT_BUILDER $lines;
        }

        if ($tag eq $TAG_PARAM_WORD || $tag eq $TAG_PARAM_LINE) {
            my $memtype = $pnode->{$NODE_ATTRS}->{$ATTR_MEMTYPE};
            if (!defined($memtype) || lc($memtype) ne "stack") {
                $ctype = "const ".$ctype;
            }
        }
        $lines = "        $ctype ".lc($id).$postfix."; /* $ktype */\n";
        print OUT_HANDLER $lines;
    }
    if (@cur_ids > 0) {
        print OUT_HANDLER "\n";
        if ($build) {
            print OUT_BUILDER "\n";
        }
    }
    
    if ($build) {
        print OUT_BUILDER "        /* ... Retrieve values from API ... */\n\n";
    }

    foreach my $id (@cur_ids) {
        my $pnode = $cur_vars{$id};
        my $tag = $pnode->{$NODE_TAG};
        my $idname = "XCID_".uc($context_id)."_".uc(name_to_id($cur_cmds[0]))."_".$id;
        
        if ($tag eq $TAG_PARAM_INTEGER || $tag eq $TAG_PARAM_OPTIONS) {
            print OUT_HANDLER
                  "        // XCMD_GET_NUMBER($idname, &".lc($id).");\n";
        } elsif ($tag eq $TAG_PARAM_CUSTOM) {
            print OUT_HANDLER
                  "        // XCMD_GET_VALUE($idname, &".lc($id).");\n";
        } else {
            print OUT_HANDLER
                  "        // XCMD_GET_STRING($idname, &".lc($id).");\n";
        }

        if ($tag eq $TAG_PARAM_OPTIONS) {
            $lines =  "\n        /* Options of \"".lc($id)."\" */\n";
            $lines .=  "        switch(".lc($id).") {\n";
            foreach my $opt (@{ $pnode->{$NODE_OPTIONS} }) {
                my $name = $opt->{$NODE_NAME};
                my $optname = "XCOPT_".uc($context_id)."_".uc(name_to_id($cur_cmds[0])).
                              "_".uc($id)."_".name_to_id($name);
                $lines .= "        case $optname:\n";
                $lines .= "            /* $name */\n";
                $lines .= "            break;\n\n";
            }
            $lines .= "        default:\n";
            $lines .= "            return XCMD_ERR_INTERNAL_INVALID_OPTION;\n";
            $lines .= "        }\n\n";
            print OUT_HANDLER $lines;
        }

        if ($build && $tag eq $TAG_PARAM_OPTIONS) {
            $lines =  "        /* \n";
            $lines .= "         * Options of \"".lc($id)."\":\n";
            foreach my $opt (@{ $pnode->{$NODE_OPTIONS} }) {
                my $name = $opt->{$NODE_NAME};
                my $optname = "XCOPT_".uc($context_id)."_".uc(name_to_id($cur_cmds[0])).
                              "_".uc($id)."_".name_to_id($name);
                $lines .= "         *    $name => $optname\n"
            }
            $lines .= "         */\n";
            print OUT_BUILDER $lines;
        }
        
        
        if ($build) {
            if ($tag eq $TAG_PARAM_INTEGER || $tag eq $TAG_PARAM_OPTIONS) {
                print OUT_BUILDER
                      "        // XCMD_SET_NUMBER($idname, ".lc($id).");\n";
            } elsif ($tag eq $TAG_PARAM_CUSTOM) {
                print OUT_BUILDER
                      "        // XCMD_SET_VALUE($idname, ".lc($id).");\n";
            } else {
                print OUT_BUILDER
                      "        // XCMD_SET_STRING($idname, ".lc($id).");\n";
            }
        }
    }

    if (@cur_ids > 0) {
        print OUT_HANDLER "\n";
        if ($build) {
            print OUT_BUILDER "\n";
        }
    }
    if (@cur_ids == 0) {
        print OUT_HANDLER 
              "        /* ... Call underlying API ... */\n\n";
    } else {
        print OUT_HANDLER 
              "        /* ... Use these values to call API ... */\n\n";
    }
    
    if (@cur_subs > 0) {
        print OUT_HANDLER "        /* Prepare prompt for sub-shell; NULL to use default */\n";
        print OUT_HANDLER "        XCMD_INVOKE_SUB_CONTEXT(prompt, xch);\n\n";
    }

    print OUT_HANDLER "        return\n";
    print OUT_HANDLER "            // XCMD_ERR_OK;\n";
    print OUT_HANDLER "      }\n\n";

    if ($build) {
        print OUT_BUILDER "        return\n";
        print OUT_BUILDER "            // XCMD_ACTION_SET_AND_STOP;\n";
        print OUT_BUILDER "            // XCMD_ACTION_SET_AND_CONTINUE;\n";
        print OUT_BUILDER "            // XCMD_ACTION_SKIP_AND_STOP;\n";
        print OUT_BUILDER "            // XCMD_ACTION_SKIP_AND_CONTINUE;\n";
        print OUT_BUILDER "      }\n\n";
    }
}

sub handle_eol_node
{
    # Collect paths
    my $branches = "XCPATH_".uc($context_id)."_H".@cur_cmds;
    foreach my $cmd (@cur_cmds) {
        if ($cmd eq "<cr>") {
            $branches .= "_cr";
        } else {
            $branches .= "_".name_to_id($cmd);
        }
    }
    my $path = "";
    foreach my $word (@cur_path) {
        $path .= $word." ";
    }
    if (defined($all_paths{$branches})) {
        error "Duplicated path detected: '$branches' in $context_file!";
    }
    $all_paths{$branches} = $path;
    
    # Flags to traverse back
    $cur_flags = $cur_cmdtype[$#cur_cmdtype] | $cur_access[$#cur_access];
    
    # Local flags
    my $flags = $cur_flags;
    if (@cur_marked > 0 && $cur_marked[$#cur_marked] eq "yes") {
        $flags |= $CMDFLAG_BEHAVIOR_MARKED;
    }
    
    # Determine callback
    my $cbkh = "xchandler_".lc($context_id)."_".lc(name_to_id($cur_cmds[0]));
    $all_handlers{$cbkh} = 1;
    my $cbkg = "xcbuilder_".lc($context_id)."_".lc(name_to_id($cur_cmds[0]));
    if (($cur_flags & $CMDFLAG_CMDTYPE_CONFIG) == $CMDFLAG_CMDTYPE_CONFIG) {
        # Require builder callback only if its cmdtype is "config"
        $all_builders{$cbkg} = 2;
    } else {
        $cbkg = "NULL";
    }
    
    # Generate table
    if ($cur_cmds[$#cur_cmds] eq "<cr>") {
        $prev_handler = "xcnode_handler_cmd_eol";
        print OUT_TABLE_C "static const XCNODE_CMD_EOL";
    } else { 
        $prev_handler = "xcnode_handler_param_eol";
        print OUT_TABLE_C "static const XCNODE_PARAM_EOL";
    }
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    $branches,\n";
    print OUT_TABLE_C "    ".decode_cmd_flags($flags).",\n";
    print OUT_TABLE_C "    { $cbkh, $cbkg },\n";
    if (@cur_subs > 0) {
        print OUT_TABLE_C "    &xcmd_context_".$cur_subs[$#cur_subs].",\n";
    } else {
        print OUT_TABLE_C "    NULL,\n";
    }
    print OUT_TABLE_C "};\n\n";
    
    # Generate code templates
    generate_path_code_template($branches, $path);
}

sub go_next
{
    my $pnode = shift;
    my $pnext = $pnode->{$NODE_NEXT};
    if (!defined($pnext)) {
        
        handle_eol_node();
        return;
    }
    my $tag = $pnext->{$NODE_TAG};
    my $fp = $g_context_traverse_map{$tag};
    $fp->($pnext);
}

sub traverse_tag_context
{
    my $pnode = shift;
    $pnode->{$NODE_ID} = uc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    
    # Check cmdtype and access for the whole context
    my $cmdtype = lc $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE};
    push @cur_cmdtype, $g_cmdtype_value_to_flags{$cmdtype};
    my $access = $pnode->{$NODE_ATTRS}->{$ATTR_ACCESS};
    if (defined($access)) {
        push @cur_access, $g_access_value_to_flags{lc($access)};
    } else {
        
        push @cur_access, $CMDFLAG_ACCESS_GUEST;
    }
    
    go_next($pnode);
    
    pop @cur_access;
    pop @cur_cmdtype;
    
    # Generate table
    my $id = lc $pnode->{$NODE_ID};
    print OUT_TABLE_C "const XCNODE_CONTEXT";
    print OUT_TABLE_C " xcmd_context_$id = {\n";
    print OUT_TABLE_C "    \"".$id."\",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 1);
    print OUT_TABLE_C "};\n\n";
}

sub traverse_tag_select
{
    my $pnode = shift;
    my %branches = ();
    my @brlist = (); # To ensure the order is not messed
    my $new_flags = 0;
    foreach my $pnext (@{ $pnode->{$NODE_CHILDREN} }) {
        my $tag = $pnext->{$NODE_TAG};
        my $fp = $g_context_traverse_map{$tag};
        
        # Go to every branches
        $fp->($pnext);
        
        # Collect all possible flags
        $new_flags |= $cur_flags;
        
        if ($tag eq $TAG_CMD_EOL) {
            $branches{$serial_num - 1} = "&xcnode_handler_cmd_eol";
        } else {
            $branches{$serial_num - 1} = "&xcnode_handler_cmd_keyword";
        }
        push @brlist, ($serial_num - 1);
    }
    
    # Update current flags
    $cur_flags = $new_flags;

    # Generate table
    print OUT_TABLE_C "static const XCMD_NODE_PTR";
    printf OUT_TABLE_C " _node%04X[] = {\n", $serial_num++;
    foreach my $sn (@brlist) {
        my $handler = $branches{$sn};
        print OUT_TABLE_C "    { $handler, ";
        printf OUT_TABLE_C "(void *)&_node%04X },\n", $sn;
    }
    print OUT_TABLE_C "};\n\n";
    print OUT_TABLE_C "static const XCNODE_SELECT";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    printf OUT_TABLE_C "    (void *)_node%04X,\n", ($serial_num - 2);
    print OUT_TABLE_C "    ".(keys %branches)."\n";
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_select";
}

sub traverse_tag_optional
{
    my $pnode = shift;
    my %branches = ();
    my @brlist = (); # To ensure the order is not messed
    my $new_flags = 0;
    foreach my $pnext (@{ $pnode->{$NODE_CHILDREN} }) {
        my $tag = $pnext->{$NODE_TAG};
        my $fp = $g_context_traverse_map{$tag};
        
        # Go to every branches
        $fp->($pnext);
        
        # Collect all possible flags
        $new_flags |= $cur_flags;
        
        if ($tag eq $TAG_CMD_EOL) {
            $branches{$serial_num - 1} = "&xcnode_handler_cmd_eol";
        } else {
            $branches{$serial_num - 1} = "&xcnode_handler_cmd_keyword";
        }
        push @brlist, ($serial_num - 1);
    }

    # Add "ALL" path
    {
        # Go to every branches
        &traverse_tag_cmd_keyword;

        # Collect all possible flags
        $new_flags |= $cur_flags;

        $branches{$serial_num - 1} = "&xcnode_handler_cmd_keyword";

        push @brlist, ($serial_num - 1);
    }
    
    # Update current flags
    $cur_flags = $new_flags;

    # Generate table
    print OUT_TABLE_C "static const XCMD_NODE_PTR";
    printf OUT_TABLE_C " _node%04X[] = {\n", $serial_num++;
    foreach my $sn (@brlist) {
        my $handler = $branches{$sn};
        print OUT_TABLE_C "    { $handler, ";
        printf OUT_TABLE_C "(void *)&_node%04X },\n", $sn;
    }
    print OUT_TABLE_C "};\n\n";
    print OUT_TABLE_C "static const XCNODE_OPTIONAL";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    printf OUT_TABLE_C "    (void *)_node%04X,\n", ($serial_num - 2);
    print OUT_TABLE_C "    ".(keys %branches)."\n";
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_optional";
}

sub traverse_tag_cmd_keyword
{
    my $pnode = shift;
    my $attr_name = $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    my $name = $pnode->{$NODE_NAME} = lc $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    my $flags = 0;
    if (!defined($attr_name)) {
        # Used for "ALL" path
        $name = $pnode->{$NODE_NAME} = "all";
    } 

    my $cmdtype = $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE};
    if (defined($cmdtype)) {
        push @cur_cmdtype, $g_cmdtype_value_to_flags{lc($cmdtype)};
        $flags |= $g_cmdtype_value_to_flags{lc($cmdtype)};
    } else {
        $flags |= $CMDFLAG_CMDTYPE_CONFIG;
    }
    my $access = $pnode->{$NODE_ATTRS}->{$ATTR_ACCESS};
    if (defined($access)) {
        push @cur_access, $g_access_value_to_flags{lc($access)};
        $flags |= $g_access_value_to_flags{lc($access)};
    } else {
        $flags |= $CMDFLAG_ACCESS_GUEST;
    }
    my $marked = $pnode->{$NODE_ATTRS}->{$ATTR_MARKED};
    if (defined($marked)) {
        push @cur_marked, lc($marked);
    }
    my $cxt = $pnode->{$NODE_ATTRS}->{$ATTR_CONTEXT};
    if (defined($cxt)) {
        print OUT_TABLE_C "extern XCNODE_CONTEXT xcmd_context_$cxt;\n\n";
        push @cur_subs, $cxt;
    }
    
    push @cur_cmds, $name;
    push @cur_path, $name;
    
    # Generate code templates
    if (@cur_cmds == 1) {
        my $cbkh = "xchandler_".lc($context_id)."_".lc(name_to_id($cur_cmds[0]));
        print OUT_HANDLER "/* \n";
        print OUT_HANDLER " * TOP-LEVEL-COMMAND: $name\n";
        print OUT_HANDLER " */\n";
        print OUT_HANDLER "XCMD_ERROR\n";
        print OUT_HANDLER "$cbkh(int path, XCMD_HANDLE xch)\n";
        print OUT_HANDLER "{\n";
        print OUT_HANDLER "    switch(path) {\n\n";

        my $cbkg = "xcbuilder_".lc($context_id)."_".lc(name_to_id($cur_cmds[0]));
        print OUT_BUILDER "/* \n";
        print OUT_BUILDER " * TOP-LEVEL-COMMAND: $name\n";
        print OUT_BUILDER " */\n";
        print OUT_BUILDER "XCMD_ACTION\n";
        print OUT_BUILDER "$cbkg(int path, unsigned int index, XCMD_HANDLE xch)\n";
        print OUT_BUILDER "{\n";
        print OUT_BUILDER "    switch(path) {\n\n";
    }
    
    go_next($pnode);

    if (@cur_cmds == 1) {
        print OUT_BUILDER "    default:\n";
        print OUT_BUILDER "        return XCMD_ERR_INTERNAL_INVALID_PATH;\n";
        print OUT_BUILDER "    }\n\n";
        print OUT_BUILDER "    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;\n";
        print OUT_BUILDER "}\n\n\n";
        
        print OUT_HANDLER "    default:\n";
        print OUT_HANDLER "        return XCMD_ERR_INTERNAL_INVALID_PATH;\n";
        print OUT_HANDLER "    }\n\n";
        print OUT_HANDLER "    return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;\n";
        print OUT_HANDLER "}\n\n\n";
    }
    
    # Generate table
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    print OUT_TABLE_C "static const XCNODE_CMD_KEYWORD";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    \"".$name."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    ".decode_cmd_flags(($cur_flags & $CMDFLAG_ACCESS_MASK & $CMDFLAG_CMDTYPE_MASK) | $flags).",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 2);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_cmd_keyword";

    pop @cur_path;
    pop @cur_cmds;
    if (defined($cxt)) {
        pop @cur_subs;
    }
    if (defined($marked)) {
        pop @cur_marked;
    }
    if (defined($access)) {
        pop @cur_access;
    }
    if (defined($cmdtype)) {
        pop @cur_cmdtype;
    }
}

sub traverse_tag_cmd_eol
{
    my $pnode = shift;
    
    # <CR> shouldn't appear as first level command
    if (@cur_cmds == 0) {
        error "$TAG_CMD_EOL tag not allowed as first level command".
              " at $context_file:".$pnode->{$NODE_SRCLINE};
    }
    $pnode->{$NODE_NAME} = "<cr>";

    my $cmdtype = $pnode->{$NODE_ATTRS}->{$ATTR_CMDTYPE};
    if (defined($cmdtype)) {
        push @cur_cmdtype, $g_cmdtype_value_to_flags{lc($cmdtype)};
    }
    my $access = $pnode->{$NODE_ATTRS}->{$ATTR_ACCESS};
    if (defined($access)) {
        push @cur_access, $g_access_value_to_flags{lc($access)};
    }
    my $marked = $pnode->{$NODE_ATTRS}->{$ATTR_MARKED};
    if (defined($marked)) {
        push @cur_marked, lc($marked);
    }
    my $cxt = $pnode->{$NODE_ATTRS}->{$ATTR_CONTEXT};
    if (defined($cxt)) {
        print OUT_TABLE_C "extern XCNODE_CONTEXT xcmd_context_$cxt;\n\n";
        push @cur_subs, $cxt;
    }
    push @cur_cmds, $pnode->{$NODE_NAME};
    
    handle_eol_node();
    
    pop @cur_cmds;
    if (defined($cxt)) {
        pop @cur_subs;
    }
    if (defined($marked)) {
        pop @cur_marked;
    }
    if (defined($access)) {
        pop @cur_access;
    }
    if (defined($cmdtype)) {
        pop @cur_cmdtype;
    }
}

sub traverse_tag_param_keyword
{
    my $pnode = shift;
    my $name = $pnode->{$NODE_NAME} = lc $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    push @cur_path, $pnode->{$NODE_NAME};
    go_next($pnode);
    
    # Generate table
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    print OUT_TABLE_C "static const XCNODE_PARAM_KEYWORD";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    \"".$name."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 2);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_param_keyword";
    
    pop @cur_path;
}

sub traverse_tag_param_word
{
    my $pnode = shift;
    my $name = $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    if (!defined($name)) {
        $name = "WORD";
    }
    $pnode->{$NODE_NAME} = $name;
    $pnode->{$NODE_ID} = uc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    my $idname = add_new_id($pnode);
    push @cur_ids, $pnode->{$NODE_ID};
    push @cur_path, "<".$pnode->{$NODE_ID}.":word>";
    $cur_vars{$pnode->{$NODE_ID}} = $pnode;

    go_next($pnode);
    
    # Generate table
    my $minlen = $pnode->{$NODE_ATTRS}->{$ATTR_MINLEN};
    if (!defined($minlen)) {
        $minlen = 1;
    }
    my $maxlen = $pnode->{$NODE_ATTRS}->{$ATTR_MAXLEN};
    if (!defined($maxlen)) {
        $maxlen = 65535;
    }
    my $memtype = $pnode->{$NODE_ATTRS}->{$ATTR_MEMTYPE};
    my $mtype = 0;
    if (defined($memtype) && lc($memtype) eq 'stack') {
        $mtype = 1;
    }
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    print OUT_TABLE_C "static const XCNODE_PARAM_WORD";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    ".$idname.",\n";
    print OUT_TABLE_C "    \"".$name."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    ".$minlen.",\n";
    print OUT_TABLE_C "    ".$maxlen.",\n";
    print OUT_TABLE_C "    ".$mtype.",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 2);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_param_word";

    delete $cur_vars{$pnode->{$NODE_ID}};
    pop @cur_path;
    pop @cur_ids;
}

sub traverse_tag_param_line
{
    my $pnode = shift;
    my $name = $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    if (!defined($name)) {
        $name = "LINE";
    }
    $pnode->{$NODE_NAME} = $name;
    $pnode->{$NODE_ID} = uc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    my $idname = add_new_id($pnode);
    push @cur_ids, $pnode->{$NODE_ID};
    push @cur_path, "<".$pnode->{$NODE_ID}.":line>";
    $cur_vars{$pnode->{$NODE_ID}} = $pnode;

    go_next($pnode);
    
    # Generate table
    my $minlen = $pnode->{$NODE_ATTRS}->{$ATTR_MINLEN};
    if (!defined($minlen)) {
        $minlen = 0;
    }
    my $maxlen = $pnode->{$NODE_ATTRS}->{$ATTR_MAXLEN};
    if (!defined($maxlen)) {
        $maxlen = 65535;
    }
    my $memtype = $pnode->{$NODE_ATTRS}->{$ATTR_MEMTYPE};
    my $mtype = 0;
    if (defined($memtype) && lc($memtype) eq 'stack') {
        $mtype = 1;
    }
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    print OUT_TABLE_C "static const XCNODE_PARAM_LINE";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    ".$idname.",\n";
    print OUT_TABLE_C "    \"".$name."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    ".$minlen.",\n";
    print OUT_TABLE_C "    ".$maxlen.",\n";
    print OUT_TABLE_C "    ".$mtype.",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 2);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_param_line";

    delete $cur_vars{$pnode->{$NODE_ID}};
    pop @cur_path;
    pop @cur_ids;
}

sub traverse_tag_param_custom
{
    my $pnode = shift;
    my $name = $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    my $typeid = lc $pnode->{$NODE_ATTRS}->{$ATTR_TYPE};
    my $ptype = $g_defined_types{$typeid};
    my $form = lc $ptype->{$NODE_ATTRS}->{$ATTR_DEF_FORMAT};
    if (!defined($name)) {
        if ($form eq 'line') {
            $name = "LINE";
        } else {
            $name = "WORD";
        }
    }
    
    $pnode->{$NODE_NAME} = $name;
    $pnode->{$NODE_ID} = uc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    my $idname = add_new_id($pnode);
    push @cur_ids, $pnode->{$NODE_ID};
    push @cur_path, "<".$pnode->{$NODE_ID}.">";
    $cur_vars{$pnode->{$NODE_ID}} = $pnode;

    go_next($pnode);
    
    # Generate table
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    my $nform = ($form eq "line") ? 1 : 0;
    print OUT_TABLE_C "static const XCNODE_PARAM_CUSTOM";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    ".$idname.",\n";
    print OUT_TABLE_C "    \"".$name."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    &xccvt_".$typeid."_converters,\n";
    print OUT_TABLE_C "    ".$nform.",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 2);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_param_custom";

    delete $cur_vars{$pnode->{$NODE_ID}};
    pop @cur_path;
    pop @cur_ids;
}

sub traverse_tag_param_integer
{
    my $pnode = shift;

    
    my $min = $pnode->{$NODE_ATTRS}->{$ATTR_MIN};
    my $max = $pnode->{$NODE_ATTRS}->{$ATTR_MAX};
    
    my $name = $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    if (!defined($name)) {
        $name = "<$min..$max>";
    }
    $pnode->{$NODE_NAME} = $name;
    $pnode->{$NODE_ID} = uc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    my $idname = add_new_id($pnode);

    push @cur_ids, $pnode->{$NODE_ID};
    push @cur_path, "<".$pnode->{$NODE_ID}.":integer>";
    $cur_vars{$pnode->{$NODE_ID}} = $pnode;

    go_next($pnode);
    
    # Generate table
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    my $signed = $pnode->{$NODE_ATTRS}->{$ATTR_SIGNED};
    if (!defined($signed)) {
        $signed = "no";
    }
    my $radix = $pnode->{$NODE_ATTRS}->{$ATTR_RADIX};
    if (!defined($radix)) {
        $radix = "dec";
    }
    print OUT_TABLE_C "static const XCNODE_PARAM_INTEGER";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    ".$idname.",\n";
    print OUT_TABLE_C "    \"".$name."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    ".$min.(($signed eq "yes")? "L" : "UL").",\n";
    print OUT_TABLE_C "    ".$max.(($signed eq "yes")? "L" : "UL").",\n";
    print OUT_TABLE_C "    ".(($signed eq "yes")? 1 : 0).",\n";
    print OUT_TABLE_C "    ".(($radix eq "hex")? 16 : 10).",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 2);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_param_integer";

    delete $cur_vars{$pnode->{$NODE_ID}};
    pop @cur_path;
    pop @cur_ids;
}

sub traverse_tag_param_options
{
    my $pnode = shift;
    $pnode->{$NODE_ID} = uc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    my $idname = add_new_id($pnode);
    push @cur_ids, $pnode->{$NODE_ID};
    push @cur_path, "<".$pnode->{$NODE_ID}.":options>";
    $cur_vars{$pnode->{$NODE_ID}} = $pnode;
    
    # Traverse to options
    my @options = ();
    foreach my $pnext (@{ $pnode->{$NODE_OPTIONS} }) {
        my $tag = $pnext->{$NODE_TAG};
        my $fp = $g_context_traverse_map{$tag};
        $fp->($pnext);
        
        push @options, ($serial_num - 1);
    }
    
    go_next($pnode);
    

    # Generate table
    print OUT_TABLE_C "static const XCMD_NODE_PTR";
    printf OUT_TABLE_C " _node%04X[] = {\n", $serial_num++;
    foreach my $sn (@options) {
        print OUT_TABLE_C "    { &xcnode_handler_option, ";
        printf OUT_TABLE_C "(void *)&_node%04X },\n", $sn;
    }
    print OUT_TABLE_C "};\n\n";
    print OUT_TABLE_C "static const XCNODE_PARAM_OPTIONS";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    ".$idname.",\n";
    printf OUT_TABLE_C "    (void *)_node%04X,\n", ($serial_num - 2);
    print OUT_TABLE_C "    ".(@options).",\n";
    print OUT_TABLE_C "    { &$prev_handler, ";
    printf OUT_TABLE_C "(void *)&_node%04X }\n", ($serial_num - 3);
    print OUT_TABLE_C "};\n\n";
    $prev_handler = "xcnode_handler_param_options";
    
    delete $cur_vars{$pnode->{$NODE_ID}};
    pop @cur_path;
    pop @cur_ids;
}

sub traverse_tag_option
{
    my $pnode = shift;
    $pnode->{$NODE_NAME} = lc $pnode->{$NODE_ATTRS}->{$ATTR_NAME};
    
    # Collect options
    my $name = name_to_id($pnode->{$NODE_NAME});
    my $optname = "XCOPT_".uc($context_id)."_".uc(name_to_id($cur_cmds[0])).
                  "_".uc($cur_ids[$#cur_ids])."_".$name;
    my $top = $all_opts{$cur_cmds[0]};
    if (!defined($top)) {
        my @optlist = ();
        $top = \@optlist;
        $all_opts{$cur_cmds[0]} = $top;
    }
    push @{ $top }, $optname;
    
    # Generate table
    my $desc = $pnode->{$NODE_ATTRS}->{$ATTR_DESC};
    if (!defined($desc)) {
        $desc = "";
    }
    print OUT_TABLE_C "static const XCNODE_OPTION";
    printf OUT_TABLE_C " _node%04X = {\n", $serial_num++;
    print OUT_TABLE_C "    \"".lc($name)."\",\n";
    print OUT_TABLE_C "    \"".$desc."\",\n";
    print OUT_TABLE_C "    ".$optname.",\n";
    print OUT_TABLE_C "};\n\n";
}

sub traverse_context_tree
{
    my $pnode = shift;
    local @cur_path = ();       # Friendly comment for the path
    local @cur_ids = ();        # IDs in the path
    local %cur_vars = ();       # Variables in the path
    local @cur_cmds = ();       # Branches in the path
    local @cur_subs = ();       # Sub contexts in the path
    local @cur_cmdtype = ();    # Cmdtype spread from upper layer
    local @cur_access = ();     # Access spread from upper layer
    local @cur_marked = ();     # Marked spread from upper layer
    local $cur_flags = 0;       # All possible flags for this branch down
    local %all_paths = ();      # All path in this context
    local %all_handlers = ();   # All handlers
    local %all_builders = ();   # All builders
    local %all_ids = ();        # Map to ID list for each top-level command
    local %all_opts = ();       # Map to option list for each top-level command

    local $context_id = lc $pnode->{$NODE_ATTRS}->{$ATTR_ID};
    local $serial_num = 0;
    local $prev_handler = "";
    
    my $out_h_file = "xccxt_".$context_id."_enums.h";
    my $fname = basename($context_file);
    $fname =~ s/\.xml//;

    my $out_table_file = "xccxt_table_".$fname.".c";
    open(OUT_TABLE_C, ">".$g_output_dir."generated/".$out_table_file)
        || error "Cannot create context table file $out_table_file!";
    binmode OUT_TABLE_C;
    print OUT_TABLE_C "$copy_right";
    print OUT_TABLE_C "#include \"system.h\"\n\n";
    print OUT_TABLE_C "#ifdef CFG_XCOMMAND_INCLUDED\n\n";
    print OUT_TABLE_C "#include \"appl/xcmd/xcmd_internal.h\"\n";
    print OUT_TABLE_C "#include \"$out_h_file\"\n";
    print OUT_TABLE_C "\n";
    foreach my $typeid (keys %g_defined_types) {
        print OUT_TABLE_C 
            "extern XCMD_TYPE_CONVERTER xccvt_".$typeid."_converters;\n";
    }

    my $out_handler_file = "xccxt_".$context_id."_handlers.c";
    open(OUT_HANDLER, ">".$g_output_dir."generated/".$out_handler_file)
        || error "Cannot create output handler file $out_handler_file!";
    binmode OUT_HANDLER;
    print OUT_HANDLER "$copy_right";
    print OUT_HANDLER "#include \"system.h\"\n\n";
    print OUT_HANDLER "#ifdef CFG_XCOMMAND_INCLUDED\n\n";
    print OUT_HANDLER "#include \"appl/xcmd/xcmd_public.h\"\n";
    print OUT_HANDLER "#include \"generated/$out_h_file\"\n";
    print OUT_HANDLER "\n";
    print OUT_HANDLER "#if 0 /* IT'S JUST A TEMPLATE! */\n\n";
    
    my $out_builder_file = "xccxt_".$context_id."_builders.c";
    open(OUT_BUILDER, ">".$g_output_dir."generated/".$out_builder_file)
        || error "Cannot create output builder file $out_builder_file!";
    binmode OUT_BUILDER;
    print OUT_BUILDER "$copy_right";
    print OUT_BUILDER "#include \"system.h\"\n\n";
    print OUT_BUILDER "#ifdef CFG_XCOMMAND_INCLUDED\n\n";
    print OUT_BUILDER "#include \"appl/xcmd/xcmd_public.h\"\n";
    print OUT_BUILDER "#include \"generated/$out_h_file\"\n";
    print OUT_BUILDER "\n";
    print OUT_BUILDER "#if 0 /* IT'S JUST A TEMPLATE! */\n\n";
    
    traverse_tag_context($pnode);
    
    print OUT_BUILDER "#endif /* 0 */\n";
    print OUT_HANDLER "#endif /* 0 */\n";
    
    print OUT_BUILDER "\n#endif /* CFG_XCOMMAND_INCLUDED */\n";
    print OUT_HANDLER "\n#endif /* CFG_XCOMMAND_INCLUDED */\n";
    print OUT_TABLE_C "#endif /* CFG_XCOMMAND_INCLUDED */\n";
    close(OUT_BUILDER);
    close(OUT_HANDLER);
    close(OUT_TABLE_C);
    
    #
    # Generate H file for all enums
    #
    open(OUT_CXT_H, ">".$g_output_dir."generated/".$out_h_file)
        || error "Cannot create output header file $out_h_file!";
    binmode OUT_CXT_H;
    print OUT_CXT_H "$copy_right";
    print OUT_CXT_H "#ifndef _XCCXT_".uc($context_id)."_ENUMS_H_\n";
    print OUT_CXT_H "#define _XCCXT_".uc($context_id)."_ENUMS_H_\n\n";
    print OUT_CXT_H "#include \"system.h\"\n\n";
    print OUT_CXT_H "#ifdef CFG_XCOMMAND_INCLUDED\n\n";
        
    # Generate paths
    print OUT_CXT_H "/*\n";
    print OUT_CXT_H " * All possible command paths\n";
    print OUT_CXT_H " */\n";
    print OUT_CXT_H "enum {\n\n";
    foreach my $path (keys %all_paths) {
        my $desc = $all_paths{$path};
        print OUT_CXT_H "    /* $desc */\n";
        print OUT_CXT_H "    $path,\n\n";
    }
    print OUT_CXT_H "};\n\n\n";
    
    # Generate IDs
    foreach my $cmd (keys %all_ids) {
        my $top = $all_ids{$cmd};
        print OUT_CXT_H "/*\n";
        print OUT_CXT_H " * Variant parameter IDs for command \"$cmd\"\n";
        print OUT_CXT_H " */\n";
        print OUT_CXT_H "enum {\n";
        foreach my $id (@{ $top }) {
            print OUT_CXT_H "    $id,\n";
        }
        print OUT_CXT_H "};\n\n\n";
    }
    
    # Generate options
    foreach my $cmd (keys %all_opts) {
        my $top = $all_opts{$cmd};
        print OUT_CXT_H "/*\n";
        print OUT_CXT_H " * All available options for command \"$cmd\"\n";
        print OUT_CXT_H " */\n";
        print OUT_CXT_H "enum {\n";
        foreach my $id (@{ $top }) {
            print OUT_CXT_H "    $id,\n";
        }
        print OUT_CXT_H "};\n\n\n";
    }
    
    # Generate callback prototypes
    print OUT_CXT_H "/*\n";
    print OUT_CXT_H " * All callbacks function that should be implemented\n";
    print OUT_CXT_H " */\n";
    foreach my $cbkh (keys %all_handlers) {
        print OUT_CXT_H "extern XCMD_ERROR $cbkh(int, XCMD_HANDLE);\n";
    }
    foreach my $cbkg (keys %all_builders) {
        print OUT_CXT_H "extern XCMD_ACTION $cbkg(int, unsigned int, XCMD_HANDLE);\n";
    }

    print OUT_CXT_H "\n#endif /* CFG_XCOMMAND_INCLUDED */\n";
    
    print OUT_CXT_H "\n#endif /* _XCCXT_".uc($context_id)."_ENUMS_H_ */\n";
    close(OUT_CXT_H);
}

sub parse_context
{
    local $context_file = shift;
    local @node_stack = ();
    local $context_node = undef;
    local $prev_node = undef;
    
    open(IN_CONTEXT, $context_file) 
        || error "Cannot open input context file $context_file!";
    binmode IN_CONTEXT;
   
    $g_output_dir = dirname($context_file) . "/";
    my $parser = new XML::Parser(ErrorContext => 2);
    $parser->setHandlers(Start => \&start_handler, 
                         Char => \&char_handler,
                         End => \&end_handler);
    $parser->parse(*IN_CONTEXT);
    close(IN_CONTEXT);
    
    if (!defined($context_node)) {
        error "File $context_file contains no $TAG_CONTEXT tag!";
    }
    
    traverse_context_tree($context_node);
}

sub validate_def_attr_format
{
    my $el = shift;
    my $at = shift;
    my $val = shift;
    if (lc($val) eq "word" ||
        lc($val) eq "line") {
        return 1;
    }
    return 0;
}

sub process_defines_tag_definitions
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};

    if (@node_stack > 0) {
        error "Invalid tag '$tag' found".
              " at $current_file:".$expat->current_line;
    }
}

sub process_defines_tag_deftype
{
    my $pnode = shift;
    my $tag = $pnode->{$NODE_TAG};
    my %attrs = %{ $pnode->{$NODE_ATTRS} };
    my $id = lc $attrs{$ATTR_DEF_ID};
    
    if (@node_stack != 1) {
        error "Invalid tag '$tag' found".
              " at $current_file:".$expat->current_line;
    }
    if (defined($g_defined_types{$id})) {
        error "Duplicated id '$id'".
              " at $current_file:".$expat->current_line;
    }

    $g_defined_types{$id} = $pnode;
}

sub def_start_handler
{
    local $expat = shift;
    my $tag = uc shift;
    my @args = @_;
    
    # Check tag name
    if (!defined($g_defines_tag_attribute_map{$tag})) {
        error "Invalid tag '$tag' found".
              " at $current_file:".$expat->current_line;
    }

    # Create a new node
    my %node = (
        $NODE_TAG => $tag,
        $NODE_ATTRS => {},
        $NODE_SRCLINE => $expat->current_line,
    );
    
    # Get attributes
    my %attmap = %{ $g_defines_tag_attribute_map{$tag} };
    while(@args) {
        my $attr = shift(@args);
        my $value = shift(@args);
        
        # Check attribute
        if (!defined($attmap{$attr})) {
            error "Invalid attribute '$attr' found".
                  " at $current_file:".$expat->current_line;
        }

        # Check attribute value
        my $func = $g_defines_attr_validators{$attr};
        if (defined($func)) {
            if (!$func->($tag, $attr, $value)) {
                error "Invalid value '$value' for '$attr'".
                      " at $current_file:".$expat->current_line;
            }
        } else {
            error "Invalid attribute '$attr' found".
                  " at $current_file:".$expat->current_line;
        }
        
        # Add this attribute to the node
        $node{$NODE_ATTRS}->{$attr} = $value;
    }
    my %attrs = %{ $node{$NODE_ATTRS} };
    
    # Check required attributes are present
    foreach my $attr (keys %attmap) {
        if ($attmap{$attr} == ATTR_REQUIRED) {
            if (!defined($attrs{$attr})) {
                error "Required attribute '$attr' for tag '$tag' not found".
                      " at $current_file:".$expat->current_line;
            }
        }
    }
    
    # Tag-level checking and processing
    my $tagf = $g_defines_processor_map{$tag};
    if (defined($tagf)) {
        $tagf->(\%node);
    } else {
        error "Invalid tag '$tag' found".
              " at $current_file:".$expat->current_line;
    }

    # Everything OK, push node the stack
    push @node_stack, \%node;
}

sub def_char_handler
{
    my $expat = shift;
    my $body = shift;
    $body =~ s/\s*//;
    if ($body) {
        error "Garbage body content present".
              " at $current_file:".$expat->current_line;
    }
}

sub def_end_handler
{
    local $expat = shift;
    my $tag = uc shift;
    pop @node_stack;
}

sub parse_definitions
{
    local $current_file = shift;
    local @node_stack = ();
    
    open(IN_DEFINES, $current_file) 
        || error "Cannot open input context file $context_file!";
    binmode IN_DEFINES;
    
    my $parser = new XML::Parser(ErrorContext => 2);
    $parser->setHandlers(Start => \&def_start_handler, 
                         Char => \&def_char_handler,
                         End => \&def_end_handler);
    $parser->parse(*IN_DEFINES);
    close(IN_DEFINES);
}

######################################################
#
# Main program begins
# 
######################################################

if (@ARGV == 0 || @ARGV > 2) {
    error "Usage: ".__FILE__." <context_table_xml> [<defintions_xml>]";
}

if (@ARGV == 2) {
    parse_definitions($ARGV[1]);
}

parse_context($ARGV[0]);

