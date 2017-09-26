#!/usr/bin/python
# encoding: utf-8

# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.
#
# This is an A/B test utility script used by calmbench.py
#
# For each bench, we get a distribution of min_ms measurements from nanobench.
# From that, we try to recover the 1/3 and 2/3 quantiles of the distribution.
# If range (1/3 quantile, 2/3 quantile) is completely disjoint between A and B,
# we report that as a regression.
#
# The more measurements we have for a bench, the more accurate our quantiles
# are. However, taking more measurements is time consuming. Hence we'll prune
# out benches and only take more measurements for benches whose current quantile
# ranges are disjoint.
#
# P.S. The current script is brute forcely translated from a ruby script. So it
# may be ugly...

import re
import os
import sys
import time
import json
import subprocess
from multiprocessing import Process
from threading import Thread
from threading import Lock
from pdb import set_trace

HELP = """
\033[31m
Please call calmbench.py to drive this script if you're not doing so.
This script is not supposed to be used by itself. (At least, it's not easy to
use by itself.)
\033[0m
"""

ARGV = sys.argv

if len(ARGV) < 13:
    print HELP
    exit()

SKIADIR = ARGV[1]
OUTDIR  = ARGV[2]
A       = ARGV[3]
B       = ARGV[4]
NANO_A  = ARGV[5]
NANO_B  = ARGV[6]
ARG_A   = ARGV[7]
ARG_B   = ARGV[8]
REPEAT  = int(ARGV[9])
SKIP_B  = ARGV[10] == "true"
CONFIG  = ARGV[11]
THREADS = int(ARGV[12])
NOINIT  = ARGV[13] == "true"

FACTOR  = 3     # lower/upper quantile factor
DIFF_T  = 0.99  # different enough threshold
TERM    = 10    # terminate after this no. of iterations without suspect changes
MAXTRY  = 30    # max number of nanobench tries to narrow down suspects

UNITS   = "ns µs ms s".split()

names   = []
files   = []
timesA  = {}
timesB  = {}

def append_dict_sorted_array(dict_array, key, value):
    if key not in dict_array:
        dict_array[key] = []
    dict_array[key].append(value)
    dict_array[key].sort()

def add_time(name, bench, t, unit):
    normalized_t = t * 1000 ** UNITS.index(unit);
    if name.startswith(A):
        append_dict_sorted_array(timesA, bench, normalized_t)
    else:
        append_dict_sorted_array(timesB, bench, normalized_t)

def append_times_from_file(name, filename):
    with open(filename) as f:
        lines = f.readlines()
    for line in lines:
        items = line.split()
        if len(items) > 10:
            bench = items[10]
            matches = re.search("([+-]?\d*.?\d+)(s|ms|µs|ns)", items[3])
            if (not matches or items[9] != CONFIG):
                continue
            time_num = matches.group(1)
            time_unit = matches.group(2)
            add_time(name, bench, float(time_num), time_unit)

# Simplest and stupidiest threaded executer
class ThreadRunner:
    def __init__(self):
        self.threads = []
        self.waitCnt = 0

    def add(self, fn):
        if len(self.threads) >= THREADS:
            self.wait()
        t = Thread(target = fn)
        t.daemon = True
        self.threads.append(t)
        t.start()

    def wait(self):
        self.waitCnt += 1
        currentWait = self.waitCnt
        def spin():
            i = 0
            spinners = [".  ", ".. ", "..."]
            while len(self.threads) > 0 and currentWait == self.waitCnt:
                timesLock.acquire()
                sys.stderr.write(
                    "\r" + spinners[i % len(spinners)] +
                    " (%d threads running)" % len(self.threads) +
                    "           \r" # spaces for erasing characters
                )
                timesLock.release()
                time.sleep(0.5)
                i += 1

        ts = Thread(target = spin);
        ts.daemon = True
        ts.start()
        for t in self.threads:
            t.join()
        self.threads = []

threadRunner = ThreadRunner()
timesLock = Lock()

def run(name, nano, arg, i):
    def task():
        file_i = "%s/%s.out%d" % (OUTDIR, name, i)

        should_run = not NOINIT and not (name == B and SKIP_B)
        if i <= 0:
            should_run = True # always run for suspects

        if should_run:
            if i > 0:
                timesLock.acquire()
                print "Init run %d for %s..." % (i, name)
                timesLock.release()
            os.system("touch %s && %s %s --config %s &> %s" \
                    % (file_i, nano, arg, CONFIG, file_i))

        timesLock.acquire()
        append_times_from_file(name, file_i)
        timesLock.release()

    threadRunner.add(task)

