#!/bin/bash

mlr --csv uniq -f $1 ~/allbots.csv | mlr --csv sort -f $1 > $1.csv