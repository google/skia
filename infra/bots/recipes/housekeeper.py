# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for the Skia PerCommit Housekeeper.


import calendar


DEPS = [
  'binary_size',
  'checkout',
  'doxygen',
  'flavor',
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/time',
  'run',
  'vars',
]


def RunSteps(api):
  # Checkout, compile, etc.
  api.vars.setup()
  checkout_root = api.checkout.default_checkout_root
  got_revision = api.checkout.bot_update(checkout_root=checkout_root)
  api.file.ensure_directory('makedirs tmp_dir', api.vars.tmp_dir)
  api.flavor.setup()

  # TODO(borenet): Detect static initializers?

  skia_dir = checkout_root.join('skia')
  if not api.vars.is_trybot:
    api.doxygen.generate_and_upload(skia_dir)

  now = api.time.utcnow()
  ts = int(calendar.timegm(now.utctimetuple()))
  filename = 'nanobench_%s_%d.json' % (got_revision, ts)
  dest_dir = api.flavor.host_dirs.perf_data_dir
  dest_file = dest_dir.join(filename)
  api.file.ensure_directory('makedirs perf_dir', dest_dir)
  api.binary_size.run_analysis(skia_dir, dest_file)


def GenTests(api):
  yield (
      api.test('Housekeeper-PerCommit') +
      api.properties(buildername='Housekeeper-PerCommit',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'])
  )
  yield (
      api.test('Housekeeper-PerCommit-Trybot') +
      api.properties(buildername='Housekeeper-PerCommit',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     patch_storage='gerrit',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.properties.tryserver(
          buildername='Housekeeper-PerCommit',
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      ) +
      api.path.exists(api.path['start_dir'])
  )
