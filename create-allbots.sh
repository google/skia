#!/bin/bash

set -e

echo Type,os,compiler,model,cpu_or_gpu,cpu_or_gpu_value,arch,configuration,test_filter,extra > allbots.csv
cat infra/bots/jobs.json | jq .[] -r | grep "^[Perf|Test]" | sed "s#-#,#g" | sed "s#All\$#All,none#g" >> allbots.csv
mlr --icsv check allbots.csv
