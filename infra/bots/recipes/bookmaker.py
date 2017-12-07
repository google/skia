# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# Recipe which runs the Skia infra tests.

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
UPDATE_DOCS_GITCOOKIES_GS_PATH = ('gs://skia-buildbots/artifacts/server/.gitcookies_update-docs')


class DownloadGitCookies(object):
  """Class to download gitcookies from GS."""
  def __init__(self, gs_path, local_path, api):
    self._gs_path = gs_path
    self._local_path = local_path
    self._api = api

  def __enter__(self):
    cmd = ['gsutil', 'cp', self._gs_path, self._local_path]
    self._api.step('download gitcookies', cmd=cmd, infra_step=True)

  def __exit__(self, exc_type, _value, _traceback):
    if self._api.path.exists(self._local_path):
      self._api.file.remove('remove %s' % self._local_path, self._local_path)


def update_go_deps(api):
  env = api.context.env
  env.update(api.infra.go_env)
  with api.context(env=env):
    api.run.with_retry(
        api.step,
        'update go pkgs',
        5,  # Update attempts.
        cmd=[api.infra.go_exe, 'get', '-u', '-t',
             'go.skia.org/infra/fiddle/go/fiddlecli'])


def RunSteps(api):
  api.vars.setup()
  api.core.checkout_steps()
  api.infra.go_version()
  update_go_deps(api)  # inline this?

  # Run the infra tests.
  with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
    bookmaker_binary = api.path.join(api.vars.skia_out, api.vars.configuration, 'bookmaker')
    fiddlecli_binary = api.path.join(api.infra.gopath, 'bin', 'fiddlecli')
    fiddlecli_input = api.path.join(api.path['start_dir'], 'fiddle.json')
    fiddlecli_output = api.path.join(api.path['start_dir'], 'fiddleout.json')

    # TODO(rmistry): Test that this fails.
    # Step 1:
    #   Extract all examples into JSON from the docs dir.
    cmd = [bookmaker_binary,
           '-b', 'docs',  # Path to a *.bmh file or directory.
           '-e', fiddlecli_input,  # Fiddle cli input.
           ]
    api.run(api.step, 'Extract all fiddles into JSON', cmd=cmd)

    # raw_input("continue?")
    # just copy it over from tmp.
    import shutil
    # shutil.copyfile('/tmp/fiddle.json', '/mnt/pd0/s/w/ir/fiddle.json')
    # shutil.copyfile('/mnt/pd0/s/w/ir/skia/infra/bots/upload_md.py', '/b/work/skia/infra/bots/upload_md.py')
    # shutil.copyfile('/mnt/pd0/s/w/ir/skia/infra/bots/git_utils.py', '/b/work/skia/infra/bots/git_utils.py')

    # Step 2:
    #  Force fiddle.skia.org to compile and run all examples.
    cmd = [fiddlecli_binary,
           '--input', fiddlecli_input,
           '--output', fiddlecli_output,
           '--logtostderr',
           # '--force',
        ]
    api.run(api.step, 'Force fiddle to compile and run all examples', cmd=cmd)

    # Display the results of fiddlecli_output.


    # raw_input("continue?")
    # TODO(rmistry): Parse json and report any errors.

    # Go through the output JSON and check to see if there are any compile or runtime errors.
    if api.path.exists(fiddlecli_output):  # pragma: nocover
      with open(fiddlecli_output, 'r') as f:
        out = json.load(f)
      failing_fiddles = []
      for fiddle_name in out:
        props = out[fiddle_name]
        if props['compile_errors'] or props['runtime_error']:
          failing_fiddles.append(fiddle_name)
      if failing_fiddles:
        # create an eror message and fail the bot!
        failure_msg = 'The following fiddles failed:'
        for fiddle_name in failing_fiddles:
          failure_msg += '\n\n\n%s' % json.dumps(out[fiddle_name], indent=4)
        raise api.step.StepFailure(failure_msg)

      # Do a dump of fiddlecli_output. Will be useful for debugging.
      print 'Dump of %s:' % fiddlecli_output
      print json.dumps(out, indent=4)

    # Try a separate checkout below!
    # Step 3:
    #   Update markdown files with output of fiddlecli.
    # Upload the docs changes.
    update_docs_gitcookies = api.path['start_dir'].join(
        UPDATE_DOCS_GITCOOKIES_FILE)
    cmd = ['python',
           api.vars.skia_dir.join('infra', 'bots', 'upload_md.py'),
           '--bookmaker_binary', bookmaker_binary,
           '--fiddlecli_output', fiddlecli_output,
           '--gitcookies', str(update_docs_gitcookies)]
    with DownloadGitCookies(
        UPDATE_DOCS_GITCOOKIES_GS_PATH, update_docs_gitcookies, api):
      with api.context(cwd=api.vars.skia_dir, env=api.infra.go_env):
        api.run(api.step, 'Generate and Upload Markdown files', cmd=cmd)


def GenTests(api):
  yield (
      api.test('bookmaker') +
      api.properties(buildername='Housekeeper-Nightly-Bookmaker',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
      api.path.exists(api.path['start_dir'].join(UPDATE_DOCS_GITCOOKIES_FILE))
  )
  """
  yield (
    api.test('failed_one_update') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1)
  )

  yield (
    api.test('failed_all_updates') +
      api.properties(buildername='Housekeeper-PerCommit-InfraTests',
                     repository='https://skia.googlesource.com/skia.git',
                     revision='abc123',
                     path_config='kitchen',
                     swarm_out_dir='[SWARM_OUT_DIR]') +
    api.step_data('update go pkgs', retcode=1) +
    api.step_data('update go pkgs (attempt 2)', retcode=1) +
    api.step_data('update go pkgs (attempt 3)', retcode=1) +
    api.step_data('update go pkgs (attempt 4)', retcode=1) +
    api.step_data('update go pkgs (attempt 5)', retcode=1)
  )
  """
