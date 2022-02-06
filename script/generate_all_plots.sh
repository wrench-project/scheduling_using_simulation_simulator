#!/bin/bash

# DISERSITY

# IDEAL
if [ ! -f improvement_ideal_extracted_results.dict ]; then
    echo "Extracting results..."
    ./improvement_ideal_extract_results.py
fi
./improvement_ideal_plot_results.py
echo cp improvement_ideal.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_ideal.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/


# WORK FRACTION BY CLUSTER
if [ ! -f improvement_vs_work_fraction_extracted_results_by_cluster.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_work_fraction_extract_results.py
fi
./improvement_vs_work_fraction_plot_results.py by_cluster
echo cp improvement_vs_work_fraction_by_cluster.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_vs_work_fraction_by_cluster.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/


# WORK FRACTION BY WORKFLOW
if [ ! -f improvement_vs_work_fraction_extracted_results_by_workflow.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_work_fraction_extract_results.py
fi
./improvement_vs_work_fraction_plot_results.py by_workflow
echo cp improvement_vs_work_fraction_by_workflow.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_vs_work_fraction_by_workflow.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# NOISE BY CLUSTER
if [ ! -f improvement_vs_noise_extracted_results_by_cluster.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_noise_extract_results.py
fi
./improvement_vs_noise_plot_results.py by_cluster
echo cp improvement_vs_noise_by_cluster.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_vs_noise_by_cluster.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# NOISE BY WORKFLOW
if [ ! -f improvement_vs_noise_extracted_results_by_workflow.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_noise_extract_results.py
fi
./improvement_vs_noise_plot_results.py by_workflow
echo cp improvement_vs_noise_by_workflow.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_vs_noise_by_workflow.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# FREQUENCY BY WORKFLOW
if [ ! -f improvement_vs_noise_and_frequency_extracted_results_by_workflow.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_frequency_extract_results.py
fi

noises=("0.0" "0.1" "0.2" "0.4" "0.8")
for noise in ${noises[@]}; do
    ./improvement_vs_frequency_plot_results.py $noise
    echo cp improvement_vs_frequency_for_noise_$noise.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
    cp improvement_vs_frequency_for_noise_$noise.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
done

# HEATMAPS
./improvement_ideal_plot_results_algorithm_usage.py
echo cp improvement_ideal_algorithm_usage.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_ideal_algorithm_usage.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

./improvement_ideal_plot_results_individual_dfb.py
echo cp improvement_ideal_individual_dfb.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/
cp improvement_ideal_individual_dfb.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/








