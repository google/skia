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
      'target_name': 'libpdfviewer',
      'type': 'static_library',
      'cflags': ['-fexceptions'],
      'cflags_cc': ['-fexceptions'],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'sources': [
        '../experimental/PdfViewer/SkPdfBasics.cpp',
        '../experimental/PdfViewer/SkPdfFont.cpp',
        '../experimental/PdfViewer/SkPdfParser.cpp',
        '../experimental/PdfViewer/SkPdfUtils.cpp',
        #'../experimental/PdfViewer/SkPdfNYI.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfObject.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeTokenizer.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkNativeParsedPDF.cpp',
        '../experimental/PdfViewer/pdfparser/native/autogen/SkPdfMapper_autogen.cpp',
        '../experimental/PdfViewer/pdfparser/native/autogen/SkPdfHeaders_autogen.cpp',
      ],
      'actions': [
        {
          'action_name': 'spec2def',
          'inputs': [
            '../experimental/PdfViewer/spec2def.py',
            '../experimental/PdfViewer/PdfReference-okular-1.txt',
          ],
          'outputs': [
            '../experimental/PdfViewer/autogen/pdfspec_autogen.py',
          ],
          'action': ['python', '../experimental/PdfViewer/spec2def.py', '../experimental/PdfViewer/PdfReference-okular-1.txt', '../experimental/PdfViewer/autogen/pdfspec_autogen.py'],
        },
        {
          'action_name': 'generate_code',
          'inputs': [
            '../experimental/PdfViewer/generate_code.py',
            '../experimental/PdfViewer/autogen/pdfspec_autogen.py',
          ],
          'outputs': [
            '../experimental/PdfViewer/pdfparser/autogen/SkPdfEnums_autogen.h',
            '../experimental/PdfViewer/pdfparser/native/autogen/SkPdfMapper_autogen.cpp',
            '../experimental/PdfViewer/pdfparser/native/autogen/SkPdfHeaders_autogen.cpp',
            # TODO(edisonn): ok, there are many more files here, which we should list but since
            # any change in the above should trigger a change here, we should be fine normally
          ],
          'action': ['python', '../experimental/PdfViewer/generate_code.py', '../experimental/PdfViewer/pdfparser/'],
        },
      ],
      'include_dirs': [
        '../tools',
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/autogen',
        '../experimental/PdfViewer/pdfparser/native',
        '../experimental/PdfViewer/pdfparser/native/autogen',
      ],
      'dependencies': [
        'core.gyp:core',
        'effects.gyp:effects',
        'images.gyp:images',
        'pdf.gyp:pdf',
        'ports.gyp:ports',
        'tools.gyp:picture_utils',
      ],
      'link_settings': {
        'libraries': [
        ],
      },
    },
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
        '../tools',
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/autogen',
        '../experimental/PdfViewer/pdfparser/native',
        '../experimental/PdfViewer/pdfparser/native/autogen',
      ],
      'dependencies': [
        'core.gyp:core',
        'images.gyp:images',
        'libpdfviewer',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
