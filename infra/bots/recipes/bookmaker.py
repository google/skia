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
  api.core.checkout_bot_update()
  api.infra.go_version()
  go_get_fiddlecli(api)

  with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
    bookmaker_binary = api.path.join(api.vars.skia_out, api.vars.configuration,
                                     'bookmaker')
    buildername = api.vars.builder_name

    if 'PerCommit' in buildername:
      # Check to see if docs matches include/core.
      cmd = [bookmaker_binary,
             '-a', 'docs/status.json',  # File containing status of docs.
             '-x',  # Check bmh against includes.
             ]
      try:
        api.run(api.step, 'Validate docs match include/core/*.h', cmd=cmd)
      except api.step.StepFailure as e:
        # Display what needs to be fixed.
        e.reason += (
            '\n\nView the output of the "Validate docs match include/core/*.h" '
            'step to see how to get this bot green.'
            '\n\nhttps://skia.org/user/api/usingBookmaker details how to build '
            'and run the bookmaker utility locally if needed.')
        raise e

    elif 'Nightly' in buildername:
      fiddlecli_binary = api.path.join(api.infra.gopath, 'bin', 'fiddlecli')
      fiddlecli_input = api.path.join(api.path['start_dir'], 'fiddle.json')
      fiddlecli_output = api.path.join(api.path['start_dir'], 'fiddleout.json')

      # Step 1: Extract all fiddles out of markdown files.
      cmd = [bookmaker_binary,
             '-a', 'docs/status.json',  # File containing status of docs.
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
        # Do a dump of fiddlecli_output. Will be useful for debugging.
        print 'Dump of %s:' % fiddlecli_output
        print json.dumps(out, indent=4)

        failing_fiddles_to_errors = {}
        for fiddle_name in out:
          props = out[fiddle_name]
          if props['compile_errors'] or props['runtime_error']:
            # Construct the error.
            error = props['runtime_error']
            if props['compile_errors']:
              for e in props['compile_errors']:
                error += '%s\n' % e['text']
            failing_fiddles_to_errors[props['fiddleHash']] = error

        if failing_fiddles_to_errors:
          # create an eror message and fail the bot!
          failure_msg = 'Failed fiddles with their errors:\n\n\n'
          counter = 0
          for fiddle_hash, error in failing_fiddles_to_errors.iteritems():
            counter += 1
            failure_msg += '%d. https://fiddle.skia.org/c/%s\n\n' % (
                counter, fiddle_hash)
            failure_msg += '%s\n\n' % error
          raise api.step.StepFailure(failure_msg)

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
             "compile_errors": [
            {
                "text": "ninja: Entering directory `out/Release'",
                "line": 0,
                "col": 0
            },
            {
                "text": "[1/7] ACTION //:skia.h(//gn/toolchain:gcc_like)",
                "line": 0,
                "col": 0
            },
            {
                "text": "[2/7] stamp obj/skia.h.stamp",
                "line": 0,
                "col": 0
            },
            {
                "text": "[3/7] compile ../../tools/fiddle/draw.cpp",
                "line": 0,
                "col": 0
            },
            {
                "text": "FAILED: obj/tools/fiddle/fiddle.draw.o ",
                "line": 0,
                "col": 0
            },
            {
                "text": "c++ -MMD -MF obj/tools/fiddle/fiddle.draw.o.d -DNDEBUG -DSK_HAS_HEIF_LIBRARY -DSK_HAS_JPEG_LIBRARY -DSK_SUPPORT_PDF -DSK_PDF_USE_SFNTLY -DSK_HAS_PNG_LIBRARY -DSK_CODEC_DECODES_RAW -DSK_HAS_WEBP_LIBRARY -DSK_XML -DSK_GAMMA_APPLY_TO_A8 -DSK_ENABLE_DISCRETE_GPU -DGR_TEST_UTILS=1 -DSK_SAMPLES_FOR_X -DSK_SUPPORT_ATLAS_TEXT=1 -I../../tools/flags -I../../include/private -I../../src/c -I../../src/codec -I../../src/core -I../../src/effects -I../../src/fonts -I../../src/image -I../../src/images -I../../src/lazy -I../../src/opts -I../../src/pathops -I../../src/pdf -I../../src/ports -I../../src/sfnt -I../../src/shaders -I../../src/shaders/gradients -I../../src/sksl -I../../src/utils -I../../src/utils/win -I../../src/xml -I../../third_party/gif -I../../src/gpu -I../../tools/gpu -I../../include/android -I../../include/c -I../../include/codec -I../../include/config -I../../include/core -I../../include/effects -I../../include/encode -I../../include/gpu -I../../include/gpu/gl -I../../include/atlastext -I../../include/pathops -I../../include/ports -I../../include/svg -I../../include/utils -I../../include/utils/mac -I../../include/atlastext -Igen -fstrict-aliasing -fPIC -Werror -Wall -Wextra -Winit-self -Wpointer-arith -Wsign-compare -Wvla -Wno-deprecated-declarations -Wno-maybe-uninitialized -Wno-unused-parameter -O3 -fdata-sections -ffunction-sections -g -std=c++11 -fno-exceptions -fno-rtti -Wnon-virtual-dtor -Wno-error -c ../../tools/fiddle/draw.cpp -o obj/tools/fiddle/fiddle.draw.o",
                "line": 0,
                "col": 0
            },
            {
                "text": "../../tools/fiddle/draw.cpp: In function 'void draw(SkCanvas*)':",
                "line": 0,
                "col": 0
            },
            {
                "text": "draw.cpp:5:12: error: aggregate 'SkMask mask' has incomplete type and cannot be defined",
                "line": 5,
                "col": 12
            },
            {
                "text": " }",
                "line": 0,
                "col": 0
            },
            {
                "text": "            ^   ",
                "line": 0,
                "col": 0
            },
            {
                "text": "draw.cpp:6:28: error: incomplete type 'SkMask' used in nested name specifier",
                "line": 6,
                "col": 28
            },
            {
                "text": " ",
                "line": 0,
                "col": 0
            },
            {
                "text": "                            ^         ",
                "line": 0,
                "col": 0
            },
            {
                "text": "draw.cpp:14:28: error: incomplete type 'SkMask' used in nested name specifier",
                "line": 14,
                "col": 28
            },
            {
                "text": "     uint8_t bytes[] = { 0, 1, 2, 3, 4, 5, 6, 7 };",
                "line": 0,
                "col": 0
            },
            {
                "text": "                            ^~~~~~~~~~",
                "line": 0,
                "col": 0
            },
            {
                "text": "[4/7] compile ../../tools/fiddle/egl_context.cpp",
                "line": 0,
                "col": 0
            },
            {
                "text": "[5/7] compile ../../tools/fiddle/fiddle_main.cpp",
                "line": 0,
                "col": 0
            },
            {
                "text": "[6/7] link libskia.a",
                "line": 0,
                "col": 0
            },
            {
                "text": "ninja: build stopped: subcommand failed.",
                "line": 0,
                "col": 0
            },
            {
                "text": "",
                "line": 0,
                "col": 0
            }
        ],
             "runtime_error": ""}}
"""
  yield (
      api.test('percommit_bookmaker') +
      api.properties(buildername='Housekeeper-PerCommit-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]')
  )

  yield (
      api.test('percommit_failed_validation') +
      api.properties(buildername='Housekeeper-PerCommit-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Validate docs match include/core/*.h', retcode=1)
  )

  yield (
      api.test('nightly_bookmaker') +
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
      api.test('nightly_failed_fiddles') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     fiddleout_test_data=fiddleout_with_errors_test_data,
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join('fiddleout.json'))
  )

  yield (
      api.test('nightly_failed_extract_fiddles') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Extract all fiddles out of md files', retcode=1)
  )

  yield (
      api.test('nightly_failed_fiddlecli') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Force fiddle to compile all examples', retcode=1)
  )

  yield (
      api.test('nightly_failed_upload') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.step_data('Generate and Upload Markdown files', retcode=1)
  )
