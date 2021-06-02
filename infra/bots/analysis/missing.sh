#!/bin/bash
# Copyright 2021 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# A script to find all cpu_or_gpu_values that don't get run as a job for the
# given filter passed in as the first argument. The filter is a boolean query
# against columns in /tmp/alljobs.csv, such as '$vulkan == "true"'.
set -e

if [ $# -ne 1 ]; then
    echo "$0 <filter>"
    echo "Example: $0 '\$vulkan == \"true\"'"
    exit 1
fi

FILTER=$1

# Ensure /tmp/alljobs.csv has been created.
./create-alljobs.sh

# Extract out the list of all cpu_or_gpu_values and associated model name.
./axis.sh cpu_or_gpu_value,model

# Find all cpus or gpus that we don't Test or Perf with Vulkan by creating a
# list of all the cpu_or_gpu_values associated with Vulkan tests, and then print
# all the rows in  /tmp/cpu_or_gpu_value,model.csv that don't match that list.
#
# The last join with --np means don't print matches, and --ul specifies to print
# values that are unmatched on the left hand side of the join, i.e. from the
# /tmp/cpu_or_gpu_value,model.csv file.
mlr --csv filter "${FILTER}" /tmp/alljobs.csv  | \
mlr --csv cut -f cpu_or_gpu_value,model | \
mlr --csv sort -f cpu_or_gpu_value | \
mlr --csv uniq -f cpu_or_gpu_value | \
mlr --csv join -f /tmp/cpu_or_gpu_value,model.csv -j cpu_or_gpu_value --ul --np
