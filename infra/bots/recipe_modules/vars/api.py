# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api
import os


BOTO_CHROMIUM_SKIA_GM = 'chromium-skia-gm.boto'

CONFIG_DEBUG = 'Debug'
CONFIG_RELEASE = 'Release'


def device_cfg(builder_dict):
  # Android.
  if 'Android' in builder_dict.get('extra_config', ''):
    if 'NoNeon' in builder_dict['extra_config']:
      return 'arm_v7'
    return {
      'Arm64': 'arm64',
      'x86': 'x86',
      'x86_64': 'x86_64',
      'Mips': 'mips',
      'Mips64': 'mips64',
      'MipsDSP2': 'mips_dsp2',
    }.get(builder_dict['target_arch'], 'arm_v7_neon')
  elif builder_dict.get('os') == 'Android':
    return {
      'AndroidOne':    'arm_v7_neon',
      'GalaxyS3':      'arm_v7_neon',
      'GalaxyS4':      'arm_v7_neon',
      'NVIDIA_Shield': 'arm64',
      'Nexus10':       'arm_v7_neon',
      'Nexus5':        'arm_v7_neon',
      'Nexus6':        'arm_v7_neon',
      'Nexus7':        'arm_v7_neon',
      'Nexus7v2':      'arm_v7_neon',
      'Nexus9':        'arm64',
      'NexusPlayer':   'x86',
    }[builder_dict['model']]

  # iOS.
  if 'iOS' in builder_dict.get('os', ''):
    return {
      'iPad4': 'iPad4,1',
    }[builder_dict['model']]

  return None


def product_board(builder_dict):
  if 'Android' in builder_dict.get('os', ''):
    return {
      'AndroidOne':    'sprout',
      'GalaxyS3':      'm0',  #'smdk4x12', Detected incorrectly by swarming?
      'GalaxyS4':      None,  # TODO(borenet,kjlubick)
      'NVIDIA_Shield': 'foster',
      'Nexus10':       'manta',
      'Nexus5':        'hammerhead',
      'Nexus6':        'shamu',
      'Nexus7':        'grouper',
      'Nexus7v2':      'flo',
      'Nexus9':        'flounder',
      'NexusPlayer':   'fugu',
    }[builder_dict['model']]
  return None


def get_builder_spec(api, builder_name):
  builder_dict = api.builder_name_schema.DictForBuilderName(builder_name)
  rv = {
    'builder_cfg': builder_dict,
  }
  device = device_cfg(builder_dict)
  if device:
    rv['device_cfg'] = device
  board = product_board(builder_dict)
  if board:
    rv['product.board'] = board

  role = builder_dict['role']
  if role == api.builder_name_schema.BUILDER_ROLE_HOUSEKEEPER:
    configuration = CONFIG_RELEASE
  else:
    configuration = builder_dict.get('configuration', CONFIG_DEBUG)
  arch = (builder_dict.get('arch') or builder_dict.get('target_arch'))
  if ('Win' in builder_dict.get('os', '') and arch == 'x86_64'):
    configuration += '_x64'
  rv['configuration'] = configuration
  if configuration == 'Coverage':
    rv['do_compile_steps'] = False
  rv['do_test_steps'] = role == api.builder_name_schema.BUILDER_ROLE_TEST
  rv['do_perf_steps'] = role == api.builder_name_schema.BUILDER_ROLE_PERF

  # Do we upload perf results?
  upload_perf_results = False
  if (role == api.builder_name_schema.BUILDER_ROLE_PERF and
      CONFIG_RELEASE in configuration):
    upload_perf_results = True
  rv['upload_perf_results'] = upload_perf_results

  # Do we upload correctness results?
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
    if s in builder_name:
      upload_dm_results = False
      break
  rv['upload_dm_results'] = upload_dm_results

  return rv


