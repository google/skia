# Copyright 2019 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


from . import util


# TODO(dogben): Move this mapping to a machine-editable file.
IMAGES = {
    'gcc-debian10': (
        'gcr.io/skia-public/gcc-debian10@sha256:'
        '89a72df1e2fdea6f774a3fa4199bb9aaa4a0526a3ac1f233e604d689b694f95c'),
    'gcc-debian10-x86': (
        'gcr.io/skia-public/gcc-debian10-x86@sha256:'
        'b1ec55403ac66d9500d033d6ffd7663894d32335711fbbb0fb4c67dfce812203'),
}


def py_to_gn(val):
  """Convert val to a string that can be used as GN args."""
  if isinstance(val, bool):
    return 'true' if val else 'false'
  elif isinstance(val, basestring):
    # TODO(dogben): Handle quoting "$\
    return '"%s"' % val
  elif isinstance(val, (list, tuple)):
    return '[%s]' % (','.join(py_to_gn(x) for x in val))
  elif isinstance(val, dict):
    gn = ' '.join(
        '%s=%s' % (k, py_to_gn(v)) for (k, v) in sorted(val.iteritems()))
    return gn
  else:  # pragma: nocover
    raise Exception('Converting %s to gn is not implemented.' % type(val))


def compile_fn(api, checkout_root, out_dir):
  compiler = api.vars.builder_cfg.get('compiler', '')
  configuration = api.vars.builder_cfg.get('configuration', '')
  extra_tokens = api.vars.extra_tokens
  extra_tokens.remove('Docker')
  os = api.vars.builder_cfg.get('os', '')
  target_arch = api.vars.builder_cfg.get('target_arch', '')

  args = {
      'extra_cflags': [],
      'extra_ldflags': [],
      'target_cpu': target_arch,
      'werror': True
  }

  if configuration == 'Debug':
    args['extra_cflags'].append('-O1')
  else:
    args['is_debug'] = False

  if 'NoGPU' in extra_tokens:
    args['skia_enable_gpu'] = False
    extra_tokens.remove('NoGPU')
  if 'Shared' in extra_tokens:
    args['is_component_build'] = True
    extra_tokens.remove('Shared')

  image_name = None
  if os == 'Debian10' and compiler == 'GCC' and not extra_tokens:
    args['cc'] = 'gcc'
    args['cxx'] = 'g++'
    # Newer GCC includes tons and tons of debugging symbols. This seems to
    # negatively affect our bots (potentially only in combination with other
    # bugs in Swarming or recipe code). Use g1 to reduce it a bit.
    args['extra_cflags'].append('-g1')
    if target_arch == 'x86_64':
      image_name = 'gcc-debian10'
    elif target_arch == 'x86':
      image_name = 'gcc-debian10-x86'

  if not image_name:
    raise Exception('Not implemented: ' + api.vars.builder_name)

  image_hash = IMAGES[image_name]
  # We always perform an incremental compile, since out dir is cached across
  # compile tasks. However, we need to force a recompile when the toolchain
  # changes. The simplest way to do that is using a C define that changes
  # anytime the image changes.
  args['extra_cflags'].append('-DREBUILD_IF_CHANGED_docker_image=%s' % image_hash)

  script = api.build.resource('docker-compile.sh')
  api.docker.run('Run build script in Docker', image_hash,
                 checkout_root, out_dir, script, args=[py_to_gn(args)])

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, util.DEFAULT_BUILD_PRODUCTS)
