#!/bin/tcsh -f  

echo "$argv[1]"

riscv32-unknown-elf-objcopy -O binary $argv[1] $argv[1].bin
xxd -p -c 4 $argv[1].bin > $argv[1].bin.txt
#rm ${0}.bin

echo "$argv[1] --> $argv[1].bin.txt"

