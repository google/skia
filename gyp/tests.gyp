# GYP file to build unit tests.
#
# To build and run on Linux:
#  ./gyp_skia tests.gyp && make
#  out/Debug/tests
#
# To build on Linux with skia_pdf_support disabled:
#  ./gyp_skia tests.gyp -Dskia_pdf_support=false && make
#
# Building on other platforms not tested yet.
#
{
  'includes': [
    'apptype_console.gypi',
    'target_defaults.gypi',
  ],
  'variables': {
    'skia_pdf_support%': 'true',
  },
  'targets': [
    {
      'target_name': 'tests',
      'type': 'executable',
      'include_dirs' : [
        '../include/pdf',
        '../src/core',
      ],
      'sources': [
        '../tests/BitmapCopyTest.cpp',
        '../tests/BitmapGetColorTest.cpp',
        '../tests/BlitRowTest.cpp',
        '../tests/ClampRangeTest.cpp',
        '../tests/ClipCubicTest.cpp',
        '../tests/ClipStackTest.cpp',
        '../tests/ClipperTest.cpp',
        '../tests/ColorFilterTest.cpp',
        '../tests/ColorTest.cpp',
        '../tests/DataRefTest.cpp',
        '../tests/DequeTest.cpp',
        '../tests/DrawBitmapRectTest.cpp',
        '../tests/FillPathTest.cpp',
        '../tests/FlateTest.cpp',
        '../tests/GeometryTest.cpp',
        '../tests/InfRectTest.cpp',
        '../tests/MathTest.cpp',
        '../tests/MatrixTest.cpp',
        '../tests/Matrix44Test.cpp',
        '../tests/MetaDataTest.cpp',
        '../tests/PackBitsTest.cpp',
        '../tests/PaintTest.cpp',
        '../tests/ParsePathTest.cpp',
        '../tests/PathMeasureTest.cpp',
        '../tests/PathTest.cpp',
        '../tests/PDFPrimitivesTest.cpp',
        '../tests/Reader32Test.cpp',
        '../tests/RefDictTest.cpp',
        '../tests/RegionTest.cpp',
        '../tests/Sk64Test.cpp',
        '../tests/skia_test.cpp',
        '../tests/SortTest.cpp',
        '../tests/SrcOverTest.cpp',
        '../tests/StreamTest.cpp',
        '../tests/StringTest.cpp',
        '../tests/Test.cpp',
        '../tests/TestSize.cpp',
        '../tests/UtilsTest.cpp',
        '../tests/Writer32Test.cpp',
        '../tests/XfermodeTest.cpp',
      ],
      'dependencies': [
        'core.gyp:core',
        'core.gyp:core_tests',
        'effects.gyp:effects',
        'experimental.gyp:experimental',
        'images.gyp:images',
        'utils.gyp:utils',
      ],
      'conditions': [
        [ 'skia_pdf_support == "true"',
          { # if skia_pdf_support is TRUE, depend on pdf.gyp...
            'dependencies': [
              'pdf.gyp:pdf',
            ],
          }, { # else, we don't need PDFPrimitivesTest.cpp after all.
            'sources!': [
              '../tests/PDFPrimitivesTest.cpp',
            ],
          }
        ],
      ],
    },
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
