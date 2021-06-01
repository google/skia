#!/bin/bash

set -e

# Write the starting set of headers.
echo Type,os,compiler,model,cpu_or_gpu,cpu_or_gpu_value,arch,configuration,test_filter,extra > allbots.csv

# Extract the CSV values from the jobs.json file.
cat infra/bots/jobs.json | jq .[] -r | grep "^[Perf|Test]" | sed "s#-#,#g" | sed "s#All\$#All,none#g" >> allbots.csv

# Add the vulkan column.
mlr --csv -I put '$vulkan=$extra =~ "Vulkan"' allbots.csv

# Add the metal column.
mlr --csv -I put '$metal=$extra =~ "Metal"' allbots.csv

# Add the skpbench column.
mlr --csv -I put '$skpbench=$extra =~ "Skpbench"' allbots.csv

# Validate the output file.
mlr --icsv check allbots.csv
