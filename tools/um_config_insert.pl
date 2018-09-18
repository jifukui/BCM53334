#!/usr/bin/perl
#
# $Id: um_config_insert.pl,v 1.15 Broadcom SDK $
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

################################################################################
#
# um_config_insert.pl
#
# This script is used to convert config.um to config.um.bin and insert "config.um.bin" 
# 
# into prebuild flash image for umweb and umplus inside ex: bcm95340x.image
# 
# 
#use strict;
use Getopt::Long;
use File::Basename;
use Carp qw( croak );
$script_name = basename(__FILE__);
local $verbose = 0;

################################################################################
#
# sub:   error ($error_string)
#
# Display how to use this script
#
sub error
{
	  my ($error_string) = @_;
	  die  $script_name . " error:" . $error_string . "\n";
#    croak  $script_name . ": " . $error_string . "\n";
}

################################################################################
#
# sub:   error ($error_string)
#
# Display how to use this script
#
sub verbose
{
	  my ($verbose_string) = @_;
    if ($verbose) {
    	  print $verbose_string;
    }
}


################################################################################
#
# sub:     help
#
# Display how to use this script
#
sub usage
{
    print "Usage: "; 
    print $script_name . " -image <image_file> [options] \n"; 
    print "      -image, -i: specify the image file where the config will be inserted \n";
    print "      -config, -c: optionally specify config file name, default name is \"config\.um\"\n";
    #print "      -ledhex: optional specify the led hex file will be attach with config\n";
    print "      -force: force overwrite previous config of the image file\n";
    print "      -verbose: show more debug log\n";
    print "      -generate, -g: only generate the config binary file for web update\n";
    print "      -h: show usage\n";
    exit; 
}
#use strict;

################################################################################
#
# sub:    char * bcm_config_hexfile_to_hexstring($hexfile)
#
# Parse hex file and return it as hex string format
#
sub bcm_config_hexfile_to_hexstring {
	
	   my ($file) = @_;	   
	   my $binary_text;
	   my $line;
	   my $lineno = 1;	   
	 
	   unless(open (HEXFILE, "<", $file)) {
	   	   print "can not open $file\n"; 	   	
	   	   return undef;                 	   	
	   };
	   
	   while(<HEXFILE>) {
	         s/\s+//g;
	         $line = $_;
	         if ($line =~ /[^0-9a-fA-F]/g) {
	         	   error "Load hex file fail at $file line:$lineno\n";
	             return undef;
	         }	else {
		             $binary_text = $binary_text . $line;	             
	         }
	         $lineno ++;
	   }	   
	   close HEXFILE;
	   $binary_text = "0x" . $binary_text;
	   print $file . " is loaded\n";
     return $binary_text;
}

################################################################################
#
# sub:    char * bcm_config_load_hexfile($hexfile)
#
# Parse hex file and return it as hex string format
#
sub bcm_default_config_parser {
    
    my ($bcm_config_file, $hash) = @_;
    my $lineno = 0;       
    open (BCMFILE, $bcm_config_file) || error "could not open config file [$bcm_config_file]\n";
    print "Perform $bcm_config_file Parsing ... \n";
    while(<BCMFILE>) {     
    	 chomp;
    	 $lineno++;
       if ($_ =~ /^[\s]*\#/g) {       	 
           next;
       };
       # Ignore empty line
       if ($_ =~ /^[\s]*$/g) {       	 
           next;
       };
       
       
       if ($_ =~ m/^[\s]*([\w\.]+)[\s]*=[\s]*([\S ]+)/g) {
     	     $attr = $1;
     	     $val = $2;
     	     chomp $attr;
     	     chomp $val;
     	     # eliminate the comment at the tail of line 
     	     $val =~ s/\#(.)*$//g;     	     
     	     if ($val =~ /=/g  || $name =~/=/g ) {
     	         error "config parse fail at $bcm_config_file line: $lineno";
             }
             # eliminate space before or after ,- 
     	     $val =~ s/[\s]*,[\s]*/,/g;
     	     $val =~ s/[\s]*-[\s]*/-/g;
             # eliminate the space at the end 
             $val =~ s/[\s]+$//;
     	     $attr =~ s/[\s]+$//;
     	     if ($val =~ /[ ]+/g || $name =~ /[ ]+/g) {
     	         error "config parse fail at $bcm_config_file line: $lineno";
     	     }
     	     if ($name =~ /#/) {
     	         error "config parse fail at $bcm_config_file line: $lineno";     	     	
     	     }

     	     print $attr . "=>" . $val . "\n";
           if ($val =~ /\.hex$/) {
        	     $tmp = bcm_config_hexfile_to_hexstring($val);
        	     if ($tmp eq undef) {
                   error "config parse fail at $bcm_config_file line: $lineno";
                   close BCMFILE;
        	  	     return -1;
        	     }
        	     $val = $tmp;
           }
           
           $hash->{$attr} = $val;
       } else {
           error "config parse fail at $bcm_config_file line: $lineno";
           close BCMFILE;
       }
       
    }
    if (keys %$hash == 0) {
    	  error "no valid config item found at $bcm_config_file";
    }
    
    print "====================================================================\n";
    close BCMFILE;
    
    return 0;
}


