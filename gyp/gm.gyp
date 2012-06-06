# GYP file to build the "gm" (golden master) executable.
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'gm',
      'type': 'executable',
      'include_dirs' : [
        '../src/core',
      ],
      'includes': [
        'gmslides.gypi',
      ],
      'sources': [
        '../gm/gm.cpp',
        '../gm/gmmain.cpp',
        '../gm/system_preferences_default.cpp',
        '../src/pipe/utils/SamplePipeControllers.h',
        '../src/pipe/utils/SamplePipeControllers.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'gpu.gyp:gr',
        'gpu.gyp:skgr',
        'images.gyp:images',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
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
      ],  
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
