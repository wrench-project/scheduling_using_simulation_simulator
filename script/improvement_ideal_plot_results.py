#!/usr/bin/env python3
import ast
import random
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure


if __name__ == "__main__":

    # Read already extracted results from the data file
    try:
        input_file_name = "ideal_extracted_results.dict"
    except OSError:
        sys.stderr.write("Can't open file 'ideal_extracted_results.dict'. Start Mongo and run the ideal_extract_results.py script first!\n");

    file = open(input_file_name, "r")
    contents = file.read()
    results = ast.literal_eval(contents)

    workflows = sorted(list(results.keys()))
    clusters = sorted(list(results[list(results.keys())[0]].keys()))

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

    clusters_id_map = {}
    clusters_id_map['96:8:100Gf:100MBps'] = "1"
    clusters_id_map['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "2"
    clusters_id_map['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "3"
    clusters_id_map['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "4"
    clusters_id_map['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "5"
    clusters_id_map['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "6"
    clusters_id_map['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "7"
    clusters_id_map['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "8"
    clusters_id_map['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "1"
    clusters = dict(sorted(clusters_id_map.items(), key=lambda item: item[1])).keys()

    workflow_color_map = {}
    workflow_color_map['montage-chameleon-2mass-10d-001.json'] = "red"
    workflow_color_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "blue"
    workflow_color_map['bwa-chameleon-large-003.json'] = "green"
    workflow_color_map['cycles-chameleon-2l-2c-12p-001.json'] = "darkslategray"
    workflow_color_map['1000genome-chameleon-8ch-250k-001.json'] = "midnightblue"
    workflow_color_map['blast-chameleon-medium-002.json'] = "olive"
    workflow_color_map['soykb-chameleon-10fastq-20ch-001.json'] = "chocolate"
    workflow_color_map['srasearch-chameleon-10a-003.json'] = "magenta"


    ### PLOTTING

    fontsize = 18

    output_file = "improvement_ideal.pdf"
    sys.stderr.write("Generating " + output_file + "...\n")

    f, ax1 = plt.subplots(1, 1, sharey=True, figsize=(12,6))
    
    ax1.yaxis.grid()

    x_value = 0.1
    x_ticks = []
    x_ticklabels = []
    for workflow in workflows:
        x_ticks.append(x_value)
        #x_ticklabels.append(workflow.split('-')[0])
        x_ticklabels.append("W"+workflow_id_map[workflow])
        y_to_plot = []
        for cluster in clusters:
            y_to_plot.append(results[workflow][cluster][0])
        x_to_plot = [x_value + random.uniform(-0.02, +0.02) for x in y_to_plot]
        mean_y = sum(y_to_plot) / len(y_to_plot)

        ax1.plot(x_to_plot,y_to_plot, '.', color=workflow_color_map[workflow], markersize=15)
        ax1.plot([x_value - 0.025, x_value + 0.025], [mean_y, mean_y], color=workflow_color_map[workflow], linewidth=4)
        #sys.stderr.write(workflow + ": average=" + str(mean_y) + "\n")
        x_value += 0.1


    ax1.set_xticks(x_ticks)
    ax1.set_xticklabels(x_ticklabels, rotation=45, fontsize=fontsize)

    ax1.set_ylabel("% makespan improvement", fontsize=fontsize)
    ax1.set_xlabel("Workflow", fontsize=fontsize)

    plt.ylim(-3, 20)
    plt.yticks(fontsize=fontsize)
    f.tight_layout()
                
    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "\n")

