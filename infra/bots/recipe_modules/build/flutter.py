# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from . import util

IMAGES = {
    # Used to build Flutter_Android in Debian9, since the underlying build
    # scripts require jdk8.
    'Debian9': (
        'gcr.io/skia-public/debian9@sha256:'
        '5541f5674eee0a1558f1335c711f674f85f96b9ae8fb6c289f42dd81479a768e'),
}

def compile_fn(api, checkout_root, out_dir):
  flutter_dir   = checkout_root.join('src')
  configuration = api.vars.builder_cfg.get('configuration').lower()
  os_name = api.vars.builder_cfg.get('os', '')
  extra_tokens = api.vars.extra_tokens
  builder_name = api.vars.builder_name

  # Setup GN args.
  gn_args = ['--runtime-mode=%s' % configuration,]
  if 'Android' in extra_tokens:
    gn_args.append('--android')

  if os_name == 'Debian9' and 'Docker' in builder_name:
    script = api.build.resource('docker-flutter-compile.sh')
    image_hash = IMAGES[os_name]
    api.docker.run('Run build script in Docker', image_hash,
                   api.path['start_dir'], out_dir, script,
                   [api.path['start_dir'], flutter_dir, out_dir] + gn_args,
                   match_directory_structure=True)
    return

  with api.context(cwd=flutter_dir):
    # Delete out_dir so that we start from a clean slate. See skbug/6310.
    api.run.rmtree(out_dir)

    # Run GN.
    api.run(
        api.step,
        'gn_gen',
        cmd=['flutter/tools/gn'] + gn_args)

    # Build Flutter.
    api.run(
        api.step,
        'build_flutter',
        cmd=['ninja', '-C', out_dir, '-j100'])


FLUTTER_BUILD_PRODUCTS_LIST = [
  '*.so',
  'lib/*.so',
]

def copy_build_products(api, src, dst):
  util.copy_listed_files(api, src, dst, FLUTTER_BUILD_PRODUCTS_LIST)
  stripped_src = src.join('lib.stripped', 'libflutter.so')
  stripped_dst = dst.join('libflutter_stripped.so')
  api.python.inline(
      name='copy stripped library',
      program='''
import os
import shutil
import sys

src = sys.argv[1]
dst = sys.argv[2]

if not os.path.isdir(os.path.dirname(dst)):
  os.makedirs(os.path.dirname(dst))

shutil.copyfile(src, dst)
''',
      args=[stripped_src, stripped_dst],
      infra_step=True)
