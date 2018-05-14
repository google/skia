# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


def compile_fn(api, _):
  flutter_dir = api.vars.checkout_root.join('src')
  configuration = api.vars.builder_cfg.get('configuration').lower()
  extra_tokens = api.vars.extra_tokens
  out_dir = configuration

  with api.context(cwd=flutter_dir):
    # Runhook to generate the gn binary in buildtools.
    api.gclient.runhooks()

    # Setup GN args.
    gn_args = [
        '--runtime-mode=%s' % configuration,
    ]
    if 'Android' in extra_tokens:
      gn_args.append('--android')
      out_dir = 'android_' + out_dir

    # Delete out_dir so that we start from a clean slate. See skbug/6310.
    api.run.rmtree(flutter_dir.join('out', out_dir))

    # Run GN.
    api.run(
        api.step,
        'gn_gen',
        cmd=['flutter/tools/gn'] + gn_args)

    # Build Flutter.
    api.run(
        api.step,
        'build_flutter',
        cmd=['ninja', '-C', 'out/' + out_dir, '-j100'])


def copy_extra_build_products(api, src, dst):
  pass
