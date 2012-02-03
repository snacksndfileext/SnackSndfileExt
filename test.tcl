#!/bin/sh
# the next line restarts using wish \
exec wish8.4 "$0" "$@"

# 'info sharedlibext' returns '.dll' on Windows and '.so' on most Unix systems

load usr/lib/snack_sndfile_ext0.0.1/libsnack_sndfile_ext[info sharedlibext]

# Create a sound object

if {[llength $argv] == 0} {
  puts {Usage: test.tcl file}
  exit
} else {
  set file [lindex $argv 0]
}

snack::sound s -file $file

# Set its length to 10000 samples

#s length 10000

# Apply the command defined in the square package

#s square

pack [button .b -text Play -command {s play}]
