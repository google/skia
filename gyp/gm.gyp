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
        '../src/effects',
        '../src/images',
        '../src/pipe/utils',
        '../src/utils',
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
        'etc1.gyp:libetc1',
        'flags.gyp:flags',
        'jsoncpp.gyp:jsoncpp',
        'pdf.gyp:pdf',
        'skia_lib.gyp:skia_lib',
        'tools.gyp:crash_handler',
        'tools.gyp:gm_expectations',
        'tools.gyp:proc_stats',
        'tools.gyp:resources',
        'tools.gyp:sk_tool_utils',
      ],
      'conditions': [
        ['skia_android_framework', {
          'libraries': [
            '-lskia',
          ],
        }],
        ['skia_run_pdfviewer_in_gm or skia_poppler_enabled', {
          'sources': [
            '../src/utils/SkPDFRasterizer.cpp',
          ],
        }],
        ['skia_run_pdfviewer_in_gm', {
          'defines': [
            'SK_BUILD_NATIVE_PDF_RENDERER',
          ],
          'include_dirs' : [
            '../experimental/PdfViewer/inc',
          ],
          'dependencies': [
            'pdfviewer_lib.gyp:pdfviewer_lib',
          ],
        }],
        ['skia_poppler_enabled', {
          'dependencies': [
            'poppler.gyp:*',
          ],
          'defines': [
            'SK_BUILD_POPPLER',
          ],
        }],
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
          'dependencies': [
            'gputest.gyp:skgputest',
          ],
        }],
      ],
    },
  ],
}
