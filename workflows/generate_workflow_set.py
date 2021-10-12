#!/opt/local/bin/python3.9

import sys

#from wfcommons.wfchef.recipes import BlastRecipe
#from wfcommons.wfchef.recipes import MontageRecipe
from wfcommons.wfchef.recipes import *

from wfcommons import WorkflowGenerator

num_samples = 5
num_tasks = [200]

generators = {}

for task in num_tasks:
    generators["Blast"] = WorkflowGenerator(BlastRecipe.from_num_tasks(task))
    generators["Bwa"]  = WorkflowGenerator(BwaRecipe.from_num_tasks(task))
    generators["Cycles"] = WorkflowGenerator(CyclesRecipe.from_num_tasks(task))
    generators["Epigenomics"]  = WorkflowGenerator(EpigenomicsRecipe.from_num_tasks(task))
    generators["Genome"] = WorkflowGenerator(GenomeRecipe.from_num_tasks(task))
    generators["Montage"] = WorkflowGenerator(MontageRecipe.from_num_tasks(task))
    generators["Seismology"] = WorkflowGenerator(SeismologyRecipe.from_num_tasks(task))
    generators["Soykb"] = WorkflowGenerator(SoykbRecipe.from_num_tasks(task))
    generators["Srasearch"] = WorkflowGenerator(SrasearchRecipe.from_num_tasks(task))

    for generator in generators:
        sys.stderr.write("Generating " + str(num_samples) + " " + generator + " workflows\n")
        workflows = generators[generator].build_workflows(num_samples)
        for i, workflow in enumerate(workflows):
            workflow.write_json(f'{generator}-workflow-{task}-{i}.json')
