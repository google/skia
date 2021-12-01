# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api


TEST_DEFAULT_ASSET_VERSION = '42'

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
    name = kwargs.pop('name', 'read %s' % self.m.path.basename(filename))
    return self.m.file.read_text(name, filename, *args, **kwargs)

  def writefile(self, filename, contents):
    """Convenience function for writing files."""
    return self.m.file.write_text('write %s' % self.m.path.basename(filename),
                                  filename, contents)

  def rmtree(self, path):
    """Wrapper around api.file.rmtree."""
    self.m.file.rmtree('rmtree %s' % self.m.path.basename(path), path)

  def asset_version(self, asset_name, skia_dir, test_data=None):
    """Return the contents of VERSION for the given asset as a string.

    If test_data is not specified, reads the property
    'test_<asset_name>_version' or if not present, uses
    TEST_DEFAULT_ASSET_VERSION."""
    version_file = skia_dir.join(
        'infra', 'bots', 'assets', asset_name, 'VERSION')
    if not test_data:
      test_data = self.m.properties.get(
          'test_%s_version' % asset_name, TEST_DEFAULT_ASSET_VERSION)
    return self.m.file.read_text('Get %s VERSION' % asset_name,
                                 version_file,
                                 test_data=test_data).rstrip()

  def __call__(self, steptype, name, abort_on_failure=True,
               fail_build_on_failure=True, **kwargs):
    """Run a step. If it fails, keep going but mark the build status failed."""
    try:
      with self.m.env(self.m.vars.default_env):
        return steptype(name=name, **kwargs)
    except self.m.step.StepFailure as e:
      if fail_build_on_failure:
        self._failed.append(e)
      if abort_on_failure:
        raise

  def with_retry(self, steptype, name, attempts, between_attempts_fn=None,
                 abort_on_failure=True, fail_build_on_failure=True, **kwargs):
    for attempt in range(attempts):
      step_name = name
      if attempt > 0:
        step_name += ' (attempt %d)' % (attempt + 1)
      try:
        res = self(steptype, name=step_name, abort_on_failure=True,
                   fail_build_on_failure=fail_build_on_failure, **kwargs)
        if attempt > 0 and fail_build_on_failure:
          del self._failed[-attempt:]
        return res
      except self.m.step.StepFailure:
        if attempt == attempts - 1:
          if abort_on_failure:
            raise
        elif between_attempts_fn:
          between_attempts_fn(attempt+1)
