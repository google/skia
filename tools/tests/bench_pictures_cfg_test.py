# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
Verify that the bench_pictures.cfg file is sane.
"""


import os
import sys


def ThrowIfNotAString(obj):
  """ Raise a TypeError if obj is not a string. """
  if str(obj) != obj:
    raise TypeError('%s is not a string!' % str(obj))


def Main(argv):
  """ Verify that the bench_pictures.cfg file is sane.

  - Exec the file to ensure that it uses correct Python syntax.
  - Make sure that every element is a string, because the buildbot scripts will
      fail to execute if this is not the case.

  This test does not verify that the well-formed configs are actually valid.
  """
  vars = {'import_path': 'tools'}
  execfile(os.path.join('tools', 'bench_pictures.cfg'), vars)
  bench_pictures_cfg = vars['bench_pictures_cfg']

  for config_name, config_list in bench_pictures_cfg.iteritems():
    ThrowIfNotAString(config_name)  
    for config in config_list:
      for key, value in config.iteritems():
        ThrowIfNotAString(key)
        if type(value).__name__ == 'list':
          for item in value:
            ThrowIfNotAString(item)
        elif not value is True:
          ThrowIfNotAString(value)

if __name__ == '__main__':
  sys.exit(Main(sys.argv))