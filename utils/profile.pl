#!/usr/bin/perl -w
#
# Program:  profile.pl
#
# Synopsis: Insert instrumentation code into a program, run it with the JIT,
#           then print out a profile report.
#
# Syntax:   profile.pl [OPTIONS] bytecodefile <arguments>
#
# OPTIONS may include one or more of the following:
#     -block    - Enable basic block profiling
#     -edge     - Enable edge profiling
#     -function - Enable function profiling
#     -o <filename> - Emit profiling information to the specified file, instead
#                     of llvmprof.out
#
# Any unrecognized options are passed into the invocation of llvm-prof
#

#
# This script is part of LLVM, c.f.:
# http://llvm.org/svn/llvm-project/llvm/trunk/utils/profile.pl
#
# We made some changes so that it meets our needs better.
#
# For example, on Debian Squeeze:
# $ perl profile.pl -load /usr/lib/libgc.so -foo -bar test.bc 12 34
#
# will be interpreted as the command lines below:
#
# $ opt -f -insert-edge-profiling test.bc -o test.bc.inst
# $ lli -fake-argv0 'test.bc'  -load /usr/lib/libgc.so \
#       -load /usr/lib/llvm/lib/libprofile_rt.so test.bc.inst  12 34
# $ rm test.bc.inst
# $ llvm-prof  -foo -bar test.bc
#
# Copyright (C)
#         2010 David Lee <live4thee@gmail.com>
#         2010 Qing He <qing.x.he@gmail.com>
#

my $ProfilePass = "-insert-edge-profiling";

my $LLVMProfOpts = "";
my $LliLoadFiles = "";
my $ProgramOpts = "";
my $ProfileFile = "";

# Parse arguments...
while (scalar(@ARGV) and ($_ = $ARGV[0], /^[-+]/)) {
  shift;
  last if /^--$/;  # Stop processing arguments on --

  # List command line options here...
  if (/^-?-block$/)    { $ProfilePass = "-insert-block-profiling"; next; }
  if (/^-?-edge$/)     { $ProfilePass = "-insert-edge-profiling"; next; }
  if (/^-?-function$/) { $ProfilePass = "-insert-function-profiling"; next; }
  if (/^-?-load$/) {      # Read -load filename...
    die "-load option requires a plug-in filename!" if (!scalar(@ARGV));
    $LliLoadFiles .= " -load $ARGV[0]";
    shift;
    next;
  }
  if (/^-?-o$/) {         # Read -o filename...
    die "-o option requires a filename argument!" if (!scalar(@ARGV));
    $ProgramOpts .= " -llvmprof-output $ARGV[0]";
    $ProfileFile = $ARGV[0];
    shift;
    next;
  }
  if (/^-?-help$/) {
    print "OVERVIEW: profile.pl - Instrumentation and profile printer.\n\n";
    print "USAGE: profile.pl [options] program.bc <program args>\n\n";
    print "OPTIONS:\n";
    print "  -block       - Enable basic block profiling\n";
    print "  -edge        - Enable edge profiling\n";
    print "  -function    - Enable function profiling\n";
    print "  -load <file> - Load a shared object for lli\n";
    print "  -o <file>    - Specify an output file other than llvm-prof.out.\n";
    print "  -help        - Print this usage information\n";
    print "\nAll other options are passed into llvm-prof.\n";
    exit 1;
  }

  # Otherwise, pass the option on to llvm-prof
  $LLVMProfOpts .= " " . $_;
}

die "Must specify LLVM byte-code file as first argument!" if (@ARGV == 0);

my $BytecodeFile = $ARGV[0];
shift @ARGV;

my $libdir = `llvm-config --libdir`;
chomp $libdir;
$LliLoadFiles .= " -load " . $libdir . "/libprofile_rt.so";

system "opt -f $ProfilePass $BytecodeFile -o $BytecodeFile.inst";
system "lli -fake-argv0 '$BytecodeFile' $LliLoadFiles " .
       "$BytecodeFile.inst $ProgramOpts " . (join ' ', @ARGV);
system "rm $BytecodeFile.inst";
system "llvm-prof $LLVMProfOpts $BytecodeFile $ProfileFile";
