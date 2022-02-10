# "Scheduling using simulation" simulator

This project contains the code and data for evaluating the potential merit
of a "scheduling using online-simulation" approach. The key idea is that
rather than executing a particular scheduling algorithm for the entire
execution of some parallel/distributed application on parallel/distributed
resources, it may be beneficial instead to select which scheduling
algorithm to use at runtime, thereby using perhaps more than one scheduling
algorithm during the application execution. Algorithm selection is made
based on simulation results, that is, one executes simulations of an
application execution throughout that very execution! This simulator
uses this appraoch (yes, simulation within simulation) in the context
of scientific workflow applications.

## Simulator usage

After building the simulator (see instructions below), just invoke it
as `./scheduling_using_simulations_simulator --help` to see the (long and detailed) usage message.

A sample invocation could be:

```
./scheduling_using_simulations_simulator_simulator --clusters 32:8:100Gf:100MBps,16:4:50Gf:200MBps,24:6:80Gf:80MBps  --workflow ../sample_workflow/genome.json --reference_flops 100Gf  --algorithms 0-8,18,19 --first_scheduler_change_trigger 0.00 --periodic_scheduler_change_trigger 0.1 --speculative_work_fraction 0.1
```

## Building the simulator from source

### Prerequisites and Dependencies

- **g++** (version 6.3 or higher) or (**clang** - version 3.8 or higher)
- **CMake** - version 3.7 or higher
- [PugiXML](http://pugixml.org/) - version 1.8 or higher
- [JSON for Modern C++](https://github.com/nlohmann/json) - version 3.9.0 or higher
- [SimGrid](https://framagit.org/simgrid/simgrid/-/releases) - version 3.29
- [WRENCH](https://framagit.org/simgrid/simgrid/-/releases) - version 1.10 or later

### Building instructions

```bash
cd simulator
mkdir build
cd build
cmake ..
make
```




