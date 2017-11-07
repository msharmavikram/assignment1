grep -i "sim_seconds" $1 >> report.txt
grep -i "sim_ticks" $1 >> report.txt
grep -i "sim_insts" $1 >> report.txt
grep -i "sim_ops" $1 >> report.txt
grep -i "host_inst_rate" $1 >> report.txt 
grep -i "system.cpu.*numCycles" $1 >> report.txt
grep -i "system.cpu.*committedInsts" $1 >>report.txt
grep -i "system.cpu.*committedOps" $1 >>report.txt
grep -i "system.cpu.*dcache.overall_hits::total" $1 >> report.txt
grep -i "system.cpu.*dcache.overall_misses::total" $1 >> report.txt
grep -i "system.l2.*overall_hits::total" $1 >> report.txt
grep -i "system.l2.*overall_misses::total" $1 >> report.txt
grep -i "system.l3.*overall_hits::total" $1 >> report.txt
grep -i "system.l3.*overall_misses::total" $1 >> report.txt
grep -i "system.cpu.*writebacks*" $1 >> report.txt
grep -i "system.l2.*writebacks*" $1 >> report.txt
grep -i "system.l3.*writebacks*" $1 >> report.txt
echo "*********Additional data************" >> report.txt
grep -i "system.mem_ctrls.avgRdBW" $1 >>report.txt
grep -i "system.mem_ctrls.avgWrBW" $1 >>report.txt
grep -i "system.mem_ctrls.peakBW" $1 >>report.txt
grep -i "system.mem_ctrls.pageHitRate" $1 >>report.txt
grep -i "system.mem_ctrls.bytes_read::cpu.*inst" $1 >> report.txt
grep -i "system.mem_ctrls.bytes_read::cpu.*data" $1 >> report.txt
grep -i "system.cpu.*num_int_insts" $1 >> report.txt
grep -i "system.cpu.*num_fp_insts" $1 >> report.txt
grep -i "system.cpu.*num_vec_insts" $1 >> report.txt
grep -i "system.cpu.*Branches" $1 >> report.txt
grep -i "system.cpu.*dcache.ReadReq_hits::total" $1 >> report.txt
grep -i "system.cpu.*dcache.WriteReq_hits::total" $1 >> report.txt
grep -i "system.cpu.*dcache.ReadReq_misses::total" $1 >> report.txt
grep -i "system.cpu.*dcache.WriteReq_misses::total" $1 >> report.txt
grep -i "system.l2.*ReadReq_hits::total" $1 >> report.txt
grep -i "system.l2.*WriteReq_hits::total" $1 >> report.txt
grep -i "system.l2.*ReadReq_misses::total" $1 >> report.txt
grep -i "system.l2.*WriteReq_misses::total" $1 >> report.txt
grep -i "system.l3.*ReadReq_hits::total" $1 >> report.txt
grep -i "system.l3.*WriteReq_hits::total" $1 >> report.txt
grep -i "system.l3.*ReadReq_misses::total" $1 >> report.txt
grep -i "system.l3.*WriteReq_misses::total" $1 >> report.txt