class SkiaVarsApi(recipe_api.RecipeApi):

  def make_path(self, *path):
    """Return a Path object for the given path."""
    key  = 'custom_%s' % '_'.join(path)
    self.m.path.c.base_paths[key] = tuple(path)
    return self.m.path[key]

  def gsutil_env(self, boto_file):
    """Environment variables for gsutil."""
    boto_path = None
    if boto_file:
      boto_path = self.m.path.join(self.home_dir, boto_file)
    return {'AWS_CREDENTIAL_FILE': boto_path,
            'BOTO_CONFIG': boto_path}

  @property
  def home_dir(self):
    """Find the home directory."""
    home_dir = os.path.expanduser('~')
    if self._test_data.enabled:
      home_dir = '[HOME]'
    return home_dir

  def get_builder_spec(self, builder_name):
    """Return the builder_spec for the given builder name."""
    return get_builder_spec(self.m, builder_name)

  def setup(self):
    """Prepare the variables."""
    # Setup
    self.builder_name = self.m.properties['buildername']
    self.master_name = self.m.properties['mastername']
    self.slave_name = self.m.properties['slavename']
    self.build_number = self.m.properties['buildnumber']

    self.slave_dir = self.m.path['slave_build']
    self.checkout_root = self.slave_dir
    self.default_env = {}
    self.gclient_env = {}
    self.is_compile_bot = self.builder_name.startswith('Build-')

    self.default_env['CHROME_HEADLESS'] = '1'
    # The 'depot_tools' directory comes from recipe DEPS and isn't provided by
    # default. We have to set it manually.
    self.m.path.c.base_paths['depot_tools'] = (
        self.m.path.c.base_paths['slave_build'] +
        ('skia', 'infra', 'bots', '.recipe_deps', 'depot_tools'))
    if 'Win' in self.builder_name:
      self.m.path.c.base_paths['depot_tools'] = (
          'c:\\', 'Users', 'chrome-bot', 'depot_tools')

    # Compile bots keep a persistent checkout.
    self.persistent_checkout = (self.is_compile_bot or
                                'RecreateSKPs' in self.builder_name)
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
    if not self.is_compile_bot:
      self.skia_out = self.slave_dir.join('out')
    self.tmp_dir = self.m.path['slave_build'].join('tmp')

    # Some bots also require a checkout of chromium.
    self.need_chromium_checkout = 'CommandBuffer' in self.builder_name
    if 'CommandBuffer' in self.builder_name:
      self.gclient_env['GYP_CHROMIUM_NO_ACTION'] = '0'
    if ((self.is_compile_bot and
         'SAN' in self.builder_name) or
        'RecreateSKPs' in self.builder_name):
      self.need_chromium_checkout = True
      if 'RecreateSKPs' in self.builder_name:
        self.gclient_env['CPPFLAGS'] = (
            '-DSK_ALLOW_CROSSPROCESS_PICTUREIMAGEFILTERS=1')

    # Some bots also require a checkout of PDFium.
    self.need_pdfium_checkout = 'PDFium' in self.builder_name

    # Obtain the spec for this builder. Use it to set more properties.
    self.builder_spec = get_builder_spec(self.m, self.builder_name)

    self.builder_cfg = self.builder_spec['builder_cfg']
    self.role = self.builder_cfg['role']

    self.configuration = self.builder_spec['configuration']
    self.default_env.update({'SKIA_OUT': self.skia_out,
                             'BUILDTYPE': self.configuration})
    self.do_compile_steps = self.builder_spec.get('do_compile_steps', True)
    self.do_test_steps = self.builder_spec['do_test_steps']
    self.do_perf_steps = self.builder_spec['do_perf_steps']
    self.is_trybot = self.builder_cfg['is_trybot']
    self.issue = None
    self.patchset = None
    self.rietveld = None
    if self.is_trybot:
      self.issue = self.m.properties['issue']
      self.patchset = self.m.properties['patchset']
      self.rietveld = self.m.properties['rietveld']
    self.upload_dm_results = self.builder_spec['upload_dm_results']
    self.upload_perf_results = self.builder_spec['upload_perf_results']
    self.dm_dir = self.m.path.join(
        self.swarming_out_dir, 'dm')
    self.perf_data_dir = self.m.path.join(self.swarming_out_dir,
        'perfdata', self.builder_name, 'data')
