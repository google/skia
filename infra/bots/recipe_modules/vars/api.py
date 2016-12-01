# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api


CONFIG_DEBUG = 'Debug'
CONFIG_RELEASE = 'Release'


class SkiaVarsApi(recipe_api.RecipeApi):

  def make_path(self, *path):
    """Return a Path object for the given path."""
    key  = 'custom_%s' % '_'.join(path)
    self.m.path.c.base_paths[key] = tuple(path)
    return self.m.path[key]

  def setup(self):
    """Prepare the variables."""
    # Setup
    self.builder_name = self.m.properties['buildername']
    self.master_name = self.m.properties['mastername']
    self.slave_name = self.m.properties['slavename']
    self.build_number = self.m.properties['buildnumber']

    self.slave_dir = self.m.path['start_dir']
    self.checkout_root = self.slave_dir
    self.default_env = {}
    self.gclient_env = {}
    self.is_compile_bot = self.builder_name.startswith('Build-')
    self.no_buildbot = self.m.properties.get('nobuildbot', '') == 'True'
    self.skia_task_id = self.m.properties.get('skia_task_id', None)

    self.default_env['CHROME_HEADLESS'] = '1'
    # The 'depot_tools' directory comes from recipe DEPS and isn't provided by
    # default. We have to set it manually.
    self.m.path.c.base_paths['depot_tools'] = (
        self.m.path.c.base_paths['start_dir'] +
        ('skia', 'infra', 'bots', '.recipe_deps', 'depot_tools'))
    if 'Win' in self.builder_name:
      self.m.path.c.base_paths['depot_tools'] = (
          'c:\\', 'Users', 'chrome-bot', 'depot_tools')

    # Compile bots keep a persistent checkout.
    self.persistent_checkout = (self.is_compile_bot or
                                'RecreateSKPs' in self.builder_name or
                                '-CT_' in self.builder_name or
                                'Presubmit' in self.builder_name or
                                'InfraTests' in self.builder_name or
                                self.builder_name == "Housekeeper-PerCommit")
    if self.persistent_checkout:
      if 'Win' in self.builder_name:
        self.checkout_root = self.make_path('C:\\', 'b', 'work')
        self.gclient_cache = self.make_path('C:\\', 'b', 'cache')
      else:
        self.checkout_root = self.make_path('/', 'b', 'work')
        self.gclient_cache = self.make_path('/', 'b', 'cache')

      # got_revision is filled in after checkout steps.
      self.got_revision = None
    else:
      # If there's no persistent checkout, then we have to asume we got the
      # correct revision of the files from isolate.
      self.got_revision = self.m.properties['revision']

    self.skia_dir = self.checkout_root.join('skia')
    if not self.persistent_checkout:
      self.m.path['checkout'] = self.skia_dir

    self.infrabots_dir = self.skia_dir.join('infra', 'bots')
    self.resource_dir = self.skia_dir.join('resources')
    self.images_dir = self.slave_dir.join('skimage')
    self.skia_out = self.skia_dir.join('out', self.builder_name)
    self.swarming_out_dir = self.make_path(self.m.properties['swarm_out_dir'])
    self.local_skp_dir = self.slave_dir.join('skp')
    self.local_svg_dir = self.slave_dir.join('svg')
    if not self.is_compile_bot:
      self.skia_out = self.slave_dir.join('out')
    self.tmp_dir = self.m.path['start_dir'].join('tmp')

    # Some bots also require a checkout of chromium.
    self.need_chromium_checkout = False
    if 'CommandBuffer' in self.builder_name:
      self.need_chromium_checkout = True
      self.gclient_env['GYP_CHROMIUM_NO_ACTION'] = '0'
    if 'RecreateSKPs' in self.builder_name:
      self.need_chromium_checkout = True
      self.gclient_env['CPPFLAGS'] = (
          '-DSK_ALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS=1')

    # Some bots also require a checkout of PDFium.
    self.need_pdfium_checkout = 'PDFium' in self.builder_name

    self.builder_cfg = self.m.builder_name_schema.DictForBuilderName(
        self.builder_name)
    self.role = self.builder_cfg['role']
    if self.role == self.m.builder_name_schema.BUILDER_ROLE_HOUSEKEEPER:
      self.configuration = CONFIG_RELEASE
    else:
      self.configuration = self.builder_cfg.get('configuration', CONFIG_DEBUG)
    arch = (self.builder_cfg.get('arch') or self.builder_cfg.get('target_arch'))
    if ('Win' in self.builder_cfg.get('os', '') and arch == 'x86_64'):
      self.configuration += '_x64'

    self.default_env.update({'SKIA_OUT': self.skia_out,
                             'BUILDTYPE': self.configuration})

    self.patch_storage = self.m.properties.get('patch_storage', 'rietveld')
    self.issue = None
    self.patchset = None
    if self.no_buildbot:
      self.is_trybot = False
      if (self.m.properties.get('issue', '') and
          self.m.properties.get('patchset', '')):
        self.is_trybot = True
        self.issue = self.m.properties['issue']
        self.patchset = self.m.properties['patchset']
      elif (self.m.properties.get('patch_issue', '') and
            self.m.properties.get('patch_set', '')):
        self.is_trybot = True
        self.issue = self.m.properties['patch_issue']
        self.patchset = self.m.properties['patch_set']
    else:
      self.is_trybot = self.builder_cfg['is_trybot']
      if self.is_trybot:
        if self.patch_storage == 'gerrit':
          self.issue = self.m.properties['patch_issue']
          self.patchset = self.m.properties['patch_set']
        else:
          self.issue = self.m.properties['issue']
          self.patchset = self.m.properties['patchset']

    self.dm_dir = self.m.path.join(
        self.swarming_out_dir, 'dm')
    self.perf_data_dir = self.m.path.join(self.swarming_out_dir,
        'perfdata', self.builder_name, 'data')
    self._swarming_bot_id = None
    self._swarming_task_id = None

    # Data should go under in _data_dir, which may be preserved across runs.
    self.android_data_dir = '/sdcard/revenge_of_the_skiabot/'
    # Executables go under _bin_dir, which, well, allows executable files.
    self.android_bin_dir  = '/data/local/tmp/'

  @property
  def upload_dm_results(self):
    # TODO(borenet): Move this into the swarm_test recipe.
    skip_upload_bots = [
      'ASAN',
      'Coverage',
      'MSAN',
      'TSAN',
      'UBSAN',
      'Valgrind',
    ]
    upload_dm_results = True
    for s in skip_upload_bots:
      if s in self.m.properties['buildername']:
        upload_dm_results = False
        break
    return upload_dm_results

  @property
  def upload_perf_results(self):
    # TODO(borenet): Move this into the swarm_perf recipe.
    if 'Release' not in self.m.properties['buildername']:
      return False
    skip_upload_bots = [
      'ASAN',
      'Coverage',
      'MSAN',
      'TSAN',
      'UBSAN',
      'Valgrind',
    ]
    upload_perf_results = True
    for s in skip_upload_bots:
      if s in self.m.properties['buildername']:
        upload_perf_results = False
        break
    return upload_perf_results

  @property
  def swarming_bot_id(self):
    if not self._swarming_bot_id:
      self._swarming_bot_id = self.m.python.inline(
          name='get swarming bot id',
          program='''import os
print os.environ.get('SWARMING_BOT_ID', '')
''',
          stdout=self.m.raw_io.output()).stdout.rstrip()
    return self._swarming_bot_id

  @property
  def swarming_task_id(self):
    if not self._swarming_task_id:
      self._swarming_task_id = self.m.python.inline(
          name='get swarming task id',
          program='''import os
print os.environ.get('SWARMING_TASK_ID', '')
''',
          stdout=self.m.raw_io.output()).stdout.rstrip()
    return self._swarming_task_id

