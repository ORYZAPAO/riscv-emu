#/bin/sh -f

mkdir -p result
./rv32uiemu ../testdat/isa/rv32ui-p-add.bin.txt    | tee ./result/rv32ui-p-add.bin.log 
./rv32uiemu ../testdat/isa/rv32ui-p-addi.bin.txt   | tee ./result/rv32ui-p-addi.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-and.bin.txt	   | tee ./result/rv32ui-p-and.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-andi.bin.txt   | tee ./result/rv32ui-p-andi.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-auipc.bin.txt  | tee ./result/rv32ui-p-auipc.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-beq.bin.txt	   | tee ./result/rv32ui-p-beq.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-bge.bin.txt	   | tee ./result/rv32ui-p-bge.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-bgeu.bin.txt   | tee ./result/rv32ui-p-bgeu.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-blt.bin.txt	   | tee ./result/rv32ui-p-blt.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-bltu.bin.txt   | tee ./result/rv32ui-p-bltu.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-bne.bin.txt    | tee ./result/rv32ui-p-bne.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-fence_i.bin.txt| tee ./result/rv32ui-p-fence_i.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-jal.bin.txt	   | tee ./result/rv32ui-p-jal.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-jalr.bin.txt   | tee ./result/rv32ui-p-jalr.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-lb.bin.txt     | tee ./result/rv32ui-p-lb.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-lbu.bin.txt	   | tee ./result/rv32ui-p-lbu.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-lh.bin.txt	   | tee ./result/rv32ui-p-lh.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-lhu.bin.txt	   | tee ./result/rv32ui-p-lhu.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-lui.bin.txt	   | tee ./result/rv32ui-p-lui.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-lw.bin.txt	   | tee ./result/rv32ui-p-lw.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-or.bin.txt	   | tee ./result/rv32ui-p-or.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-ori.bin.txt	   | tee ./result/rv32ui-p-ori.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sb.bin.txt	   | tee ./result/rv32ui-p-sb.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sh.bin.txt	   | tee ./result/rv32ui-p-sh.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-simple.bin.txt | tee ./result/rv32ui-p-simple.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sll.bin.txt	   | tee ./result/rv32ui-p-sll.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-slli.bin.txt   | tee ./result/rv32ui-p-slli.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-slt.bin.txt	   | tee ./result/rv32ui-p-slt.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-slti.bin.txt   | tee ./result/rv32ui-p-slti.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sltiu.bin.txt  | tee ./result/rv32ui-p-sltiu.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sltu.bin.txt   | tee ./result/rv32ui-p-sltu.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sra.bin.txt	   | tee ./result/rv32ui-p-sra.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-srai.bin.txt   | tee ./result/rv32ui-p-srai.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-srl.bin.txt	   | tee ./result/rv32ui-p-srl.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-srli.bin.txt   | tee ./result/rv32ui-p-srli.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sub.bin.txt	   | tee ./result/rv32ui-p-sub.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-sw.bin.txt     | tee ./result/rv32ui-p-sw.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-xor.bin.txt	   | tee ./result/rv32ui-p-xor.bin.log
./rv32uiemu ../testdat/isa/rv32ui-p-xori.bin.txt   | tee ./result/rv32ui-p-xori.bin.log


echo ""
echo " Result"
echo ""
grep gp result/*
