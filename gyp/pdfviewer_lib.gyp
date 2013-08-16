# GYP file to build pdfviewer.
#
# To build on Linux:
#  ./gyp_skia pdfviewer.gyp && make pdfviewer
#
{
  # TODO(edisonn): Hack! on mack, SHARED_INTERMEDIATE_DIR can't be reliable used in a sources context
  'conditions' : [
    [ 'skia_os != "mac"', {
        'variables': {
          'GENERATE_DIR%' : '<(SHARED_INTERMEDIATE_DIR)',
        },
      },
    ],
    [ 'skia_os == "mac"', {
        'variables': {
          'GENERATE_DIR%' : '../src/tmp_autogen',
        },
      },
    ],
  ],
  'targets': [
    {
      'target_name': 'pdfviewer_lib',
      'type': 'static_library',
      'sources': [
        '../experimental/PdfViewer/SkPdfGraphicsState.cpp',
        '../experimental/PdfViewer/SkPdfFont.cpp',
        '../experimental/PdfViewer/SkPdfRenderer.cpp',
        '../experimental/PdfViewer/SkPdfUtils.cpp',
        #'../experimental/PdfViewer/SkPdfNYI.cpp',
        '../experimental/PdfViewer/SkTrackDevice.cpp',
        '../experimental/PdfViewer/SkTracker.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeObject.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeTokenizer.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeDoc.cpp',
        '<(GENERATE_DIR)/native/autogen/SkPdfMapper_autogen.cpp',
        '<(GENERATE_DIR)/native/autogen/SkPdfHeaders_autogen.cpp',
      ],
      'actions': [
        {
          'action_name': 'spec2def',
          'inputs': [
            '../experimental/PdfViewer/spec2def.py',
            '../experimental/PdfViewer/PdfReference-okular-1.txt',
          ],
          'outputs': [
            '<(GENERATE_DIR)/pdfspec_autogen.py',
          ],
          'action': ['python', '../experimental/PdfViewer/spec2def.py', '../experimental/PdfViewer/PdfReference-okular-1.txt', '<(GENERATE_DIR)/pdfspec_autogen.py'],
        },
        {
          'action_name': 'copy_files1',
          'inputs' : ['../experimental/PdfViewer/datatypes.py'],
          'outputs': [
            '<(GENERATE_DIR)/datatypes.py',
          ],
          'action': ['python', '../experimental/PdfViewer/copy_files.py', '<(GENERATE_DIR)', '../experimental/PdfViewer/datatypes.py'],
        },
        {
          'action_name': 'copy_files2',

          'inputs' : ['../experimental/PdfViewer/generate_code.py'],
          'outputs': [
            '<(GENERATE_DIR)/generate_code.py',
          ],
          'action': ['python', '../experimental/PdfViewer/copy_files.py', '<(GENERATE_DIR)', '../experimental/PdfViewer/generate_code.py'],
        },
        {
          'action_name': 'generate_code',
          'inputs': [
            '<(GENERATE_DIR)/datatypes.py',
            '<(GENERATE_DIR)/generate_code.py',
            '<(GENERATE_DIR)/pdfspec_autogen.py',
          ],
          'outputs': [
            '<(GENERATE_DIR)/native/autogen/SkPdfEnums_autogen.h',
            '<(GENERATE_DIR)/native/autogen/SkPdfMapper_autogen.h',
            '<(GENERATE_DIR)/native/autogen/SkPdfHeaders_autogen.h',
            '<(GENERATE_DIR)/native/autogen/SkPdfMapper_autogen.cpp',
            '<(GENERATE_DIR)/native/autogen/SkPdfHeaders_autogen.cpp',
            # TODO(edisonn): ok, there are many more files here, which we should list but since
            # any change in the above should trigger a change here, we should be fine normally
          ],
          'action': ['python', '<(GENERATE_DIR)/generate_code.py', '<(GENERATE_DIR)'],
        },
      ],
      'include_dirs': [
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/native',
        '<(GENERATE_DIR)/native/autogen',
      ],
      'dependencies': [
        'skia_lib.gyp:skia_lib',
        'zlib.gyp:zlib',
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
