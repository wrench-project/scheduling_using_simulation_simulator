#!/opt/local/bin/python3.9

import sys

from wfcommons.wfchef.recipes import BlastRecipe
from wfcommons.wfchef.recipes import MontageRecipe
from wfcommons.wfchef.recipes import SoykbRecipe

from wfcommons import WorkflowGenerator

num_samples = 5
num_tasks = [100]

generators = {}

for task in num_tasks:
    generators["Blast"]   = WorkflowGenerator(BlastRecipe.from_num_tasks(task))
    generators["Montage"] = WorkflowGenerator(MontageRecipe.from_num_tasks(task))
    generators["Soykb"]   = WorkflowGenerator(SoykbRecipe.from_num_tasks(task))

    for generator in generators:
        sys.stderr.write("Generating " + str(num_samples) + " " + generator + " workflows\n")
        workflows = generators[generator].build_workflows(num_samples)
        for i, workflow in enumerate(workflows):
            workflow.write_json(f'{generator}-workflow-{task}-{i}.json')
