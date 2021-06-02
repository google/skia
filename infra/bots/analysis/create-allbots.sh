#!/bin/bash

# Create a file /tmp/allbots.csv by parsing jobs.json.

set -e

# Write the starting set of headers.
echo Type,os,compiler,model,cpu_or_gpu,cpu_or_gpu_value,arch,configuration,test_filter,extra > /tmp/allbots.csv

# Extract the CSV values from the jobs.json file.
# The seds parse up the job names into columns, ensuring that every column has a value.
cat ../jobs.json | jq .[] -r | grep "^[Perf|Test]" | sed "s#-#,#g" | sed "s#All\$#All,none#g" >> /tmp/allbots.csv

# Add the vulkan column.
mlr --csv -I put '$vulkan=$extra =~ "Vulkan"' /tmp/allbots.csv

# Add the metal column.
mlr --csv -I put '$metal=$extra =~ "Metal"' /tmp/allbots.csv

# Add the skpbench column.
mlr --csv -I put '$skpbench=$extra =~ "Skpbench"' /tmp/allbots.csv

# Validate the output file is a valid CSV file.
mlr --icsv check /tmp/allbots.csv
