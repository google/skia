#!/bin/bash

set -e

cat infra/bots/jobs.json | jq .[] -r | grep "^[Perf|Test]" | sed "s#-#,#g" | sed "s#All\$#All,none#g" > allbots.json
mlr --icsv check allbots.json