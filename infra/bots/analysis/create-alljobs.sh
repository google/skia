#!/bin/bash
# Copyright 2021 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Create a file /tmp/alljobs.csv by parsing jobs.json.

set -e

# Write the starting set of headers.
echo Type,os,compiler,model,cpu_or_gpu,cpu_or_gpu_value,arch,configuration,test_filter,extra > /tmp/alljobs.csv

# Extract the CSV values from the jobs.json file.
# The seds parse up the job names into columns, ensuring that every column has a value.
cat ../jobs.json | jq .[] -r | grep "^[Perf|Test]" | sed "s#-#,#g" | sed "s#All\$#All,none#g" >> /tmp/alljobs.csv

# Add the vulkan column.
mlr --csv -I put '$vulkan=$extra =~ "Vulkan"' /tmp/alljobs.csv

# Add the metal column.
mlr --csv -I put '$metal=$extra =~ "Metal"' /tmp/alljobs.csv

# Add the skpbench column.
mlr --csv -I put '$skpbench=$extra =~ "Skpbench"' /tmp/alljobs.csv

# Validate the output file is a valid CSV file.
mlr --icsv check /tmp/alljobs.csv
