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

    return workflow;
}