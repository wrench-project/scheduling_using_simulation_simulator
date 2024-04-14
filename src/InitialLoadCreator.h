#ifndef SCHEDULING_USING_SIMULATION_SIMULATOR_INITIALLOADCREATOR_H
#define SCHEDULING_USING_SIMULATION_SIMULATOR_INITIALLOADCREATOR_H

#include <string>
#include <wrench-dev.h>

class InitialLoadCreator {

public:
    static void create_initial_load(
            double initial_load_max_duration,
            int initial_load_duration_seed,
            double initial_load_prob_core_loaded,
            std::unordered_map<std::shared_ptr<wrench::BareMetalComputeService>, std::map<std::string, unsigned long>> &idle_cores_map);

};

#endif //SCHEDULING_USING_SIMULATION_SIMULATOR_INITIALLOADCREATOR_H
