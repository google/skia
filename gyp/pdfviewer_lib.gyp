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
        # FIXME: Include directory is named "inc" (instead of "include") in
        # order to not be considered the public API.
        '../experimental/PdfViewer/inc/SkPdfContext.h',
        '../experimental/PdfViewer/inc/SkPdfDiffEncoder.h',
        '../experimental/PdfViewer/inc/SkPdfRenderer.h',
        '../experimental/PdfViewer/inc/SkPdfTokenLooper.h',

        '../experimental/PdfViewer/src/SkPdfContext.cpp',
        '../experimental/PdfViewer/src/SkPdfRenderer.cpp',
        '../experimental/PdfViewer/src/SkTDStackNester.h',
        '../experimental/PdfViewer/src/SkPdfDiffEncoder.cpp',

        '../experimental/PdfViewer/SkPdfGraphicsState.cpp',
        '../experimental/PdfViewer/SkPdfFont.cpp',
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
        '../experimental/PdfViewer/inc',
        '../experimental/PdfViewer/src',
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
