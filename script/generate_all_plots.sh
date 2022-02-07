#!/bin/bash

if [ ! -f ideal_extracted_results.dict ] || [ ! -f work_fraction_extracted_results.dict ] || [ ! -f noise_extracted_results.dict.dict ] || [ ! -f frequency_noise_extracted_results.dict ]; then
    ./extract_all_results.py
fi

# DIVERSITY
./plot_baseline_algorithm_relative_differences.py

# IDEAL
./plot_improvement_ideal_results.py

# WORK FRACTION BY WORKFLOW
./improvement_vs_work_fraction_plot_results.py by_workflow

# NOISE BY WORKFLOW
./improvement_vs_noise_plot_results.py by_workflow

# FREQUENCY BY WORKFLOW
./improvement_vs_frequency_extract_results.py

noises=("0.0" "0.1" "0.2" "0.4" "0.8")
for noise in ${noises[@]}; do
    ./improvement_vs_frequency_plot_results.py $noise
done







