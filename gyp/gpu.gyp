# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# GYP for building gpu
{
  'target_defaults': {
    'conditions': [
      ['skia_os != "win"', {
        'sources/': [ ['exclude', '_win.(h|cpp)$'],
        ],
      }],
      ['skia_os != "mac"', {
        'sources/': [ ['exclude', '_mac.(h|cpp|m|mm)$'],
        ],
      }],
      ['skia_os != "linux" and skia_os != "chromeos"', {
        'sources/': [ ['exclude', '_glx.(h|cpp)$'],
        ],
      }],
      ['skia_os != "ios"', {
        'sources/': [ ['exclude', '_iOS.(h|cpp|m|mm)$'],
        ],
      }],
      ['skia_os != "android"', {
        'sources/': [ ['exclude', '_android.(h|cpp)$'],
        ],
      }],
      ['skia_egl == 0', {
        'sources/': [ ['exclude', '_egl.(h|cpp)$'],
        ],
      }],
      ['skia_os == "android"', {
        'sources/': [ ['exclude', 'GrGLCreateNativeInterface_egl.cpp'],
        ],
      }],
      ['skia_egl == 1', {
        'sources/': [ ['exclude', '_glx.(h|cpp)$'],
        ],
      }],
      # nullify the targets in this gyp file if skia_gpu is 0
      [ 'skia_gpu == 0', {
        'sources/': [
          ['exclude', '.*'],
        ],
        'defines/': [
          ['exclude', '.*'],
        ],
        'include_dirs/': [
           ['exclude', '.*'],
        ],
        'link_settings': {
          'libraries/': [
            ['exclude', '.*'],
          ],
        },
        'direct_dependent_settings': {
          'defines/': [
            ['exclude', '.*'],
          ],
          'include_dirs/': [
            ['exclude', '.*'],
          ],
        },
      }],
    ],
    'direct_dependent_settings': {
      'conditions': [
        [ 'skia_os == "win"', {
          'defines': [
            'GR_GL_FUNCTION_TYPE=__stdcall',
          ],
        }],
      ],
      'include_dirs': [
        '../include/gpu',
      ],
    },
  },
  'targets': [
    {
      'target_name': 'skgpu',
      'product_name': 'skia_skgpu',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        'core.gyp:*',
        'utils.gyp:utils',
        'etc1.gyp:libetc1',
        'ktx.gyp:libSkKTX',
      ],
      'includes': [
        'gpu.gypi',
      ],
      'include_dirs': [
        '../include/gpu',
        '../include/private',
        '../src/core',
        '../src/gpu',
        '../src/image/',
      ],
      'sources': [
        '<@(skgpu_sources)',
        '<@(skgpu_native_gl_sources)',
        '<@(skgpu_angle_gl_sources)',
        '<@(skgpu_command_buffer_gl_sources)',
        '<@(skgpu_mesa_gl_sources)',
        '<@(skgpu_debug_gl_sources)',
        '<@(skgpu_null_gl_sources)',
        'gpu.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'conditions': [
        [ 'skia_gpu_extra_dependency_path', {
          'dependencies' : [
              '<(skia_gpu_extra_dependency_path):*',
          ],
          'export_dependent_settings': [
            '<(skia_gpu_extra_dependency_path):*',
          ],
        }],
        [ 'skia_chrome_utils', {
          'sources': [
            '../experimental/ChromeUtils/SkBorder.cpp',
            '../experimental/ChromeUtils/SkBorder.h',
          ],
          'defines': [
            'GR_CHROME_UTILS=1',
          ],
        }],
        [ 'skia_os == "linux" or skia_os == "chromeos"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ '(skia_os == "linux" or skia_os == "chromeos") and skia_egl == 1', {
          'link_settings': {
            'libraries': [
              '-lEGL',
              '-lGLESv2',
            ],
          },
        }],
        [ '(skia_os == "linux" or skia_os == "chromeos") and skia_egl == 0', {
          'link_settings': {
            'libraries': [
              '-lGL',
              '-lGLU',
              '-lX11',
            ],
          },
        }],
        [ 'skia_egl == 1', {
          'defines': [
            'SK_EGL=1',
          ],
        }],
        [ 'skia_egl == 0', {
          'defines': [
            'SK_EGL=0',
          ],
        }],
        [ 'skia_mesa and skia_os == "linux"', {
          'link_settings': {
            'libraries': [
              '-lOSMesa',
            ],
          },
        }],
        [ 'skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'not skia_mesa', {
          'sources!': [
            '../src/gpu/gl/mesa/SkMesaGLContext.cpp',
            '../src/gpu/gl/mesa/GrGLCreateMesaInterface.cpp',
          ],
        }],
        [ 'skia_mesa and skia_os == "mac"', {
          'link_settings': {
            'libraries': [
              '/opt/X11/lib/libOSMesa.dylib',
            ],
          },
          'include_dirs': [
             '/opt/X11/include/',
          ],
        }],
        [ 'skia_os in ["win", "ios"]', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
        }],
        [ 'skia_angle', {
          'dependencies': [
            'angle.gyp:*',
          ],
          'export_dependent_settings': [
            'angle.gyp:*',
          ],
        }, { # not skia_angle
          'sources!': [
            '<@(skgpu_angle_gl_sources)',
          ],
        }],
        [ 'skia_command_buffer', {
        }, { # not skia_command_buffer
          'sources!': [
            '<@(skgpu_command_buffer_gl_sources)',
          ],
        }],
        [ 'skia_os == "android"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
          ],
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
      ],
    },
  ],
}
