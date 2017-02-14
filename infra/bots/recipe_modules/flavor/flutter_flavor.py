# Copyright 2017 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

import default_flavor


"""Flutter flavor utils, used for building Flutter with Skia."""


class PDFiumFlavorUtils(default_flavor.DefaultFlavorUtils):

  def compile(self, target, **kwargs):
    """Build Flutter with Skia."""
    flutter_dir = self.m.vars.checkout_root.join('engine', 'src')

    # Runhook to generate the gn binary in buildtools.
    self.m.run(
        self.m.step,
        'runhook',
        cmd=['gclient', 'runhook', 'gn_linux64'],
        cwd=pdfium_dir,
        **kwargs)

    # Install the sysroot.
    self.m.run(
        self.m.step,
        'sysroot',
        cmd=['python', 'build/linux/sysroot_scripts/install-sysroot.py',
             '--arch=amd64'],
        cwd=pdfium_dir)

    configuration = self.m.vars.builder_cfg.get('configuration')
    extra_config = self.m.vars.builder_cfg.get('extra_config', '')

    # Setup gn args.
    gn_args = [
        'runtime-mode=%s' % configuration,
    ]
    if 'Android' in extra_config:
      gn_args.append('android')


    env = kwargs.pop('env', {})
    env['CHROMIUM_BUILDTOOLS_PATH'] = str(pdfium_dir.join('buildtools'))
    self.m.run(
        self.m.step,
        'gn_gen',
        cmd=['flutter/tools/gn', 'gen', 'out/skia', '--args=%s' % ' '.join(gn_args)],
        cwd=pdfium_dir,
        env=env)

    # Build Flutter.
    self.m.run(
        self.m.step,
        'build_flutter',
        cmd=['ninja', '-C', 'out/android_release', '-j100'],  # FIX THIS!
        cwd=flutter_dir,
        env=env,
        **kwargs)
