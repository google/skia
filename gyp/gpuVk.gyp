#
# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#

# GYP for building gpu
{
  'variables': {
    'variables':
    {
      'vulkan_merged_into_skia%': '0',
    },
    'vulkan_merged_into_skia%': '<(vulkan_merged_into_skia)',
    'conditions' : [
      [ 'vulkan_merged_into_skia == 1', {
        'skia_gyp_path%': '../gyp',
        'skia_root_path%': '../',
        'vulkan_third_party_path%': '..\\third_party',
      }, {
        'skia_gyp_path%': '../skia/gyp',
        'skia_root_path%': '../skia',
        'vulkan_third_party_path%': '..\\..\\third_party',
      }],
    ],
  },
  'target_defaults': {
    'defines': [
      'VK_PROTOTYPES',
    ],
    'conditions': [
      ['skia_os == "win"', {
        'all_dependent_settings': {
          'msvs_settings': {
            'VCLinkerTool': {
              'AdditionalDependencies': [
                'vulkan-1.lib',
                'shaderc.lib',
                'shaderc_util.lib',
                'glslang.lib',
                'OSDependent.lib',
                'OGLCompiler.lib',
                'SPIRV-Tools.lib',
                'SPIRV.lib',
              ],
            },
          },
        },
        'link_settings': {
          'configurations': {
            'Debug': {
              'msvs_settings': {
                'VCLinkerTool': {
                'AdditionalLibraryDirectories': [
                    '<(vulkan_third_party_path)\\vulkan\\',
                    '<(vulkan_third_party_path)\\shaderc\\Debug\\',
                  ],
                },
              },
            },
            'Release': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'AdditionalLibraryDirectories': [
                    '<(vulkan_third_party_path)\\vulkan\\',
                    '<(vulkan_third_party_path)\\shaderc\\Release\\',
                  ],
                },
              },
            },
            'Debug_x64': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'AdditionalLibraryDirectories': [
                    '<(vulkan_third_party_path)\\vulkan\\',
                    '<(vulkan_third_party_path)\\shaderc\\Debug\\',
                  ],
                },
              },
            },
            'Release_x64': {
              'msvs_settings': {
                'VCLinkerTool': {
                  'AdditionalLibraryDirectories': [
                    '<(vulkan_third_party_path)\\vulkan\\',
                    '<(vulkan_third_party_path)\\shaderc\\Release\\',
                  ],
                },
              },
            },
          },
        },
      }],
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
      ['skia_os != "nacl"', {
        'sources/': [ ['exclude', '_nacl.(h|cpp)$'],
        ],
      }],
      ['skia_os == "nacl" or skia_egl == 0', {
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
        '../third_party/'
      ],
    },
  },
  'targets': [
    {
      'target_name': 'skgpu_vk',
      'product_name': 'skia_skgpu_vk',
      'type': 'static_library',
      'standalone_static_library': 1,
      'dependencies': [
        '<(skia_gyp_path)/core.gyp:*',
        '<(skia_gyp_path)/utils.gyp:utils',
        '<(skia_gyp_path)/etc1.gyp:libetc1',
        '<(skia_gyp_path)/ktx.gyp:libSkKTX',
      ],
      'includes': [
        'gpuVk.gypi',
      ],
      'include_dirs': [
        '../include/gpu',
        '../src/gpu',
        '../third_party',
        '<(skia_root_path)/include/gpu',
        '<(skia_root_path)/include/private',
        '<(skia_root_path)/src/core',
        '<(skia_root_path)/src/gpu',
        '<(skia_root_path)/src/image/',
      ],
      'sources': [
        '<@(skgpu_vk_sources)',
        'gpuVk.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
    },
  ],
}
