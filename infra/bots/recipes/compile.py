# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe module for Skia Swarming compile.


DEPS = [
  'build',
  'checkout',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/platform',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/step',
  'run',
  'vars',
]


def RunSteps(api):
  api.vars.setup()

  # Check out code.
  if 'NoDEPS' in api.properties['buildername']:
    checkout_root = api.path['start_dir']
    api.checkout.git(checkout_root=checkout_root)
  else:
    checkout_root = api.checkout.default_checkout_root
    if 'Flutter' in api.vars.builder_name:
      checkout_root = checkout_root.join('flutter')
    api.checkout.bot_update(checkout_root=checkout_root)

  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)

  out_dir = checkout_root.join(
      'skia', 'out', api.vars.builder_name, api.vars.configuration)
  if 'Flutter' in api.vars.builder_name:
    out_dir = checkout_root.join('src', 'out', 'android_release')

  try:
    api.build(checkout_root=checkout_root, out_dir=out_dir)

    # TODO(borenet): Move this out of the try/finally.
    dst = api.vars.swarming_out_dir.join('out', api.vars.configuration)
    if 'ParentRevision' in api.vars.builder_name:
      dst = api.vars.swarming_out_dir.join(
          'ParentRevision', 'out', api.vars.configuration)
    api.build.copy_build_products(out_dir=out_dir, dst=dst)
    if 'SKQP' in api.vars.extra_tokens:
      wlist = checkout_root.join(
          'skia', 'infra','cts', 'whitelist_devices.json')
      api.file.copy('copy whitelist', wlist, dst)

  finally:
    if 'Win' in api.vars.builder_cfg.get('os', ''):
      api.python.inline(
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

  api.run.check_failure()


TEST_BUILDERS = [
  'Build-Debian9-Clang-universal-devrel-Android_SKQP',
  'Build-Debian9-Clang-x86_64-Release-NoDEPS',
  'Build-Debian9-Clang-x86_64-Release-ParentRevision',
  'Build-Debian9-GCC-x86_64-Release-Flutter_Android',
  'Build-Win-Clang-x86-Debug',
]


def GenTests(api):
  for builder in TEST_BUILDERS:
    test = (
      api.test(builder) +
      api.properties(buildername=builder,
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(
          api.path['start_dir'].join('tmp', 'uninteresting_hashes.txt')
      )
    )
    yield test
