#!/bin/bash


if [ ! -f ideal_extracted_results.dict ] || [ ! -f work_fraction_extracted_results.dict ] || [ ! -f noise_extracted_results.dict ]; then
    ./extract_all_results.py
fi

# DIVERSITY
./plot_baseline_algorithm_relative_differences.py

# IDEAL
./plot_improvement_ideal_results.py

# WORK FRACTION BY WORKFLOW
./plot_improvement_vs_work_fraction_results.py

# NOISE BY WORKFLOW
./plot_improvement_vs_noise_results.py


