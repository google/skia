# GYP file to build pdfviewer.
#
# To build on Linux:
#  ./gyp_skia pdfviewer.gyp && make pdfviewer
#
{
  'variables': {
    'skia_warnings_as_errors': 0,
  },
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'pdfviewer',
      'type': 'executable',
      'cflags': ['-fexceptions'],
      'cflags_cc': ['-fexceptions'],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'sources': [
        '../experimental/PdfViewer/pdf_viewer_main.cpp',
      ],
      'include_dirs': [
        '../third_party/externals/podofo/src/base',
        '../third_party/externals/podofo/src',
        '../third_party/externals/podofo',
        '../tools',
        '../experimental/PdfViewer',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'tools.gyp:picture_utils',
        '../third_party/externals/podofo/podofo.gyp:podofo',
      ],
      'link_settings': {
        'libraries': [
        ],
      },
      'defines': [
        'BUILDING_PODOFO',
      ],
    },
  ],
  'conditions': [
    ['skia_os == "win"',
      {
        'targets': [
          {
            'target_name': 'win_lcid',
            'type': 'executable',
            'sources': [
              '../tools/win_lcid.cpp',
            ],
          },
        ],
      },
    ],
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
