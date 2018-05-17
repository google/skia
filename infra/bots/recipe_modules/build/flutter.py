# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


def compile_fn(api, build_dir, out_dir):
  configuration = api.vars.builder_cfg.get('configuration').lower()
  extra_tokens = api.vars.extra_tokens

  with api.context(cwd=build_dir):
    # Setup GN args.
    gn_args = [
        '--runtime-mode=%s' % configuration,
    ]
    if 'Android' in extra_tokens:
      gn_args.append('--android')

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


def copy_extra_build_products(api, skia_dir, src, dst):
  pass
