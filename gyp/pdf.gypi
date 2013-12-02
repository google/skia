# Include this gypi to include all 'pdf' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
    'sources': [
        '<(skia_include_path)/pdf/SkPDFDevice.h',
        '<(skia_include_path)/pdf/SkPDFDocument.h',

        '<(skia_src_path)/pdf/SkPDFCatalog.cpp',
        '<(skia_src_path)/pdf/SkPDFCatalog.h',
        '<(skia_src_path)/pdf/SkPDFDevice.cpp',
        '<(skia_src_path)/pdf/SkPDFDeviceFlattener.cpp',
        '<(skia_src_path)/pdf/SkPDFDeviceFlattener.h',
        '<(skia_src_path)/pdf/SkPDFDocument.cpp',
        '<(skia_src_path)/pdf/SkPDFFont.cpp',
        '<(skia_src_path)/pdf/SkPDFFont.h',
        '<(skia_src_path)/pdf/SkPDFFontImpl.h',
        '<(skia_src_path)/pdf/SkPDFFormXObject.cpp',
        '<(skia_src_path)/pdf/SkPDFFormXObject.h',
        '<(skia_src_path)/pdf/SkPDFGraphicState.cpp',
        '<(skia_src_path)/pdf/SkPDFGraphicState.h',
        '<(skia_src_path)/pdf/SkPDFImage.cpp',
        '<(skia_src_path)/pdf/SkPDFImage.h',
        '<(skia_src_path)/pdf/SkPDFPage.cpp',
        '<(skia_src_path)/pdf/SkPDFPage.h',
        '<(skia_src_path)/pdf/SkPDFResourceDict.cpp',
        '<(skia_src_path)/pdf/SkPDFResourceDict.h',
        '<(skia_src_path)/pdf/SkPDFShader.cpp',
        '<(skia_src_path)/pdf/SkPDFShader.h',
        '<(skia_src_path)/pdf/SkPDFStream.cpp',
        '<(skia_src_path)/pdf/SkPDFStream.h',
        '<(skia_src_path)/pdf/SkPDFTypes.cpp',
        '<(skia_src_path)/pdf/SkPDFTypes.h',
        '<(skia_src_path)/pdf/SkPDFUtils.cpp',
        '<(skia_src_path)/pdf/SkPDFUtils.h',
        '<(skia_src_path)/pdf/SkTSet.h',
    ],
}
