#! /usr/bin/env python
# Copyright 2019 Google LLC.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import os
import shutil
import subprocess
import sys

def copy_git_directory(src, dst, out=None):
  '''
  Makes a copy of `src` directory in `dst` directory.  If files already exist
  and are identical, do not touch them.  If extra files or directories exist,
  remove them.  Assume that `src` is a git directory so that `git ls-files` can
  be used to enumerate files.  This has the added benefit of ignoring files
  not tracked by git.  Also, if out is not None, write summary of actions to out.
  If `dst` is a top-level git directory, the `.git` directory will be removed.
  '''
  if not os.path.isdir(src):
    raise Exception('Directory "%s" does not exist.' % src)
  if not os.path.isdir(dst):
    os.makedirs(dst)
  ls_files = subprocess.check_output(['git', 'ls-files', '-z', '.'], cwd=src)
  src_files = set(p for p in ls_files.split('\0') if p)
  abs_src = os.path.abspath(src)
  cwd = os.getcwd()
  try:
    os.chdir(dst)
    def output(out, sym, dst, path):
      if out:
        out.write('%s %s%s%s\n' % (sym, dst, os.sep, path))
    for dirpath, dirnames, filenames in os.walk('.', topdown=False):
      for filename in filenames:
        path = os.path.normpath(os.path.join(dirpath, filename))
        if path not in src_files:
          output(out, '-', dst, path)
          os.remove(path)
      for filename in dirnames:
        path = os.path.normpath(os.path.join(dirpath, filename))
        if not os.listdir(path):  # Remove empty subfolders.
          output(out, '-', dst, path + os.sep)
          os.rmdir(path)
    for path in src_files:
      src_path = os.path.join(abs_src, path)
      if os.path.exists(path):
        with open(path) as f1:
          with open(src_path) as f2:
            if f1.read() == f2.read():
              continue
      output(out, '+', dst, path)
      shutil.copy2(src_path, path)
  finally:
    os.chdir(cwd)

if __name__ == '__main__':
  if len(sys.argv) != 3:
    sys.stderr.write('\nUsage:\n  %s SRC_DIR DST_DIR\n\n' % sys.argv[0])
    sys.exit(1)
  copy_git_directory(sys.argv[1], sys.argv[2], sys.stdout)
