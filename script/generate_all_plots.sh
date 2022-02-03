#!/bin/bash


# IDEAL
if [ ! -f improvement_ideal_extracted_results.dict ]; then
    echo "Extracting results..."
    ./improvement_ideal_extract_results.py
fi
./improvement_ideal_plot_results.py
cp improvement_ideal.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# WORK FRACTION BY CLUSTER
if [ ! -f improvement_vs_work_fraction_extracted_results_by_cluster.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_work_fraction_extract_results.py
fi
./improvement_vs_work_fraction_plot_results.py by_cluster
cp improvement_vs_work_fraction_by_cluster.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# WORK FRACTION BY WORKFLOW
if [ ! -f improvement_vs_work_fraction_extracted_results_by_workflow.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_work_fraction_extract_results.py
fi
./improvement_vs_work_fraction_plot_results.py by_workflow
cp improvement_vs_work_fraction_by_workflow.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# NOISE BY CLUSTER
if [ ! -f improvement_vs_noise_extracted_results_by_cluster.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_noise_extract_results.py
fi
./improvement_vs_noise_plot_results.py by_cluster
cp improvement_vs_noise_by_cluster.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/

# NOISE BY WORKFLOW
if [ ! -f improvement_vs_noise_extracted_results_by_workflow.dict ]; then
    echo "Extracting results..."
    ./improvement_vs_noise_extract_results.py
fi
./improvement_vs_noise_plot_results.py by_workflow
cp improvement_vs_noise_by_workflow.pdf $HOME/PAPERS/WRENCH/wrench-papers/jsspp_2022/figures/




