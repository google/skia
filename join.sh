#!/bin/bash

./axis.sh $1

mlr --csv join  -f $1.csv -j $1 allbots.csv | mlr --csv sort -f $1 | mlr --csv uniq -f $1,$2