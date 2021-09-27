# "Scheduling using simulation" simulator

Using SimGrid master

Sample command-line:

```
./simulator --cluster cluster1:32:8:100Gf:100MBps --cluster cluster2:16:4:50Gf:200MBps --cluster cluster3:24:6:80Gf:80MBps  --workflow ../data/genome.json --reference_flops 100Gf  --initial_scheduler most_flops:fastest_cores:minimum
```

