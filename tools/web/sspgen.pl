#!/usr/bin/perl -w

#
# $Id: sspgen.pl,v 1.5 Broadcom SDK $
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
 
use integer;
use File::Basename;

######################################################
#
# Configurations
# 
######################################################

# Run this program with debugging trace
$debug_mode = 0;

# Show results
$verbose = 0;

# Format of the array name of generating C code
$text_var_format = "_text%04d";

# struct name for data entries
$struct_name = "RES_CONST_DECL SSP_DATA_ENTRY";

# array name format
$array_name_format = "sspfile_%s";

# func delimitor for category and tag
$func_delimiter = "tag";

# var function format
$func_var_format = "sspvar_%s_${func_delimiter}_%s";

# loop function format
$func_loop_format = "ssploop_%s_${func_delimiter}_%s";

# var function prototype
####$func_var_prototype = "void %s(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem);";
$func_var_prototype = "void %s(SSPTAG_PARAM *params, SSPVAR_RET *ret, SSP_PSMH psmem) REENTRANT;";

# loop function prototype
####$func_loop_prototype = "SSPLOOP_RETVAL %s(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem);";
$func_loop_prototype = "SSPLOOP_RETVAL %s(SSPTAG_PARAM *params, SSPLOOP_INDEX index, SSP_PSMH psmem) REENTRANT;";

# Macro file format
$macro_file_format = "sspmacro_%s.h";

# C macro prefix
$macro_regexp_format = "SSPMACRO";

# C macro format
$macro_c_format = "${macro_regexp_format}_%s_%s";

# Macro comment if forced assignement
$macro_force_comment = "/* Assigned */";

# file header for generated file
$copy_right = "/*\n * \$Id\$\n *\n * \$Copyright: (c) 2013 Broadcom Corp.\n * All Rights Reserved.\$\n *\n */\n\n";

# file header for generated file
$file_header = "/***** GENERATED FILE; DO NOT EDIT. *****/\n\n";

# include files
$include_lines = "#include \"appl/ssp.h\"\n\n";

# Characters per line in C code
$chars_per_line = 8;

# Max loop depth
$max_loop_depth = 3;

# Max number of parameters
$max_parameters = 3;

# Default value for undefined parameters
$parameter_default = "0";

######################################################
#
# Global variables
# 
######################################################

# Number of constructed text entries
$text_entries = 0;

# Whether a text is currently being constructed
$text_adding = 0;

# Number of characters 
$text_count = 0;

# For better text printing on C code
$text_line = '';

# Line number of the input file
$line_num = 0;

# Stack for current loop and its index
@loop_tags = ();

# Data entries
@data_entries = ();

# Macros: hashes (categories) of arrays (macors)
%macro_categories = ();

# Whether macros of a category is dirty
%macro_dirty = ();


######################################################
#
# Subroutines
# 
######################################################

sub error
{
    my $msg = shift;
    die "***** ERROR: $msg\n";
}

sub debug
{
    if ($debug_mode) {
        print "    " x @loop_tags . "$_[0]\n";
    }
}

sub append_text
{
    my $string = shift;
    unless ($text_adding) {
        # create new array head: 
        $text_adding = 1;
        $text_count = 0;
        $text_line = '';
        printf OUT_C_FILE "static RES_CONST_DECL unsigned char CODE ${text_var_format}[] = \{", ${text_entries};
        $text_entries++;
    }
    
    # append C code
    my @ascii = unpack("C*", $string);
    foreach my $ch (@ascii) {
        unless ($text_count % $chars_per_line) {
            if ($text_line) {
                $text_line =~ s/\/\*/.\*/;
                $text_line =~ s/\*\//\*./;
                print OUT_C_FILE "     /* $text_line */";
                $text_line = '';
            }
            print OUT_C_FILE "\n    ";
        }
        
        printf OUT_C_FILE "0x%02x, ", $ch;
        if ($ch < 32 || $ch > 126) {
            $text_line = $text_line.".";
        } else {
            $text_line = $text_line.chr($ch);
        }
        $text_count++;
    }
}

