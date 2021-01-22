# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import default

"""Docker flavor, used for running inside a Docker container."""


# TODO(dogben): Move this mapping to a machine-editable file.
# TODO(dogben): Use images without extra packages installed.
IMAGES = {
    'gcc-debian10': (
        'gcr.io/skia-public/gcc-debian10@sha256:'
        '89a72df1e2fdea6f774a3fa4199bb9aaa4a0526a3ac1f233e604d689b694f95c'),
    'gcc-debian10-x86': (
        'gcr.io/skia-public/gcc-debian10-x86@sha256:'
        'b1ec55403ac66d9500d033d6ffd7663894d32335711fbbb0fb4c67dfce812203'),
}


class DockerFlavor(default.DefaultFlavor):
  def __init__(self, m, app_name):
    super(DockerFlavor, self).__init__(m, app_name)

  def _map_host_path_to_docker(self, path):
    """Returns the path in the Docker container mapped to the given path.

    Returns None if the path is not mapped into the Docker container.
    """
    path = str(path)
    for (docker_dir, host_dir) in [
        (self.m.docker.mount_out(), str(self.m.vars.swarming_out_dir)),
        (self.m.docker.mount_src(), str(self.m.path['start_dir'])),
    ]:
      if path.startswith(host_dir):
        return docker_dir + path[len(host_dir):]
    return None

  def step(self, name, cmd, **unused_kwargs):
    extra_tokens = [t for t in self.m.vars.extra_tokens if t != 'Docker']
    os = self.m.vars.builder_cfg.get('os', '')
    model = self.m.vars.builder_cfg.get('model', '')
    cpu_or_gpu = self.m.vars.builder_cfg.get('cpu_or_gpu', '')
    arch = self.m.vars.builder_cfg.get('arch', '')

    image_name = None
    if (os == 'Debian10' and model == 'GCE' and cpu_or_gpu == 'CPU' and
        not extra_tokens):
      if arch == 'x86_64':
        image_name = 'gcc-debian10'
      elif arch == 'x86':
        image_name = 'gcc-debian10-x86'

    if not image_name:  # pragma: nocover
      raise Exception('Not implemented: ' + self.m.vars.builder_name)

    image_hash = IMAGES[image_name]

    # TODO(dogben): Currently Linux-specific.
    app = self._map_host_path_to_docker(self.device_dirs.bin_dir.join(cmd[0]))
    args = [self.m.docker.mount_src(), 'catchsegv', app] + [
        self._map_host_path_to_docker(x) or x for x in cmd[1:]]
    self.m.docker.run('symbolized %s in Docker' % name, image_hash,
                      self.m.path['start_dir'], self.m.vars.swarming_out_dir,
                      self.module.resource('symbolize_stack_trace.py'),
                      args=args)
