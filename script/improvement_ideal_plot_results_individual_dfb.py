#!/usr/bin/env python3
import ast
import random
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure


if __name__ == "__main__":

    # Read already extracted results from the data file
    input_file_name = "improvement_ideal_extracted_results_individual_dfb.dict"
    file = open(input_file_name, "r")
    contents = file.read()
    results = ast.literal_eval(contents)

    workflows = list(results.keys())
    algorithms = list(results['montage-chameleon-2mass-10d-001.json'].keys())
    algorithms = [str(y) for y in sorted([int(x) for x in algorithms])]
    print(algorithms)
    

    workflow_id_map = {}
    workflow_id_map['montage-chameleon-2mass-10d-001.json'] = "1"
    workflow_id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
    workflow_id_map['bwa-chameleon-large-003.json'] = "3"
    workflow_id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
    workflow_id_map['1000genome-chameleon-8ch-250k-001.json'] = "5"
    workflow_id_map['blast-chameleon-medium-002.json'] = "6"
    workflow_id_map['soykb-chameleon-10fastq-20ch-001.json'] = "7"
    workflow_id_map['srasearch-chameleon-10a-003.json'] = "8"
    workflows = dict(sorted(workflow_id_map.items(), key=lambda item: item[1])).keys()


    ### PLOTTING

    fontsize = 18

    output_file = "improvement_ideal_individual_dfb.pdf"
    sys.stderr.write("Generating " + output_file + "...\n")

    f, ax1 = plt.subplots(1, 1, sharey=True, figsize=(12,6))

    data = []

    ytick_labels = []
    for workflow in reversed(workflows):
        ytick_labels.append("W"+workflow_id_map[workflow])
        alg_data = [0] * 36
        sort_orders = sorted(results[workflow].items(), key=lambda x: x[1], reverse=False)
        rank = 1
        for a in sort_orders:
            alg_data[int(a[0])] = rank
            rank += 1
        data.append(alg_data)

    yticks = list(range(0,len(workflows)))
    ax1.set_yticks(yticks)
    ax1.set_yticklabels(ytick_labels, fontsize = fontsize-2)

    xticks = list(range(0,len(algorithms)))
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(xticks, fontsize = fontsize-4)

    ax1.set_ylabel("Workflow", fontsize=fontsize-1)
    ax1.set_xlabel("Algorithm", fontsize=fontsize-1)

    plt.tick_params(left=False, bottom=False)

    plt.imshow(data, cmap='viridis', interpolation='nearest')


    f.tight_layout()
                
    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "\n")