sub close_text
{
    if ($text_adding) {
        if ($text_line) {
            my $pad = $chars_per_line - length($text_line);
            $text_line = $text_line.(" " x $pad);
            $text_line =~ s/\/\*/.\*/;
            $text_line =~ s/\*\//\*./;
            print OUT_C_FILE "      " x $pad;
            print OUT_C_FILE "     /* $text_line */";
            $text_line = '';
        }
        print OUT_C_FILE "\n\};\n\n";
        
        if ($text_count > 65535) {
            
            error "Too many characters in single entry!";
        }
        
        # Adding new date entry
        push @data_entries, [ 1, $text_entries - 1, $text_count ];
        $text_adding = 0;
    } 
}

sub check_identifier
{
    my $id = shift;
 
    if ($id =~ /^[a-z]\w*$/) {
        return 1;
    } else {
        return 0;
    }   
}

sub read_macro_file
{
    my $mfile = shift;
    my $cat = shift;
    my %mcs = ();
    
    open (IN_MACRO_FILE, $mfile) || error "Cannot read macro file $mfile!";
    
    my $ucat = uc $cat;
    while(<IN_MACRO_FILE>) {
        if (/^\#define\s${macro_regexp_format}_${ucat}_(\w+)\s+\((\d+)\)\s*(\/\*.+\*\/)?\s*$/) {
        
            my $num = $2;
            if (defined($3)) { # Containing /* Assigned */
                $num = "@".$num;       
            }
            $mcs{lc($1)} = $num;
        }
    }
    
    close (IN_MACRO_FILE);
    
    return \%mcs;
}

