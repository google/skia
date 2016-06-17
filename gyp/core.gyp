# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Core Skia library code.
{
  'targets': [
    {
      'target_name': 'core',
      'product_name': 'skia_core',
      'type': 'static_library',
      'standalone_static_library': 1,
      'msvs_guid': 'B7760B5E-BFA8-486B-ACFD-49E3A6DE8E76',

      'includes': [
        'core.gypi',
      ],

      'include_dirs': [
        '../include/c',
        '../include/config',
        '../include/core',
        '../include/pathops',
        '../include/ports',
        '../include/private',
        '../include/utils',
        '../include/images',
        '../src/core',
        '../src/sfnt',
        '../src/image',
        '../src/opts',
        '../src/utils',
      ],
      'sources': [
        'core.gypi', # Makes the gypi appear in IDEs (but does not modify the build).
      ],
      'msvs_disabled_warnings': [4244, 4267,4345, 4390, 4554, 4800],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'link_settings': {
            'libraries': [
              '-lpthread',
            ],
          },
        }],
        [ 'skia_os == "mac"', {
          'include_dirs': [
            '../include/utils/mac',
          ],
          'sources': [
            '../include/utils/mac/SkCGUtils.h',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/ApplicationServices.framework',
            ],
          },
        }],
        [ 'skia_os == "ios"', {
          'include_dirs': [
            '../include/utils/ios',
          ],
          'sources': [
            '../include/utils/mac/SkCGUtils.h',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/CoreFoundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreGraphics.framework',
              '$(SDKROOT)/System/Library/Frameworks/CoreText.framework',
              '$(SDKROOT)/System/Library/Frameworks/UIKit.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
              '$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
              '$(SDKROOT)/System/Library/Frameworks/ImageIO.framework',
              '$(SDKROOT)/System/Library/Frameworks/MobileCoreServices.framework',
            ],
          },
        }],
        [ 'skia_os == "win"', {
          'include_dirs': [
            'config/win',
          ],
        }],
        [ 'skia_os == "android"', {
          'dependencies': [
            'android_deps.gyp:cpu_features',
          ],
        }],
        ['skia_gpu == 1', {
          'include_dirs': [
              '../include/gpu',
              '../src/gpu',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../include/c',
          '../include/config',
          '../include/core',
          '../include/pathops',
        ],
        'conditions': [
          [ 'skia_os == "mac"', {
            'include_dirs': [
              '../include/utils/mac',
            ],
          }],
          [ 'skia_os == "ios"', {
            'include_dirs': [
              '../include/utils/ios',
            ],
          }],
          [ 'skia_os == "win"', {
            'include_dirs': [
              'config/win',
            ],
          }],
        ],
      },
    },
  ],
}
