# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


from recipe_engine import recipe_api


def build_targets_from_builder_dict(builder_dict):
  """Return a list of targets to build, depending on the builder type."""
  if builder_dict.get('extra_config') == 'iOS':
    return ['iOSShell']
  return ['most']


def get_extra_env_vars(builder_dict):
  env = {}
  if builder_dict.get('compiler') == 'Clang':
    env['CC'] = '/usr/bin/clang'
    env['CXX'] = '/usr/bin/clang++'

  # SKNX_NO_SIMD, SK_USE_DISCARDABLE_SCALEDIMAGECACHE, etc.
  extra_config = builder_dict.get('extra_config', '')
  if extra_config.startswith('SK') and extra_config.isupper():
    env['CPPFLAGS'] = '-D' + extra_config

  return env


class CompileApi(recipe_api.RecipeApi):
  def run(self):
    self.m.core.setup()

    env = get_extra_env_vars(self.m.vars.builder_cfg)
    build_targets = build_targets_from_builder_dict(self.m.vars.builder_cfg)

    try:
      for target in build_targets:
        with self.m.step.context({'env': env}):
          self.m.flavor.compile(target)
      self.m.run.copy_build_products(
          self.m.flavor.out_dir,
          self.m.vars.swarming_out_dir.join(
              'out', self.m.vars.configuration))
      self.m.flavor.copy_extra_build_products(self.m.vars.swarming_out_dir)
    finally:
      if 'Win' in self.m.vars.builder_cfg.get('os', ''):
        self.m.python.inline(
            name='cleanup',
            program='''import psutil
for p in psutil.process_iter():
  try:
    if p.name in ('mspdbsrv.exe', 'vctip.exe', 'cl.exe', 'link.exe'):
      p.kill()
  except psutil._error.AccessDenied:
    pass
''',
            infra_step=True)

    self.m.flavor.cleanup_steps()
    self.m.run.check_failure()
