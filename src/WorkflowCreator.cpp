#include <algorithm>

#include "WorkflowCreator.h"
#include "PlatformCreator.h"
#include "SimpleStandardJobScheduler.h"
#include <wrench.h>
#include <wrench/util/UnitParser.h>


std::shared_ptr<wrench::Workflow> WorkflowCreator::create_workflow(
        std::string &workflow_file,
        std::string &reference_flops,
        double file_size_factor,
        int initial_load_max_duration,
        int initial_load_duration_seed,
        double initial_load_prob_core_loaded,
        const std::string& cluster_specs) {

    // Parse the workflow
    // As a performance optimization, in this whole simulator, instead of calling getMinNumCores() and getMaxNumCores(), we just
    // hardcode 1 and 64. Check out the macros.
    auto workflow = wrench::WfCommonsWorkflowParser::createWorkflowFromJSON(
            workflow_file, reference_flops, true, false, true, 1, 64, true, true, false);

    // Apply the file size factor
    if (file_size_factor != 1.0) {
        // Compute the number of hosts
        for (auto const &file: workflow->getFileMap()) {
            file.second->setSize(file.second->getSize() * file_size_factor);
        }
    }

    // Add the fictitious tasks to implement initial loads
    if (initial_load_max_duration > 0) {
        // Calculate the total number of cores
        int total_num_cores = 0;
        auto tokens = SimpleStandardJobScheduler::stringSplit(cluster_specs, ',');
        double highest_flop_rate = 0.0;
        double lowest_flop_rate = DBL_MAX;
        for (auto const &cluster_spec : tokens) {
            std::tuple<int, int, std::string, std::string, std::string, std::string> specs = PlatformCreator::parseClusterSpecification(cluster_spec);
            auto [num_hosts, num_cores, flop_rate_str, ignore2, ignore3, ignore4] = PlatformCreator::parseClusterSpecification(cluster_spec);
            double flop_rate = wrench::UnitParser::parse_compute_speed(flop_rate_str);
            highest_flop_rate = std::max(highest_flop_rate, flop_rate);
            lowest_flop_rate = std::min(lowest_flop_rate, flop_rate);
            total_num_cores += num_hosts * num_cores;
        }

        // Create one fictitious task, making sure it runs in 0.5s or less on any core
        {
            auto short_task = workflow->addTask("initial_load_task_short", lowest_flop_rate  * 0.5, 1, 1, 0.0);
            for (auto const &entry_task : workflow->getEntryTasks()) {
                workflow->addControlDependency(short_task, entry_task);
            }
        }

        // Compute the number of other fictitious tasks to create based on the probability of being loaded
        int num_initial_load_tasks = (int) std::floor(initial_load_prob_core_loaded * (total_num_cores -1));

        // Create  fictitious tasks, making sure that each of them
        // runs for at least 1s on any core
        auto macro_random_dist = new std::uniform_real_distribution<double>(highest_flop_rate * 1.0, highest_flop_rate * initial_load_max_duration);
        auto macro_rng = new std::mt19937(initial_load_duration_seed);
        for (int i=0; i < num_initial_load_tasks - 1; i++) {
            double flops = (*macro_random_dist)(*macro_rng);
            workflow->addTask("initial_load_task_" + std::to_string(i), flops, 1, 1, 0.0);
        }
    }

    return workflow;
}