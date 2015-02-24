#!/usr/bin/env python

usage = '''
Write extra flags to outfile for DM based on the bot name:
  $ python dm_flags.py outfile Test-Mac10.9-MacMini6.2-HD4000-x86_64-Release
Or run self-tests:
  $ python dm_flags.py test
'''

import inspect
import json
import os
import sys


def lineno():
  caller = inspect.stack()[1]  # Up one level to our caller.
  return inspect.getframeinfo(caller[0]).lineno


cov_start = lineno()+1   # We care about coverage starting just past this def.
def get_dm_args(bot):
  # Just in case we missed anything, let DM ignore unknown flags.
  args = ['--undefok']

  configs = ['565', '8888', 'gpu', 'nvprmsaa4']
  if 'ANGLE' in bot:
    configs.append('angle')
  args.append('--config')
  args.extend(configs)

  # For simplicity, disable mode tests on all bots.
  args.extend(['--nopipe', '--noserialize', '--noquilt'])

  match = []
  if 'Alex' in bot:  # skia:2793
    # This machine looks to be running out of heap.
    # Running with fewer threads may help.
    args.extend(['--threads', '1'])
  if 'Valgrind' in bot: # skia:3021
    match.append('~Threaded')
  if 'Xoom' in bot:  # skia:1699
    match.append('~WritePixels')

  # skia:3249: these images flakily don't decode on Android.
  if 'Android' in bot:
    match.append('~tabl_mozilla_0')
    match.append('~desk_yahoonews_0')

  if match:
    args.append('--match')
    args.extend(match)

  # Though their GPUs are interesting, these don't test anything on
  # the CPU that other ARMv7+NEON bots don't test faster (N5).
  if ('Nexus10'  in bot or
      'Nexus7'   in bot or
      'GalaxyS3' in bot or
      'GalaxyS4' in bot):
    args.append('--nocpu')
  return args
cov_end = lineno()   # Don't care about code coverage past here.


def self_test():
  import coverage  # This way the bots don't need coverage.py to be installed.
  args = {}
  cases = [
    'Test-Android-Nexus7-Tegra3-Arm7-Release',
    'Test-Android-Xoom-Tegra2-Arm7-Release',
    'Test-ChromeOS-Alex-GMA3150-x86-Debug',
    'Test-Ubuntu12-ShuttleA-GTX550Ti-x86_64-Release-Valgrind',
    'Test-Win7-ShuttleA-HD2000-x86-Debug-ANGLE',
  ]

  cov = coverage.coverage()
  cov.start()
  for case in cases:
    args[case] = get_dm_args(case)
  cov.stop()

  this_file = os.path.basename(__file__)
  _, _, not_run, _ = cov.analysis(this_file)
  filtered = [line for line in not_run if line > cov_start and line < cov_end]
  if filtered:
    print 'Lines not covered by test cases: ', filtered
    sys.exit(1)

  golden = this_file.replace('.py', '.json')
  with open(os.path.join(os.path.dirname(__file__), golden), 'w') as f:
    json.dump(args, f, indent=2, sort_keys=True)


if __name__ == '__main__':
  if len(sys.argv) == 2 and sys.argv[1] == 'test':
    self_test()
    sys.exit(0)

  if len(sys.argv) != 3:
    print usage
    sys.exit(1)

  with open(sys.argv[1], 'w') as out:
    json.dump(get_dm_args(sys.argv[2]), out)
