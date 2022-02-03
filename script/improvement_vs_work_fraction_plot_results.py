#!/usr/bin/env python3
import ast
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure

global collection

if __name__ == "__main__":

    if len(sys.argv) != 2:
        sys.stderr.write("Usage: " + sys.argv[0] + " [by_cluster|by_workflow]\n")
        sys.exit(1)

    if sys.argv[1] != "by_cluster" and sys.argv[1] != "by_workflow":
        sys.stderr.write("Invalid arguments\n")
        sys.exit(1)

    # Read already extracted results from the data file
    input_file_name = "improvement_vs_work_fraction_extracted_results_" + sys.argv[1] + ".dict"
    file = open(input_file_name, "r")
    contents = file.read()
    results = ast.literal_eval(contents)

    speculative_work_fractions = sorted(list(results.keys()))

    color_map = {}
    id_map = {}
    items = {}
    if sys.argv[1] == "by_workflow":
        id_map['montage-chameleon-2mass-10d-001.json'] = "1"
        id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
        id_map['bwa-chameleon-large-003.json'] = "3"
        id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
        id_map['1000genome-chameleon-8ch-250k-001.json'] = "5"
        id_map['blast-chameleon-medium-002.json'] = "6"
        id_map['soykb-chameleon-10fastq-20ch-001.json'] = "7"
        id_map['srasearch-chameleon-10a-003.json'] = "8"
    
        color_map['montage-chameleon-2mass-10d-001.json'] = "red"
        color_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "blue"
        color_map['bwa-chameleon-large-003.json'] = "green"
        color_map['cycles-chameleon-2l-2c-12p-001.json'] = "darkslategray"
        color_map['1000genome-chameleon-8ch-250k-001.json'] = "orange"
        color_map['blast-chameleon-medium-002.json'] = "olive"
        color_map['soykb-chameleon-10fastq-20ch-001.json'] = "chocolate"
        color_map['srasearch-chameleon-10a-003.json'] = "magenta"

    else:
        id_map['96:8:100Gf:100MBps'] = "1"
        id_map['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "2"
        id_map['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "3"
        id_map['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "4"
        id_map['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "5"
        id_map['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "6"
        id_map['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "7"
        id_map['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "8"
        id_map['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "9"

        color_map['96:8:100Gf:100MBps'] = "red"
        color_map['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "blue"
        color_map['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "green"
        color_map['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "darkslategray"
        color_map['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "chocolate"
        color_map['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "orange"
        color_map['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "olive"
        color_map['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "gray"
        color_map['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "magenta"

    items = dict(sorted(id_map.items(), key=lambda item: item[1])).keys()

    output_file = "improvement_vs_work_fraction_"+sys.argv[1]+".pdf"

    f, (ax1, ax2) = plt.subplots(2, 1, sharey=True, figsize=(12,6))

    ax1.yaxis.grid()
    ax2.yaxis.grid()

    item_top_or_bottom = {}
    item_offset = {}

    if sys.argv[1] == "by_workflow":

        key_character = "W"

        item_top_or_bottom['montage-chameleon-2mass-10d-001.json'] = "top"
        item_top_or_bottom['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "top"
        item_top_or_bottom['bwa-chameleon-large-003.json'] = "top"
        item_top_or_bottom['1000genome-chameleon-8ch-250k-001.json'] = "top"

        item_top_or_bottom['cycles-chameleon-2l-2c-12p-001.json'] = "bottom"
        item_top_or_bottom['blast-chameleon-medium-002.json'] = "bottom"
        item_top_or_bottom['soykb-chameleon-10fastq-20ch-001.json'] = "bottom"
        item_top_or_bottom['srasearch-chameleon-10a-003.json'] = "bottom"

        scale = 1.45
        item_offset['montage-chameleon-2mass-10d-001.json'] = -.015 * scale
        item_offset['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = -.005 * scale
        item_offset['bwa-chameleon-large-003.json'] = .005 * scale
        item_offset['1000genome-chameleon-8ch-250k-001.json'] = .015 * scale
    
        item_offset['cycles-chameleon-2l-2c-12p-001.json'] = -.015 * scale
        item_offset['blast-chameleon-medium-002.json'] = -.005 * scale
        item_offset['srasearch-chameleon-10a-003.json'] = .005 * scale
        item_offset['soykb-chameleon-10fastq-20ch-001.json'] = .015 * scale

    else:

        key_character = "P"

        item_top_or_bottom['96:8:100Gf:100MBps'] = "top"
        item_top_or_bottom['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = "top"
        item_top_or_bottom['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = "top"
        item_top_or_bottom['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = "top"
        item_top_or_bottom['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = "top"
        item_top_or_bottom['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = "bottom"
        item_top_or_bottom['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = "bottom"
        item_top_or_bottom['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = "bottom"
        item_top_or_bottom['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = "bottom"
    
        scale = 1.45
        item_offset['96:8:100Gf:100MBps'] = -.02 * scale
        item_offset['48:8:150Gf:100MBps,48:8:50Gf:100MBps'] = -.01 * scale
        item_offset['48:8:400Gf:10MBps,48:8:50Gf:100MBps'] = .0 * scale
        item_offset['32:8:100Gf:100MBps,32:8:200Gf:200MBps,32:8:300Gf:300MBps'] = .01 * scale
        item_offset['32:8:100Gf:100MBps,32:8:200Gf:300MBps,32:8:300Gf:200MBps'] = .02 * scale
    
        item_offset['32:8:100Gf:200MBps,32:8:200Gf:100MBps,32:8:300Gf:300MBps'] = -.015 * scale
        item_offset['32:8:100Gf:200MBps,32:8:200Gf:300MBps,32:8:300Gf:100MBps'] = -.005 * scale
        item_offset['32:8:100Gf:300MBps,32:8:200Gf:200MBps,32:8:300Gf:100MBps'] = .005 * scale
        item_offset['32:8:100Gf:300MBps,32:8:200Gf:100MBps,32:8:300Gf:200MBps'] = .015 * scale

    items = dict(sorted(id_map.items(), key=lambda item: item[1])).keys()
 

    fontsize = 18

    ax1.set_xlim(0.06,0.74)
    ax2.set_xlim(0.06,0.74)

    speculative_work_fractions = [1.0, 0.9, 0.8, 0.6, 0.4, 0.2, 0.1]

    xticks_mapping = {}
    xticks_mapping[1.0] = 0.1;
    xticks_mapping[0.9] = 0.2;
    xticks_mapping[0.8] = 0.3;
    xticks_mapping[0.6] = 0.4;
    xticks_mapping[0.4] = 0.5;
    xticks_mapping[0.2] = 0.6;
    xticks_mapping[0.1] = 0.7;


    violin_width = 0.017
    
    top_violins = []
    bottom_violins = []
    for speculative_work_fraction in speculative_work_fractions:
        for item in items:
            y_to_plot = results[speculative_work_fraction][item]
            offset = item_offset[item]
            if (item_top_or_bottom[item] == "top"):
                position = xticks_mapping[speculative_work_fraction]
                tmp = ax1.violinplot(y_to_plot, positions=[ position + offset], widths=[violin_width], showmeans=True)

                top_violins.append(tmp)
            else:
                position = xticks_mapping[speculative_work_fraction]
                tmp = ax2.violinplot(y_to_plot, positions=[position + offset], widths=[violin_width], showmeans=True)
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
    
    
        ax1.legend([x['bodies'][0] for x in top_violins], [key_character+id_map[x] for x in item_top_or_bottom if item_top_or_bottom[x] == "top"], loc=3, fontsize=fontsize-1, ncol=2)
        ax2.legend([x['bodies'][0] for x in bottom_violins], [key_character+id_map[x] for x in item_top_or_bottom if item_top_or_bottom[x] == "bottom"], loc=3, fontsize=fontsize-1, ncol=2)
        ax1.set_ylim(-150, 100)
    
    xticks = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7]
    ax1.set_xticks(xticks)
    ax1.set_yticklabels(["  -150", "-100", "-50", "0", "50", "100", "150"], fontsize=fontsize-1)
    ax1.set_xticklabels([1.0, 0.9, 0.8, 0.6, 0.4, 0.2, 0.1], fontsize=fontsize)
    ax2.set_xticks(xticks)
    ax2.set_xticklabels([1.0, 0.9, 0.8, 0.6, 0.4, 0.2, 0.1], fontsize=fontsize)
    ax2.set_yticklabels(["  -150", "-100", "-50", "0", "50", "100", "150"], fontsize=fontsize-1)
    ax2.set_xlabel("Simulated work fraction", fontsize=fontsize)

    f.text(0.00, 0.5, '% makespan improvement', fontsize=fontsize, va='center', rotation='vertical')


    plt.tight_layout()
    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "...\n")
