# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

import default_flavor


"""Flutter flavor utils, used for building Flutter with Skia."""


class FlutterFlavorUtils(default_flavor.DefaultFlavorUtils):

  def compile(self, target):
    """Build Flutter with Skia."""

    flutter_dir = self.m.vars.checkout_root.join('src')
    configuration = self.m.vars.builder_cfg.get('configuration').lower()
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')
    out_dir = configuration

    with self.m.context(cwd=flutter_dir):
      # Runhook to generate the gn binary in buildtools.
      self.m.run(
          self.m.step,
          'runhook',
          cmd=['gclient', 'runhooks'])

      # Setup GN args.
      gn_args = [
          '--runtime-mode=%s' % configuration,
      ]
      if 'Android' in extra_config:
        gn_args.append('--android')
        out_dir = 'android_' + out_dir

      # Delete out_dir so that we start from a clean slate. See skbug/6310.
      self.m.run.rmtree(flutter_dir.join('out', out_dir))

      # Run GN.
      self.m.run(
          self.m.step,
          'gn_gen',
          cmd=['flutter/tools/gn'] + gn_args)

      # Build Flutter.
      self.m.run(
          self.m.step,
          'build_flutter',
          cmd=['ninja', '-C', 'out/' + out_dir, '-j100'])
