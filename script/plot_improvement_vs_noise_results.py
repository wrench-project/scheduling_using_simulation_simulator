#!/usr/bin/env python3
import ast
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure


def plot_violin(axis, position, width, data, color, alpha):

    # color = matplotlib.colors.to_rgba("red", alpha)
    v = axis.violinplot(data, positions=[position], widths=[width], showmeans=True)
    for pc in v['bodies']:
        pc.set_color(color)
        pc.set_alpha(alpha)
    bar_color = "black"
    bar_linewidth = 1.15
    v['cmaxes'].set_color(bar_color)
    v['cmaxes'].set_linewidth(bar_linewidth)
    v['cmins'].set_color(bar_color)
    v['cmins'].set_linewidth(bar_linewidth)
    v['cbars'].set_color(bar_color)
    v['cbars'].set_linewidth(bar_linewidth)
    v['cmeans'].set_color(bar_color)
    v['cmeans'].set_linewidth(bar_linewidth)
    return v


if __name__ == "__main__":

    # Read already extracted results from the data file
    input_file_name = "noise_extracted_results.dict"
    try:
        file = open(input_file_name, "r")
    except OSError:
        sys.stderr.write("Can't find extracted result file. Start Mongo and run the extract_all_results.py script first!\n");
        sys.exit(1)

    contents = file.read()
    results = ast.literal_eval(contents)

    noises = [0.0, 0.1, 0.2, 0.4, 0.8]
    workflows = list(results[0.0].keys())
    clusters = list(results[0.0][workflows[0]].keys())

    # Create violin plot figures for several algos across all experimental scenarios
    algos = ["8", "20", "31", "15", "9"]
    available_colors = ["blue", "red", "green", "orange", "magenta"]
    algo_color_map = {}
    index = 0
    for algo in algos:
        algo_color_map[algo] = available_colors[index]
        index += 1
        if index >= len(available_colors):
            index = 0

    algo_position_offset = {}
    position_increment = 0.013
    violin_width = 0.010
    starting_position_offset = 0.0 - int(len(algos)/2) * position_increment
    for algo in algos:
        algo_position_offset[algo] = starting_position_offset
        starting_position_offset += position_increment

    f, ax1 = plt.subplots(1, 1, sharey=True, figsize=(12, 6))

    ax1.yaxis.grid()
    fontsize = 18

    ax1.set_xlim(0.06, 0.54)

    xticks_mapping = {}
    increment = 0.1
    for noise in noises:
        xticks_mapping[noise] = increment
        increment += 0.1

    violins = []
    for noise in noises:
        for alg in algos:
            violin_data = []
            for workflow in workflows:
                for cluster in clusters:
                    alg_makespan = results[noise][workflow][cluster][alg]
                    us_makespan = results[noise][workflow][cluster]["us"]
                    #violin_data.append(100.0*(alg_makespan - us_makespan) / alg_makespan)
                    for ms in us_makespan:
                        violin_data.append(100.0*(alg_makespan - ms) / alg_makespan)

            position = xticks_mapping[noise] + algo_position_offset[alg]
            violin = plot_violin(ax1, position, violin_width, violin_data, algo_color_map[alg], 1.0)
            violins.append(violin)

    ax1.legend([x['bodies'][0] for x in violins], [(r"$A_{" + a + r"}$") for a in algos], loc=3, fontsize=fontsize-1, ncol=2)
    ax1.set_ylim(-100, 100)

    xticks = [0.1, 0.2, 0.3, 0.4, 0.5]
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(noises, fontsize=fontsize)
    ax1.set_xlabel("Simulation error range", fontsize=fontsize)
    f.text(0.00, 0.5, '% makespan improvement', fontsize=fontsize, va='center', rotation='vertical')

    plt.tight_layout()

    output_file = "improvement_vs_noise_selected_algos.pdf"

    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "...\n")


    # RESULTS PER WORKFLOW
    workflows = ['montage-chameleon-2mass-10d-001.json',
                 'epigenomics-chameleon-ilmn-4seq-50k-001.json',
                 'bwa-chameleon-large-003.json',
                 'cycles-chameleon-2l-2c-12p-001.json',
                 '1000genome-chameleon-8ch-250k-001.json',
                 'blast-chameleon-medium-002.json',
                 'soykb-chameleon-10fastq-20ch-001.json',
                 'srasearch-chameleon-10a-003.json']

    workflow_id_map = {}
    workflow_id_map['montage-chameleon-2mass-10d-001.json'] = "1"
    workflow_id_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "2"
    workflow_id_map['bwa-chameleon-large-003.json'] = "3"
    workflow_id_map['cycles-chameleon-2l-2c-12p-001.json'] = "4"
    workflow_id_map['1000genome-chameleon-8ch-250k-001.json'] = "5"
    workflow_id_map['blast-chameleon-medium-002.json'] = "6"
    workflow_id_map['soykb-chameleon-10fastq-20ch-001.json'] = "7"
    workflow_id_map['srasearch-chameleon-10a-003.json'] = "8"

    workflow_color_map = {}
    workflow_color_map['montage-chameleon-2mass-10d-001.json'] = "red"
    workflow_color_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "blue"
    workflow_color_map['bwa-chameleon-large-003.json'] = "green"
    workflow_color_map['cycles-chameleon-2l-2c-12p-001.json'] = "darkslategray"
    workflow_color_map['1000genome-chameleon-8ch-250k-001.json'] = "midnightblue"
    workflow_color_map['blast-chameleon-medium-002.json'] = "olive"
    workflow_color_map['soykb-chameleon-10fastq-20ch-001.json'] = "chocolate"
    workflow_color_map['srasearch-chameleon-10a-003.json'] = "magenta"

    workflow_alpha_map = {}
    workflow_alpha_map['montage-chameleon-2mass-10d-001.json'] = 0.6
    workflow_alpha_map['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = 0.5
    workflow_alpha_map['bwa-chameleon-large-003.json'] = 0.4
    workflow_alpha_map['cycles-chameleon-2l-2c-12p-001.json'] = 0.2
    workflow_alpha_map['1000genome-chameleon-8ch-250k-001.json'] = 0.6
    workflow_alpha_map['blast-chameleon-medium-002.json'] = 0.5
    workflow_alpha_map['soykb-chameleon-10fastq-20ch-001.json'] = 0.4
    workflow_alpha_map['srasearch-chameleon-10a-003.json'] = 0.2

    f, (ax1, ax2) = plt.subplots(2, 1, sharey=True, figsize=(12,6))

    ax1.yaxis.grid()
    ax2.yaxis.grid()

    workflow_top_or_bottom = {}
    workflow_offset = {}

    key_character = "W"

    workflow_top_or_bottom['montage-chameleon-2mass-10d-001.json'] = "top"
    workflow_top_or_bottom['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = "top"
    workflow_top_or_bottom['bwa-chameleon-large-003.json'] = "top"
    workflow_top_or_bottom['cycles-chameleon-2l-2c-12p-001.json'] = "top"

    workflow_top_or_bottom['1000genome-chameleon-8ch-250k-001.json'] = "bottom"
    workflow_top_or_bottom['blast-chameleon-medium-002.json'] = "bottom"
    workflow_top_or_bottom['soykb-chameleon-10fastq-20ch-001.json'] = "bottom"
    workflow_top_or_bottom['srasearch-chameleon-10a-003.json'] = "bottom"

    scale = 0.8
    workflow_offset['montage-chameleon-2mass-10d-001.json'] = -.015 * scale
    workflow_offset['epigenomics-chameleon-ilmn-4seq-50k-001.json'] = -.005 * scale
    workflow_offset['bwa-chameleon-large-003.json'] = .005 * scale
    workflow_offset['cycles-chameleon-2l-2c-12p-001.json'] = .015 * scale

    workflow_offset['1000genome-chameleon-8ch-250k-001.json'] = -.015 * scale
    workflow_offset['blast-chameleon-medium-002.json'] = -.005 * scale
    workflow_offset['soykb-chameleon-10fastq-20ch-001.json'] = .005 * scale
    workflow_offset['srasearch-chameleon-10a-003.json'] = .015 * scale

    fontsize = 18

    ax1.set_xlim(0.06, 0.54)
    ax2.set_xlim(0.06, 0.54)

    violin_width = 0.017

    alg = "8"
    
    top_violins = []
    bottom_violins = []
    for noise in noises:
        for workflow in workflows:
            violin_data = []
            for cluster in clusters:
                alg_makespan = results[noise][workflow][cluster][alg]
                #us_makespan = results[noise][workflow][cluster]["us"]
                us_makespans = results[noise][workflow][cluster]["us"]
                #violin_data.append(100.0 * (alg_makespan - us_makespan) / alg_makespan)
                for ms in us_makespans:
                    violin_data.append(100.0 * (alg_makespan - ms) / alg_makespan)
            
            offset = workflow_offset[workflow]
            if workflow_top_or_bottom[workflow] == "top":
                position = xticks_mapping[noise] + workflow_offset[workflow]
                top_violins.append(plot_violin(ax1, position + offset, violin_width, violin_data, workflow_color_map[workflow], workflow_alpha_map[workflow]))
            else:
                position = xticks_mapping[noise] + workflow_offset[workflow]
                bottom_violins.append(plot_violin(ax2, position + offset, violin_width, violin_data, workflow_color_map[workflow], workflow_alpha_map[workflow]))


        ax1.legend([x['bodies'][0] for x in top_violins], ["W"+workflow_id_map[x] for x in workflow_top_or_bottom if workflow_top_or_bottom[x] == "top"], loc=3, fontsize=fontsize-1, ncol=2)
        ax1.set_ylim(-100, 100)
        ax2.legend([x['bodies'][0] for x in bottom_violins], ["W"+workflow_id_map[x] for x in workflow_top_or_bottom if workflow_top_or_bottom[x] == "bottom"], loc=3, fontsize=fontsize-1, ncol=2)
        ax2.set_ylim(-100, 100)

    xticks = [0.1, 0.2, 0.3, 0.4, 0.5]
    ax1.set_xticks(xticks)
    ax1.set_xticklabels(noises, fontsize=fontsize)
    ax1.set_yticklabels(["  -150", "-100", "-50", "0", "50", "100", "150"], fontsize=fontsize - 1)
    ax2.set_xticks(xticks)
    ax2.set_xticklabels(noises, fontsize=fontsize)
    ax2.set_yticklabels(["  -150", "-100", "-50", "0", "50", "100", "150"], fontsize=fontsize - 1)
    ax2.set_xlabel("Simulation error range " + r"($e$)", fontsize=fontsize)
    f.text(0.00, 0.5, '% makespan improvement', fontsize=fontsize, va='center', rotation='vertical')

    plt.tight_layout()

    output_file = "improvement_vs_noise_one_algo_per_workflow.pdf"

    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "...\n")
