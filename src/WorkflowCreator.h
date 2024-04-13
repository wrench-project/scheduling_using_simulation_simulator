#ifndef SCHEDULING_USING_SIMULATION_SIMULATOR_WORKFLOWCREATOR_H
#define SCHEDULING_USING_SIMULATION_SIMULATOR_WORKFLOWCREATOR_H

#include <string>
#include <wrench-dev.h>

class WorkflowCreator {

public:
    static std::shared_ptr<wrench::Workflow> create_workflow(
            std::string &workflow_file,
            std::string &reference_flops,
            double file_size_factor,
            int initial_load_max_duration,
            int initial_load_duration_seed,
            double initial_load_prob_core_loaded,
            const std::string& cluster_specs);

};

#endif //SCHEDULING_USING_SIMULATION_SIMULATOR_WORKFLOWCREATOR_H
