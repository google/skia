# Copyright 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import re

import default_flavor


"""PDFium flavor utils, used for building PDFium with Skia."""


class PDFiumFlavorUtils(default_flavor.DefaultFlavorUtils):

  def compile(self, target):
    """Build PDFium with Skia."""
    pdfium_dir = self._skia_api.checkout_root.join('pdfium')

    # Runhook to generate the gn binary in buildtools.
    self._skia_api.run(
        self._skia_api.m.step,
        'runhook',
        cmd=['gclient', 'runhook', 'gn_linux64'],
        cwd=pdfium_dir)

    # Setup gn args.
    gn_args = ['pdf_use_skia=true', 'pdf_is_standalone=true',
               'clang_use_chrome_plugins=false']
    self._skia_api.run(
        self._skia_api.m.step,
        'gn_gen',
        cmd=['gn', 'gen', 'out/skia', '--args=%s' % ' '.join(gn_args)],
        cwd=pdfium_dir,
        env={'CHROMIUM_BUILDTOOLS_PATH': str(pdfium_dir.join('buildtools'))})

    # Modify DEPS file to contain the current Skia revision.
    skia_revision = self._skia_api.got_revision
    deps_file = pdfium_dir.join('DEPS')
    test_data = "'skia_revision': 'abc'"

    original_contents = self._skia_api.m.file.read(
        'read PDFium DEPS', deps_file, test_data=test_data, infra_step=True)

    deps_skia_regexp = re.compile(
        r'(?<=["\']skia_revision["\']: ["\'])([a-fA-F0-9]+)(?=["\'])',
        re.MULTILINE)
    patched_contents = re.sub(deps_skia_regexp, str(skia_revision),
                              original_contents)
    self._skia_api.m.file.write('write PDFium DEPs', deps_file,
                                patched_contents, infra_step=True)

    # gclient sync after updating DEPS.
    self._skia_api.run(
        self._skia_api.m.step,
        'sync_pdfium',
        cmd=['gclient', 'sync'],
        cwd=pdfium_dir)

    # Build PDFium.
    self._skia_api.run(
        self._skia_api.m.step,
        'build_pdfium',
        cmd=['ninja', '-C', 'out/skia', '-j100'],
        cwd=pdfium_dir)
