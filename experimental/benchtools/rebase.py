#!/usr/bin/env python
# Copyright (c) 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""rebase.py: standalone script to batch update bench expectations.

    Requires gsutil to access gs://chromium-skia-gm and Rietveld credentials.

    Usage:
      Copy script to a separate dir outside Skia repo. The script will create a
          skia dir on the first run to host the repo, and will create/delete
          temp dirs as needed.
      ./rebase.py --githash <githash prefix to use for getting bench data>
"""


import argparse
import filecmp
import os
import re
import shutil
import subprocess
import time
import urllib2


# googlesource url that has most recent Skia git hash info.
SKIA_GIT_HEAD_URL = 'https://skia.googlesource.com/skia/+log/HEAD'

# Google Storage bench file prefix.
GS_PREFIX = 'gs://chromium-skia-gm/perfdata'

# List of Perf platforms we want to process. Populate from expectations/bench.
PLATFORMS = []

# Regular expression for matching githash data.
HA_RE = '<a href="/skia/\+/([0-9a-f]+)">'
HA_RE_COMPILED = re.compile(HA_RE)


def get_git_hashes():
  print 'Getting recent git hashes...'
  hashes = HA_RE_COMPILED.findall(
      urllib2.urlopen(SKIA_GIT_HEAD_URL).read())

  return hashes

def filter_file(f):
  if f.find('_msaa') > 0 or f.find('_record') > 0:
    return True

  return False

def clean_dir(d):
  if os.path.exists(d):
    shutil.rmtree(d)
  os.makedirs(d)

def get_gs_filelist(p, h):
  print 'Looking up for the closest bench files in Google Storage...'
  proc = subprocess.Popen(['gsutil', 'ls',
      '/'.join([GS_PREFIX, p, 'bench_' + h + '_data_skp_*'])],
          stdout=subprocess.PIPE)
  out, err = proc.communicate()
  if err or not out:
    return []
  return [i for i in out.strip().split('\n') if not filter_file(i)]

def download_gs_files(p, h, gs_dir):
  print 'Downloading raw bench files from Google Storage...'
  proc = subprocess.Popen(['gsutil', 'cp',
      '/'.join([GS_PREFIX, p, 'bench_' + h + '_data_skp_*']),
          '%s/%s' % (gs_dir, p)],
          stdout=subprocess.PIPE)
  out, err = proc.communicate()
  if err:
    clean_dir(gs_dir)
    return False
  files = 0
  for f in os.listdir(os.path.join(gs_dir, p)):
    if filter_file(f):
      os.remove(os.path.join(gs_dir, p, f))
    else:
      files += 1
  if files:
    return True
  return False

def calc_expectations(p, h, gs_dir, exp_dir, repo_dir):
  exp_filename = 'bench_expectations_%s.txt' % p
  proc = subprocess.Popen(['python', 'skia/bench/gen_bench_expectations.py',
      '-r', h, '-b', p, '-d', os.path.join(gs_dir, p), '-o',
          os.path.join(exp_dir, exp_filename)],
              stdout=subprocess.PIPE)
  out, err = proc.communicate()
  if err:
    print 'ERR_CALCULATING_EXPECTATIONS: ' + err
    return False
  print 'CALCULATED_EXPECTATIONS: ' + out
  repo_file = os.path.join(repo_dir, 'expectations', 'bench', exp_filename)
  if (os.path.isfile(repo_file) and
      filecmp.cmp(repo_file, os.path.join(exp_dir, exp_filename))):
      print 'NO CHANGE ON %s' % repo_file
      return False
  return True

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

def git_commit_expectations(repo_dir, exp_dir, update_li, h, commit):
  commit_msg = """manual bench rebase after %s

TBR=robertphillips@google.com

Bypassing trybots:
NOTRY=true""" % h
  old_cwd = os.getcwd()
  os.chdir(repo_dir)
  upload = ['git', 'cl', 'upload', '-f', '--bypass-hooks',
            '--bypass-watchlists', '-m', commit_msg]
  branch = exp_dir.split('/')[-1]
  if commit:
    upload.append('--use-commit-queue')
  cmds = ([['git', 'checkout', 'master'],
           ['git', 'pull'],
           ['git', 'checkout', '-b', branch, '-t', 'origin/master']] +
          [['cp', '%s/%s' % (exp_dir, f), 'expectations/bench'] for f in
           update_li] +
          [['git', 'add'] + ['expectations/bench/%s' % i for i in update_li],
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


def main():
  d = os.path.dirname(os.path.abspath(__file__))
  os.chdir(d)
  if not subprocess.call(['git', 'rev-parse'], stderr=subprocess.PIPE):
    print 'Please copy script to a separate dir outside git repos to use.'
    return
  parser = argparse.ArgumentParser()
  parser.add_argument('--githash',
                      help='Githash prefix (7+ chars) to rebaseline to.')
  parser.add_argument('--commit', action='store_true',
                      help='Whether to commit changes automatically.')
  args = parser.parse_args()

  repo_dir = os.path.join(d, 'skia')
  if not os.path.exists(repo_dir):
    os.makedirs(repo_dir)
  if not checkout_or_update_skia(repo_dir):
    print 'ERROR setting up Skia repo at %s' % repo_dir
    return 1

  file_in_repo = os.path.join(d, 'skia/experimental/benchtools/rebase.py')
  if not filecmp.cmp(__file__, file_in_repo):
    shutil.copy(file_in_repo, __file__)
    print 'Updated this script from repo; please run again.'
    return

  for item in os.listdir(os.path.join(d, 'skia/expectations/bench')):
    PLATFORMS.append(
        item.replace('bench_expectations_', '').replace('.txt', ''))

  if not args.githash or len(args.githash) < 7:
    raise Exception('Please provide --githash with a longer prefix (7+).')
  commit = False
  if args.commit:
    commit = True
  rebase_hash = args.githash[:7]
  hashes = get_git_hashes()
  short_hashes = [h[:7] for h in hashes]
  if rebase_hash not in short_hashes:
    raise Exception('Provided --githash not found in recent history!')
  hashes = hashes[:short_hashes.index(rebase_hash) + 1]
  update_li = []

  ts_str = '%s' % time.time()
  gs_dir = os.path.join(d, 'gs' + ts_str)
  exp_dir = os.path.join(d, 'exp' + ts_str)
  clean_dir(gs_dir)
  clean_dir(exp_dir)
  for p in PLATFORMS:
    clean_dir(os.path.join(gs_dir, p))
    hash_to_use = ''
    for h in reversed(hashes):
      li = get_gs_filelist(p, h)
      if not len(li):  # no data
        continue
      if download_gs_files(p, h, gs_dir):
        print 'Copied %s/%s' % (p, h)
        hash_to_use = h
        break
      else:
        print 'DOWNLOAD BENCH FAILED %s/%s' % (p, h)
        break
    if hash_to_use:
      if calc_expectations(p, h, gs_dir, exp_dir, repo_dir):
        update_li.append('bench_expectations_%s.txt' % p)
  if not update_li:
    print 'No bench data to update after %s!' % args.githash
  elif not git_commit_expectations(
      repo_dir, exp_dir, update_li, args.githash[:7], commit):
    print 'ERROR uploading expectations using git.'
  elif not commit:
    print 'CL created. Please take a look at the link above.'
  else:
    print 'New bench baselines should be in CQ now.'
  delete_dirs([gs_dir, exp_dir])


if __name__ == "__main__":
  main()
