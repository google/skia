# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import gn_flavor


"""Utils for running under Valgrind."""


class ValgrindFlavorUtils(gn_flavor.GNFlavorUtils):
  def __init__(self, m):
    super(ValgrindFlavorUtils, self).__init__(m)
    self._suppressions_file = self.m.vars.skia_dir.join(
        'tools', 'valgrind.supp')

  def step(self, name, cmd, **kwargs):
    new_cmd = ['valgrind', '--gen-suppressions=all', '--leak-check=full',
               '--track-origins=yes', '--error-exitcode=1', '--num-callers=40',
               '--suppressions=%s' % self._suppressions_file]
    path_to_app = self.out_dir.join(cmd[0])
    new_cmd.append(path_to_app)
    new_cmd.extend(cmd[1:])
    return self.m.run(self.m.step, name, cmd=new_cmd,
                            **kwargs)

