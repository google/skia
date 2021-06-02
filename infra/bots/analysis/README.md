# Job Analysis

A set of scripts that analyzes `jobs.json` to look for possible holes in our
testing.

## Requirements

To run the scripts you need to have both `jq` and `mlr` installed on your
machine.

    $ sudo apt install jq miller

## Running

The Makefile contains common queries that can be run against the data.

For example, to find all cpu_or_gpu_values that we currently don't run Perf
tests on you would run:

    $ make missing_perf_jobs

See https://miller.readthedocs.io/en/latest/reference-dsl.html more details on
the the kinds of queries that can be done against CSV files.
