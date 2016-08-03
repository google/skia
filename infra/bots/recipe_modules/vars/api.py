# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# pylint: disable=W0201


from recipe_engine import recipe_api
import os


BOTO_CHROMIUM_SKIA_GM = 'chromium-skia-gm.boto'


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


  def update_with_builder_spec(self, builder_spec):
    """Set more variables based on the builder_spec."""
    # Obtain the spec for this builder from the Skia repo. Use it to set more
    # properties.
    self.builder_spec = builder_spec

    self.builder_cfg = self.builder_spec['builder_cfg']
    self.role = self.builder_cfg['role']

    self.configuration = self.builder_spec['configuration']
    self.default_env.update({'SKIA_OUT': self.skia_out,
                             'BUILDTYPE': self.configuration})
    self.default_env.update(self.builder_spec['env'])
    self.build_targets = [str(t) for t in self.builder_spec['build_targets']]
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
    self.dm_flags = self.builder_spec['dm_flags']
    self.nanobench_flags = self.builder_spec['nanobench_flags']
