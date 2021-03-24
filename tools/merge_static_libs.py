#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from __future__ import print_function
import os
import shutil
import subprocess
import sys
import tempfile


def _Usage():
  print('Usage: merge_static_libs OUTPUT_LIB INPUT_LIB [INPUT_LIB]*')
  sys.exit(1)


def MergeLibs(in_libs, out_lib):
  """ Merges multiple static libraries into one.

  in_libs: list of paths to static libraries to be merged
  out_lib: path to the static library which will be created from in_libs
  """
  if os.name == 'posix':
    tempdir = tempfile.mkdtemp()
    abs_in_libs = []
    for in_lib in in_libs:
      abs_in_libs.append(os.path.abspath(in_lib))
    curdir = os.getcwd()
    os.chdir(tempdir)
    objects = []
    ar = os.environ.get('AR', 'ar')
    for in_lib in abs_in_libs:
      proc = subprocess.Popen([ar, '-t', in_lib], stdout=subprocess.PIPE)
      proc.wait()
      obj_str = proc.communicate()[0]
      current_objects = obj_str.rstrip().split('\n')
      proc = subprocess.Popen([ar, '-x', in_lib], stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT)
      proc.wait()
      if proc.poll() == 0:
        # The static library is non-thin, and we extracted objects
        for obj in current_objects:
          objects.append(os.path.abspath(obj))
      elif 'thin archive' in proc.communicate()[0]:
        # The static library is thin, so it contains the paths to its objects
        for obj in current_objects:
          objects.append(obj)
      else:
        raise Exception('Failed to extract objects from %s.' % in_lib)
    os.chdir(curdir)
    if not subprocess.call([ar, '-crs', out_lib] + objects) == 0:
      raise Exception('Failed to add object files to %s' % out_lib)
    shutil.rmtree(tempdir)
  elif os.name == 'nt':
    subprocess.call(['lib', '/OUT:%s' % out_lib] + in_libs)
  else:
    raise Exception('Error: Your platform is not supported')


def Main():
  if len(sys.argv) < 3:
    _Usage()
  out_lib = sys.argv[1]
  in_libs = sys.argv[2:]
  MergeLibs(in_libs, out_lib)


if '__main__' == __name__:
  sys.exit(Main())
