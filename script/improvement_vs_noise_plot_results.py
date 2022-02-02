#!/usr/bin/env python3
import ast
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import random

global collection

if __name__ == "__main__":


    # Read already extracted results from the data file
    input_file_name = "improvement_vs_noise_extracted_results.dict"
    file = open(input_file_name, "r")
    contents = file.read()
    results = ast.literal_eval(contents)

    noises = sorted(list(results.keys()))

    workflow_id_map = {}
    workflow_id_map['montage-chameleon-2mass-10d-001.json'] = "1"
    workflow_id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
    workflow_id_map['bwa-chameleon-large-003.json'] = "3"
    workflow_id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
    workflow_id_map['seismology-chameleon-700p-001.json'] = "5"
    workflow_id_map['1000genome-chameleon-8ch-250k-001.json'] = "6"
    workflow_id_map['blast-chameleon-medium-002.json'] = "7"
    workflow_id_map['soykb-chameleon-10fastq-20ch-001.json'] = "8"
    workflow_id_map['srasearch-chameleon-10a-003.json'] = "9"
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

    workflow_color_map = {}
    workflow_color_map['montage-chameleon-2mass-10d-001.json'] = "red"
    workflow_color_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "blue"
    workflow_color_map['bwa-chameleon-large-003.json'] = "green"
    workflow_color_map['cycles-chameleon-2l-2c-12p-001.json'] = "darkslategray"
    workflow_color_map['seismology-chameleon-700p-001.json'] = "chocolate"
    workflow_color_map['1000genome-chameleon-8ch-250k-001.json'] = "orange"
    workflow_color_map['blast-chameleon-medium-002.json'] = "olive"
    workflow_color_map['soykb-chameleon-10fastq-20ch-001.json'] = "gray"
    workflow_color_map['srasearch-chameleon-10a-003.json'] = "magenta"


    output_file = "improvement_vs_noise.pdf"

    f, (ax1, ax2) = plt.subplots(2, 1, sharey=True, figsize=(12,6))
    
    ax1.yaxis.grid()
    ax2.yaxis.grid()

    workflow_top_or_bottom = {}
    workflow_top_or_bottom['montage-chameleon-2mass-10d-001.json'] = "top"
    workflow_top_or_bottom['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "top"
    workflow_top_or_bottom['bwa-chameleon-large-003.json'] = "top"
    workflow_top_or_bottom['cycles-chameleon-2l-2c-12p-001.json'] = "top"
    workflow_top_or_bottom['seismology-chameleon-700p-001.json'] = "top"
    workflow_top_or_bottom['1000genome-chameleon-8ch-250k-001.json'] = "bottom"
    workflow_top_or_bottom['blast-chameleon-medium-002.json'] = "bottom"
    workflow_top_or_bottom['soykb-chameleon-10fastq-20ch-001.json'] = "bottom"
    workflow_top_or_bottom['srasearch-chameleon-10a-003.json'] = "bottom"


    workflow_offset = {}
    scale = 1.2
    workflow_offset['montage-chameleon-2mass-10d-001.json'] = -.02 * scale
    workflow_offset['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = -.01 * scale
    workflow_offset['bwa-chameleon-large-003.json'] = .0 * scale
    workflow_offset['cycles-chameleon-2l-2c-12p-001.json'] = +.01 * scale
    workflow_offset['seismology-chameleon-700p-001.json'] = +.02 * scale

    workflow_offset['1000genome-chameleon-8ch-250k-001.json'] = -.015 * scale
    workflow_offset['soykb-chameleon-10fastq-20ch-001.json'] = -.005 * scale
    workflow_offset['blast-chameleon-medium-002.json'] = .005 * scale
    workflow_offset['srasearch-chameleon-10a-003.json'] = .015 * scale


    ax1.set_xlim(-0.04,0.44)
    ax2.set_xlim(-0.04,0.44)

    mapping_to_x_axis = {}
    mapping_to_x_axis[0.0] = 0.0
    mapping_to_x_axis[0.1] = 0.1
    mapping_to_x_axis[0.2] = 0.2
    mapping_to_x_axis[0.4] = 0.3
    mapping_to_x_axis[0.8] = 0.4

    violin_width = 0.013
    top_violins = []
    bottom_violins = []
    for simulation_noise in noises:
        for workflow in workflows:
            y_to_plot = results[simulation_noise][workflow]
            offset = workflow_offset[workflow]
            if (workflow_top_or_bottom[workflow] == "top"):
                tmp = ax1.violinplot(y_to_plot, positions=[mapping_to_x_axis[simulation_noise] + offset], widths=[violin_width], showmeans=True)
                top_violins.append(tmp)
            else:
                tmp = ax2.violinplot(y_to_plot, positions=[mapping_to_x_axis[simulation_noise] + offset -0.01], widths=[violin_width], showmeans=True)
                bottom_violins.append(tmp)
            for pc in tmp['bodies']:
                pc.set_facecolor(workflow_color_map[workflow])
                pc.set_facecolor(workflow_color_map[workflow])
                pc.set_edgecolor(workflow_color_map[workflow])
            tmp['cmaxes'].set_color(workflow_color_map[workflow])
            tmp['cmaxes'].set_linewidth(2)
            tmp['cmins'].set_color(workflow_color_map[workflow])
            tmp['cbars'].set_color(workflow_color_map[workflow])
            tmp['cbars'].set_linewidth(1)
            tmp['cmeans'].set_color(workflow_color_map[workflow])


    ax1.legend([x['bodies'][0] for x in top_violins], ["W"+workflow_id_map[x] for x in workflow_top_or_bottom if workflow_top_or_bottom[x] == "top"], loc=3)
    ax2.legend([x['bodies'][0] for x in bottom_violins], ["W"+workflow_id_map[x] for x in workflow_top_or_bottom if workflow_top_or_bottom[x] == "bottom"], loc=3)
    ax1.set_ylim(-150, 100)

    xticks = [0.0, 0.1, 0.2, 0.3, 0.4]
    xticks_labels = [0.0, 0.1, 0.2, 0.4, 0.8]
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(xticks_labels)
    ax1.set_ylabel("% makespan improvement")
    ax2.set_xticks(xticks)
    ax2.set_xticklabels(xticks_labels)
    ax2.set_ylabel("% makespan improvement")
    ax2.set_xlabel("Simulation noise")


    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "...\n")

