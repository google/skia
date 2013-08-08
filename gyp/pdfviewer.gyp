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
      'target_name': 'libpdfviewer',
      'type': 'static_library',
      'sources': [
        '../experimental/PdfViewer/SkPdfBasics.cpp',
        '../experimental/PdfViewer/SkPdfFont.cpp',
        '../experimental/PdfViewer/SkPdfRenderer.cpp',
        '../experimental/PdfViewer/SkPdfUtils.cpp',
        #'../experimental/PdfViewer/SkPdfNYI.cpp',
        '../experimental/PdfViewer/SkTrackDevice.cpp',
        '../experimental/PdfViewer/SkTracker.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfObject.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeTokenizer.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkNativeParsedPDF.cpp',
        '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfMapper_autogen.cpp',
        '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfHeaders_autogen.cpp',
      ],
      'copies': [
        {
          'files': [
            '../experimental/PdfViewer/datatypes.py',
            '../experimental/PdfViewer/generate_code.py',
          ],
          'destination': '<(SHARED_INTERMEDIATE_DIR)',
        },
      ],
      'actions': [
        {
          'action_name': 'spec2def',
          'inputs': [
            '../experimental/PdfViewer/spec2def.py',
            '../experimental/PdfViewer/PdfReference-okular-1.txt',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/pdfspec_autogen.py',
          ],
          'action': ['python', '../experimental/PdfViewer/spec2def.py', '../experimental/PdfViewer/PdfReference-okular-1.txt', '<(SHARED_INTERMEDIATE_DIR)/pdfspec_autogen.py'],
        },
        {
          'action_name': 'generate_code',
          'inputs': [
            '<(SHARED_INTERMEDIATE_DIR)/datatypes.py',
            '<(SHARED_INTERMEDIATE_DIR)/generate_code.py',
            '<(SHARED_INTERMEDIATE_DIR)/pdfspec_autogen.py',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfEnums_autogen.h',
            '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfMapper_autogen.h',
            '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfHeaders_autogen.h',
            '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfMapper_autogen.cpp',
            '<(SHARED_INTERMEDIATE_DIR)/native/autogen/SkPdfHeaders_autogen.cpp',
            # TODO(edisonn): ok, there are many more files here, which we should list but since
            # any change in the above should trigger a change here, we should be fine normally
          ],
          'action': ['python', '<(SHARED_INTERMEDIATE_DIR)/generate_code.py', '<(SHARED_INTERMEDIATE_DIR)'],
        },
      ],
      'include_dirs': [
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/native',
        '<(SHARED_INTERMEDIATE_DIR)/native/autogen',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
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
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/autogen',
        '../experimental/PdfViewer/pdfparser/native',
        '../experimental/PdfViewer/pdfparser/native/autogen',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'flags.gyp:flags',
        'libpdfviewer',
        'chop_transparency',
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
        'skia_lib.gyp:skia_lib',
        'flags.gyp:flags',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
