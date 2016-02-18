#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import default_flavor
import os


"""Utils for running under Valgrind."""


class ValgrindFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, *args, **kwargs):
    super(ValgrindFlavorUtils, self).__init__(*args, **kwargs)
    self._suppressions_file = os.path.join(self._bot_info.skia_dir,
        'tools', 'valgrind.supp')

  def step(self, name, cmd, **kwargs):
    new_cmd = ['valgrind', '--gen-suppressions=all', '--leak-check=full',
               '--track-origins=yes', '--error-exitcode=1', '--num-callers=40',
               '--suppressions=%s' % self._suppressions_file]
    path_to_app = os.path.join(self._bot_info.out_dir,
                               self._bot_info.configuration, cmd[0])
    new_cmd.append(path_to_app)
    new_cmd.extend(cmd[1:])
    return self._bot_info.run(new_cmd, **kwargs)

