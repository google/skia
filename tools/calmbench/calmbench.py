#!/usr/bin/python

# Copyright 2017 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import sys
import subprocess
import multiprocessing

from argparse import ArgumentParser


README = """
Simply run
\033[36m
    python {0} TEST_GIT_BRANCH
\033[0m
to see if TEST_GIT_BRANCH has performance regressions against master in 8888.

To compare a specific config with svg and skp resources included, add --config
and --extraarg option. For exampe,
\033[36m
    python {0} TEST_GIT_BRANCH --config gl \\
        --extraarg "--svgs ~/Desktop/bots/svgs --skps ~/Desktop/bots/skps"
\033[0m
For more options, please see

    python {0} --help
""".format(__file__)


CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
AB_SCRIPT = "ab.py"


def parse_args():
  if len(sys.argv) <= 1 or sys.argv[1] == '-h' or sys.argv[1] == '--help':
    print README

  parser = ArgumentParser(
    description='Noiselessly (hence calm) becnhmark a git branch against ' +
                'another baseline branch (e.g., master) using multiple ' +
                ' nanobench runs.'
  )

  default_threads = max(1, multiprocessing.cpu_count() / 2);
  default_skiadir = os.path.normpath(CURRENT_DIR + "/../../")

  config_help = (
      'nanobench config; we currently support only one config '
      'at a time (default: %(default)s)')
  reps_help = (
      'initial repititions of the nanobench run; this may be '
      'overridden when we have many threads (default: %(default)s)')
  extraarg_help = (
      'nanobench args (example: --svgs ~/Desktop/bots/svgs --skps '
      '~/Desktop/bots/skps)')
  baseline_help = (
      'baseline branch to compare against (default: %(default)s)')
  basearg_help = (
      'nanobench arg for the baseline branch; if not given, we use '
      ' the same arg for both the test branch and the baseline branch')
  threads_help = (
      'number of threads to be used (default: %(default)s); '
      'for GPU config, this will always be 1')
  no_compile_help = (
      'whether NOT to compile nanobench and copy it to WRITEDIR '
      '(i.e., reuse previous nanobench compiled)')
  skip_base_help = (
      'whether NOT to run nanobench on baseline branch '
      '(i.e., reuse previous baseline measurements)')
  noinit_help = (
      'whether to skip initial nanobench runs (default: %(default)s)')
  branch_help = (
      "the test branch to benchmark; if it's 'modified', we'll benchmark the "
      "current modified code against 'git stash'.")

  definitions = [
    # argname, type, default value, help
    ['--config',    str, '8888', config_help],
    ['--skiadir',   str, default_skiadir, 'default: %(default)s'],
    ['--ninjadir',  str, 'out/Release', 'default: %(default)s'],
    ['--writedir',  str, '/var/tmp', 'default: %(default)s'],
    ['--extraarg',  str, '', extraarg_help],
    ['--baseline',  str, 'master', baseline_help],
    ['--basearg',   str, '', basearg_help],
    ['--reps',      int, 2, reps_help],
    ['--threads',   int, default_threads, threads_help],
  ]

  for d in definitions:
    parser.add_argument(d[0], type=d[1], default=d[2], help=d[3])

  parser.add_argument('branch', type=str, help=branch_help)
  parser.add_argument('--no-compile', dest='no_compile', action="store_true",
      help=no_compile_help)
  parser.add_argument('--skip-base', dest='skipbase', action="store_true",
      help=skip_base_help)
  parser.add_argument('--noinit', dest='noinit', action="store_true",
      help=noinit_help)
  parser.add_argument('--concise', dest='concise', action="store_true",
      help="If set, no verbose thread info will be printed.")
  parser.set_defaults(no_compile=False);
  parser.set_defaults(skipbase=False);
  parser.set_defaults(noinit=False);
  parser.set_defaults(concise=False);

  # Additional args for bots
  BHELP = "bot specific options"
  parser.add_argument('--githash', type=str, help=BHELP)
  parser.add_argument('--keys', type=str, default=[], nargs='+', help=BHELP)

  args = parser.parse_args()
  if not args.basearg:
    args.basearg = args.extraarg

  return args


def nano_path(args, branch):
  return args.writedir + '/nanobench_' + branch


def compile_branch(args, branch):
  print "Compiling branch %s" % args.branch

  commands = [
    ['git', 'checkout', branch],
    ['ninja', '-C', args.ninjadir, 'nanobench'],
    ['cp', args.ninjadir + '/nanobench', nano_path(args, branch)]
  ]
  for command in commands:
    subprocess.check_call(command, cwd=args.skiadir)


def compile_modified(args):
  print "Compiling modified code"
  subprocess.check_call(
      ['ninja', '-C', args.ninjadir, 'nanobench'], cwd=args.skiadir)
  subprocess.check_call(
      ['cp', args.ninjadir + '/nanobench', nano_path(args, args.branch)],
      cwd=args.skiadir)

  print "Compiling stashed code"
  stash_output = subprocess.check_output(['git', 'stash'], cwd=args.skiadir)
  if 'No local changes to save' in stash_output:
    subprocess.check_call(['git', 'reset', 'HEAD^', '--soft'])
    subprocess.check_call(['git', 'stash'])

  subprocess.check_call(['gclient', 'sync'], cwd=args.skiadir)
  subprocess.check_call(
      ['ninja', '-C', args.ninjadir, 'nanobench'], cwd=args.skiadir)
  subprocess.check_call(
      ['cp', args.ninjadir + '/nanobench', nano_path(args, args.baseline)],
      cwd=args.skiadir)
  subprocess.check_call(['git', 'stash', 'pop'], cwd=args.skiadir)

def compile_nanobench(args):
  if args.branch == 'modified':
    compile_modified(args)
  else:
    compile_branch(args, args.branch)
    compile_branch(args, args.baseline)


def main():
  args = parse_args()

  # copy in case that it will be gone after git branch switching
  orig_ab_name = CURRENT_DIR + "/" + AB_SCRIPT
  temp_ab_name = args.writedir + "/" + AB_SCRIPT
  subprocess.check_call(['cp', orig_ab_name, temp_ab_name])

  if not args.no_compile:
    compile_nanobench(args)

  command = [
    'python',
    temp_ab_name,
    args.writedir,
    args.branch + ("_A" if args.branch == args.baseline else ""),
    args.baseline + ("_B" if args.branch == args.baseline else ""),
    nano_path(args, args.branch),
    nano_path(args, args.baseline),
    args.extraarg,
    args.basearg,
    str(args.reps),
    "true" if args.skipbase else "false",
    args.config,
    str(args.threads if args.config in ["8888", "565"] else 1),
    "true" if args.noinit else "false"
  ]

  if args.githash:
    command += ['--githash', args.githash]
  if args.keys:
    command += (['--keys'] + args.keys)

  if args.concise:
    command.append("--concise")

  p = subprocess.Popen(command, cwd=args.skiadir)
  try:
    p.wait()
  except KeyboardInterrupt:
    try:
      p.terminate()
    except OSError as e:
      print e


if __name__ == "__main__":
  main()
