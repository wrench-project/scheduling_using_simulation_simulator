#!/usr/bin/env python3
import ast
import random
import math
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure


def plot_violin(axis, position, width, data, color):

    v = axis.violinplot(data, positions=[position], widths=[width], showmeans=True)
    for pc in v['bodies']:
        pc.set_facecolor(color)
        pc.set_facecolor(color)
        pc.set_edgecolor(color)
    v['cmaxes'].set_color(color)
    v['cmaxes'].set_linewidth(2)
    v['cmins'].set_color(color)
    v['cbars'].set_color(color)
    v['cbars'].set_linewidth(1)
    v['cmeans'].set_color(color)
    return v


if __name__ == "__main__":

    # Read already extracted results from the data file
    try:
        input_file_name = "ideal_extracted_results.dict"
        file = open(input_file_name, "r")
    except OSError:
        sys.stderr.write("Can't find extracted result file. Start Mongo and run the extract_all_results.py script first!\n");
        sys.exit(1)


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

    violin_width = 0.03
    algos = ["8"]
    
    algo_position_offset = {}
    violin_width = 0.060 / len(algos)
    position_increment = violin_width + 0.005
    starting_position_offset = 0.0 - int(len(algos)/2) * position_increment + (1 - len(algos)%2) * position_increment/2
    for algo in algos:
        algo_position_offset[algo] = starting_position_offset
        starting_position_offset += position_increment


    for algo in algos:
        x_value = 0.1
        x_ticks = []
        x_ticklabels = []
        for workflow in workflows:
            x_ticks.append(0.1 * float(workflow_id_map[workflow]))
            x_ticklabels.append("W"+workflow_id_map[workflow])
            y_to_plot = []
            for cluster in clusters:
                algo_makespan = results[workflow][cluster][algo]
                us_makespan = results[workflow][cluster]["us"][0]
                relative_improvement = 100.0 * (algo_makespan - us_makespan) / algo_makespan
                y_to_plot.append(relative_improvement)
        
            plot_violin(ax1, 0.1 * float(workflow_id_map[workflow]) + algo_position_offset[algo] , violin_width, y_to_plot, workflow_color_map[workflow])


    ax1.set_xticks(x_ticks)
    ax1.set_xticklabels(x_ticklabels, rotation=45, fontsize=fontsize)

    ax1.set_ylabel("% makespan improvement", fontsize=fontsize)
    ax1.set_xlabel("Workflow", fontsize=fontsize)

    plt.yticks(fontsize=fontsize)
    f.tight_layout()
                
    plt.savefig(output_file)
    plt.close()

    sys.stderr.write("Plot saved to file " + output_file + "\n")

    print("\nWins over the oracle:")
    num_wins_over_oracle = 0
    num_wins_over_oracle_by_more_than_1_percent = 0
    num_wins_over_oracle_by_more_than_5_percent = 0
    for workflow in workflows:
        y_to_plot = []
        for cluster in clusters:
            oracle_makespan = math.inf
            for algo in results[workflow][cluster]:
                if algo == "us":
                    us_makespan = results[workflow][cluster][algo][0]
                else:
                    oracle_makespan = min(oracle_makespan, results[workflow][cluster][algo])
            if us_makespan < oracle_makespan:
                num_wins_over_oracle += 1
            if us_makespan < oracle_makespan*0.95:
                num_wins_over_oracle_by_more_than_5_percent += 1
            if us_makespan < oracle_makespan*0.99:
                num_wins_over_oracle_by_more_than_1_percent += 1
    print("  Number of wins over the oracle: " + str(num_wins_over_oracle) + " / " + str(len(workflows) * len(clusters)))
    print("  Number of wins over the oracle by > 1%: " + str(num_wins_over_oracle_by_more_than_1_percent) + " / " + str(len(workflows) * len(clusters)))
    print("  Number of wins over the oracle by > 5%: " + str(num_wins_over_oracle_by_more_than_5_percent) + " / " + str(len(workflows) * len(clusters)))

