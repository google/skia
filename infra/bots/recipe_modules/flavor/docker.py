# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import default

"""Docker flavor, used for running inside a Docker container."""


class DockerFlavor(default.DefaultFlavor):
  def __init__(self, m):
    super(DockerFlavor, self).__init__(m)
    self.device_dirs = default.DeviceDirs(
        bin_dir        = self.m.docker.mount_src() + '/build',
        dm_dir         = self.m.docker.mount_out(),
        perf_data_dir  = self.m.docker.mount_out(),
        resource_dir   = self.m.docker.mount_src() + '/skia/resources',
        images_dir     = self.m.docker.mount_src() + '/skimage',
        lotties_dir    = self.m.docker.mount_src() + '/lottie-samples',
        skp_dir        = self.m.docker.mount_src() + '/skp',
        svg_dir        = self.m.docker.mount_src() + '/svg',
        mskp_dir       = self.m.docker.mount_src() + '/mskp',
        tmp_dir        = self.m.docker.mount_src() + '/tmp',
        texttraces_dir = self.m.docker.mount_src() + '/text_blob_traces')


  def _map_docker_path_to_host(self, docker_path):
    """Returns the host path that is mapped to the given Docker path.

    Raises ValueError if the Docker path is not mapped.
    """
    for (docker_dir, host_dir) in [
        (self.m.docker.mount_src(), self.m.path['start_dir']),
        (self.m.docker.mount_out(), self.m.vars.swarming_out_dir),
    ]:
      if docker_path == docker_dir:
        return host_dir
      if docker_path.startswith(docker_dir+'/'):
        return host_dir.join(*docker_path[len(docker_dir+'/'):].split('/'))
    else: # pragma: nocover
      raise ValueError('Path %s in Docker container is not mapped.' %
                       docker_path)

  def copy_directory_contents_to_device(self, host_dir, device_dir):
    """Asserts that no copy is needed."""
    expected_host = self._map_docker_path_to_host(device_dir)
    if str(expected_host) != str(host_dir):  # pragma: nocover
      raise ValueError('Expected %s to be mapped from %s, '
                       'but instead was mapped from %s' % (
                           device_dir, expected_host, host_dir))

  def copy_directory_contents_to_host(self, device_dir, host_dir):
    """Asserts that no copy is needed."""
    expected_host = self._map_docker_path_to_host(device_dir)
    if str(expected_host) != str(host_dir):  # pragma: nocover
      raise ValueError('Expected %s to be mapped from %s, '
                       'but instead was mapped from %s' % (
                           device_dir, expected_host, host_dir))

  def copy_file_to_device(self, host_path, device_path):
    self.copy_directory_contents_to_device(host_path, device_path)

  def create_clean_device_dir(self, path):
    """Cleans the corresponding host dir."""
    self.create_clean_host_dir(self._map_docker_path_to_host(path))

  def read_file_on_device(self, path, **kwargs):
    """Reads the specified file."""
    host_file = self._map_docker_path_to_host(path)
    return self.m.file.read_text('read %s' % host_file, host_file)

  def remove_file_on_device(self, path):
    """Removes the specified file."""
    host_file = self._map_docker_path_to_host(path)
    return self.m.file.remove('remove %s' % host_file, host_file)

  def step(self, name, cmd, **kwargs):
    extra_tokens = self.m.vars.extra_tokens
    extra_tokens.remove('Docker')
    os = self.m.vars.builder_cfg.get('os', '')
    model = self.m.vars.builder_cfg.get('model', '')
    cpu_or_gpu = self.m.vars.builder_cfg.get('cpu_or_gpu', '')

    image = None
    if (os == 'Debian10' and model == 'GCE' and cpu_or_gpu == 'CPU' and
        not extra_tokens):
      # TODO(dogben): Use an image without extra packages installed.
      image = (
          'gcr.io/skia-public/gcc-debian10@sha256:'
          '89a72df1e2fdea6f774a3fa4199bb9aaa4a0526a3ac1f233e604d689b694f95c')

    if not image:  # pragma: nocover
      raise Exception('Not implemented: ' + self.m.vars.builder_name)

    # TODO(dogben): Currently Linux-specific.
    symbolize_rel = ('recipe_bundle/skia/infra/bots/recipe_modules/flavor/'
                     'resources/symbolize_stack_trace.py')
    app = self.device_dirs.bin_dir.join(cmd[0])
    args = [self.m.docker.mount_src(), 'catchsegv', app] + cmd[1:]
    self.m.docker.run('symbolized %s in Docker' % name, image,
                      self.m.path['start_dir'], self.m.vars.swarming_out_dir,
                      symbolize_rel, args=args)
