#!/bin/bash

# Creates a CSV file with all the unique values from one or more columns from
# /tmp/allbots.csv.

# The only arg is a comma separated list of column names that will also be used
# as the output filename.

mlr --csv uniq -f $1 /tmp/allbots.csv | mlr --csv sort -f $1 > /tmp/$1.csv