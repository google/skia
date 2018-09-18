#!/usr/bin/python
# encoding: utf-8

# Copyright 2017 Google Inc.
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
import shlex
import multiprocessing
import traceback
from argparse import ArgumentParser
from multiprocessing import Process
from threading import Thread
from threading import Lock
from pdb import set_trace


HELP = """
\033[31mPlease call calmbench.py to drive this script if you're not doing so.
This script is not supposed to be used by itself. (At least, it's not easy to
use by itself. The calmbench bots may use this script directly.)
\033[0m
"""

FACTOR  = 3     # lower/upper quantile factor
DIFF_T  = 0.99  # different enough threshold
TERM    = 10    # terminate after this no. of iterations without suspect changes
MAXTRY  = 30    # max number of nanobench tries to narrow down suspects

UNITS   = "ns µs ms s".split()


timesLock = Lock()
timesA  = {}
timesB  = {}


def parse_args():
  parser = ArgumentParser(description=HELP)

  parser.add_argument('outdir', type=str, help="output directory")
  parser.add_argument('a', type=str, help="name of A")
  parser.add_argument('b', type=str, help="name of B")
  parser.add_argument('nano_a', type=str, help="path to A's nanobench binary")
  parser.add_argument('nano_b', type=str, help="path to B's nanobench binary")
  parser.add_argument('arg_a', type=str, help="args for A's nanobench run")
  parser.add_argument('arg_b', type=str, help="args for B's nanobench run")
  parser.add_argument('repeat', type=int, help="number of initial runs")
  parser.add_argument('skip_b', type=str, help=("whether to skip running B"
                                                " ('true' or 'false')"))
  parser.add_argument('config', type=str, help="nanobenh config")
  parser.add_argument('threads', type=int, help="number of threads to run")
  parser.add_argument('noinit', type=str, help=("whether to skip running B"
                                                " ('true' or 'false')"))

  parser.add_argument('--concise', dest='concise', action="store_true",
      help="If set, no verbose thread info will be printed.")
  parser.set_defaults(concise=False)

  # Additional args for bots
  BHELP = "bot specific options"
  parser.add_argument('--githash', type=str, default="", help=BHELP)
  parser.add_argument('--keys', type=str, default=[], nargs='+', help=BHELP)

  args = parser.parse_args()
  args.skip_b = args.skip_b == "true"
  args.noinit = args.noinit == "true"

  if args.threads == -1:
    args.threads = 1
    if args.config in ["8888", "565"]: # multi-thread for CPU only
        args.threads = max(1, multiprocessing.cpu_count() / 2)

  return args

def append_dict_sorted_array(dict_array, key, value):
  if key not in dict_array:
    dict_array[key] = []
  dict_array[key].append(value)
  dict_array[key].sort()


def add_time(args, name, bench, t, unit):
  normalized_t = t * 1000 ** UNITS.index(unit);
  if name.startswith(args.a):
    append_dict_sorted_array(timesA, bench, normalized_t)
  else:
    append_dict_sorted_array(timesB, bench, normalized_t)


def append_times_from_file(args, name, filename):
  with open(filename) as f:
    lines = f.readlines()
  for line in lines:
    items = line.split()
    if len(items) > 10:
      bench = items[10]
      matches = re.search("([+-]?\d*.?\d+)(s|ms|µs|ns)", items[3])
      if (not matches or items[9] != args.config):
        continue
      time_num = matches.group(1)
      time_unit = matches.group(2)
      add_time(args, name, bench, float(time_num), time_unit)


class ThreadWithException(Thread):
  def __init__(self, target):
    super(ThreadWithException, self).__init__(target = target)
    self.exception = None

  def run(self):
    try:
      self._Thread__target(*self._Thread__args, **self._Thread__kwargs)
    except BaseException as e:
      self.exception = e

  def join(self, timeout=None):
    super(ThreadWithException, self).join(timeout)


class ThreadRunner:
  """Simplest and stupidiest threaded executer."""
  def __init__(self, args):
    self.concise = args.concise
    self.threads = []

  def add(self, args, fn):
    if len(self.threads) >= args.threads:
      self.wait()
    t = ThreadWithException(target = fn)
    t.daemon = True
    self.threads.append(t)
    t.start()

  def wait(self):
    def spin():
      i = 0
      spinners = [".  ", ".. ", "..."]
      while len(self.threads) > 0:
        timesLock.acquire()
        sys.stderr.write(
            "\r" + spinners[i % len(spinners)] +
            " (%d threads running)" % len(self.threads) +
            "           \r" # spaces for erasing characters
        )
        timesLock.release()
        time.sleep(0.5)
        i += 1

    if not self.concise:
      ts = Thread(target = spin);
      ts.start()

    for t in self.threads:
      t.join()

    exceptions = []
    for t in self.threads:
      if t.exception:
        exceptions.append(t.exception)

    self.threads = []

    if not self.concise:
      ts.join()

    if len(exceptions):
      for exc in exceptions:
        print exc
      raise exceptions[0]


