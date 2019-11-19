# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import default

"""Docker flavor, used for running inside a Docker container."""


class DockerFlavor(default.DefaultFlavor):
  def __init__(self, m):
    super(DockerFlavor, self).__init__(m)

  def _map_host_path_to_docker(self, path):
    """Returns the path in the Docker container mapped to the given path.

    Returns None if the path is not mapped into the Docker container.
    """
    abs_path = self.m.path.abspath(path)
    for (docker_dir, host_dir) in [
        (self.m.docker.mount_out(), self.m.vars.swarming_out_dir),
        (self.m.docker.mount_src(), self.m.path['start_dir']),
    ]:
      abs_host = self.m.path.abspath(host_dir)
      if abs_path.startswith(abs_host):
        return self.device_path_join(docker_dir, *abs_path[len(abs_host):].split('/'))
    else:
      return None

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
    app = self._map_host_path_to_docker(self.device_dirs.bin_dir.join(cmd[0]))
    args = [self.m.docker.mount_src(), 'catchsegv', app] + [
        self._map_host_path_to_docker(x) or x for x in cmd[1:]]
    self.m.docker.run('symbolized %s in Docker' % name, image,
                      self.m.path['start_dir'], self.m.vars.swarming_out_dir,
                      symbolize_rel, args=args)
