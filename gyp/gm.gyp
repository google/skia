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
        '../src/utils/',
      ],
      'sources': [
        '../gm/gm_expectations.h',
        '../gm/gm_expectations.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'jsoncpp.gyp:jsoncpp',
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
        '../src/core',
        '../src/images',
        '../src/lazy',
        '../src/effects',
        '../src/pipe/utils/',
        '../src/utils/',
        '../src/utils/debugger',
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

        '../src/utils/debugger/SkDrawCommand.h',
        '../src/utils/debugger/SkDrawCommand.cpp',
        '../src/utils/debugger/SkDebugCanvas.h',
        '../src/utils/debugger/SkDebugCanvas.cpp',
        '../src/utils/debugger/SkObjectParser.h',
        '../src/utils/debugger/SkObjectParser.cpp',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'flags.gyp:flags',
        'gm.gyp:gm_expectations',
        'jsoncpp.gyp:jsoncpp',
        'pdf.gyp:pdf',
      ],
      'conditions': [
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
        ['skia_os in ["linux", "mac", "win"]', {
          'dependencies': [
            'poppler.gyp:libpoppler-cpp-gpl',
          ],
          'sources': [
            '../src/utils/SkPDFRasterizer.cpp',
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
