# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for the Skia PerCommit Housekeeper.

DEPS = [
  'recipe_engine/context',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'core',
  'flavor',
  'run',
  'vars',
]


def RunSteps(api):
  # Checkout, compile, etc.
  api.core.setup()

  cwd = api.path['checkout']

  with api.context(cwd=cwd):
    # Get a baseline diff. This should be empty, but we want to be flexible for
    # cases where we have local diffs on purpose.
    diff1 = api.run(
        api.step,
        'git diff #1',
        cmd=['git', 'diff', '--no-ext-diff'],
        stdout=api.m.raw_io.output()).stdout

    # Touch all .fp files so that the generated files are rebuilt.
    api.run(
        api.python.inline,
        'touch fp files',
        program="""import os
import subprocess

for r, d, files in os.walk('%s'):
  for f in files:
    if f.endswith('.fp'):
      path = os.path.join(r, f)
      print 'touch %%s' %% path
      subprocess.check_call(['touch', path])
""" % cwd)

    # Regenerate the SKSL files.
    api.flavor.compile('compile_processors')

    # Get a second diff. If this doesn't match the first, then there have been
    # modifications to the generated files.
    diff2 = api.run(
        api.step,
        'git diff #2',
        cmd=['git', 'diff', '--no-ext-diff'],
        stdout=api.m.raw_io.output()).stdout

    api.run(
        api.python.inline,
        'compare diffs',
        program="""
diff1 = '''%s'''

diff2 = '''%s'''

if diff1 != diff2:
  print 'Generated files have been edited!'
  exit(1)
""" % (diff1, diff2))


def GenTests(api):
  yield (
      api.test('Housekeeper-PerCommit-CheckGeneratedFiles') +
      api.properties(buildername='Housekeeper-PerCommit-CheckGeneratedFiles',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'])
  )
