# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


"""Utils for running under *SAN"""


import default_flavor


class XSanFlavorUtils(default_flavor.DefaultFlavorUtils):
  def __init__(self, m):
    super(XSanFlavorUtils, self).__init__(m)
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
    }[self.m.vars.builder_cfg['extra_config'].replace('Swarming', '')]

  def compile(self, target, **kwargs):
    cmd = [self.m.vars.skia_dir.join('tools', 'xsan_build'),
           self._sanitizer, target]
    self.m.run(self.m.step, 'build %s' % target, cmd=cmd,
               cwd=self.m.vars.skia_dir, **kwargs)

  def copy_extra_build_products(self, swarming_out_dir):
    # Include msan_out if MSAN.
    if 'MSAN' in self.m.vars.builder_cfg['extra_config']:
      msan_out = self.m.path.join(
          'third_party', 'externals', 'llvm', 'msan_out')
      self.m.file.copytree(
          'copy msan_out',
          self.m.vars.skia_dir.join(msan_out),
          swarming_out_dir.join(msan_out),
          symlinks=True)
    # Include llvm_symbolizer from the Chromium DEPS so that suppressions work
    # by symbol name.
    # TODO(benjaminwagner): Figure out how to add this to Skia DEPS for
    # target_os 'llvm'.
    self.m.file.copytree(
        'copy llvm-build',
        self.m.vars.checkout_root.join('src', 'third_party', 'llvm-build'),
        swarming_out_dir.join('llvm-build'),
        symlinks=True)

  def step(self, name, cmd, env=None, **kwargs):
    """Wrapper for the Step API; runs a step as appropriate for this flavor."""
    env = dict(env or {})
    env['ASAN_OPTIONS'] = 'symbolize=1 detect_leaks=1'
    env['LSAN_OPTIONS'] = 'symbolize=1 print_suppressions=1'
    self.m.vars.default_env['PATH'] = '%%(PATH)s:%s' % (
        self.m.vars.slave_dir.join('llvm-build', 'Release+Asserts', 'bin'))
    env['LD_LIBRARY_PATH'] = self.m.vars.slave_dir.join(
        'third_party', 'externals', 'llvm', 'msan_out', 'lib')

    path_to_app = self.out_dir.join(cmd[0])
    new_cmd = [path_to_app]
    new_cmd.extend(cmd[1:])
    return self.m.run(self.m.step, name, cmd=new_cmd, env=env, **kwargs)
