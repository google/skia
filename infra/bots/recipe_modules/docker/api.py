# Copyright 2019 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


import posixpath
from recipe_engine import recipe_api


MOUNT_SRC = '/SRC'
MOUNT_OUT = '/OUT'


class DockerApi(recipe_api.RecipeApi):
  def _chmod(self, filepath, mode, recursive=False):
    cmd = ['chmod']
    if recursive:
      cmd.append('-R')
    cmd.extend([mode, filepath])
    name = ' '.join([str(elem) for elem in cmd])
    self.m.step(name, cmd=cmd, infra_step=True)

  def mount_src(self):
    return MOUNT_SRC

  def mount_out(self):
    return MOUNT_OUT

  # Unless match_directory_structure ==True, src_dir must be
  # self.m.path['start_dir'] for the script to be located correctly.
  def run(self, name, docker_image, src_dir, out_dir, script, args=None, docker_args=None, copies=None, recursive_read=None, attempts=1, match_directory_structure=False):
    # Setup. Docker runs as a different user, so we need to give it access to
    # read, write, and execute certain files.
    with self.m.step.nest('Docker setup'):
      step_stdout = self.m.python.inline(
          name='Get uid and gid',
          program='''import os
print '%d:%d' % (os.getuid(), os.getgid())
''',
          stdout=self.m.raw_io.output(),
          step_test_data=(
              lambda: self.m.raw_io.test_api.stream_output('13:17'))).stdout
      uid_gid_pair = step_stdout.rstrip() if step_stdout else ''
      # Make sure out_dir exists, otherwise mounting will fail.
      # (Note that the docker --mount option, unlike the --volume option, does
      # not create this dir as root if it doesn't exist.)
      self.m.file.ensure_directory('mkdirs out_dir', out_dir, mode=0o777)
      # ensure_directory won't change the permissions if the dir already exists,
      # so we need to do that explicitly.
      self._chmod(out_dir, '777')

      # chmod the src_dir, but not recursively; Swarming writes some files which
      # we can't access, so "chmod -R" will fail if this is the root workdir.
      self._chmod(src_dir, '755')

      # Need to make the script executable, or Docker can't run it.
      self._chmod(script, '0755')

      # Copy any requested files.
      if copies:
        for src, dest in copies.iteritems():
          dirname = self.m.path.dirname(dest)
          self.m.file.ensure_directory(
              'mkdirs %s' % dirname, dirname, mode=0o777)
          self.m.file.copy('cp %s %s' % (src, dest), src, dest)
          self._chmod(dest, '644')

      # Recursive chmod any requested directories.
      if recursive_read:
        for elem in recursive_read:
          self._chmod(elem, 'a+r', recursive=True)

    # Run.
    cmd = [
      'docker', 'run', '--shm-size=2gb', '--rm', '--user', uid_gid_pair,
      '--mount', 'type=bind,source=%s,target=%s' %
                 (src_dir, src_dir if match_directory_structure else MOUNT_SRC),
      '--mount', 'type=bind,source=%s,target=%s' %
                 (out_dir, out_dir if match_directory_structure else MOUNT_OUT),
    ]
    if docker_args:
      cmd.extend(docker_args)
    if not match_directory_structure:
      # This only works when src_dir == self.m.path['start_dir'] but that's our
      # only use case for now.
      script = MOUNT_SRC + '/' + posixpath.relpath(str(script), str(self.m.path['start_dir']))
    cmd.extend([docker_image, script])
    if args:
      cmd.extend(args)

    env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
    with self.m.env(env):
      self.m.run.with_retry(self.m.step, name, attempts, cmd=cmd)
