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

def get_expectations_dict(f):
  """Given an expectations file f, returns a dictionary of data."""
  # maps row_key to (expected, lower_bound, upper_bound) float tuple.
  dic = {}
  for l in open(f).readlines():
    line_parts = l.strip().split(',')
    if line_parts[0].startswith('#') or len(line_parts) != 5:
      continue
    dic[','.join(line_parts[:2])] = (float(line_parts[2]), float(line_parts[3]),
                                     float(line_parts[4]))

  return dic

def calc_expectations(p, h, gs_dir, exp_dir, repo_dir, extra_dir, extra_hash):
  exp_filename = 'bench_expectations_%s.txt' % p
  exp_fullname = os.path.join(exp_dir, exp_filename)
  proc = subprocess.Popen(['python', 'skia/bench/gen_bench_expectations.py',
      '-r', h, '-b', p, '-d', os.path.join(gs_dir, p), '-o', exp_fullname],
              stdout=subprocess.PIPE)
  out, err = proc.communicate()
  if err:
    print 'ERR_CALCULATING_EXPECTATIONS: ' + err
    return False
  print 'CALCULATED_EXPECTATIONS: ' + out
  if extra_dir:  # Adjust data with the ones in extra_dir
    print 'USE_EXTRA_DATA_FOR_ADJUSTMENT.'
    proc = subprocess.Popen(['python', 'skia/bench/gen_bench_expectations.py',
        '-r', extra_hash, '-b', p, '-d', os.path.join(extra_dir, p), '-o',
            os.path.join(extra_dir, exp_filename)],
                stdout=subprocess.PIPE)
    out, err = proc.communicate()
    if err:
      print 'ERR_CALCULATING_EXTRA_EXPECTATIONS: ' + err
      return False
    extra_dic = get_expectations_dict(os.path.join(extra_dir, exp_filename))
    output_lines = []
    for l in open(exp_fullname).readlines():
      parts = l.strip().split(',')
      if parts[0].startswith('#') or len(parts) != 5:
        output_lines.append(l.strip())
        continue
      key = ','.join(parts[:2])
      if key in extra_dic:
        exp, lb, ub = (float(parts[2]), float(parts[3]), float(parts[4]))
        alt, _, _ = extra_dic[key]
        avg = (exp + alt) / 2
        # Keeps the extra range in lower/upper bounds from two actual values.
        new_lb = min(exp, alt) - (exp - lb)
        new_ub = max(exp, alt) + (ub - exp)
        output_lines.append('%s,%.2f,%.2f,%.2f' % (key, avg, new_lb, new_ub))
      else:
        output_lines.append(l.strip())
    with open(exp_fullname, 'w') as f:
      f.write('\n'.join(output_lines))

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

def git_commit_expectations(repo_dir, exp_dir, update_li, h, commit,
                            extra_hash):
  if extra_hash:
    extra_hash = ', adjusted with ' + extra_hash
  commit_msg = """manual bench rebase after %s%s

TBR=robertphillips@google.com

Bypassing trybots:
NOTRY=true""" % (h, extra_hash)
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
                      help=('Githash prefix (7+ chars) to rebaseline to. If '
                            'a second one is supplied after comma, and it has '
                            'corresponding bench data, will shift the range '
                            'center to the average of two expected values.'))
  parser.add_argument('--bots',
                      help=('Comma-separated list of bots to work on. If no '
                            'matching bots are found in the list, will default '
                            'to processing all bots.'))
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

  all_platforms = []  # Find existing list of platforms with expectations.
  for item in os.listdir(os.path.join(d, 'skia/expectations/bench')):
    all_platforms.append(
        item.replace('bench_expectations_', '').replace('.txt', ''))

  platforms = []
  # If at least one given bot is in all_platforms, use list of valid args.bots.
  if args.bots:
    bots = args.bots.strip().split(',')
    for bot in bots:
      if bot in all_platforms:  # Filters platforms with given bot list.
        platforms.append(bot)
  if not platforms:  # Include all existing platforms with expectations.
    platforms = all_platforms

  if not args.githash or len(args.githash) < 7:
    raise Exception('Please provide --githash with a longer prefix (7+).')
  githashes = args.githash.strip().split(',')
  if len(githashes[0]) < 7:
    raise Exception('Please provide --githash with longer prefixes (7+).')
  commit = False
  if args.commit:
    commit = True
  rebase_hash = githashes[0][:7]
  extra_hash = ''
  if len(githashes) == 2:
    extra_hash = githashes[1][:7]
  hashes = get_git_hashes()
  short_hashes = [h[:7] for h in hashes]
  if (rebase_hash not in short_hashes or
      (extra_hash and extra_hash not in short_hashes) or
      rebase_hash == extra_hash):
    raise Exception('Provided --githashes not found, or identical!')
  if extra_hash:
    extra_hash = hashes[short_hashes.index(extra_hash)]
  hashes = hashes[:short_hashes.index(rebase_hash) + 1]
  update_li = []

  ts_str = '%s' % time.time()
  gs_dir = os.path.join(d, 'gs' + ts_str)
  exp_dir = os.path.join(d, 'exp' + ts_str)
  extra_dir = os.path.join(d, 'extra' + ts_str)
  clean_dir(gs_dir)
  clean_dir(exp_dir)
  clean_dir(extra_dir)
  for p in platforms:
    clean_dir(os.path.join(gs_dir, p))
    clean_dir(os.path.join(extra_dir, p))
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
      if extra_hash and download_gs_files(p, extra_hash, extra_dir):
        print 'Copied extra data %s/%s' % (p, extra_hash)
        if calc_expectations(p, h, gs_dir, exp_dir, repo_dir, extra_dir,
                             extra_hash):
          update_li.append('bench_expectations_%s.txt' % p)
      elif calc_expectations(p, h, gs_dir, exp_dir, repo_dir, '', ''):
        update_li.append('bench_expectations_%s.txt' % p)
  if not update_li:
    print 'No bench data to update after %s!' % args.githash
  elif not git_commit_expectations(
      repo_dir, exp_dir, update_li, rebase_hash, commit, extra_hash):
    print 'ERROR uploading expectations using git.'
  elif not commit:
    print 'CL created. Please take a look at the link above.'
  else:
    print 'New bench baselines should be in CQ now.'
  delete_dirs([gs_dir, exp_dir, extra_dir])


if __name__ == "__main__":
  main()
