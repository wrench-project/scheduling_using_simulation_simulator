#!/usr/bin/env python3
import ast
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import random

global collection

if __name__ == "__main__":

    if len(sys.argv) != 2:
        sys.stderr.write("Usage: " + sys.argv[0] + " <noise>\n")
        sys.exit(1)

    noise = float(sys.argv[1])

    # Read already extracted results from the data file
    input_file_name = "improvement_vs_noise_and_frequency_extracted_results_by_workflow.dict"
    file = open(input_file_name, "r")
    contents = file.read()
    results = ast.literal_eval(contents)
    noises = sorted(list(results.keys()))
    frequencies = sorted(list(results[0].keys()))

    print(noises)
    print(frequencies)
    color_map = {}
    id_map = {}
    items = {}

    id_map['montage-chameleon-2mass-10d-001.json'] = "1"
    id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
    id_map['bwa-chameleon-large-003.json'] = "3"
    id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
    id_map['seismology-chameleon-700p-001.json'] = "5"
    id_map['1000genome-chameleon-8ch-250k-001.json'] = "6"
    id_map['blast-chameleon-medium-002.json'] = "7"
    id_map['soykb-chameleon-10fastq-20ch-001.json'] = "8"
    id_map['srasearch-chameleon-10a-003.json'] = "9"

    color_map['montage-chameleon-2mass-10d-001.json'] = "red"
    color_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "blue"
    color_map['bwa-chameleon-large-003.json'] = "green"
    color_map['cycles-chameleon-2l-2c-12p-001.json'] = "darkslategray"
    color_map['seismology-chameleon-700p-001.json'] = "chocolate"
    color_map['1000genome-chameleon-8ch-250k-001.json'] = "orange"
    color_map['blast-chameleon-medium-002.json'] = "olive"
    color_map['soykb-chameleon-10fastq-20ch-001.json'] = "gray"
    color_map['srasearch-chameleon-10a-003.json'] = "magenta"

    items = dict(sorted(id_map.items(), key=lambda item: item[1])).keys()

    output_file = "improvement_vs_frequency_for_noise_"+sys.argv[1]+".pdf"

    f, (ax1, ax2) = plt.subplots(2, 1, sharey=True, figsize=(12,6))
    
    ax1.yaxis.grid()
    ax2.yaxis.grid()

    item_top_or_bottom = {}
    item_offset = {}

    key_character = "W"

    item_top_or_bottom['montage-chameleon-2mass-10d-001.json'] = "top"
    item_top_or_bottom['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "top"
    item_top_or_bottom['bwa-chameleon-large-003.json'] = "top"
    item_top_or_bottom['cycles-chameleon-2l-2c-12p-001.json'] = "top"
    item_top_or_bottom['seismology-chameleon-700p-001.json'] = "top"
    item_top_or_bottom['1000genome-chameleon-8ch-250k-001.json'] = "bottom"
    item_top_or_bottom['blast-chameleon-medium-002.json'] = "bottom"
    item_top_or_bottom['soykb-chameleon-10fastq-20ch-001.json'] = "bottom"
    item_top_or_bottom['srasearch-chameleon-10a-003.json'] = "bottom"

    scale = 1.4
    item_offset['montage-chameleon-2mass-10d-001.json'] = -.02 * scale
    item_offset['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = -.01 * scale
    item_offset['bwa-chameleon-large-003.json'] = .0 * scale
    item_offset['cycles-chameleon-2l-2c-12p-001.json'] = +.01 * scale
    item_offset['seismology-chameleon-700p-001.json'] = +.02 * scale

    item_offset['1000genome-chameleon-8ch-250k-001.json'] = -.015 * scale
    item_offset['soykb-chameleon-10fastq-20ch-001.json'] = -.005 * scale
    item_offset['blast-chameleon-medium-002.json'] = .005 * scale
    item_offset['srasearch-chameleon-10a-003.json'] = .015 * scale

    fontsize = 18

    ax1.set_xlim(-0.04,0.44)
    ax2.set_xlim(-0.04,0.44)

    mapping_to_x_axis = {}
    x_asis_init = 0.0
    for f in frequencies:
        mapping_to_x_axis[f] = x_axis_init
        x_axis_init += 1

    violin_width = 0.017
    top_violins = []
    bottom_violins = []
    for frequency in freqencies:
        for item in items:
            y_to_plot = results[noise][frequency][item]
            offset = item_offset[item]
            if (item_top_or_bottom[item] == "top"):
                tmp = ax1.violinplot(y_to_plot, positions=[mapping_to_x_axis[simulation_noise] + offset], widths=[violin_width], showmeans=True)
                top_violins.append(tmp)
            else:
                tmp = ax2.violinplot(y_to_plot, positions=[mapping_to_x_axis[simulation_noise] + offset -0.00], widths=[violin_width], showmeans=True)
                bottom_violins.append(tmp)
            for pc in tmp['bodies']:
                pc.set_facecolor(color_map[item])
                pc.set_facecolor(color_map[item])
                pc.set_edgecolor(color_map[item])
            tmp['cmaxes'].set_color(color_map[item])
            tmp['cmaxes'].set_linewidth(2)
            tmp['cmins'].set_color(color_map[item])
            tmp['cbars'].set_color(color_map[item])
            tmp['cbars'].set_linewidth(1)
            tmp['cmeans'].set_color(color_map[item])


    fontsize = 18

    ax1.legend([x['bodies'][0] for x in top_violins], [key_character+id_map[x] for x in item_top_or_bottom if item_top_or_bottom[x] == "top"], loc=3, fontsize=fontsize-1, ncol=3)
    ax2.legend([x['bodies'][0] for x in bottom_violins], [key_character+id_map[x] for x in item_top_or_bottom if item_top_or_bottom[x] == "bottom"], loc=3, fontsize=fontsize-1, ncol=2)
    ax1.set_ylim(-150, 100)

    xticks = [0.0, 0.1, 0.2, 0.3, 0.4]
    xticks_labels = [0.0, 0.1, 0.2, 0.4, 0.8]
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(xticks_labels, fontsize = fontsize)
    ax1.set_yticklabels(["  -150", "100", "-50", "0", "50", "100", "150"], fontsize=fontsize-1)

    ax2.set_xticks(xticks)
    ax2.set_xticklabels(xticks_labels, fontsize = fontsize)
    ax2.set_yticklabels(["  -150", "100", "-50", "0", "50", "100", "150"], fontsize=fontsize-1)
    ax2.set_xlabel("Simulation noise", fontsize = fontsize)

    f.text(0.00, 0.5, '% makespan improvement', fontsize=fontsize, va='center', rotation='vertical')
    plt.tight_layout()

    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "...\n")

