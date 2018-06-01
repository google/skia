# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe for uploading Coverage results.


import calendar


DEPS = [
  'gsutil',
  'recipe_engine/file',
  'recipe_engine/json',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/python',
  'recipe_engine/raw_io',
  'recipe_engine/step',
  'recipe_engine/time',
]


TRY_JOB_FOLDER = 'trybot/%s/%s/' # % (issue_number, patchset_number)
COMMIT_FOLDER = 'commit/%s/'      # % (git_revision)

RAW_FILE = '*.profraw'
PARSED_FILE = '%s.profdata'
SUMMARY_FILE = '%s.summary'

COVERAGE_RAW_ARCHIVE = '%s.profraw.tar.gz'
# Text is an easier format to read with machines (e.g. for Gerrit).
COVERAGE_TEXT_FILE = '%s.text.tar'
# HTML is a quick and dirty browsable format. (e.g. for coverage.skia.org)
COVERAGE_HTML_FILE = '%s.html.tar'

def RunSteps(api):
  # See https://clang.llvm.org/docs/SourceBasedCodeCoverage.html for a
  # detailed explanation of getting code coverage from LLVM.
  # Since we have already compiled the binary with the special flags
  # and run the executable to generate an output.profraw, we
  # need to merge and index the data, create the coverage output,
  # and then upload the results to GCS. We also upload the intermediate
  # results to GCS so we can regenerate reports if needed.
  builder_name = api.properties['buildername']
  bucket = api.properties['gs_bucket']

  # The raw data files are brought in as isolated inputs. It is possible
  # for there to be 1 if the coverage task wasn't broken up.
  raw_inputs = api.file.glob_paths(
      'find raw inputs', api.path['start_dir'].join('coverage'),
      RAW_FILE, test_data=['a.raw', 'b.raw', 'c.raw'])


  # The instrumented executable is brought in as an isolated input.
  executable = api.path['start_dir'].join('build', 'dm')
  # clang_dir is brought in via CIPD.
  clang_dir = api.path['start_dir'].join('clang_linux', 'bin')

  revision = api.properties['revision']
  path = COMMIT_FOLDER % revision

  issue = api.properties.get('patch_issue')
  patchset = api.properties.get('patch_set')
  if issue and patchset:
    path = TRY_JOB_FOLDER % (issue, patchset)

  # Upload the raw files, tarred together to decrease upload time and
  # improve compression.
  tar_file = api.path['start_dir'].join('raw_data.profraw.tar.gz')
  cmd = ['tar', '-zcvf', tar_file]
  cmd.extend(raw_inputs)
  api.step('create raw data archive', cmd=cmd)

  gcs_file = COVERAGE_RAW_ARCHIVE % builder_name
  api.gsutil.cp('raw data archive', tar_file,
                'gs://%s/%s%s' % (bucket, path, gcs_file))

  # Merge all the raw data files together, then index the data.
  # This creates one cohesive
  indexed_data = api.path['start_dir'].join('output.profdata')
  cmd = [clang_dir.join('llvm-profdata'),
         'merge',
         '-sparse',
         '-o',
         indexed_data]
  cmd.extend(raw_inputs)
  api.step('merge and index',
           cmd=cmd)

  gcs_file = PARSED_FILE % builder_name
  api.gsutil.cp('parsed data', indexed_data,
                   'gs://%s/%s%s' % (bucket, path, gcs_file), extra_args=['-Z'])

  # Create text coverage output
  output_data = api.path['start_dir'].join('coverage_text')
  api.step('create text summary',
           cmd=[clang_dir.join('llvm-cov'),
               'show',
               executable,
               '-instr-profile=' + str(indexed_data),
               '-use-color=0',
               '-format=text',
               '-output-dir=' + str(output_data)])

  # Upload the summary by itself so we can get easier access to it (instead of
  # downloading and untarring all the coverage data.
  gcs_file = SUMMARY_FILE % builder_name
  api.gsutil.cp('coverage summary', output_data.join('index.txt'),
                   'gs://%s/%s%s' % (bucket, path, gcs_file), extra_args=['-Z'])

  tar_file = api.path['start_dir'].join('coverage.text.tar')

  # Tar and upload the coverage data. We tar it to ease downloading/ingestion,
  # otherwise, there is a 1:1 mapping of source code files -> coverage files.
  api.step('create text coverage archive', cmd=['tar', '-cvf',
                                           tar_file, output_data])

  gcs_file = COVERAGE_TEXT_FILE % builder_name
  api.gsutil.cp('text coverage data', tar_file,
                   'gs://%s/%s%s' % (bucket, path, gcs_file), extra_args=['-Z'])

  # Create html coverage output
  output_data = api.path['start_dir'].join('coverage_html')
  api.step('create html summary',
           cmd=[clang_dir.join('llvm-cov'),
               'show',
               executable,
               '-instr-profile=' + str(indexed_data),
               '-use-color=1',
               '-format=html',
               '-output-dir=' + str(output_data)])

  tar_file = api.path['start_dir'].join('coverage.html.tar')

  # Tar and upload the coverage data. We tar it to ease downloading/ingestion,
  # otherwise, there is a 1:1 mapping of source code files -> coverage files.
  api.step('create html coverage archive',
           cmd=['tar', '-cvf', tar_file, output_data])

  gcs_file = COVERAGE_HTML_FILE % builder_name
  api.gsutil.cp('html coverage data', tar_file,
                   'gs://%s/%s%s' % (bucket, path, gcs_file), extra_args=['-Z'])


def GenTests(api):
  builder = 'Test-Debian9-GCC-GCE-CPU-AVX2-x86_64-Debug-All'
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
    api.step_data('upload parsed data', retcode=1)
  )

  yield (
    api.test('failed_all') +
    api.properties(buildername=builder,
                   gs_bucket='skia-coverage',
                   revision='abc123',
                   path_config='kitchen') +
    api.step_data('upload parsed data', retcode=1) +
    api.step_data('upload parsed data (attempt 2)', retcode=1) +
    api.step_data('upload parsed data (attempt 3)', retcode=1) +
    api.step_data('upload parsed data (attempt 4)', retcode=1) +
    api.step_data('upload parsed data (attempt 5)', retcode=1)
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
