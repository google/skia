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
      ['skia_os != "linux"', {
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
        '<@(skgpu_vk_sources)',
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
        [ 'skia_os == "linux"', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
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
        [ 'skia_os in ["win", "ios"]', {
          'sources!': [
            '../src/gpu/gl/GrGLDefaultInterface_none.cpp',
            '../src/gpu/gl/GrGLCreateNativeInterface_none.cpp',
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
        [ 'skia_vulkan', {
          'conditions': [
            [ 'skia_os == "win"', {
             'variables': {
                'vulkan_lib_name': '-lvulkan-1',
                'vulkan_sdk_path' : '<!(echo %VK_SDK_PATH%)',
              },
              'include_dirs': [
                '<(vulkan_sdk_path)/Include',
              ],
              'direct_dependent_settings': {
                'include_dirs': [
                  '<(vulkan_sdk_path)/Include',
                ],
              },
              'link_settings': {
                'conditions': [
                  [ 'skia_arch_type == "x86"', {
                    'library_dirs': [ '<(vulkan_sdk_path)/Bin32', ],
                  }, {
                    'library_dirs': [ '<(vulkan_sdk_path)/Bin', ],
                  }],
                ]
              },
            }, {
              'variables': {
                'vulkan_lib_name': '-lvulkan',
              },
            }],
            [ 'skia_os == "linux"', {
              'variables': {
                'vulkan_sdk_path' : '<!(echo $VULKAN_SDK)',
              },
              'include_dirs': [
                '<(vulkan_sdk_path)/include',
              ],
              'direct_dependent_settings': {
                'include_dirs': [
                  '<(vulkan_sdk_path)/include',
                ],
              },
              'link_settings': {
                'library_dirs': [ '<(vulkan_sdk_path)/lib', ],
              },
            }],
          ],
          'dependencies': [
            'shaderc.gyp:shaderc_combined',
          ],
          'include_dirs': [
            '../third_party/externals/shaderc2/libshaderc/include',
          ],
          'direct_dependent_settings': {
            'include_dirs': [
              '../third_party/externals/shaderc2/libshaderc/include',
            ],
          },
          'link_settings': {
            'libraries': [ '<(vulkan_lib_name)', ],
          },
        }, {
          'sources!': [
            '<@(skgpu_vk_sources)',
          ],
        }],
      ],
    },
  ],
}
