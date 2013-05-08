# GYP file to build the "gm" (golden master) executable.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'gm_expectations',
      'type': 'static_library',
      'include_dirs' : [
        '../include/core/',
        '../src/utils/',
      ],
      'sources': [
        '../gm/gm_expectations.h',
        '../gm/gm_expectations.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'core.gyp:core',
        'images.gyp:images',
        'jsoncpp.gyp:jsoncpp',
        'utils.gyp:utils',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '../gm/',
        ],
      },
    },
    {
      'target_name': 'gm',
      'type': 'executable',
      'include_dirs' : [
        '../debugger',
        '../src/core',
        '../src/effects',
        '../src/pipe/utils/',
        '../src/utils/',
      ],
      'includes': [
        'gmslides.gypi',
      ],
      'sources': [
        '../debugger/SkDrawCommand.h',
        '../debugger/SkDrawCommand.cpp',
        '../debugger/SkDebugCanvas.h',
        '../debugger/SkDebugCanvas.cpp',
        '../debugger/SkObjectParser.h',
        '../debugger/SkObjectParser.cpp',

        '../gm/gm.cpp',
        '../gm/gmmain.cpp',
        '../gm/system_preferences_default.cpp',

        '../src/pipe/utils/SamplePipeControllers.h',
        '../src/pipe/utils/SamplePipeControllers.cpp',
      ],
      'dependencies': [
        'skia_base_libs.gyp:skia_base_libs',
        'effects.gyp:effects',
        'flags.gyp:flags',
        'gm.gyp:gm_expectations',
        'images.gyp:images',
        'jsoncpp.gyp:jsoncpp',
        'pdf.gyp:pdf',
        'utils.gyp:utils',
      ],
      'conditions': [
        ['skia_os == "mac"', {
          'sources!': [
            '../gm/system_preferences_default.cpp',
          ],
          'sources': [
            '../gm/system_preferences_mac.mm',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/Cocoa.framework',
              '$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
            ],
          },
        }],
        ['skia_os == "win"', {
          'dependencies': [
            'xps.gyp:xps',
          ],
        }],
        ['skia_gpu == 1', {
          'include_dirs': [
            '../src/gpu',
          ],
        }],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
