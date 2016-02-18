#!/usr/bin/env python
#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Utils for running under *SAN"""


import default_flavor
import os


class XSanFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, *args, **kwargs):
    super(XSanFlavorUtils, self).__init__(*args, **kwargs)
    self._sanitizer = {
      # We'd love to just pass 'address,undefined' and get all the checks, but
      # we're not anywhere close to being able to do that.  Instead we start
      # with a set of checks that we know pass or nearly pass.  See here for
      # more information:
      # http://clang.llvm.org/docs/UsersManual.html#controlling-code-generation
      'ASAN': ('address,bool,function,integer-divide-by-zero,nonnull-attribute,'
               'null,object-size,return,returns-nonnull-attribute,shift,'
               'signed-integer-overflow,unreachable,vla-bound,vptr'),
      # MSAN and TSAN can't run together with ASAN, so they're their own bots.
      'MSAN': 'memory',
      'TSAN': 'thread',
    }[self._bot_info.bot_cfg['extra_config']]

  def compile(self, target):
    cmd = [os.path.join(self._bot_info.skia_dir, 'tools', 'xsan_build'),
           self._sanitizer, target]
    self._bot_info.run(cmd)

  def step(self, cmd, env=None, **kwargs):
    """Wrapper for the Step API; runs a step as appropriate for this flavor."""
    lsan_suppressions = self._bot_info.skia_dir.join('tools', 'lsan.supp')
    tsan_suppressions = self._bot_info.skia_dir.join('tools', 'tsan.supp')
    ubsan_suppressions = self._bot_info.skia_dir.join('tools', 'ubsan.supp')
    env = dict(env or {})
    env['ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
    env['LSAN_OPTIONS'] = ('symbolize=1 print_suppressions=1 suppressions=%s' %
                           lsan_suppressions)
    env['TSAN_OPTIONS'] = 'suppressions=%s' % tsan_suppressions
    env['UBSAN_OPTIONS'] = 'suppressions=%s' % ubsan_suppressions

    path_to_app = os.path.join(self._bot_info.out_dir,
                               self._bot_info.configuration, cmd[0])
    new_cmd = [path_to_app]
    new_cmd.extend(cmd[1:])
    return self._bot_info.run(new_cmd, env=env, **kwargs)
