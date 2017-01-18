# change forceupdate to 1 to update the fuses to values specified below
set forceupdate 0
set v0 0xD8E0C7FF
set v1 0xFFFFFC5D

init

set CPU_MAX_ADDRESS 0xFFFFFFFF
source [find bitsbytes.tcl]
source [find memory.tcl]

set fuse0 0x00804000
set fuse1 0x00804004
set nvmctrl 0x41004000

set cmd_ear 0x5
set cmd_pbc 0x44
set cmd_wap 0x6

proc cmd {C} {
  global nvmctrl
  global fuse0
  memwrite32 [expr $nvmctrl + 0x1C] [expr $fuse0 / 2]
  memwrite16 $nvmctrl [expr 0xA500 | $C]
}

set f0 [memread32 $fuse0]
set f1 [memread32 $fuse1]

puts "Fuses:"
puts [format %x $f0]
puts [format %x $f1]

set updatefuses 0

if { $forceupdate } then {
  set updatefuses 1
} else {
  if { ($f0 & 0x7) != 0x7 } then {
    set updatefuses 1
    set v0 [expr $f0 | 0x7]
    set v1 $f1
  }
}

if { $updatefuses } then {
  puts "*** Updating fuses! ***"
  puts [format %x $v0]
  puts [format %x $v1]
  cmd $cmd_pbc
  cmd $cmd_ear

  memwrite32 $fuse0 $v0
  memwrite32 $fuse1 $v1

  cmd $cmd_wap

  reset
}

shutdown
