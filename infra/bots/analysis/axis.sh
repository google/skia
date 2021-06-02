#!/bin/bash
# Copyright 2021 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Creates a CSV file with all the unique values from one or more columns from
# /tmp/alljobs.csv.

# The only arg is a comma separated list of column names that will also be used
# as the output filename.

mlr --csv uniq -f $1 /tmp/alljobs.csv | mlr --csv sort -f $1 > /tmp/$1.csv
