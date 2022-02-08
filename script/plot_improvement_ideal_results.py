#!/usr/bin/env python3
import ast
import random
import math
import sys
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
from matplotlib.patches import Patch
from matplotlib.lines import Line2D


def plot_data(axis, position, width, data, color):

    spread = [-width + x * 2 * width/len(data) for x in range(0, len(data))]
    random.shuffle(spread)
    idx = 0
    for imp, best in data:
        if best:
            marker = 'v'
        else:
            marker = 'o'

        axis.plot([position + spread[idx]], [imp], marker=marker, color=color, markersize=8)
        idx += 1

    average = sum([x[0] for x in data]) / len(data)
    axis.plot([position - width, position + width], [average, average], '-', linewidth=2, color=color)


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

    baseline_algo = "8"
    algos = [str(x) for x in range(0, 36)]

    display_width = 0.027

    handles = []
    x_value = 0.1
    x_ticks = []
    x_ticklabels = []
    for workflow in workflows:
        x_ticks.append(0.1 * float(workflow_id_map[workflow]))
        x_ticklabels.append("W"+workflow_id_map[workflow])
        y_to_plot = []
        for cluster in clusters:
            baseline_makespan = results[workflow][cluster][baseline_algo]
            us_makespan = results[workflow][cluster]["us"][0]
            baseline_algo_is_best_one_algo = True
            for algo in algos:
                if results[workflow][cluster][algo] < 0.99 * baseline_makespan:
                    baseline_algo_is_best_one_algo = False
                    break
            relative_improvement = 100.0 * (baseline_makespan - us_makespan) / baseline_makespan
            y_to_plot.append([relative_improvement, baseline_algo_is_best_one_algo])

        handles.append(plot_data(ax1, 0.1 * float(workflow_id_map[workflow]),
                                 display_width, y_to_plot, workflow_color_map[workflow]))

    ax1.set_xticks(x_ticks)
    ax1.set_xticklabels(x_ticklabels, rotation=45, fontsize=fontsize)

    ax1.set_ylabel("% makespan improvement", fontsize=fontsize)
    ax1.set_xlabel("Workflow", fontsize=fontsize)

    legend_elements = []
    legend_elements.append(Line2D([0], [0], marker='o', color='k', label='was not best one-algorithm solution',
                                  markerfacecolor='k', markersize=8, linewidth=0))
    legend_elements.append(Line2D([0], [0], marker='v', color='k', label='was best one-algorithm solution',
                                  markerfacecolor='k', markersize=8, linewidth=0))
    legend_elements.append(Line2D([0], [0], color='k', lw=2, label='average improvement'))

    # Create the figure
    ax1.legend(handles=legend_elements, loc='upper center', fontsize=fontsize-2)

    # ax1.legend(handles,
    #            ["W" + workflow_id_map[x] for x in workflows], loc=3,
    #            fontsize=fontsize - 1, ncol=2)


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

