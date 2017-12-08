# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Recipe which:
# 1) Extracts all fiddles out of markdown files.
# 2) Forces fiddle.skia.org to compile all those fiddles and get output in JSON.
# 3) Scans the output and reports any compiletime/runtime errors.
# 4) Updates markdown in site/user/api/ using the new hashes (if any) from
#    fiddle.skia.org.

import json


DEPS = [
  'recipe_engine/context',
  'recipe_engine/file',
  'recipe_engine/path',
  'recipe_engine/properties',
  'recipe_engine/step',
  'core',
  'infra',
  'run',
  'vars',
]

UPDATE_DOCS_GITCOOKIES_FILE = 'update_docs.git_cookies'
UPDATE_DOCS_GITCOOKIES_GS_PATH = (
    'gs://skia-buildbots/artifacts/server/.gitcookies_update-docs')


def go_get_fiddlecli(api):
  env = api.context.env
  env.update(api.infra.go_env)
  with api.context(env=env):
    api.run.with_retry(
        api.step,
        'go get fiddlecli',
        5,  # Update attempts.
        cmd=[api.infra.go_exe, 'get', '-u', '-t',
             'go.skia.org/infra/fiddle/go/fiddlecli'])


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()
  api.infra.go_version()
  go_get_fiddlecli(api)

  with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
    bookmaker_binary = api.path.join(api.vars.skia_out, api.vars.configuration,
                                     'bookmaker')
    fiddlecli_binary = api.path.join(api.infra.gopath, 'bin', 'fiddlecli')
    fiddlecli_input = api.path.join(api.path['start_dir'], 'fiddle.json')
    fiddlecli_output = api.path.join(api.path['start_dir'], 'fiddleout.json')

    # Step 1: Extract all fiddles out of markdown files.
    cmd = [bookmaker_binary,
           '-b', 'docs',  # Path to a *.bmh file or directory.
           '-e', fiddlecli_input,  # Fiddle cli input.
           ]
    api.run(api.step, 'Extract all fiddles out of md files', cmd=cmd)

    # Step 2: Forces fiddle.skia.org to compile all fiddles extracted out of
    #         markdown files and get output in JSON.
    cmd = [fiddlecli_binary,
           '--input', fiddlecli_input,
           '--output', fiddlecli_output,
           '--logtostderr',
           '--force',
        ]
    api.run(api.step, 'Force fiddle to compile all examples', cmd=cmd)

    # Step 3: Scan the output of fiddlecli for any compiletime/runtime errors.
    #         Fail the recipe is there are any errors and summarize results at
    #         the end.
    if api.path.exists(fiddlecli_output):
      test_data = api.properties.get('fiddleout_test_data', '{}')
      content = api.file.read_text('Read fiddleout.json',
                                   fiddlecli_output, test_data=test_data)
      out = json.loads(content)
      failing_fiddles = []
      for fiddle_name in out:
        props = out[fiddle_name]
        if props['compile_errors'] or props['runtime_error']:
          failing_fiddles.append(props['fiddleHash'])
      if failing_fiddles:
        # create an eror message and fail the bot!
        failure_msg = 'The following fiddles failed:\n\n'
        for fiddle_hash in failing_fiddles:
          failure_msg += 'https://fiddle.skia.org/c/%s\n' % fiddle_hash
        raise api.step.StepFailure(failure_msg)

      # Do a dump of fiddlecli_output. Will be useful for debugging.
      print 'Dump of %s:' % fiddlecli_output
      print json.dumps(out, indent=4)

    # Step 4: Update docs in site/user/api/ with the output of fiddlecli.
    #         If there are any new changes then upload and commit the changes.
    update_docs_gitcookies = api.path['start_dir'].join(
        UPDATE_DOCS_GITCOOKIES_FILE)
    cmd = ['python',
           api.vars.skia_dir.join('infra', 'bots', 'upload_md.py'),
           '--bookmaker_binary', bookmaker_binary,
           '--fiddlecli_output', fiddlecli_output,
           '--gitcookies', str(update_docs_gitcookies)]
    with api.infra.DownloadGitCookies(
        UPDATE_DOCS_GITCOOKIES_GS_PATH, update_docs_gitcookies, api):
      with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
        api.run(api.step, 'Generate and Upload Markdown files', cmd=cmd)


def GenTests(api):
  fiddleout_no_errors_test_data = """
{"fiddle1": {"fiddleHash": "abc",
             "compile_errors": [],
             "runtime_error": ""}}
"""
  fiddleout_with_errors_test_data = """
{"fiddle1": {"fiddleHash": "abc",
             "compile_errors": [],
             "runtime_error": "runtime error"}}
"""
  yield (
      api.test('bookmaker') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     fiddleout_test_data=fiddleout_no_errors_test_data,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('fiddleout.json'),
                      api.path['start_dir'].join(UPDATE_DOCS_GITCOOKIES_FILE))
  )

  yield (
      api.test('failed_fiddles') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     fiddleout_test_data=fiddleout_with_errors_test_data,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('fiddleout.json'))
  )

  yield (
      api.test('failed_extract_fiddles') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Extract all fiddles out of md files', retcode=1)
  )

  yield (
      api.test('failed_fiddlecli') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Force fiddle to compile all examples', retcode=1)
  )

  yield (
      api.test('failed_upload') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Generate and Upload Markdown files', retcode=1)
  )
