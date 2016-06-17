# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Include this gypi to include all 'pdf' files
# The parent gyp/gypi file must define
#       'skia_src_path'     e.g. skia/trunk/src
#       'skia_include_path' e.g. skia/trunk/include
#
# The skia build defines these in common_variables.gypi
#
{
    'sources': [
        '<(skia_src_path)/pdf/SkBitmapKey.h',
        '<(skia_src_path)/pdf/SkDeflate.cpp',
        '<(skia_src_path)/pdf/SkDeflate.h',
        '<(skia_src_path)/pdf/SkJpegInfo.cpp',
        '<(skia_src_path)/pdf/SkJpegInfo.h',
        '<(skia_src_path)/pdf/SkPDFBitmap.cpp',
        '<(skia_src_path)/pdf/SkPDFBitmap.h',
        '<(skia_src_path)/pdf/SkPDFCanon.cpp',
        '<(skia_src_path)/pdf/SkPDFCanon.h',
        '<(skia_src_path)/pdf/SkPDFCanvas.cpp',
        '<(skia_src_path)/pdf/SkPDFCanvas.h',
        '<(skia_src_path)/pdf/SkPDFDevice.cpp',
        '<(skia_src_path)/pdf/SkPDFDevice.h',
        '<(skia_src_path)/pdf/SkPDFDocument.cpp',
        '<(skia_src_path)/pdf/SkPDFDocument.h',
        '<(skia_src_path)/pdf/SkPDFFont.cpp',
        '<(skia_src_path)/pdf/SkPDFFont.h',
        '<(skia_src_path)/pdf/SkPDFFontImpl.h',
        '<(skia_src_path)/pdf/SkPDFFormXObject.cpp',
        '<(skia_src_path)/pdf/SkPDFFormXObject.h',
        '<(skia_src_path)/pdf/SkPDFGraphicState.cpp',
        '<(skia_src_path)/pdf/SkPDFGraphicState.h',
        '<(skia_src_path)/pdf/SkPDFMetadata.cpp',
        '<(skia_src_path)/pdf/SkPDFMetadata.h',
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
    ],
}