def init_run():
    for i in range(1, max(REPEAT, THREADS / 2) + 1):
        run(A, NANO_A, ARG_A, i)
        run(B, NANO_B, ARG_B, i)
    threadRunner.wait()

def get_lower_upper(values):
    i = max(0, (len(values) - 1) / FACTOR)
    return values[i], values[-i - 1]

def different_enough(lower1, upper2):
    return upper2 < DIFF_T * lower1

def get_suspects():
    suspects = []
    for bench in timesA.keys():
        if bench not in timesB:
            continue
        lowerA, upperA = get_lower_upper(timesA[bench])
        lowerB, upperB = get_lower_upper(timesB[bench])
        if different_enough(lowerA, upperB) or different_enough(lowerB, upperA):
            suspects.append(bench)
    return suspects

def process_bench_pattern(s):
    if ".skp" in s: # skp bench won't match their exact names...
        return "^\"" + s[0:(s.index(".skp") + 3)] + "\""
    else:
        return "^\"" + s + "\"$"

def suspects_arg(suspects):
    patterns = map(process_bench_pattern, suspects)
    return " --match " + (" ".join(patterns))

def median(array):
    return array[len(array) / 2]

def regression(bench):
    a = median(timesA[bench])
    b = median(timesB[bench])
    if (a == 0): # bad bench, just return no regression
        return 1
    return b / a

def percentage(x):
    return (x - 1) * 100

def format_r(r):
    return ('%6.2f' % percentage(r)) + "%"

def test():
    init_run()
    last_unchanged_iter = 0
    last_suspect_number = -1
    tryCnt = 0
    it = 0
    while tryCnt < MAXTRY:
        it += 1
        suspects = get_suspects()
        if len(suspects) != last_suspect_number:
            last_suspect_number = len(suspects)
            last_unchanged_iter = it
        if (len(suspects) == 0 or it - last_unchanged_iter >= TERM):
            break

        print "Number of suspects at iteration %d: %d" % (it, len(suspects))
        for j in range(1, max(1, THREADS / 2) + 1):
            run(A, NANO_A, ARG_A + suspects_arg(suspects), -j)
            run(B, NANO_B, ARG_B + suspects_arg(suspects), -j)
            tryCnt += 1
        threadRunner.wait()

    suspects = get_suspects()
    if len(suspects) == 0:
        print ("%s and %s does not seem to have significant " + \
              "performance differences.") % (A, B)
    else:
        suspects.sort(key = regression)
        print "%s (compared to %s) is likely" % (A, B)
        for suspect in suspects:
            r = regression(suspect)
            if r < 1:
                print "\033[31m  %s slower in %s\033[0m" % \
                        (format_r(1/r), suspect)
            else:
                print "\033[32m  %s faster in %s\033[0m" % \
                        (format_r(r), suspect)

    with open("%s/bench_%s_%s.json" % (OUTDIR, A, B), 'w') as jsonfile:
        jsonfile.write(json.dumps(map(
            lambda bench: {bench: regression(bench)},
            suspects
        )))
        print ("\033[36mJSON results available in %s\033[0m" % jsonfile.name)

    with open("%s/bench_%s_%s.csv" % (OUTDIR, A, B), 'w') as outfile:
        outfile.write(("bench, significant?, raw regresion, " +
                       "%(A)s quantile (ns), %(B)s quantile (ns), " +
                       "%(A)s (ns), %(B)s (ns)\n") % {'A': A, 'B': B})
        for bench in suspects + timesA.keys():
            if (bench not in timesA or bench not in timesB):
                continue
            ta = timesA[bench]
            tb = timesB[bench]
            outfile.write(
                "%s, %s, %f, " % (bench, bench in suspects, regression(bench)) +
                ' '.join(map(str, get_lower_upper(ta))) + ", " +
                ' '.join(map(str, get_lower_upper(tb))) + ", " +
                ("%s, %s\n" % (' '.join(map(str, ta)), ' '.join(map(str, tb))))
            )
        print (("\033[36m" +
               "Compared %d benches. " +
               "%d of them seem to be significantly differrent." +
               "\033[0m") %
               (len([x for x in timesA if x in timesB]), len(suspects)))
        print ("\033[36mPlease see detailed bench results in %s\033[0m" %
                outfile.name)

if __name__ == "__main__":
    try:
        test()
    except Exception as e:
        print e
        print HELP
        raise
