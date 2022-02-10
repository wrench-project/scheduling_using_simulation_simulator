# "Scheduling using simulation" simulator

This project contains the code and data for evaluating the potential merit
of a "scheduling using online-simulation" approach. The key idea is that
rather than executing a particular scheduling algorithm for the entire
execution of some parallel/distributed application on parallel/distributed
resources, it may be beneficial instead to select which scheduling
algorithm to use at runtime, thereby using perhaps more than one scheduling
algorithm during the application execution. Algorithm selection is made
based on simulation results, that is, one executes simulations of an
application execution throughout that very execution!

This repository contains:
    - The code of the simulator 
    - Simulation input data, output data, and scripts for a [JSSPP](https://jsspp.org/) submission (so as to foster reproducible research), along with a Dockerfile to run everything in a Docker container


## Building the simulator from source

### Prerequisites and Dependencies

- **g++** (version 6.3 or higher) or (**clang** - version 3.8 or higher)
- **CMake** - version 3.7 or higher
- [PugiXML](http://pugixml.org/) - version 1.8 or higher
- [JSON for Modern C++](https://github.com/nlohmann/json) - version 3.9.0 or higher
- [SimGrid](https://framagit.org/simgrid/simgrid/-/releases) - version 3.29
- [WRENCH](https://framagit.org/simgrid/simgrid/-/releases) - version 1.10

### Building instructions

```bash
cd simulator
mkdir build
cd build
cmake ..
make
```