################################################################################
#
# sub:    char * bcm_calculate_chksum_from_uint8($val8, $previous_chksum)
#
# Parse hex file and return it as hex string format
#
local $kcount = 0;
local $kval16 = 0;

sub bcm_calculate_chksum_from_uint8
{
    my ($val8, $previous_chksum, $drain) = @_;	  
    my @character;
    
    if ($drain ne undef) {
    	  if ($count == 0) {
    	  	  $kval16 = 0;
    	  	  $kcount = 0;
            return $previous_chksum;
        }
    }	  
	  if ($previous_chksum eq undef) {
	  	  $previous_chksum = 0;
	  }

    if ($kcount == 1) {    	  
     	  $kval16 = $kval16 * 256 + $val8;
     	  $kcount = 0;   
        $previous_chksum += ($kval16 %  65536);
        if ($previous_chksum >= 65536) {	          
            $previous_chksum = $previous_chksum + 1;	      
	      } 
        $previous_chksum = $previous_chksum % 65536;	      	      
	      $kval16 = 0;
    } else {
    	  $kval16 = $val8 % 256;
    	  $kcount = 1;    
    }
    return $previous_chksum;
}
################################################################################
#
# sub:    char * bcm_calculate_chksum_from_string($string,$previous_chksum)
#
# Parse hex file and return it as hex string format
#
sub bcm_calculate_chksum_from_string
{
	  my ($string, $previous_chksum) = @_;
	  my @character;
	  
	  foreach $char (split("", $string)) {
	  	      $previous_chksum = bcm_calculate_chksum_from_uint8(ord($char), $previous_chksum);
	  }	  
	  return $previous_chksum;
}
################################################################################
#
# sub:    char * bcm_calculate_chksum_from_uint32($val32,$previous_chksum)
#
# Parse hex file and return it as hex string format
#
sub bcm_calculate_chksum_from_uint32
{
	  my ($val32, $previous_chksum) = @_;	  
	  my @character;
	  if ($previous_chksum eq undef) {
	  	  $previous_chksum = 0;
	  }
    $previous_chksum = bcm_calculate_chksum_from_uint8(($val32 %  256), $previous_chksum);
    $previous_chksum = bcm_calculate_chksum_from_uint8(($val32 / 256) % 256, $previous_chksum);
    $previous_chksum = bcm_calculate_chksum_from_uint8(($val32 / 65536) % 256, $previous_chksum);
    $previous_chksum = bcm_calculate_chksum_from_uint8(($val32 / 16777216), $previous_chksum);
	  return $previous_chksum;
}
################################################################################
#
# sub:    char * bcm_calculate_chksum_from_uint32($val32,$previous_chksum)
#
# Parse hex file and return it as hex string format
#

