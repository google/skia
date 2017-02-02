# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

import default_flavor


"""PDFium flavor utils, used for building PDFium with Skia."""


class PDFiumFlavorUtils(default_flavor.DefaultFlavorUtils):

  def compile(self, target, **kwargs):
    """Build PDFium with Skia."""
    pdfium_dir = self.m.vars.checkout_root.join('pdfium')

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

    # Setup gn args.
    gn_args = [
        'pdf_is_standalone=true',
        'clang_use_chrome_plugins=false',
        'is_component_build=false',
        'is_debug=false',
    ]
    if 'SkiaPaths' in self.m.vars.builder_name:
      gn_args.append('pdf_use_skia_paths=true')
    else:
      gn_args.append('pdf_use_skia=true')


    env = kwargs.pop('env', {})
    env['CHROMIUM_BUILDTOOLS_PATH'] = str(pdfium_dir.join('buildtools'))
    self.m.run(
        self.m.step,
        'gn_gen',
        cmd=['gn', 'gen', 'out/skia', '--args=%s' % ' '.join(gn_args)],
        cwd=pdfium_dir,
        env=env)

    # Build PDFium.
    self.m.run(
        self.m.step,
        'build_pdfium',
        cmd=['ninja', '-C', 'out/skia', '-j100'],
        cwd=pdfium_dir,
        env=env,
        **kwargs)
