#!/usr/bin/env python


# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""greenify.py: standalone script to correct flaky bench expectations.

    Requires Rietveld credentials on the running machine.

    Usage:
      Copy script to a separate dir outside Skia repo. The script will create a
          skia dir on the first run to host the repo, and will create/delete
          temp dirs as needed.
      ./greenify.py --url <the stdio url from failed CheckForRegressions step>
"""

import argparse
import filecmp
import os
import re
import shutil
import subprocess
import time
import urllib2


# Regular expression for matching exception data.
EXCEPTION_RE = ('Bench (\S+) out of range \[(\d+.\d+), (\d+.\d+)\] \((\d+.\d+) '
                'vs (\d+.\d+), ')
EXCEPTION_RE_COMPILED = re.compile(EXCEPTION_RE)


def clean_dir(d):
  if os.path.exists(d):
    shutil.rmtree(d)
  os.makedirs(d)

def checkout_or_update_skia(repo_dir):
  status = True
  old_cwd = os.getcwd()
  os.chdir(repo_dir)
  print 'CHECK SKIA REPO...'
  if subprocess.call(['git', 'pull'],
                     stderr=subprocess.PIPE):
    print 'Checking out Skia from git, please be patient...'
    os.chdir(old_cwd)
    clean_dir(repo_dir)
    os.chdir(repo_dir)
    if subprocess.call(['git', 'clone', '-q', '--depth=50', '--single-branch',
                        'https://skia.googlesource.com/skia.git', '.']):
      status = False
  subprocess.call(['git', 'checkout', 'master'])
  subprocess.call(['git', 'pull'])
  os.chdir(old_cwd)
  return status

def git_commit_expectations(repo_dir, exp_dir, bot, build, commit):
  commit_msg = """Greenify bench bot %s at build %s

TBR=bsalomon@google.com

Bypassing trybots:
NOTRY=true""" % (bot, build)
  old_cwd = os.getcwd()
  os.chdir(repo_dir)
  upload = ['git', 'cl', 'upload', '-f', '--bypass-hooks',
            '--bypass-watchlists', '-m', commit_msg]
  if commit:
    upload.append('--use-commit-queue')
  branch = exp_dir[exp_dir.rfind('/') + 1:]
  filename = 'bench_expectations_%s.txt' % bot
  cmds = ([['git', 'checkout', 'master'],
           ['git', 'pull'],
           ['git', 'checkout', '-b', branch, '-t', 'origin/master'],
           ['cp', '%s/%s' % (exp_dir, filename), 'expectations/bench'],
           ['git', 'add', 'expectations/bench/' + filename],
           ['git', 'commit', '-m', commit_msg],
           upload,
           ['git', 'checkout', 'master'],
           ['git', 'branch', '-D', branch],
          ])
  status = True
  for cmd in cmds:
    print 'Running ' + ' '.join(cmd)
    if subprocess.call(cmd):
      print 'FAILED. Please check if skia git repo is present.'
      subprocess.call(['git', 'checkout', 'master'])
      status = False
      break
  os.chdir(old_cwd)
  return status

def delete_dirs(li):
  for d in li:
    print 'Deleting directory %s' % d
    shutil.rmtree(d)

def widen_bench_ranges(url, bot, repo_dir, exp_dir):
  fname = 'bench_expectations_%s.txt' % bot
  src = os.path.join(repo_dir, 'expectations', 'bench', fname)
  if not os.path.isfile(src):
    print 'This bot has no expectations! %s' % bot
    return False
  row_dic = {}
  for l in urllib2.urlopen(url).read().split('\n'):
    data = EXCEPTION_RE_COMPILED.search(l)
    if data:
      row = data.group(1)
      lb = float(data.group(2))
      ub = float(data.group(3))
      actual = float(data.group(4))
      exp = float(data.group(5))
      avg = (actual + exp) / 2
      shift = avg - exp
      lb = lb + shift
      ub = ub + shift
      # In case outlier really fluctuates a lot
      if actual < lb:
        lb = actual - abs(shift) * 0.1 + 0.5
      elif actual > ub:
        ub = actual + abs(shift) * 0.1 + 0.5
      row_dic[row] = '%.2f,%.2f,%.2f' % (avg, lb, ub)
  if not row_dic:
    print 'NO out-of-range benches found at %s' % url
    return False

  changed = 0
  li = []
  for l in open(src).readlines():
    parts = l.strip().split(',')
    if parts[0].startswith('#') or len(parts) != 5:
      li.append(l.strip())
      continue
    if ','.join(parts[:2]) in row_dic:
      li.append(','.join(parts[:2]) + ',' + row_dic[','.join(parts[:2])])
      changed += 1
    else:
      li.append(l.strip())
  if not changed:
    print 'Not in source file:\n' + '\n'.join(row_dic.keys())
    return False

  dst = os.path.join(exp_dir, fname)
  with open(dst, 'w+') as f:
    f.write('\n'.join(li))
  return True


def main():
  d = os.path.dirname(os.path.abspath(__file__))
  os.chdir(d)
  if not subprocess.call(['git', 'rev-parse'], stderr=subprocess.PIPE):
    print 'Please copy script to a separate dir outside git repos to use.'
    return
  ts_str = '%s' % time.time()

  parser = argparse.ArgumentParser()
  parser.add_argument('--url',
                      help='Broken bench build CheckForRegressions page url.')
  parser.add_argument('--commit', action='store_true',
                      help='Whether to commit changes automatically.')
  args = parser.parse_args()
  repo_dir = os.path.join(d, 'skia')
  if not os.path.exists(repo_dir):
    os.makedirs(repo_dir)
  if not checkout_or_update_skia(repo_dir):
    print 'ERROR setting up Skia repo at %s' % repo_dir
    return 1

  file_in_repo = os.path.join(d, 'skia/experimental/benchtools/greenify.py')
  if not filecmp.cmp(__file__, file_in_repo):
    shutil.copy(file_in_repo, __file__)
    print 'Updated this script from repo; please run again.'
    return

  if not args.url:
    raise Exception('Please provide a url with broken CheckForRegressions.')
  path = args.url.split('/')
  if len(path) != 11 or not path[6].isdigit():
    raise Exception('Unexpected url format: %s' % args.url)
  bot = path[4]
  build = path[6]
  commit = False
  if args.commit:
    commit = True

  exp_dir = os.path.join(d, 'exp' + ts_str)
  clean_dir(exp_dir)
  if not widen_bench_ranges(args.url, bot, repo_dir, exp_dir):
    print 'NO bench exceptions found! %s' % args.url
  elif not git_commit_expectations(
      repo_dir, exp_dir, bot, build, commit):
    print 'ERROR uploading expectations using git.'
  elif not commit:
    print 'CL created. Please take a look at the link above.'
  else:
    print 'New bench baselines should be in CQ now.'
  delete_dirs([exp_dir])


if __name__ == "__main__":
  main()