################################################################################
#
# sub:    char * bcm_config_binary_file_generate($hexfile)
#
# Parse hex file and return it as hex string format
#
sub bcm_config_binary_file_generate {
 	
    my ($variables, $parameters) = @_;
    my $total_len = 0;
    my @items;  
    my $item;
    my $binfile;
    my %item_chksums;
    my %item_string;
    
    #if ($parameters->{"leds"} ne undef) {
    #    @items = split(/ /, $parameters->{"leds"});
    #}
    #foreach $item (@items) {
    #        if ($variables->{$item} eq undef) {
    #            $variables->{$item} = bcm_config_hexfile_to_hexstring($item);
    #        } else {
    #        	  error "hexfile " . $item . "is loaded before";
    #        }
    #}
 	  
   	#
   	# Calculate Item Check Sum
   	#  
	  foreach my $key (keys %$variables) {
	  	  $string = $key . "=" . $variables->{$key};
	  	  $item_chksum = 0;
	  	  $item_chksum = bcm_calculate_chksum_from_string($string, $item_chksum);
	  	  $item_chksum = bcm_calculate_chksum_from_uint8(0, $item_chksum);      
	  	  $item_chksum = bcm_calculate_chksum_from_uint8(0, $item_chksum, 1);	 	
	  	  $item_chksum_string = sprintf("%04X", $item_chksum);
        $item_string{$key} = $item_chksum_string . $string;
	  }	 	  
  
   	#
   	# Calculate Totoal Length
   	#
   	$total_len  = 0;
	  foreach my $key (keys %$variables) {
	  	  $total_len = $total_len + length($item_string{$key}) + 1;
	  }	     

   	#
   	# Calculate Total Check Sum
   	#  	  
   	$chksum = 0;
   	$chksum = bcm_calculate_chksum_from_uint32($total_len, $chksum);
   	$chksum = bcm_calculate_chksum_from_uint32(1, $chksum);

	  foreach my $key (keys %$variables) {
	  	  $chksum = bcm_calculate_chksum_from_string($item_string{$key}, $chksum);
	  	  $chksum = bcm_calculate_chksum_from_uint8(0, $chksum);	 	  	  
	  }	 	  
	  $chksum = bcm_calculate_chksum_from_uint8(0, $chksum, 1);	  	  

    $binfile = $parameters->{"config"} . "\.bin";
    open (BIN_FILE, ">", $binfile) || error "could not open file[$binfile]\n";
    binmode(BIN_FILE);	  
	  
    
   	#
   	# Output check sum and data into file
   	#  	  
	  printf BIN_FILE ("%s", "FLSH");  	  
	  print BIN_FILE pack("I",$chksum);
	  print BIN_FILE pack("I",$total_len);
	  print BIN_FILE pack("I",1);	  
	  foreach my $key (keys %$variables) {
	  	  printf BIN_FILE ("%s", $item_string{$key});
	  	  print BIN_FILE pack("C",0);
	  }	   	  	  	  	  	  	  
	  close BIN_FILE;	   
}

################################################################################
#
# sub:    void bcm_image_info_get($image_file_name, \%firmware_info)
#
# Parse firmware information at the beginning of image
#

sub bcm_image_info_get {
	   my ($image_file_name, $firmware_info) = @_;
	   my $flag, $chksum, $len, $val32, $string;
	   
       if(defined($generate_bin_file)) {
           return;
       }
	   
	   open(IMAGE_FILE, "<", $image_file_name) or error "1Can't open $image_file_name";
	   
	   seek(IMAGE_FILE, 64, 0);	   
	   #readline(IMAGE_FILE, $header);    
	   $head = <IMAGE_FILE>;
	   if ($head !~ /^UM$/) {
	   		  close(IMAGE_FILE);	
	        return;
	   }
	   $firmware_info->{"UM"} = 1;
	   
     while($um_signature_item_name = <IMAGE_FILE>) {    
  
        if ($um_signature_item_name =~ /^END$/) {
        	  last;
        }
        
        if ($um_signature_item_name =~ /(.+)_W$/g) {                      	  
            $um_signature_item_name = $1;
            #print $1 . "\n";
            binmode IMAGE_FILE, ::raw;
            read(IMAGE_FILE, $val8, 1);
            $multiple = 1;
            $value = hex(unpack("H*", $val8));            
            read(IMAGE_FILE, $val8, 1);
            $multiple = $multiple *256;
            $value = $value + hex(unpack("H*", $val8)) * $multiple;                                   
            read(IMAGE_FILE, $val8, 1);            
            $multiple = $multiple *256;
            $value = $value + hex(unpack("H*", $val8)) * $multiple;                                   
            read(IMAGE_FILE, $val8, 1);
            $multiple = $multiple *256;
            $value = $value + hex(unpack("H*", $val8)) * $multiple;                                   
            binmode IMAGE_FILE, ::crlf;               
            $firmware_info->{$um_signature_item_name} = $value;
     
            next;
        }	   

        if ($um_signature_item_name =~ /(.+)_S$/g) {               
            $string = <IMAGE_FILE>;
            $um_signature_item_name = $1;
            $firmware_info->{$um_signature_item_name} = $string;
            next;
        }	   

     }	   
	   
	   if (defined($firmware_info->{"CFG_CONFIG_BASE"}) &&  
	       defined($firmware_info->{"CFG_CONFIG_OFFSET"}) && 
	       defined($firmware_info->{"BOARD_LOADER_ADDR"})) {                   
	   	   seek(IMAGE_FILE, $firmware_info->{"CFG_CONFIG_BASE"} - $firmware_info->{"BOARD_LOADER_ADDR"} + $firmware_info->{"CFG_CONFIG_OFFSET"}, 0);
	   	   binmode IMAGE_FILE, ::raw;
	   	   if (read(IMAGE_FILE, $val32, 4)) {
     	   	   if (unpack("H*", $val32) eq "464c5348") {
 	   	   	        read(IMAGE_FILE, $chksum, 4);
 	   	   	        read(IMAGE_FILE, $len, 4);
 	   	   	        $len = unpack("V*", $len);
 	   	   	        read(IMAGE_FILE, $flag, 4);
 	   	   	        binmode IMAGE_FILE, ::crlf;  
 	   	   	        $firmware_info->{"_PREVIOUS_CONFIG_SETTING"} = ""; 	   	        	
 	   	   	        $string = "";
 	   	   	        while(read(IMAGE_FILE, $val8, 1) && $len > 0) { 	   	   	                 	
 	   	   	            	 $len -= 1; 	   	  	   	   	            	  	   	   	            	  	   	   	            	 
 	   	   	             	 if ($val8 eq "\x00") {
 	   	   	             	 	    $string =~ s/^[0-9A-Fa-f]{4}//;
 	   	   	             	 	  	$firmware_info->{"_PREVIOUS_CONFIG_SETTING"} = 	$firmware_info->{"_PREVIOUS_CONFIG_SETTING"} . $string . "\n"; 	   	   	             	
 	   	   	             	 	  	$string = "";
 	   	   	             	     next;
 	   	   	             	 }
 	   	   	               $string = $string . $val8; 	   	   	            
 	   	   	        } 
 	   	   	             
 	   	   	              	   	   	              	   	   	        
	      	   }  
	   	   }
	   }

	   close(IMAGE_FILE);	
}

