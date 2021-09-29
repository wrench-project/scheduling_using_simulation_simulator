# "Scheduling using simulation" simulator

Using SimGrid master

Sample command-line:

```
./simulator --cluster cluster1:32:8:100Gf:100MBps --cluster cluster2:16:4:50Gf:200MBps --cluster cluster3:24:6:80Gf:80MBps  --workflow ../data/genome.json --reference_flops 100Gf  --algorithms 0-8,18,19,22  --first_scheduler_change_trigger 0.05 --periodic_scheduler_change_trigger 0.1 --speculative_work_fraction 0.1
```

