# Copyright 2018 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


def compile(api, target, _):
    """Build PDFium with Skia."""
    pdfium_dir = api.vars.checkout_root.join('pdfium')

    # Runhook to generate the gn binary in buildtools.
    with api.context(cwd=pdfium_dir, env=api.vars.gclient_env):
      api.gclient.runhooks()

      # Install the sysroot.
      api.run(
          api.step,
          'sysroot',
          cmd=['python', 'build/linux/sysroot_scripts/install-sysroot.py',
               '--arch=amd64'])

      # Setup gn args.
      gn_args = [
          'pdf_is_standalone=true',
          'clang_use_chrome_plugins=false',
          'is_component_build=false',
          'is_debug=false',
      ]
      if 'SkiaPaths' in api.vars.builder_name:
        gn_args.append('pdf_use_skia_paths=true')
      else:
        gn_args.append('pdf_use_skia=true')


      env = api.context.env
      env['CHROMIUM_BUILDTOOLS_PATH'] = str(pdfium_dir.join('buildtools'))
      with api.context(env=env):
        api.run(
            api.step,
            'gn_gen',
            cmd=['gn', 'gen', 'out/skia', '--args=%s' % ' '.join(gn_args)])

        # Build PDFium.
        api.run(
            api.step,
            'build_pdfium',
            cmd=['ninja', '-C', 'out/skia', '-j100'])


def copy_extra_build_products(api, src, dst):
  pass
