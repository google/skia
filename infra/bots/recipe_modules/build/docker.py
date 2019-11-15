# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util


def py_to_gn(val):
  """Convert val to a string that can be used as GN args."""
  if isinstance(val, bool):
    return 'true' if val else 'false'
  if isinstance(val, basestring):
    # TODO(dogben): Handle quoting "$\
    return '"%s"' % val
  if isinstance(val, (list, tuple)):
    return '[%s]' % (','.join(py_to_gn(x) for x in val))
  if isinstance(val, dict):
    gn = ' '.join(
        '%s=%s' % (k, py_to_gn(v)) for (k, v) in sorted(val.iteritems()))
    return gn


def compile_fn(api, checkout_root, out_dir):
  compiler = api.vars.builder_cfg.get('compiler', '')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra_tokens = api.vars.extra_tokens
  extra_tokens.remove('Docker')
  os = api.vars.builder_cfg.get('os', '')
  target_arch = api.vars.builder_cfg.get('target_arch', '')

  args = {
      'extra_cflags': [],
      'target_cpu': target_arch,
      'werror': True
  }
  if configuration != 'Debug':
    args['is_debug'] = False

  if 'NoGPU' in extra_tokens:
    args['skia_enable_gpu'] = False
    extra_tokens.remove('NoGPU')
  if 'Shared' in extra_tokens:
    args['is_component_build'] = True
    extra_tokens.remove('Shared')
  if 'Valgrind' in extra_tokens:
    args['extra_cflags'].append('-DSK_CPU_LIMIT_SSE41')
    args['extra_cflags'].append('-DSKCMS_PORTABLE')
    extra_tokens.remove('Valgrind')

  image = None
  if os == 'Debian10':
    if compiler == 'GCC':
      if not extra_tokens:
        image = (
            'gcr.io/skia-public/gcc-debian10@sha256:'
            '89a72df1e2fdea6f774a3fa4199bb9aaa4a0526a3ac1f233e604d689b694f95c')
        args['cc'] = 'gcc'
        args['cxx'] = 'g++'
  if not image:
    raise Exception('Not implemented: ' + api.vars.builder_name)

  # We always perform an incremental compile, since out dir is cached across
  # compile tasks. However, we need to force a recompile when the toolchain
  # changes. The simplest way to do that is using a C define that changes
  # anytime the image changes.
  args['extra_cflags'].append('-DDUMMY_docker_image=%s' % image)

  # We want to make sure the directories exist and were created by chrome-bot.
  # (Note that the docker --mount option, unlike the --volume option, does not
  # create this dir as root if it doesn't exist.)
  api.file.ensure_directory('mkdirs out_dir', out_dir, mode=0777)

  # Run the compile script inside the docker container. It expects two mounts:
  # the start_dir at /SRC and the output directory at /OUT.
  src_mnt = 'type=bind,source=%s,target=/SRC' % checkout_root
  out_mnt = 'type=bind,source=%s,target=/OUT' % out_dir
  inner_script_path = ('/SRC/recipe_bundle/skia/infra/bots/recipe_modules'
                       '/build/resources/docker-compile.sh')
  cmd = ['docker', 'run', '--rm', '--mount', src_mnt, '--mount', out_mnt, image,
         inner_script_path, py_to_gn(args)]
  # Override DOCKER_CONFIG set by Kitchen.
  env = {'DOCKER_CONFIG': '/home/chrome-bot/.docker'}
  with api.env(env):
    api.run(
        api.step,
        'Run build script in Docker',
        cmd=cmd)

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
