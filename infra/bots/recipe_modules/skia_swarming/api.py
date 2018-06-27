# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from recipe_engine import recipe_api
import shlex


DEFAULT_TASK_EXPIRATION = 20*60*60
DEFAULT_TASK_TIMEOUT = 4*60*60
DEFAULT_IO_TIMEOUT = 40*60

MILO_LOG_LINK = 'https://luci-milo.appspot.com/swarming/task/%s'


class SkiaSwarmingApi(recipe_api.RecipeApi):
  """Provides steps to run Skia tasks on swarming bots."""

  @property
  def swarming_temp_dir(self):
    """Path where artifacts like isolate file and json output will be stored."""
    return self.m.path['start_dir'].join('swarming_temp_dir')

  @property
  def tasks_output_dir(self):
    """Directory where the outputs of the swarming tasks will be stored."""
    return self.swarming_temp_dir.join('outputs')

  def isolated_file_path(self, task_name):
    """Get the path to the given task's .isolated file."""
    return self.swarming_temp_dir.join('skia-task-%s.isolated' % task_name)

  def setup(self, luci_go_dir, swarming_rev=None):
    """Performs setup steps for swarming."""
    self.m.swarming_client.checkout(revision=swarming_rev)
    self.m.swarming.check_client_version(step_test_data=(0, 8, 6))
    self.setup_go_isolate(luci_go_dir)
    self.m.swarming.add_default_tag('allow_milo:1')

  # TODO(rmistry): Remove once the Go binaries are moved to recipes or buildbot.
  def setup_go_isolate(self, luci_go_dir):
    """Generates and puts in place the isolate Go binary."""
    depot_tools_path = self.m.depot_tools.package_repo_resource()
    env = {'PATH': self.m.path.pathsep.join([
                       str(depot_tools_path), '%(PATH)s'])}
    with self.m.context(env=env):
      self.m.step('download luci-go linux',
                  ['download_from_google_storage', '--no_resume',
                   '--platform=linux*', '--no_auth',
                   '--bucket', 'chromium-luci',
                   '-d', luci_go_dir.join('linux64')])
      self.m.step('download luci-go mac',
                  ['download_from_google_storage', '--no_resume',
                   '--platform=darwin', '--no_auth',
                   '--bucket', 'chromium-luci',
                   '-d', luci_go_dir.join('mac64')])
      self.m.step('download luci-go win',
                  ['download_from_google_storage', '--no_resume',
                   '--platform=win32', '--no_auth', '--bucket',
                   'chromium-luci',
                   '-d', luci_go_dir.join('win64')])
    # Copy binaries to the expected location.
    dest = self.m.path['start_dir'].join('luci-go')
    self.m.run.rmtree(dest)
    self.m.file.copytree('Copy Go binary',
                         source=luci_go_dir,
                         dest=dest)

  def create_isolated_gen_json(self, isolate_path, base_dir, os_type,
                               task_name, extra_variables, blacklist=None):
    """Creates an isolated.gen.json file (used by the isolate recipe module).

    Args:
      isolate_path: path obj. Path to the isolate file.
      base_dir: path obj. Dir that is the base of all paths in the isolate file.
      os_type: str. The OS type to use when archiving the isolate file.
          Eg: linux.
      task_name: str. The isolated.gen.json file will be suffixed by this str.
      extra_variables: dict of str to str. The extra vars to pass to isolate.
          Eg: {'SLAVE_NUM': '1', 'MASTER': 'ChromiumPerfFYI'}
      blacklist: list of regular expressions indicating which files/directories
          not to archive.
    """
    self.m.file.ensure_directory(
        'makedirs swarming tmp dir',
        self.swarming_temp_dir)
    isolated_path = self.isolated_file_path(task_name)
    isolate_args = [
      '--isolate', isolate_path,
      '--isolated', isolated_path,
      '--config-variable', 'OS', os_type,
    ]
    if blacklist:
      for b in blacklist:
        isolate_args.extend(['--blacklist', b])
    for k, v in extra_variables.iteritems():
      isolate_args.extend(['--extra-variable', k, v])
    isolated_gen_dict = {
      'version': 1,
      'dir': base_dir,
      'args': isolate_args,
    }
    isolated_gen_json = self.swarming_temp_dir.join(
        '%s.isolated.gen.json' % task_name)
    self.m.file.write_text(
        'Write %s.isolated.gen.json' % task_name,
        isolated_gen_json,
        self.m.json.dumps(isolated_gen_dict, indent=4),
    )

  def batcharchive(self, targets):
    """Calls batcharchive on the skia.isolated.gen.json file.

    Args:
      targets: list of str. The suffixes of the isolated.gen.json files to
               archive.

    Returns:
      list of tuples containing (task_name, swarming_hash).
    """
    return self.m.isolate.isolate_tests(
        verbose=True,  # To avoid no output timeouts.
        build_dir=self.swarming_temp_dir,
        targets=targets).presentation.properties['swarm_hashes'].items()

  def trigger_swarming_tasks(
      self, swarm_hashes, dimensions, idempotent=False, store_output=True,
      extra_args=None, expiration=None, hard_timeout=None, io_timeout=None,
      cipd_packages=None):
    """Triggers swarming tasks using swarm hashes.

    Args:
      swarm_hashes: list of str. List of swarm hashes from the isolate server.
      dimensions: dict of str to str. The dimensions to run the task on.
                  Eg: {'os': 'Ubuntu', 'gpu': '10de', 'pool': 'Skia'}
      idempotent: bool. Whether or not to de-duplicate tasks.
      store_output: bool. Whether task output should be stored.
      extra_args: list of str. Extra arguments to pass to the task.
      expiration: int. Task will expire if not picked up within this time.
                  DEFAULT_TASK_EXPIRATION is used if this argument is None.
      hard_timeout: int. Task will timeout if not completed within this time.
                    DEFAULT_TASK_TIMEOUT is used if this argument is None.
      io_timeout: int. Task will timeout if there is no output within this time.
                  DEFAULT_IO_TIMEOUT is used if this argument is None.
      cipd_packages: CIPD packages which these tasks depend on.

    Returns:
      List of swarming.SwarmingTask instances.
    """
    swarming_tasks = []
    for task_name, swarm_hash in swarm_hashes:
      swarming_task = self.m.swarming.task(
          title=task_name,
          cipd_packages=cipd_packages,
          isolated_hash=swarm_hash)
      if store_output:
        swarming_task.task_output_dir = self.tasks_output_dir.join(task_name)
      swarming_task.dimensions = dimensions
      swarming_task.idempotent = idempotent
      swarming_task.priority = 90
      swarming_task.expiration = (
          expiration if expiration else DEFAULT_TASK_EXPIRATION)
      swarming_task.hard_timeout = (
          hard_timeout if hard_timeout else DEFAULT_TASK_TIMEOUT)
      swarming_task.io_timeout = (
          io_timeout if io_timeout else DEFAULT_IO_TIMEOUT)
      if extra_args:
        swarming_task.extra_args = extra_args
      revision = self.m.properties.get('revision')
      if revision:
        swarming_task.tags.add('revision:%s' % revision)
      swarming_tasks.append(swarming_task)
    step_results = self.m.swarming.trigger(swarming_tasks)
    for step_result in step_results:
      self._add_log_links(step_result, step_result.json.output)
    return swarming_tasks

  def collect_swarming_task(self, swarming_task):
    """Collects the specified swarming task.

    Args:
      swarming_task: An instance of swarming.SwarmingTask.
    """
    try:
      rv = self.m.swarming.collect_task(swarming_task)
    except self.m.step.StepFailure as e:  # pragma: no cover
      step_result = self.m.step.active_result
      # Change step result to Infra failure if the swarming task failed due to
      # expiration, time outs, bot crashes or task cancelations.
      # Infra failures have step.EXCEPTION.
      states_infra_failure = (
          self.m.swarming.State.EXPIRED, self.m.swarming.State.TIMED_OUT,
          self.m.swarming.State.BOT_DIED, self.m.swarming.State.CANCELED)
      summary = step_result.swarming.summary
      if summary['shards'][0]['state'] in states_infra_failure:
        step_result.presentation.status = self.m.step.EXCEPTION
        raise self.m.step.InfraFailure(e.name, step_result)
      raise
    finally:
      step_result = self.m.step.active_result
      # Add log link.
      self._add_log_links(step_result, step_result.swarming.summary)
    return rv

  def _add_log_links(self, step_result, summary):
    """Add Milo log links to all shards in the step."""
    ids = []
    shards = summary.get('shards')
    if shards:
      for shard in shards:
        ids.append(shard['id'])
    else:
      for _, task in summary.get('tasks', {}).iteritems():
        ids.append(task['task_id'])
    for idx, task_id in enumerate(ids):
      link = MILO_LOG_LINK % task_id
      k = 'view steps on Milo'
      if len(ids) > 1:  # pragma: nocover
        k += ' (shard index %d, %d total)' % (idx, len(ids))
      step_result.presentation.links[k] = link

