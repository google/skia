# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""
Verify that the bench_pictures.cfg file is sane.
"""


import os
import sys


def Main(argv):
  vars = {'import_path': 'tools'}
  execfile(os.path.join('tools', 'bench_pictures.cfg'), vars)
  bench_pictures_cfg = vars['bench_pictures_cfg']

  for config_name, config_list in bench_pictures_cfg.iteritems():
    if str(config_name) != config_name:
      raise TypeError('%s is not a string!' % str(config_name))    
    for config in config_list:
      for key, value in config.iteritems():
        if str(key) != key:
          raise TypeError('%s is not a string!\n%s' % (str(key), config))
        if type(value).__name__ == 'list':
          for item in value:
            if str(item) != item:
              raise TypeError('%s is not a string!\n%s' % (str(item), config))
        else:
          if str(value) != value:
            raise TypeError('%s is not a string!\n%s' % (str(value), config))

if __name__ == '__main__':
  sys.exit(Main(sys.argv))