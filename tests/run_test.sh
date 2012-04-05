#!/bin/bash

# certainly not the prettiest script of all time but it stream lined the suite :-)

for o in `seq 500000000 500000000 2000000000`
do
~/llds/tests/rbtree/rb_rmtree

for i in 2 4 8 16 24
do
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux --event=CPU_CLK_UNHALTED:10000
opcontrol --reset
echo "userspace w/${i} threads & ${o} items"
free
time ~/llds/tests/rbtree/rb_userspace ${i} ${o}
free
opreport -l > /var/tmp/rb_userspace_cpu_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_user_cpu_unhalted_${i}_${o}
opannotate --assembly > /var/tmp/rb_user_cpu_unhalted_asm_${i}_${o}
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux --event=CPU_CLK_UNHALTED:10000
opcontrol --reset
echo "llds w/${i} threads & ${o} items"
mknod /dev/llds c 834 0
rmmod llds
insmod ~/llds/linux/llds/llds.ko
free
time ~/llds/tests/rbtree/rb_llds ${i} ${o}
free
opreport -l > /var/tmp/rb_llds_cpu_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_llds_cpu_unhalted_${i}_${o}
opannotate --assembly > /var/tmp/rb_llds_cpu_unhalted_asm_${i}_${o}
~/llds/tests/rbtree/rb_rmtree
done

for i in 2 4 8 16 24
do
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e MEM_LOAD_RETIRED:200000:0x01:1:1
opcontrol --reset
echo "userspace w/${i} threads & ${o} items"
free
time ~/llds/tests/rbtree/rb_userspace ${i} ${o}
free
opreport -l > /var/tmp/rb_userspace_l1_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_user_mem_retired_l1_${i}_${o}
opannotate --assembly > /var/tmp/rb_user_mem_retired_l1_asm_${i}_${o}
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e MEM_LOAD_RETIRED:200000:0x01:1:1
opcontrol --reset
echo "llds w/${i} threads & ${o} items"
mknod /dev/llds c 834 0
rmmod llds
insmod ~/llds/linux/llds/llds.ko
free
time ~/llds/tests/rbtree/rb_llds ${i} ${o}
free
opreport -l > /var/tmp/rb_llds_l1_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_llds_mem_retired_l1_${i}_${o}
opannotate --assembly > /var/tmp/rb_llds_mem_retired_l1_asm_${i}_${o}
~/llds/tests/rbtree/rb_rmtree
done

for i in 1 2 4 8 16 24
do
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e MEM_LOAD_RETIRED:200000:0x02:1:1
opcontrol --reset
echo "userspace w/${i} threads & ${o} items"
free
time ~/llds/tests/rbtree/rb_userspace ${i} ${o}
free
opreport -l > /var/tmp/rb_userspace_l2_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_user_mem_retired_l2_${i}_${o}
opannotate --assembly > /var/tmp/rb_user_mem_retired_l2_asm_${i}_${o}
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e MEM_LOAD_RETIRED:200000:0x02:1:1
opcontrol --reset
echo "llds w/${i} threads & ${o} items"
mknod /dev/llds c 834 0
rmmod llds
insmod ~/llds/linux/llds/llds.ko
free
time ~/llds/tests/rbtree/rb_llds ${i} ${o}
free
opreport -l > /var/tmp/rb_llds_l2_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_llds_mem_retired_l2_${i}_${o}
opannotate --assembly > /var/tmp/rb_llds_mem_retired_l2_asm_${i}_${o}
~/llds/tests/rbtree/rb_rmtree
done

for i in 1 2 4 8 16 24
do
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e MEM_LOAD_RETIRED:200000:0x04:1:1
opcontrol --reset
echo "userspace w/${i} threads & ${o} items"
free
time ~/llds/tests/rbtree/rb_userspace ${i} ${o}
free
opreport -l > /var/tmp/rb_userspace_llc_${i}
opannotate --source --output-dir=/var/tmp/rb_user_mem_retired_llc_${i}_${o}
opannotate --assembly > /var/tmp/rb_user_mem_retired_llc_asm_${i}_${o}
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e MEM_LOAD_RETIRED:200000:0x04:1:1
opcontrol --reset
echo "llds w/${i} threads & ${o} items"
mknod /dev/llds c 834 0
rmmod llds
insmod ~/llds/linux/llds/llds.ko
free
time ~/llds/tests/rbtree/rb_llds ${i} ${o}
free
opreport -l > /var/tmp/rb_llds_llc_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_llds_mem_retired_llc_${i}_${o}
opannotate --assembly > /var/tmp/rb_llds_mem_retired_llc_asm_${i}_${o}
~/llds/tests/rbtree/rb_rmtree
done

for i in 1 2 4 8 16 24
do
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e OFFCORE_REQUESTS:100000
opcontrol --reset
echo "userspace w/${i} threads & ${o} items"
free
time ~/llds/tests/rbtree/rb_userspace ${i} ${o}
free
opreport -l > /var/tmp/rb_userspace_offcore_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_user_offcore_${i}_${o}
opannotate --assembly > /var/tmp/rb_user_offcore_asm_${i}_${o}
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e OFFCORE_REQUESTS:100000
opcontrol --reset
echo "llds w/${i} threads & ${o} items"
mknod /dev/llds c 834 0
rmmod llds
insmod ~/llds/linux/llds/llds.ko
free
time ~/llds/tests/rbtree/rb_llds ${i} ${o}
free
opreport -l > /var/tmp/rb_llds_offcore_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_llds_offcore_${i}_${o}
opannotate --assembly > /var/tmp/rb_llds_offcore_asm_${i}_${o}
~/llds/tests/rbtree/rb_rmtree
done

for i in 1 2 4 8 16 24
do
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e L1D_PREFETCH:200000
opcontrol --reset
echo "userspace w/${i} threads & ${o} items"
free
time ~/llds/tests/rbtree/rb_userspace ${i} ${o}
free
opreport -l > /var/tmp/rb_userspace_l1d_pf_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_user_l1d_prefetch_${i}_${o}
opannotate --assembly > /var/tmp/rb_user_l1d_prefetch_asm_${i}_${o}
opcontrol --shutdown
opcontrol --start --vmlinux=/boot/vmlinux -e L1D_PREFETCH:200000
opcontrol --reset
echo "llds w/${i} threads & ${o} items"
mknod /dev/llds c 834 0
rmmod llds
insmod ~/llds/linux/llds/llds.ko
free
time ~/llds/tests/rbtree/rb_llds ${i} ${o}
free
opreport -l > /var/tmp/rb_llds_l1d_pf_${i}_${o}
opannotate --source --output-dir=/var/tmp/rb_llds_l1d_prefetch_${i}_${o}
opannotate --assembly > /var/tmp/rb_llds_l1d_prefetch_asm_${i}_${o}
~/llds/tests/rbtree/rb_rmtree
done
done
