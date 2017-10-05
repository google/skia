# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading Coverage results.


import calendar


DEPS = [
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'recipe_engine/time',
]


TRY_JOB_FOLDER = 'trybot/%s/%s/' # % (cl_number, patchset_number)
COMMIT_FOLDER = 'commit/%s/'      # % (git_revision)

RAW_FILE = '%s.profraw'
PARSED_FILE = '%s.profdata'

UPLOAD_ATTEMPTS = 5

def cp(api, name, src, dst, extra_args=None):
  cmd = ['gsutil', 'cp']
  # if extra_args:
  #   cmd.extend(extra_args)
  cmd.extend([src, dst])

  name = 'upload %s' % name
  for i in xrange(UPLOAD_ATTEMPTS):
    step_name = name
    if i > 0:
      step_name += ' (attempt %d)' % (i+1)
    try:
      api.step(step_name, cmd=cmd)
      break
    except api.step.StepFailure:
      if i == UPLOAD_ATTEMPTS - 1:
        raise

def RunSteps(api):
  builder_name = api.properties['buildername']
  revision = api.properties['revision']
  bucket = api.properties['gs_bucket']

  raw_data = api.path['start_dir'].join('output.profraw')
  executable = api.path['start_dir'].join('out','Debug','dm')
  clang_dir = api.path['start_dir'].join('clang_linux', 'bin')

  gcs_file = RAW_FILE % builder_name
  path = COMMIT_FOLDER % revision

  issue = api.properties.get('patch_issue')
  patchset = api.properties.get('patch_set')
  if issue and patchset:
    path = TRY_JOB_FOLDER % (issue, patchset)

  cp(api, 'raw data', raw_data, 'gs://%s/%s%s' % (bucket, path, gcs_file))

  # Merge and Index reports
  indexed_data = api.path['start_dir'].join('output.profdata')
  api.step('Merge and index', cmd=[clang_dir.join('llvm-profdata'), 'merge', '-sparse', raw_data, '-o', indexed_data ])

  gcs_file = PARSED_FILE % builder_name

  cp(api, 'parsed data', indexed_data, 'gs://%s/%s%s' % (bucket, path, gcs_file))

  # Create output
  output_data = api.path['start_dir'].join('coverage_output')
  api.step('Create Summary', cmd=[clang_dir.join('llvm-cov'), 'show', executable, '-instr-profile='+str(indexed_data), '-use-color=0', '-output-dir='+str(output_data) ])

  gcs_file = PARSED_FILE % builder_name

  #cp(api, 'parsed data', indexed_data, 'gs://%s/%s%s' % (bucket, path, gcs_file))


def GenTests(api):
  builder = 'Test-Debian9-GCC-GCE-CPU-AVX2-x86_64-Debug'
  yield (
    api.test('normal_bot') +
    api.properties(buildername=builder,
                   gs_bucket='skia-coverage',
                   revision='abc123',
                   path_config='kitchen')
  )

  yield (
    api.test('alternate_bucket') +
    api.properties(buildername=builder,
                   gs_bucket='skia-coverage-alt',
                   revision='abc123',
                   path_config='kitchen')
  )

  yield (
    api.test('failed_once') +
    api.properties(buildername=builder,
                   gs_bucket='skia-coverage',
                   revision='abc123',
                   path_config='kitchen') +
    api.step_data('upload raw data', retcode=1)
  )

  yield (
    api.test('failed_all') +
    api.properties(buildername=builder,
                   gs_bucket='skia-coverage',
                   revision='abc123',
                   path_config='kitchen') +
    api.step_data('upload raw data', retcode=1) +
    api.step_data('upload raw data (attempt 2)', retcode=1) +
    api.step_data('upload raw data (attempt 3)', retcode=1) +
    api.step_data('upload raw data (attempt 4)', retcode=1) +
    api.step_data('upload raw data (attempt 5)', retcode=1)
  )

  yield (
      api.test('trybot') +
      api.properties(
          buildername=builder,
          gs_bucket='skia-coverage',
          revision='abc123',
          path_config='kitchen',
          patch_storage='gerrit') +
      api.properties.tryserver(
          buildername=builder,
          gerrit_project='skia',
          gerrit_url='https://skia-review.googlesource.com/',
      )
  )
