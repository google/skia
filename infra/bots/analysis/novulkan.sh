#!/bin/bash

./create-allbots.sh

./axis.sh cpu_or_gpu_value,model

# Find all cpus or gpus that we don't Test or Perf with Vulkan.
mlr --csv filter '$vulkan == "true"' /tmp/allbots.csv  | mlr --csv cut -f cpu_or_gpu_value,model | mlr --csv sort -f cpu_or_gpu_value | mlr --csv uniq -f cpu_or_gpu_value | mlr --csv join -f /tmp/cpu_or_gpu_value,model.csv -j cpu_or_gpu_value --ul --np