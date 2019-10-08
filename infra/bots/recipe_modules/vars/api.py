# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api


CONFIG_DEBUG = 'Debug'
CONFIG_RELEASE = 'Release'


class SkiaVarsApi(recipe_api.RecipeApi):

  def setup(self):
    """Prepare the variables."""
    # Setup
    self.builder_name = self.m.properties['buildername']

    self.slave_dir = self.m.path['start_dir']

    # Special input/output directories.
    self.build_dir = self.slave_dir.join('build')

    self.default_env = self.m.context.env
    self.default_env['CHROME_HEADLESS'] = '1'
    self.default_env['PATH'] = self.m.path.pathsep.join([
        self.default_env.get('PATH', '%(PATH)s'),
        str(self.m.bot_update.repo_resource()),
    ])
    self.cache_dir = self.slave_dir.join('cache')

    self.swarming_out_dir = self.slave_dir.join(
        self.m.properties.get('swarm_out_dir', 'tmp'))

    self.tmp_dir = self.m.path['start_dir'].join('tmp')

    self.builder_cfg = self.m.builder_name_schema.DictForBuilderName(
        self.builder_name)
    self.role = self.builder_cfg['role']
    if self.role in [self.m.builder_name_schema.BUILDER_ROLE_HOUSEKEEPER,
                     self.m.builder_name_schema.BUILDER_ROLE_CALMBENCH]:
      self.configuration = CONFIG_RELEASE
    else:
      self.configuration = self.builder_cfg.get('configuration', CONFIG_DEBUG)
    arch = (self.builder_cfg.get('arch') or self.builder_cfg.get('target_arch'))
    if ('Win' in self.builder_cfg.get('os', '') and arch == 'x86_64'):
      self.configuration += '_x64'

    self.extra_tokens = []
    if len(self.builder_cfg.get('extra_config', '')) > 0:
      if self.builder_cfg['extra_config'].startswith('SK'):
        assert self.builder_cfg['extra_config'].isupper()
        self.extra_tokens = [self.builder_cfg['extra_config']]
      else:
        self.extra_tokens = self.builder_cfg['extra_config'].split('_')

    self.patch_storage = self.m.properties.get('patch_storage', 'gerrit')
    self.issue = None
    self.patchset = None
    self.is_trybot = False
    if (self.m.properties.get('patch_issue', '') and
        self.m.properties['patch_issue'] != '0' and
        self.m.properties.get('patch_set', '') and
        self.m.properties['patch_set'] != '0' and
        self.m.properties.get('patch_ref', '')):
      self.is_trybot = True
      self.issue = self.m.properties['patch_issue']
      self.patchset = self.m.properties['patch_set']

    self._swarming_bot_id = None
    self._swarming_task_id = None

    # Internal bot support.
    self.internal_hardware_label = (
        self.m.properties.get('internal_hardware_label'))
    self.is_internal_bot = self.internal_hardware_label is not None

  @property
  def is_linux(self):
    return 'Ubuntu' in self.builder_name or 'Debian' in self.builder_name

  @property
  def swarming_bot_id(self):
    if not self._swarming_bot_id:
      step_stdout = self.m.python.inline(
          name='get swarming bot id',
          program='''import os
print os.environ.get('SWARMING_BOT_ID', '')
''',
          stdout=self.m.raw_io.output()).stdout
      self._swarming_bot_id = step_stdout.rstrip() if step_stdout else ''
    return self._swarming_bot_id

  @property
  def swarming_task_id(self):
    if not self._swarming_task_id:
      step_stdout = self.m.python.inline(
          name='get swarming task id',
          program='''import os
print os.environ.get('SWARMING_TASK_ID', '')
''',
          stdout=self.m.raw_io.output()).stdout
      self._swarming_task_id = step_stdout.rstrip() if step_stdout else ''
    return self._swarming_task_id
