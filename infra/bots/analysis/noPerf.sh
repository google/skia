#!/bin/bash

# Ensure /tmp/allbots.csv has been created.
./create-allbots.sh

# Extract out the list of all cpu_or_gpu_values.
./axis.sh cpu_or_gpu_value,model

# Find all cpus or gpus that we don't Perf by creating a list of all the
# cpu_or_gpu_values associated with Perf jobs, and then print all the rows in
# /tmp/cpu_or_gpu_value,model.csv that don't match that list.
#
# The last join with --np means don't print matches, and --ul specifies to print
# values that are unmatched on the left hand side of the join, i.e. from the
# /tmp/cpu_or_gpu_value,model.csv file.
mlr --csv filter '$Type == "Perf"' /tmp/allbots.csv  | mlr --csv cut -f cpu_or_gpu_value,model | mlr --csv sort -f cpu_or_gpu_value | mlr --csv uniq -f cpu_or_gpu_value | mlr --csv join -f /tmp/cpu_or_gpu_value,model.csv -j cpu_or_gpu_value --ul --np