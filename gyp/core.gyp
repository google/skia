# Core Skia library code.
{
  'targets': [
    {
      'target_name': 'core',
      'type': 'static_library',
      'msvs_guid': 'B7760B5E-BFA8-486B-ACFD-49E3A6DE8E76',

      'includes': [
        'core.gypi',
      ],

      'include_dirs': [
        '../include/config',
        '../include/core',
        '../include/pipe',
        '../include/ports',
        '../include/xml',
        '../src/core',
      ],
      'msvs_disabled_warnings': [4244, 4267,4345, 4390, 4554, 4800],
      'conditions': [
        [ 'skia_os in ["linux", "freebsd", "openbsd", "solaris"]', {
          'cflags': [
            '-Wno-unused',
            '-Wno-unused-function',
          ],
          'link_settings': {
            'libraries': [
              '-lfreetype',
              '-lpthread',
            ],
          },
        }],
        [ 'skia_os == "mac"', {
          'include_dirs': [
            '../include/utils/mac',
            '../third_party/freetype/include/**',
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
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/CoreFoundation.framework',
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/CoreGraphics.framework',
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/CoreText.framework',
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/UIKit.framework',
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/Foundation.framework',
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/QuartzCore.framework',
              '/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS4.3.sdk/System/Library/Frameworks/OpenGLES.framework',
            ],
          },
        }],
        [ 'skia_os == "win"', {
          'include_dirs': [
            'config/win',
          ],
          'sources!': [
            '../include/core/SkMMapStream.h',
            '../src/core/SkMMapStream.cpp',
          ],
        }],
        [ 'skia_os == "android"', {
          'dependencies': [
             'android_system.gyp:ft2',
          ],
        }],
        [ 'skia_os == "android" and skia_arch_type == "arm" and armv7 == 1', {
          # The code in SkUtilsArm.cpp can be used on an ARM-based Linux system, not only Android.
          'sources': [
            '../src/core/SkUtilsArm.cpp',
            '../src/core/SkUtilsArm.h',
          ],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          'config',
          '../include/config',
          '../include/core',
          '../include/pipe',
          'ext',
        ],
      },
      'dependencies': [
        'opts.gyp:opts'
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
