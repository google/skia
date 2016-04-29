# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'targets': [
    {
      'target_name': 'skgputest',
      'product_name': 'skia_skgputest',
      'type': 'static_library',
      'standalone_static_library': 1,
      'include_dirs': [
        '../include/core',
        '../include/config',
        '../include/gpu',
        '../include/private',
        '../include/utils',
        '../src/core',
        '../src/gpu',
        '../src/utils',
        '../tools/gpu',
      ],
      'all_dependent_settings': {
        'include_dirs': [
          '../tools/gpu',
        ],
      },
      'dependencies': [
        'skia_lib.gyp:skia_lib',
      ],
      'sources': [
        '<!@(python find.py ../tools/gpu "*")'
      ],
      'conditions': [
        [ 'skia_mesa', { 'dependencies': [ 'osmesa' ] } ],
        [ 'skia_angle', {
          'dependencies': [
            'angle.gyp:*',
          ],
          'export_dependent_settings': [
            'angle.gyp:*',
          ],
        }],
        [ 'skia_os == "linux" and skia_egl == 1', {
          'link_settings': {
            'libraries': [
              '-lEGL',
              '-lGLESv2',
            ],
          },
        }],
        [ 'skia_os == "linux" and skia_egl == 0', {
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lGLU',
              '-lX11',
            ],
          },
        }],
        [ 'skia_os == "android"', {
          'defines': [
            'GR_GL_USE_NEW_SHADER_SOURCE_SIGNATURE=1',
          ],
          'link_settings': {
            'libraries': [
              '-lGLESv2',
              '-lEGL',
            ],
          },
        }],
        ['skia_os in ["linux", "win", "mac", "android", "ios"]', {
          'sources/': [ ['exclude', '_none\.(h|cpp)$'],],
        }],
        ['skia_os != "win"', {
          'sources/': [ ['exclude', '_win\.(h|cpp)$'],],
        }],
        ['skia_os != "mac"', {
          'sources/': [ ['exclude', '_mac\.(h|cpp|m|mm)$'],],
        }],
        ['skia_os != "linux"', {
          'sources/': [ ['exclude', '_glx\.(h|cpp)$'],],
        }],
        ['skia_os != "ios"', {
          'sources/': [ ['exclude', '_iOS\.(h|cpp|m|mm)$'],],
        }],
        ['skia_os != "android"', {
          'sources/': [ ['exclude', '_android\.(h|cpp)$'],],
        }],
        ['skia_egl == 0', {
          'sources/': [ ['exclude', '_egl\.(h|cpp)$'],],
        }],
        [ 'skia_mesa == 0', {
          'sources/': [
            ['exclude', '_mesa\.(h|cpp)$'],
          ],
        }],
        [ 'skia_angle == 0', {
          'sources/': [
            ['exclude', '_angle\.(h|cpp)$'],
          ],
        }],
        [ 'skia_command_buffer == 0', {
          'sources/': [ ['exclude', '_command_buffer\.(h|cpp)$'], ],
        }],
      ],
    },
    {
      'target_name': 'osmesa',
      'type': 'none',
      'direct_dependent_settings': {
        'link_settings': { 'libraries': [ '-lOSMesa', ], },
        'conditions': [
          [ 'skia_os == "mac"', {
            'link_settings': { 'library_dirs' : [ '/opt/X11/lib' ], },
            'include_dirs': [ '/opt/X11/include', ],
          }],
        ],
      },
    },
  ],
}
