# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api


BUILD_PRODUCTS_ISOLATE_WHITELIST = [
  'dm',
  'dm.exe',
  'get_images_from_skps',
  'get_images_from_skps.exe',
  'nanobench',
  'nanobench.exe',
  'skpbench',
  '*.so',
  '*.dll',
  '*.dylib',
  'skia_launcher',
  'lib/*.so',
  'iOSShell.app',
  'iOSShell.ipa',
  'visualbench',
  'visualbench.exe',
  'vulkan-1.dll',
]


class SkiaStepApi(recipe_api.RecipeApi):

  def __init__(self, *args, **kwargs):
    """Initialize the recipe module."""
    super(SkiaStepApi, self).__init__(*args, **kwargs)

    self._already_ran = {}
    self._ccache = None
    self._checked_for_ccache = False
    self._failed = []

  def check_failure(self):
    """Raise an exception if any step failed."""
    if self._failed:
      raise self.m.step.StepFailure('Failed build steps: %s' %
                                    ', '.join([f.name for f in self._failed]))

  @property
  def failed_steps(self):
    return self._failed[:]

  def run_once(self, fn, *args, **kwargs):
    if not fn.__name__ in self._already_ran:
      self._already_ran[fn.__name__] = fn(*args, **kwargs)
    return self._already_ran[fn.__name__]

  def readfile(self, filename, *args, **kwargs):
    """Convenience function for reading files."""
    name = kwargs.pop('name') or 'read %s' % self.m.path.basename(filename)
    return self.m.file.read(name, filename, infra_step=True, *args, **kwargs)

  def writefile(self, filename, contents):
    """Convenience function for writing files."""
    return self.m.file.write('write %s' % self.m.path.basename(filename),
                             filename, contents, infra_step=True)

  def rmtree(self, path):
    """Wrapper around api.file.rmtree with environment fix."""
    env = {}
    env['PYTHONPATH'] = str(self.m.path['start_dir'].join(
        'skia', 'infra', 'bots', '.recipe_deps', 'build', 'scripts'))
    self.m.file.rmtree(self.m.path.basename(path),
                       path,
                       env=env,
                       infra_step=True)

  def _run(self, steptype, **kwargs):
    """Wrapper for running a step."""
    cwd = kwargs.pop('cwd', None)
    if cwd:
      with self.m.step.context({'cwd': cwd}):
        return steptype(**kwargs)
    return steptype(**kwargs)

  def __call__(self, steptype, name, abort_on_failure=True,
               fail_build_on_failure=True, env=None, **kwargs):
    """Run a step. If it fails, keep going but mark the build status failed."""
    env = env or {}
    env.update(self.m.vars.default_env)
    try:
      return self._run(steptype, name=name, env=env, **kwargs)
    except self.m.step.StepFailure as e:
      if abort_on_failure or fail_build_on_failure:
        self._failed.append(e)
      if abort_on_failure:
        raise  # pragma: no cover

  def copy_build_products(self, src, dst):
    """Copy whitelisted build products from src to dst."""
    self.m.python.inline(
        name='copy build products',
        program='''import errno
import glob
import os
import shutil
import sys

src = sys.argv[1]
dst = sys.argv[2]
build_products_whitelist = %s

try:
  os.makedirs(dst)
except OSError as e:
  if e.errno != errno.EEXIST:
    raise

for pattern in build_products_whitelist:
  path = os.path.join(src, pattern)
  for f in glob.glob(path):
    dst_path = os.path.join(dst, os.path.relpath(f, src))
    if not os.path.isdir(os.path.dirname(dst_path)):
      os.makedirs(os.path.dirname(dst_path))
    print 'Copying build product %%s to %%s' %% (f, dst_path)
    shutil.move(f, dst_path)
''' % str(BUILD_PRODUCTS_ISOLATE_WHITELIST),
        args=[src, dst],
        infra_step=True)

  def with_retry(self, steptype, name, attempts, **kwargs):
    for attempt in xrange(attempts):
      step_name = name
      if attempt > 0:
        step_name += ' (attempt %d)' % (attempt + 1)
      try:
        res = self._run(steptype, name=step_name, **kwargs)
        return res
      except self.m.step.StepFailure:
        if attempt == attempts - 1:
          raise
