# GYP file to build pdfviewer.
#
# To build on Linux:
#  ./gyp_skia pdfviewer.gyp && make pdfviewer
#
{
  'includes': [
    'apptype_console.gypi',
  ],
  'targets': [
    {
      'target_name': 'pdfviewer',
      'type': 'executable',
      'sources': [
        '../experimental/PdfViewer/pdf_viewer_main.cpp',
      ],
      'include_dirs': [
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/inc',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/native',
      ],
      'dependencies': [
        'chop_transparency',
        'flags.gyp:flags',
        'pdfviewer_lib.gyp:pdfviewer_lib',
        'skia_lib.gyp:skia_lib',
      ],
    },
    {
      'target_name': 'chop_transparency',
      'type': 'executable',
      'sources': [
        '../experimental/PdfViewer/chop_transparency_main.cpp',
      ],
      'include_dirs': [
        # For SkBitmapHasher.h
        '../src/utils/',
      ],
      'dependencies': [
        'flags.gyp:flags',
        'skia_lib.gyp:skia_lib',
      ],
    },
  ],
}