################################################################################
#
# sub:    char * bcm_config_insert($hexfile)
#
# Parse hex file and return it as hex string format
#
sub bcm_config_insert {
	  my ($parameter_hash, $variable_hash, $image_info_hash) = @_;
	  my $image_filesize;
	  my $config_filesize;
	  my $position;
	  my $force_insert = $parameter_hash->{"force"};
	  my $insert_offset = $parameter_hash->{"config_offset"};
	  my $config_maximun_size = $parameter_hash->{"config_maximun_size"};
	  my $image_file = $parameter_hash->{"image"};  	  
	  my $config_file = $parameter_hash->{"config"} . "\.bin";

      if(defined($generate_bin_file)) {
          return;
      }

	  #print "force_insert $force_insert \n";
	  #print "insert_offset $insert_offset \n";
	  #print "config_maximun_size $config_maximun_size \n";
	  #print "image_file $image_file \n";
	  #print "config_file $config_file \n";
	  #print $variable_hash->{"CFG_BOARDNAME"} . "\n";

	  $image_filesize = -s $image_file;
	  $config_filesize = -s $config_file;
	  if (defined($variable_hash->{"board_name"})) {
	      chomp $image_info_hash->{"CFG_BOARDNAME"};
	  	  if ($image_info_hash->{"CFG_BOARDNAME"} ne $variable_hash->{"board_name"}) {
	  	  	  error "Board names in image header and config file are mismatch";
	  	  }
	  }
	  if ($config_maximun_size <= $config_filesize) {
	  	  error "Generated config file($config_filesize) is larger than config maximun space ($config_maximun_size)\n";
	  }
    
  
	  open(OUTPUT_FILE, ">", $image_file."\.tmp") or error "Can't open $image_file\.tmp";
	  open(CONFIG_FILE, "<", $config_file) or error "Can't open $config_file";
	  open(IMAGE_FILE, "<", $image_file) or error "Can't open $image_file";
	  binmode(OUTPUT_FILE);
	  binmode(CONFIG_FILE);
    binmode(IMAGE_FILE);
    
    $position = 0;    
    while (read(IMAGE_FILE, $buf, 1)) {
          if ($position < $insert_offset  || $position >= ($insert_offset + $config_maximun_size)) {
          } else {      	                	        
                  if (unpack("H*", $buf) ne "00" && !defined($force_insert)) {
                      close OUTPUT_FILE;
                      close CONFIG_FILE;
                      close IMAGE_FILE;
                      unlink $image_file."\.tmp";
                      error "$image_file at ". sprintf("0x%x", $position) . " has nonzero data on it\n";

                  }
                  if ($position >= ($insert_offset + $config_filesize)) {
                  	  $buf = pack("C", 0);
                  } else {
                    #printf "buf = %s\n", unpack("H*", $buf);
                    read(CONFIG_FILE, $buf, 1);
                  }
          }
          print OUTPUT_FILE $buf;
          $position++;
    }

    while ($position < ($insert_offset + $config_filesize)) {
          if ($position < $insert_offset  || $position >= ($insert_offset + $config_maximun_size)) {
              $buf = pack("C", 0);
          } else {
              read(CONFIG_FILE, $buf, 1);
          }
          print OUTPUT_FILE $buf;
          $position++;        	
    }
  
    close OUTPUT_FILE;
    close CONFIG_FILE;
    close IMAGE_FILE;
    rename($image_file, $image_file."\.old");
    rename($image_file."\.tmp", $image_file);
    unlink $config_file or die "Could not delete the file ".$config_file."!\n";
    print "Config inserted successfully \n";
    print "====================================================================\n";
}
 
