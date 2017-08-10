# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


# TODO(borenet): This module was copied from build.git and heavily modified to
# remove dependencies on other modules in build.git.  It belongs in a different
# repo. Remove this once it has been moved.


from recipe_engine import recipe_api


class SwarmingClientApi(recipe_api.RecipeApi):
  """Code that both isolate and swarming recipe modules depend on.

  Both swarming and isolate scripts live in a single repository called
  'swarming client'. This module include common functionality like finding
  existing swarming client checkout, fetching a new one, getting version of
  a swarming script, etc.
  """

  def __init__(self, **kwargs):
    super(SwarmingClientApi, self).__init__(**kwargs)
    self._client_path = None
    self._script_version = {}

  def checkout(self, revision=None, curl_trace_file=None, can_fail_build=True):
    """Returns a step to checkout swarming client into a separate directory.

    Ordinarily swarming client is checked out via Chromium DEPS into
    src/tools/swarming_client. This step configures recipe module to use
    a separate checkout.

    If |revision| is None, this requires the build property
    'parent_got_swarming_client_revision' to be present, and raises an exception
    otherwise. Fail-fast behavior is used because if machines silently fell back
    to checking out the entire workspace, that would cause dramatic increases
    in cycle time if a misconfiguration were made and it were no longer possible
    for the bot to check out swarming_client separately.
    """
    # If the following line throws an exception, it either means the
    # bot is misconfigured, or, if you're testing locally, that you
    # need to pass in some recent legal revision for this property.
    if revision is None:
      revision = self.m.properties['parent_got_swarming_client_revision']
    self._client_path = self.m.path['start_dir'].join('swarming.client')
    self.m.git.checkout(
        url='https://chromium.googlesource.com/external/swarming.client.git',
        ref=revision,
        dir_path=self._client_path,
        step_suffix='swarming_client',
        curl_trace_file=curl_trace_file,
        can_fail_build=can_fail_build)

  @property
  def path(self):
    """Returns path to a swarming client checkout.

    It's subdirectory of Chromium src/ checkout or a separate directory if
    'checkout_swarming_client' step was used.
    """
    if self._client_path:
      return self._client_path
    # Default is swarming client path in chromium src/ checkout.
    # TODO(vadimsh): This line assumes the recipe is working with
    # Chromium checkout.
    return self.m.path['checkout'].join('tools', 'swarming_client')

  def query_script_version(self, script, step_test_data=None):
    """Yields a step to query a swarming script for its version.

    Version tuple is later accessible via 'get_script_version' method. If
    |step_test_data| is given, it is a tuple with version to use in expectation
    tests by default.

    Does nothing if script's version is already known.
    """
    # Convert |step_test_data| from tuple of ints back to a version string.
    if step_test_data:
      assert isinstance(step_test_data, tuple)
      assert all(isinstance(x, int) for x in step_test_data)
      as_text = '.'.join(map(str, step_test_data))
      step_test_data_cb = lambda: self.m.raw_io.test_api.stream_output(as_text)
    else:
      step_test_data_cb = None

    if script not in self._script_version:
      try:
        self.m.python(
          name='%s --version' % script,
          script=self.path.join(script),
          args=['--version'],
          stdout=self.m.raw_io.output_text(),
          step_test_data=step_test_data_cb)
      finally:
        step_result = self.m.step.active_result
        version = step_result.stdout.strip()
        step_result.presentation.step_text = version
        self._script_version[script] = tuple(map(int, version.split('.')))

      return step_result

  def get_script_version(self, script):
    """Returns a version of some swarming script as a tuple (Major, Minor, Rev).

    It should have been queried by 'query_script_version' step before. Raises
    AssertionError if it wasn't.
    """
    assert script in self._script_version, script
    return self._script_version[script]

  def ensure_script_version(self, script, min_version, step_test_data=None):
    """Yields steps to ensure a script version is not older than |min_version|.

    Will abort recipe execution if it is.
    """
    step_result = self.query_script_version(
        script, step_test_data=step_test_data or min_version)
    version = self.get_script_version(script)
    if version < min_version:
      expecting = '.'.join(map(str, min_version))
      got = '.'.join(map(str, version))
      abort_reason = 'Expecting at least v%s, got v%s' % (expecting, got)

      # TODO(martiniss) remove once recipe 1.5 migration done
      step_result = self.m.python.inline(
          '%s is too old' % script,
          'import sys; sys.exit(1)',
          add_python_log=False)
      # TODO(martiniss) get rid of this bare string.
      step_result.presentation.status = self.m.step.FAILURE
      step_result.presentation.step_text = abort_reason

      raise self.m.step.StepFailure(abort_reason)
