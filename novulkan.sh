#!/bin/bash

# Find all cpus or gpus that we don't Test or Perf with Vulkan.
mlr --csv filter '$vulkan == "true"' allbots.csv  | mlr --csv cut -f cpu_or_gpu_value,model | mlr --csv sort -f cpu_or_gpu_value | mlr --csv uniq -f cpu_or_gpu_value | mlr --csv join -f cpu_or_gpu_value.csv -j cpu_or_gpu_value --ul --np