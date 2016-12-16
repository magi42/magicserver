#!/usr/bin/perl -w

open (IN, $ARGV[0]);

print "/** \@file $ARGV[0] */\n";
$commentblockrow = "";

while (<IN>) {
  # Reformat function comment blocks.
  if (/^\/\*\*\*+\s*$/) {
    $commentblockrow = $_;
    chop $commentblockrow;
  } else {
    if (not $commentblockrow eq "") {
      if (/^ *\* (.+)$/) {
	#print "$commentblockrow/\n";
	print "/** $1\n";
      } else {
	print "$commentblockrow\n";
	print "$_";
      }
      $commentblockrow = "";
    } else {
      if (/^ ?\*\*\*+\/$/) {
	print "**/\n";
      } else {
	print $_;
      }
    }
  }
}
