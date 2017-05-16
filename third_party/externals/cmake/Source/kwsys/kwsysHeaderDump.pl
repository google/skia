#!/usr/bin/perl
#=============================================================================
# KWSys - Kitware System Library
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

if ( $#ARGV+1 < 2 )
{
    print "Usage: ./kwsysHeaderDump.pl <name> <header>\n";
    exit(1);
}

$name = $ARGV[0];
$max = 0;
open(INFILE, $ARGV[1]);
while (chomp ($line = <INFILE>))
{
    if (($line !~ /^\#/) &&
        ($line =~ s/.*kwsys${name}_([A-Za-z0-9_]*).*/\1/) &&
        ($i{$line}++ < 1))
    {
        push(@lines, "$line");
        if (length($line) > $max)
        {
            $max = length($line);
        }
    }
}
close(INFILE);

$width = $max + 13;
print sprintf("#define %-${width}s kwsys_ns(${name})\n", "kwsys${name}");
foreach $l (@lines)
{
    print sprintf("#define %-${width}s kwsys_ns(${name}_$l)\n",
                  "kwsys${name}_$l");
}
print "\n";
print sprintf("# undef kwsys${name}\n");
foreach $l (@lines)
{
    print sprintf("# undef kwsys${name}_$l\n");
}
