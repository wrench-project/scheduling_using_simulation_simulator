# "Scheduling using simulation" simulator

Using SimGrid master

Sample command-line:

```
./simulator --cluster cluster1:32:8:100Gf:100MBps --cluster cluster2:16:4:50Gf:200MBps --cluster cluster3:24:6:80Gf:80MBps  --workflow ../data/genome.json --reference_flops 100Gf  --initial_scheduler most_flops:fastest_cores:minimum --scheduler most_flops:fastest_cores:as_many_as_possible --first_scheduler_change_trigger 0.05 --periodic_scheduler_change_trigger 0.1 --speculative_work_fraction 0.5 
```

```
./simulator --cluster cluster1:32:8:100Gf:100MBps --cluster cluster2:16:4:50Gf:200MBps --cluster cluster3:24:6:80Gf:80MBps  --workflow ../data/genome.json --reference_flops 100Gf --first_scheduler_change_trigger 0.0 --periodic_scheduler_change_trigger 0.1 --speculative_work_fraction 0.1 --initial_scheduler most_data:fastest_cores:as_many_as_possible --scheduler most_data:fastest_cores:minimum --scheduler most_data:fastest_cores:parallel_efficiency_fifty_percent --scheduler most_data:least_idle_cores:as_many_as_possible --scheduler most_data:least_idle_cores:minimum --scheduler most_data:least_idle_cores:parallel_efficiency_fifty_percent --scheduler most_data:most_idle_cores:as_many_as_possible --scheduler most_data:most_idle_cores:minimum --scheduler most_data:most_idle_cores:parallel_efficiency_fifty_percent --scheduler most_data:most_local_data:as_many_as_possible --scheduler most_data:most_local_data:minimum --scheduler most_data:most_local_data:parallel_efficiency_fifty_percent --scheduler most_flops:fastest_cores:as_many_as_possible --scheduler most_flops:fastest_cores:minimum --scheduler most_flops:fastest_cores:parallel_efficiency_fifty_percent --scheduler most_flops:least_idle_cores:as_many_as_possible --scheduler most_flops:least_idle_cores:minimum --scheduler most_flops:least_idle_cores:parallel_efficiency_fifty_percent --scheduler most_flops:most_idle_cores:as_many_as_possible --scheduler most_flops:most_idle_cores:minimum --scheduler most_flops:most_idle_cores:parallel_efficiency_fifty_percent --scheduler most_flops:most_local_data:as_many_as_possible --scheduler most_flops:most_local_data:minimum most_flops:most_local_data:parallel_efficiency_fifty_percent
```