################################################################################
#
# void main(void)
#
# Parse hex file and return it as hex string format
#	  
local %variables_hash;
local %parameter_hash;
local @leds;
local %image_info_hash;

#
#  parameter default value
#

$parameter_hash{"config"} = "config.um";


#
#  Get options
#

GetOptions("config|c=s" => \$parameter_hash{"config"},
            "image|i=s" => \$parameter_hash{"image"},
            #"config_insert_offset|offset=s" => \$parameter_hash{"config_offset"},
            #"ledhex=s" => \@leds,
            "force!" => \$parameter_hash{"force"},
            "generate|g!" => \$generate_bin_file,
            "verbose!" => \$verbose,
            "help|h!" => \$parameter_hash{"help"}
);

if ($parameter_hash{"help"}) {
      usage()
}

bcm_image_info_get($parameter_hash{"image"}, \%image_info_hash);

if (defined($image_info_hash{"UM"})) {
	  print "====================================================================\n";
	  print "UM signature is found at " . $parameter_hash{"image"} . "\n";
	  print "Detail parameters of image are shown below\n";
	  printf "Board name = %s", $image_info_hash{"CFG_BOARDNAME"};
	  printf "Firmware Type = %s", $image_info_hash{"target"};
	  printf "Version = 0x%x\n", $image_info_hash{"CFE_VER"};
	  
	  if (defined($image_info_hash{"CFG_CONFIG_BASE"}) && defined($image_info_hash{"BOARD_LOADER_ADDR"})) {
	       $parameter_hash{"config_offset"} = $image_info_hash{"CFG_CONFIG_BASE"} - $image_info_hash{"BOARD_LOADER_ADDR"} + $image_info_hash{"CFG_CONFIG_OFFSET"};
	       printf "Config base address =  0x%x \n", $parameter_hash{"config_offset"}; 
	  } 
	  if (defined($image_info_hash{"CFG_CONFIG_SIZE"})) {
	      $parameter_hash{"config_maximun_size"} = $image_info_hash{"CFG_CONFIG_SIZE"};
	      printf "Config maximun size = 0x%x \n",   $parameter_hash{"config_maximun_size"}; 
	  } else {
	  	  $parameter_hash{"config_maximun_size"} = 4096;
	  	  printf "Config default maximun size = 0x%x \n", $parameter_hash{"config_maximun_size"}; 
	  	  
	  }	  
	   print "====================================================================\n";
	  if (defined($image_info_hash{"_PREVIOUS_CONFIG_SETTING"})) {
	      print "Previous config setting is found\n";
	      print $image_info_hash{"_PREVIOUS_CONFIG_SETTING"};	      
	  } else {
	     $parameter_hash{"force"} = undef;
	  }
	  print "====================================================================\n";
} else {
    if(!defined($generate_bin_file)) {
        error "UM signature is not found at " . $parameter_hash{"image"} . "\n";
    }
}

if ((!defined($parameter_hash{"config"})) || 
    (!defined($parameter_hash{"image"}))  ||
    (!defined($parameter_hash{"config_offset"}))) {
    if(!defined($generate_bin_file)) {
        usage();   	
    }
}

foreach $led (@leds) {
	     if (!defined($parameter_hash{"leds"})) {
	     	   $parameter_hash{"leds"} = $led;
	     } else {
	     	   $parameter_hash{"leds"} = $parameter_hash{"leds"} . " " . $led;
	     }
	     
}

bcm_default_config_parser($parameter_hash{"config"}, \%variables_hash);
bcm_config_binary_file_generate(\%variables_hash, \%parameter_hash);
bcm_config_insert(\%parameter_hash, \%variables_hash, \%image_info_hash);




	