sub process_macro
{
    my $cat = shift;
    my $macro = shift;
    my $num = shift;
    my %macros = ();
    my @vals = (); 
    
    # Check number format if any
    if (defined($num)) {
        if ($num < 0 || $num > 65535) {
            error "Incorrect range for parameters at $line_num!";
        }
    }
    
    # Check if the macro list of category is already there
    if (exists($macro_categories{"$cat"})) {
        %macros = %{ $macro_categories{"$cat"} };
    } else {
        # Does the macro header file exists?
        my $mf = sprintf("${out_path}${macro_file_format}", $cat);
        if (-e $mf) {
            # Read macro header file into the list
            %macros = %{ read_macro_file($mf, $cat) };
        }
        
        # Add to our current list of macros
        $macro_categories{"$cat"} = \%macros;
        $macro_dirty{"$cat"} = 0;
    }
    
    # Check if the macro already exists
    foreach my $mc (keys %macros) {
        if ($mc eq $macro) {
            if (defined($num)) {
                # This macro is specified with a forced value
                my $num2 = $macros{$mc};
                if ($num2 =~ s/^@//) { # The previous value is also force-assigned
                    if ($num != $num2) {
                        error "Duplicated macro ($mc) with value ($num) different from previous definition ($num2)!";
                    }
                    debug "Found macro: [$mc], forced value matched: $num";
                } else { # The previous value is auto-assigned
                        # Value should be changed to force-assigned
                        $macros{$mc} = "@".$num;
                        $macro_categories{"$cat"} = \%macros;
                        $macro_dirty{"$cat"} = 1;
                }
                return uc sprintf ${macro_c_format}, ${cat}, ${mc};
            } else {
                # Just get the value in the list
                debug "Found macro: [$mc], using pre-assigned value: $macros{$mc}";
                $num = $macros{$mc};
                $num =~ s/^@//; # Regardless it's force-assigned or not
                return uc sprintf ${macro_c_format}, ${cat}, ${mc};
            }
        }
    }
    
    # Macro not yet defined, decide the value first
    if (defined($num)) {
        my $num_old = $num;
        debug "Found macro: [$macro], forced value: $num";
        $num = "@".$num;
        
        # Check if any auto-assigned macro has the same value
        my @ks = keys %macros;
        for $k (0 .. ($#ks)) {
            if ($macros{$ks[$k]} =~ /^@/) {
                next;
            }
            if ($macros{$ks[$k]} eq $num_old) {
                # Found duplicated value from auto-assigned value
                @vals = values %macros;
                
                # Remove '@' in values
                for $i (0 .. ($#vals)) {
                    $vals[$i] =~ s/^@//;
                }
                
                # Sort the values
                @vals = sort {$a<=>$b} @vals;
                
                # Remove duplicated values
                if ($#vals > 0) { # More than one element
                    for($i=1; $i < @vals; $i++) {
                        if ($vals[$i] == $vals[$i - 1]) {
                            splice @vals, $i, 1;
                            $i = $i - 1;
                        }
                    }
                }
                
                # Find a slot that $vals[i] != i
                for $i (0 .. ($#vals + 1)) {
                    if ($i > $#vals) {
                        $macros{$ks[$k]} = $i;
                        debug "Changing old macro [$ks[$k]] to value $i";
                        last;
                    }
                    if ($vals[$i] != $i) {
                        debug "Changing old macro [$ks[$k]] to value $i";
                        $macros{$ks[$k]} = $i;
                        last;
                    }
                }
                
                last;
            }
        }
        
    } else {
        # Find an empty slot
        if (0 == keys %macros) {
            # Not even one pair
            $num = 0;
        } else {
            @vals = values %macros;
            
            # Remove '@' in values
            for $i (0 .. ($#vals)) {
                $vals[$i] =~ s/^@//;
            }
            
            # Sort the values
            @vals = sort {$a<=>$b} @vals;
            
            # Remove duplicated values
            if ($#vals > 0) { # More than one element
                for($i=1; $i < @vals; $i++) {
                    if ($vals[$i] == $vals[$i - 1]) {
                        splice @vals, $i, 1;
                        $i = $i - 1;
                    }
                }
            }
            
            # Find a slot that $vals[i] != i
            for $i (0 .. ($#vals + 1)) {
                if ($i > $#vals) {
                    $num = $i;
                    last;
                }
                if ($vals[$i] != $i) {
                    $num = $i;
                    last;
                }
            }

            unless(defined($num)) {
                $num = $#vals + 1;
            }
        }
        debug "Found macro: [$macro], using auto-assigned value: $num";
    }
    
    # Add the macro into the list
    debug "Adding macro ($macro,$num) to category \"$cat\"";
    $macros{"$macro"} = $num;
    $macro_categories{"$cat"} = \%macros;
    $macro_dirty{"$cat"} = 1;
    
    $num =~ s/^@//;
    return uc sprintf ${macro_c_format}, ${cat}, ${macro};
}

sub parse_parameters
{
    my $cat = shift;
    my $param = shift;
    my @params = ();
    
    if ($param) {

        if ($param =~ /^\(\s*(.*?)\s*\)$/) {
            if ($1) {
                # contain not only parentheses: (...)
                @params = split /\s*,\s*/, $1;
                
                # Check parameters format: number, id or macro           
                for my $i (0 .. $#params) {
                    unless ($params[$i] =~ /^\d+$/) {
                        
                        # Is it a macro?
                        if ($params[$i] =~ /^\#([a-z]\w*)\s*(\=\s*(\d+))?$/) {
                        
                            $params[$i] = process_macro($cat, $1, $3);
                            
                        } else {
                            unless(check_identifier($params[$i])) {
                                error "Invalid parameter($param) at line $line_num!";
                            }
                        }
                    }
                }
            }
        } else {
            error "Invalid tag format at line $line_num!";
        }
    }
    
    while(@params < $max_parameters) { 
        $params[@params] = $parameter_default; 
    }
    
    return @params;
}

sub check_param
{
    my $param = shift;
    
    if ($param =~ /^\d+$/) {
        if ($param < 0 || $param > 65535) {
            error "Incorrect range for parameters at $line_num!";
        }
        return $param;
    } else {
        for my $i (0 .. $#loop_tags) {
            if ($param eq $loop_tags[$i][0]) {
                return "d".$i;
            }
        }
        if ($param =~ /^SSPMACRO_/) {
            return $param;
        }
        error "Invalid parameter ($param) at line $line_num!";
    }

    return '';
}

sub tag_loop_start
{
    my $idx = shift;
    my $cat = shift;
    my $func = shift;
    my $params = shift;
    
    unless(check_identifier($idx)) {
        error "Invalid loop index($idx) at line $line_num!";
    }
    
    unless(check_identifier($cat)) {
        error "Invalid tag category($cat) at line $line_num!";
    }
    
    unless(check_identifier($func)) {
        error "Invalid tag name($func) at line $line_num!";
    }
    
    # Parse and check parameters
    my @params = parse_parameters($cat, $params);
    if (@params > $max_parameters) {
        error "Too many parameters at $line_num!";
        return;
    }
    
    # Check and change parameters to loop index if any
    for my $i (0 .. $#params) {
        $params[$i] = check_param($params[$i]);
    }
    
    # Check index variable not defined
    for my $i (0 .. $#loop_tags) {
        if ($idx eq $loop_tags[$i][0]) {
            error "Index variable is already defined at $line_num!";
            return;
        }
    }
    
    # Check depth
    if (@loop_tags == $max_loop_depth) {
        error "Max depth of loops reached at $line_num!";
        return;
    }
    
    # New data entry
    # debug "Adding data entry: 2 0 $cat $func @params";
    push @data_entries, [2, 0, $cat, $func, @params];
    
    # Advance loop depth
    debug "Loop ".@loop_tags." start with idx=\"$idx\" at entry $#data_entries";
    push @loop_tags, [ "$idx", $#data_entries ];
}

sub tag_loop_end
{
    my $idx = shift;
    unless(check_identifier($idx)) {
        error "Invalid loop index($idx) at line $line_num!";
        return;
    }
    
    if (@loop_tags == 0) {
        error "Unmatched loop index($idx) at line $line_num!";
        return;
    }
    
    unless ($idx eq $loop_tags[$#loop_tags][0]) {
        error "Unmatched loop index($idx) at line $line_num!";
        return;
    }
    
    # Exit loop
    my $loop = pop @loop_tags;
    debug "Loop ".@loop_tags." end";
    
    # New data entry
    my $dif = @data_entries - @$loop[1];
    # debug "Adding data entry: 3 $dif ";
    push @data_entries, [3, $dif];
    
    # Update loop start index
    $data_entries[@$loop[1]][1] = $dif;
}

sub tag_replacement
{
    my $cat = shift;
    my $func = shift;
    my $params = shift;
    
    unless(check_identifier($cat)) {
        error "Invalid tag category($cat) at line $line_num!";
    }
    
    unless(check_identifier($func)) {
        error "Invalid tag name($func) at line $line_num!";
    }
    
    my @params = parse_parameters($cat, $params);
    if (@params > $max_parameters) {
        error "Too many parameters at $line_num!";
        return;
    }
    
    # Check and change parameters to loop index if any
    for my $i (0 .. $#params) {
        $params[$i] = check_param($params[$i]);
    }
    
    # New data entry
    # debug "Adding data entry: 4 $cat $func @params";
    push @data_entries, [4, $cat, $func, @params];
}

sub found_tag
{
    my $tag = lc shift; # make it all lower case
    
    # Close previous plain text if any
    &close_text;
    
    debug "Found tag: [$tag]";
    
    if ($tag =~ /^\s*\=\s*(\w+)\s*\:\s*(\w+)\s*(.*?)\s*$/) {
        
        # Replacemant tag
        tag_replacement($1, $2, $3);
        
    } elsif ($tag =~ /^\s*\/\s*(.+?)\s*$/) {
        
        # Loop end tag
        tag_loop_end($1);
        
    } elsif ($tag =~ /^\s*(\w+)\s*\@\s*(\w+)\s*\:\s*(\w+)\s*(.*?)\s*$/) {
        
        # Loop start tag
        tag_loop_start($1, $2, $3, $4);
        
    } else {
        error "Invalid tag format at line $line_num!";
    }
}

sub final_parameters
{
    my $opbyte = shift;
    my @params = @_;
    
    for my $i (0 .. $#params) {
        if (substr($params[$i], 0, 1) eq 'd') {
            $opbyte |= 1 << ($i + 4);
            $params[$i] = substr($params[$i], 1, 1);
        }
    }
    
    return ($opbyte, @params);
}

######################################################
#
# Main program begins
# 
######################################################

# Parameters from command line
$in_file = $ARGV[0];
unless ($in_file) {
    print "Usage: ".__FILE__." <input_file> [<output_path> [<output_basename>]]\n";
    exit 0;
}

unless (-e $in_file) {
    error "Cannot open input file $in_file!";
}

$out_path = $ARGV[1];
$out_path = "." unless ($out_path);
unless ($out_path =~ /(\/|\\)$/) {
    $out_path = $out_path."/";
}

# Build up output name
if ($ARGV[2]) {
    $out_name = $ARGV[2];
} else {
    $out_name = basename($in_file);
}
while ($out_name =~ s/\W+/_/) {};

# Output C file name
$out_c_file = "${out_path}${out_name}.c";

# Output H file name
$out_h_file = "${out_path}${out_name}.h";

# Output array name
$out_array_name = sprintf($array_name_format, $out_name);

# Open file
open (IN_FILE, $in_file) || error "Cannot open input file $in_file!";
open (OUT_C_FILE, ">".$out_c_file) || error "Cannot create output file $out_c_file!";
open (OUT_H_FILE, ">".$out_h_file) || error "Cannot create output file $out_h_file!";

# Add file headers
print OUT_C_FILE $copy_right.$file_header.$include_lines;
print OUT_H_FILE $copy_right.$file_header;

# Now parsing
if (-T $in_file) {
    # TEXT file
    binmode(IN_FILE); # Make it a binary file to avoid character processing
    while(<IN_FILE>) {
        $line = $_;
        $line_num++;
        while($line) {
            
            # Check if the line contains <% %> or [% %]
            if ($line =~ /[<\[]%.*?%[>\]]/) {
                while ($line =~ /^(.*?)[<\[][\?%](.*?)[\?%][>\]]/) {
                    append_text($1) if ($1);
                    found_tag($2);
    
                    # Remove what've been processed
                    $line =~ s///;
                }
            } else {
                

                # It shouldn't contain <% or [% (incomplete tag)
                # NOTE: We don't check for %> or %] because it may happen 
                #       in HTML, eg. <table width=50%>
                if ($line =~ /[\[<]%/) {
                    error "Incomplete tag at line $line_num!";
                }

                append_text($line);
                $line = '';
            }
        }
    }
} else {
    # Binary file
    binmode(IN_FILE);
    while(read(IN_FILE, $line, 256)) {
        append_text($line);
    }
}

# Close previous plain text if any
&close_text;

# Check all loops are closed
if (@loop_tags != 0) {
    error "Loop not closed at $line_num!";
}

# Include macro files
foreach my $cat (keys %macro_categories) {
    print OUT_C_FILE "#include \"".sprintf("${macro_file_format}", $cat)."\"\n";
}

# Generate tag/loop function forwards
@func_prototypes = ();
for my $i (0 .. $#data_entries) {
    my $str = "";
    if ($data_entries[$i][0] == 2) {
        # Loop start
        my $ptr = sprintf($func_loop_format, $data_entries[$i][2], $data_entries[$i][3]);
        $str = sprintf($func_loop_prototype, $ptr);
    } elsif ($data_entries[$i][0] == 4) {
        # Replacement tag
        my $ptr = sprintf($func_var_format, $data_entries[$i][1], $data_entries[$i][2]);
        $str = sprintf($func_var_prototype, $ptr);
    }
    # Don't print duplicated one
    foreach my $ft (@func_prototypes) {
        if ($ft eq $str) {
            $str = "";
            last;
        }
    }
    if ($str) {
        print OUT_C_FILE "$str\n";
        push @func_prototypes, $str;
    }
}

# Generate final data entry array
print OUT_C_FILE "\n$struct_name CODE ${out_array_name}[] = {\n";
for my $i (0 .. $#data_entries) {
    my $opbyte = 0;
    my $ptr = '';
    my @params = (0, 0, 0);
    my $idx = 0;

    if ($data_entries[$i][0] == 1) {
        
        # Simple text entry
        $opbyte = 0;
        $ptr = sprintf($text_var_format, $data_entries[$i][1]);
        $params[0] = $data_entries[$i][2];
        
    } elsif ($data_entries[$i][0] == 2) {
        # Loop start
        $opbyte = 1;
        $ptr = sprintf($func_loop_format, $data_entries[$i][2], $data_entries[$i][3]);
        @params = ($data_entries[$i][4], $data_entries[$i][5], $data_entries[$i][6]);
        @params = final_parameters($opbyte, @params);
        $opbyte = shift @params;
        $idx = $data_entries[$i][1];
    } elsif ($data_entries[$i][0] == 3) {
        # Loop end
        $opbyte = 1;
        $ptr = "NULL";
        $idx = $data_entries[$i][1];
    } elsif ($data_entries[$i][0] == 4) {
        # Replacement tag
        $opbyte = 2;
        $ptr = sprintf($func_var_format, $data_entries[$i][1], $data_entries[$i][2]);
        @params = ($data_entries[$i][3], $data_entries[$i][4], $data_entries[$i][5]);
        @params = final_parameters($opbyte, @params);
        $opbyte = shift @params;
    }
    
    if ($i == $#data_entries) {
        $opbyte |= 0x80;
    }
    printf OUT_C_FILE "    { 0x%02X, $idx, $params[0], $params[1], $params[2], $ptr },\n", $opbyte;
    
}
print OUT_C_FILE "};\n";

# Generate .h file
$h_file_def = uc "_${out_array_name}_H_";
print OUT_H_FILE "#ifndef $h_file_def\n#define $h_file_def\n";
print OUT_H_FILE "\nextern $struct_name CODE ${out_array_name}[];\n";
print OUT_H_FILE "\n#endif /* $h_file_def */\n";

# Generate macro files
my @mfiles = ();
foreach my $cat (keys %macro_categories) {
    if ($macro_dirty{"$cat"} == 1) {
        # Only generate "dirty" categories 
        my $out_macro_file = sprintf("${out_path}${macro_file_format}", $cat);
        push @mfiles, basename($out_macro_file);
        open (OUT_MACRO_FILE, ">".$out_macro_file) || error "Cannot create macro file $out_macro_file!";

        # Copyright
        print OUT_MACRO_FILE "$copy_right";
        
        # File header
        print OUT_MACRO_FILE "$file_header";
        
        # To prevent multiple inclusion 
        my $def = uc sprintf("_${macro_file_format}_", $cat);
        $def =~ s/\W+/_/;
        print OUT_MACRO_FILE "#ifndef $def\n#define $def\n\n";
        
        # Print macro definitions
        my %macros = %{ $macro_categories{$cat} };
        foreach my $mc (sort keys %macros) {
            my $val = $macros{$mc};
            my $forced = 0;
            if ($val =~ s/^@//) {
                $forced = 1;
            }
            print OUT_MACRO_FILE "#define ".(uc sprintf("$macro_c_format\t\t(%d)", $cat, $mc, $val));
            if ($forced == 1) {
                print OUT_MACRO_FILE "\t${macro_force_comment}";
            }
            print OUT_MACRO_FILE "\n";
        }
        
        # End of preventing multiple inclusion
        print OUT_MACRO_FILE "\n#endif /* $def */\n";
        
        # close the file
        close (OUT_MACRO_FILE);
    }
}


# close all files
close (IN_FILE);
close (OUT_C_FILE);
close (OUT_H_FILE);

# Final results
if ($verbose) {
    my $pgfile = basename(__FILE__);
    $pgfile =~ s/.pl$//;
    printf "$pgfile: Files generated: %s %s", basename($out_c_file), basename($out_h_file);
    printf "\n$pgfile: Macro files generated/updated: ";
    if (@mfiles > 0) {
        foreach my $mf (@mfiles) {
            print $mf." ";
        }
    } else {
        print "(none)";
    }
    print "\n";
}
