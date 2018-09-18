#!/usr/bin/perl -w
#
# $Id: config2h.pl,v 1.5 Broadcom SDK $
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


use strict;
use warnings;


######################################################
#
#         Global 
# 
######################################################

use constant { true => 1, false => 0 };

# config type for context
our $CONFIG_VAR_TYPE_UNDEF = "undef";
our $CONFIG_VAR_TYPE_STRING = "string";
our $CONFIG_VAR_TYPE_BOOL   = "bool";
our $CONFIG_VAR_TYPE_HEXDECIMAL = "hex";
our $CONFIG_VAR_TYPE_DECIMAL = "dec";

######################################################
#
# Copyright
#
######################################################
our $copy_right = "/*\n * \$Id\$\n *\n * \$Copyright: (c) 2014 Broadcom Corp.\n * All Rights Reserved.\$\n *\n */\n\n";


######################################################
#
#          Util
# 
######################################################


sub config_parse_var_is_decimal
{
  my $var = shift;
	if ($var =~ /^\d+$/) {
		  return true;
  }
  return false;
}

sub config_parse_var_is_hexdecimal
{
  my $var = shift;
  if ($var =~ /^0[xX][0-9A-Fa-f]+$/) {
		  return true;
  }
  return false;
	
}

sub config_parse_var_is_bool
{
  my $var = shift;
	if ($var =~ /^[01]$/) {
		  return true;
  }
  return false;
  	
}

sub config_parse_line_is_comment
{
   my $line = shift;
   my $ret = false; 
   if ($line =~ /^[\s\t]*#.*/) {
       $ret = true;
   }
   return $ret; 
}

sub config_parse_line_var
{
   my $line = shift;
   my %var;
   my $var_value; 
   $var{"type"} = $CONFIG_VAR_TYPE_UNDEF; 
  
   if ($line =~ /^[\s\t]*(\w+)[\s\t]*\?{0,1}=[\s\t]*(\w+)/) {
      
      $var{"name"} = $1;
      $var{"value"} = $2;
      $var_value = $2;

      if (!defined($var{"name"}) || !defined($var_value)) { 
      	  return undef;
      }

      if ($var{"name"} eq "" ||
          $var_value eq "") 
      { 
      	  return undef;
      }
      
      if (config_parse_var_is_bool($var_value)) {
          $var{"type"} = $CONFIG_VAR_TYPE_BOOL;
      } elsif (config_parse_var_is_decimal($var_value)) {
          $var{"type"} = $CONFIG_VAR_TYPE_DECIMAL;  	
      } elsif (config_parse_var_is_hexdecimal($var_value)) {
          $var{"type"} = $CONFIG_VAR_TYPE_HEXDECIMAL;  	   	
      } else {
          $var{"type"} = $CONFIG_VAR_TYPE_STRING;
      }
      #print "$line : " . $var{"name"} . " " . $var{"value"} . " " . $var{"type"} .  "\n";   
      return \%var;
   }  else {
   	  return undef;
   }
   
   
}

sub config_parse
{
   my $file_name = shift;
   my $row = 1;
   my $CONFIG_HANDLE;
   my $var;
   my @var_list;
   # open config file 
   open($CONFIG_HANDLE, '<:encoding(UTF-8)' , $file_name) or die("Can not open config file: $file_name ");
   
   # parse config file 
   while (my $line = <$CONFIG_HANDLE>) {
         
          $row = $row + 1;
          chomp $line;
          if (config_parse_line_is_comment($line) == true) {
              next; 
          }
          $var = config_parse_line_var($line);         
          
          if (defined($var)) {
              push @var_list, $var;
          } 
   }

   close($CONFIG_HANDLE); 
   return @var_list;
} 

sub config_to_header_file 
{
   my $config_file_name = shift;
   my $config_header_name = shift;
   my @var_list;
   my $HEADER_HANDLE;
   my $var;

  
   @var_list = config_parse($config_file_name);
   # open config file 
   open($HEADER_HANDLE, ">$config_header_name") or die("Can not open config file: $config_header_name ");

   print $HEADER_HANDLE "$copy_right";
   print $HEADER_HANDLE "/*\n";
   print $HEADER_HANDLE " *\n";
   print $HEADER_HANDLE " *  Auto generated base on .config. Please do not edit me \n";  
   print $HEADER_HANDLE " *\n";
   print $HEADER_HANDLE " */\n";
   foreach $var (@var_list)
   { 
     if ($var->{"type"} eq $CONFIG_VAR_TYPE_STRING) {
         	   printf $HEADER_HANDLE "#define %-30s \"%s\"\n", $var->{"name"}, $var->{"value"};
     } else {
         	   printf $HEADER_HANDLE "#define %-30s %s\n", $var->{"name"}, $var->{"value"};
     }
     
   } 
   
   close($HEADER_HANDLE);

}

config_to_header_file(".config", "conf.h");