def split_arg(arg):
  raw = shlex.split(arg)
  result = []
  for r in raw:
    if '~' in r:
      result.append(os.path.expanduser(r))
    else:
      result.append(r)
  return result


def run(args, threadRunner, name, nano, arg, i):
  def task():
    file_i = "%s/%s.out%d" % (args.outdir, name, i)

    should_run = not args.noinit and not (name == args.b and args.skip_b)
    if i <= 0:
      should_run = True # always run for suspects

    if should_run:
      if i > 0:
        timesLock.acquire()
        print "Init run %d for %s..." % (i, name)
        timesLock.release()
      subprocess.check_call(["touch", file_i])
      with open(file_i, 'w') as f:
        subprocess.check_call([nano] + split_arg(arg) +
                              ["--config", args.config], stderr=f, stdout=f)

    timesLock.acquire()
    append_times_from_file(args, name, file_i)
    timesLock.release()

  threadRunner.add(args, task)


def init_run(args):
  threadRunner = ThreadRunner(args)
  for i in range(1, max(args.repeat, args.threads / 2) + 1):
    run(args, threadRunner, args.a, args.nano_a, args.arg_a, i)
    run(args, threadRunner, args.b, args.nano_b, args.arg_b, i)
  threadRunner.wait()


def get_lower_upper(values):
  i = max(0, (len(values) - 1) / FACTOR)
  return values[i], values[-i - 1]


def different_enough(lower1, upper2):
  return upper2 < DIFF_T * lower1


# TODO(liyuqian): we used this hacky criteria mainly because that I didn't have
# time to study more rigorous statistical tests. We should adopt a more rigorous
# test in the future.
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


def normalize_r(r):
  if r > 1.0:
    return r - 1.0
  else:
    return 1.0 - 1/r


def test():
  args = parse_args()

  init_run(args)
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
    threadRunner = ThreadRunner(args)
    for j in range(1, max(1, args.threads / 2) + 1):
      run(args, threadRunner, args.a, args.nano_a,
          args.arg_a + suspects_arg(suspects), -j)
      run(args, threadRunner, args.b, args.nano_b,
          args.arg_b + suspects_arg(suspects), -j)
      tryCnt += 1
    threadRunner.wait()

  suspects = get_suspects()
  if len(suspects) == 0:
    print ("%s and %s does not seem to have significant " + \
           "performance differences.") % (args.a, args.b)
  else:
    suspects.sort(key = regression)
    print "%s (compared to %s) is likely" % (args.a, args.b)
    for suspect in suspects:
      r = regression(suspect)
      if r < 1:
        print "\033[31m  %s slower in %s\033[0m" % \
                (format_r(1/r), suspect)
      else:
        print "\033[32m  %s faster in %s\033[0m" % \
                (format_r(r), suspect)

  with open("%s/bench_%s_%s.json" % (args.outdir, args.a, args.b), 'w') as f:
    results = {}
    for bench in timesA:
      r = regression(bench) if bench in suspects else 1.0
      results[bench] = {
        args.config: {
          "signed_regression": normalize_r(r),
          "lower_quantile_ms": get_lower_upper(timesA[bench])[0] * 1e-6,
          "upper_quantile_ms": get_lower_upper(timesA[bench])[1] * 1e-6,
          "options": {
            # TODO(liyuqian): let ab.py call nanobench with --outResultsFile so
            # nanobench could generate the json for us that's exactly the same
            # as that being used by perf bots. Currently, we cannot guarantee
            # that bench is the name (e.g., bench may have additional resolution
            # information appended after name).
            "name": bench
          }
        }
      }

    output = {"results": results}
    if args.githash:
      output["gitHash"] = args.githash
    if args.keys:
      keys = {}
      for i in range(len(args.keys) / 2):
        keys[args.keys[i * 2]] = args.keys[i * 2 + 1]
      output["key"] = keys
    f.write(json.dumps(output, indent=4))
    print ("\033[36mJSON results available in %s\033[0m" % f.name)

  with open("%s/bench_%s_%s.csv" % (args.outdir, args.a, args.b), 'w') as out:
    out.write(("bench, significant?, raw regresion, " +
                   "%(A)s quantile (ns), %(B)s quantile (ns), " +
                   "%(A)s (ns), %(B)s (ns)\n") % {'A': args.a, 'B': args.b})
    for bench in suspects + timesA.keys():
      if (bench not in timesA or bench not in timesB):
        continue
      ta = timesA[bench]
      tb = timesB[bench]
      out.write(
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
            out.name)


if __name__ == "__main__":
  try:
    test()
  except Exception as e:
    print e
    print HELP
    traceback.print_exc()
    raise e
