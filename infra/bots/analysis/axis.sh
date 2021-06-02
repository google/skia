#!/bin/bash

mlr --csv uniq -f $1 /tmp/allbots.csv | mlr --csv sort -f $1 > /tmp/$1.csv