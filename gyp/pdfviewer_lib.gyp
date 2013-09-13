# GYP file to build pdfviewer.
#
# To build on Linux:
#  ./gyp_skia pdfviewer.gyp && make pdfviewer
#
{
  'targets': [
    {
      'target_name': 'pdfviewer_lib',
      'type': 'static_library',
      'sources': [
        '../experimental/PdfViewer/SkPdfGraphicsState.cpp',
        '../experimental/PdfViewer/SkPdfFont.cpp',
        '../experimental/PdfViewer/SkPdfRenderer.cpp',
        '../experimental/PdfViewer/SkPdfReporter.cpp',
        '../experimental/PdfViewer/SkPdfUtils.cpp',
        #'../experimental/PdfViewer/SkPdfNYI.cpp',
        '../experimental/PdfViewer/SkTrackDevice.cpp',
        '../experimental/PdfViewer/SkTracker.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeObject.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeTokenizer.cpp',
        '../experimental/PdfViewer/pdfparser/native/SkPdfNativeDoc.cpp',
        '../experimental/PdfViewer/pdfparser/native/pdfapi/SkPdfMapper_autogen.cpp',
        '../experimental/PdfViewer/pdfparser/native/pdfapi/SkPdfHeaders_autogen.cpp',
      ],
      'include_dirs': [
        '../experimental/PdfViewer',
        '../experimental/PdfViewer/pdfparser',
        '../experimental/PdfViewer/pdfparser/native',
        '../experimental/PdfViewer/pdfparser/native/pdfapi',
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
