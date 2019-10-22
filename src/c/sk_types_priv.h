/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_types_priv_DEFINED
#define sk_types_priv_DEFINED

#include "include/c/sk_types.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define ASSERT_MSG(SK, C) "ABI changed, you must write a enumeration mapper for " TOSTRING(#SK) " to " TOSTRING(#C) "."

// Define a mapping between a C++ type and the C type.
//
// Usual Values:
//  - C++  |  SkType   |  SkSomeType
//  - C    |  sk_type  |  sk_some_type_t
//  - Map  |  Name     |  ToSomeType / AsSomeType
//
#define DEF_MAP_DECL(SkType, sk_type, Name, Declaration)       \
    Declaration;                                               \
    static inline const SkType& As##Name(const sk_type& t) {   \
        return reinterpret_cast<const SkType&>(t);             \
    }                                                          \
    static inline const SkType* As##Name(const sk_type* t) {   \
        return reinterpret_cast<const SkType*>(t);             \
    }                                                          \
    static inline SkType& As##Name(sk_type& t) {               \
        return reinterpret_cast<SkType&>(t);                   \
    }                                                          \
    static inline SkType* As##Name(sk_type* t) {               \
        return reinterpret_cast<SkType*>(t);                   \
    }                                                          \
    static inline const sk_type& To##Name(const SkType& t) {   \
        return reinterpret_cast<const sk_type&>(t);            \
    }                                                          \
    static inline const sk_type* To##Name(const SkType* t) {   \
        return reinterpret_cast<const sk_type*>(t);            \
    }                                                          \
    static inline sk_type& To##Name(SkType& t) {               \
        return reinterpret_cast<sk_type&>(t);                  \
    }                                                          \
    static inline sk_type* To##Name(SkType* t) {               \
        return reinterpret_cast<sk_type*>(t);                  \
    }

#define DEF_CLASS_MAP(SkType, sk_type, Name)                   \
    DEF_MAP_DECL(SkType, sk_type, Name, class SkType)

#define DEF_STRUCT_MAP(SkType, sk_type, Name)                  \
    DEF_MAP_DECL(SkType, sk_type, Name, struct SkType)

#define DEF_MAP(SkType, sk_type, Name)                         \
    DEF_MAP_DECL(SkType, sk_type, Name, )


DEF_CLASS_MAP(Sk3DView, sk_3dview_t, 3DView)
DEF_CLASS_MAP(SkBitmap, sk_bitmap_t, Bitmap)
DEF_CLASS_MAP(SkCanvas, sk_canvas_t, Canvas)
DEF_CLASS_MAP(SkCodec, sk_codec_t, Codec)
DEF_CLASS_MAP(SkColorFilter, sk_colorfilter_t, ColorFilter)
DEF_CLASS_MAP(SkColorSpace, sk_colorspace_t, ColorSpace)
DEF_CLASS_MAP(SkColorTable, sk_colortable_t, ColorTable)
DEF_CLASS_MAP(SkData, sk_data_t, Data)
DEF_CLASS_MAP(SkDocument, sk_document_t, Document)
DEF_CLASS_MAP(SkDrawable, sk_drawable_t, Drawable)
DEF_CLASS_MAP(SkDynamicMemoryWStream, sk_wstream_dynamicmemorystream_t, DynamicMemoryWStream)
DEF_CLASS_MAP(SkFILEStream, sk_stream_filestream_t, FileStream)
DEF_CLASS_MAP(SkFILEWStream, sk_wstream_filestream_t, FileWStream)
DEF_CLASS_MAP(SkFont, sk_font_t, Font)
DEF_CLASS_MAP(SkFontMgr, sk_fontmgr_t, FontMgr)
DEF_CLASS_MAP(SkFontStyle, sk_fontstyle_t, FontStyle)
DEF_CLASS_MAP(SkFontStyleSet, sk_fontstyleset_t, FontStyleSet)
DEF_CLASS_MAP(SkImage, sk_image_t, Image)
DEF_CLASS_MAP(SkImageFilter, sk_imagefilter_t, ImageFilter)
DEF_CLASS_MAP(SkMaskFilter, sk_maskfilter_t, MaskFilter)
DEF_CLASS_MAP(SkMatrix44, sk_matrix44_t, Matrix44)
DEF_CLASS_MAP(SkMemoryStream, sk_stream_memorystream_t, MemoryStream)
DEF_CLASS_MAP(SkNWayCanvas, sk_nway_canvas_t, NWayCanvas)
DEF_CLASS_MAP(SkNoDrawCanvas, sk_nodraw_canvas_t, NoDrawCanvas)
DEF_CLASS_MAP(SkOverdrawCanvas, sk_overdraw_canvas_t, OverdrawCanvas)
DEF_CLASS_MAP(SkOpBuilder, sk_opbuilder_t, OpBuilder)
DEF_CLASS_MAP(SkPaint, sk_paint_t, Paint)
DEF_CLASS_MAP(SkPath, sk_path_t, Path)
DEF_CLASS_MAP(SkPathEffect, sk_path_effect_t, PathEffect)
DEF_CLASS_MAP(SkPathMeasure, sk_pathmeasure_t, PathMeasure)
DEF_CLASS_MAP(SkPicture, sk_picture_t, Picture)
DEF_CLASS_MAP(SkPictureRecorder, sk_picture_recorder_t, PictureRecorder)
DEF_CLASS_MAP(SkPixmap, sk_pixmap_t, Pixmap)
DEF_CLASS_MAP(SkRegion, sk_region_t, Region)
DEF_CLASS_MAP(SkRRect, sk_rrect_t, RRect)
DEF_CLASS_MAP(SkShader, sk_shader_t, Shader)
DEF_CLASS_MAP(SkStream, sk_stream_t, Stream)
DEF_CLASS_MAP(SkStreamAsset, sk_stream_asset_t, StreamAsset)
DEF_CLASS_MAP(SkStreamRewindable, sk_stream_streamrewindable_t, StreamRewindable)
DEF_CLASS_MAP(SkString, sk_string_t, String)
DEF_CLASS_MAP(SkSurface, sk_surface_t, Surface)
DEF_CLASS_MAP(SkSurfaceProps, sk_surfaceprops_t, SurfaceProps)
DEF_CLASS_MAP(SkTextBlob, sk_textblob_t, TextBlob)
DEF_CLASS_MAP(SkTextBlobBuilder, sk_textblob_builder_t, TextBlobBuilder)
DEF_CLASS_MAP(SkTypeface, sk_typeface_t, Typeface)
DEF_CLASS_MAP(SkVertices, sk_vertices_t, Vertices)
DEF_CLASS_MAP(SkWStream, sk_wstream_t, WStream)

DEF_CLASS_MAP(GrContext, gr_context_t, GrContext)
DEF_CLASS_MAP(GrBackendTexture, gr_backendtexture_t, GrBackendTexture)
DEF_CLASS_MAP(GrBackendRenderTarget, gr_backendrendertarget_t, GrBackendRenderTarget)

DEF_STRUCT_MAP(SkColorSpacePrimaries, sk_colorspace_primaries_t, ColorSpacePrimaries)
DEF_STRUCT_MAP(SkHighContrastConfig, sk_highcontrastconfig_t, HighContrastConfig)
DEF_STRUCT_MAP(SkIPoint, sk_ipoint_t, IPoint)
DEF_STRUCT_MAP(SkIRect, sk_irect_t, IRect)
DEF_STRUCT_MAP(SkISize, sk_isize_t, ISize)
DEF_STRUCT_MAP(SkMask, sk_mask_t, Mask)
DEF_STRUCT_MAP(SkPoint, sk_point_t, Point)
DEF_STRUCT_MAP(SkPoint3, sk_point3_t, Point3)
DEF_STRUCT_MAP(SkRect, sk_rect_t, Rect)
DEF_STRUCT_MAP(SkSize, sk_size_t, Size)

DEF_STRUCT_MAP(GrGLTextureInfo, gr_gl_textureinfo_t, GrGLTextureInfo)
DEF_STRUCT_MAP(GrGLFramebufferInfo, gr_gl_framebufferinfo_t, GrGLFramebufferInfo)
DEF_STRUCT_MAP(GrGLInterface, gr_glinterface_t, GrGLInterface)

#include "include/core/SkCanvas.h"
DEF_MAP(SkCanvas::Lattice, sk_lattice_t, Lattice)

#include "include/codec/SkCodec.h"
DEF_MAP(SkCodec::FrameInfo, sk_codec_frameinfo_t, FrameInfo)
DEF_MAP(SkCodec::Options, sk_codec_options_t, CodecOptions)

#include "include/core/SkImageFilter.h"
DEF_MAP(SkImageFilter::CropRect, sk_imagefilter_croprect_t, ImageFilterCropRect)

#include "include/encode/SkJpegEncoder.h"
DEF_MAP(SkJpegEncoder::Options, sk_jpegencoder_options_t, JpegEncoderOptions)

#include "include/core/SkFontMetrics.h"
DEF_MAP(SkFontMetrics, sk_fontmetrics_t, FontMetrics)

#include "include/core/SkPath.h"
DEF_MAP(SkPath::Iter, sk_path_iterator_t, PathIter)
DEF_MAP(SkPath::RawIter, sk_path_rawiterator_t, PathRawIter)

#include "include/encode/SkPngEncoder.h"
DEF_MAP(SkPngEncoder::Options, sk_pngencoder_options_t, PngEncoderOptions)

#include "include/core/SkTime.h"
DEF_MAP(SkTime::DateTime, sk_time_datetime_t, TimeDateTime)

#include "include/encode/SkWebpEncoder.h"
DEF_MAP(SkWebpEncoder::Options, sk_webpencoder_options_t, WebpEncoderOptions)

static inline skcms_Matrix3x3 AsNamedGamut(sk_named_gamut_t gamut) {
    switch (gamut)
    {
        case SRGB_SK_NAMED_GAMUT:
            return SkNamedGamut::kSRGB;
        case ADOBE_RGB_SK_NAMED_GAMUT:
            return SkNamedGamut::kAdobeRGB;
        case DCIP3_D65_SK_NAMED_GAMUT:
            return SkNamedGamut::kDCIP3;
        case REC2020_SK_NAMED_GAMUT:
            return SkNamedGamut::kRec2020;
        default:
            SkASSERTF(false, "An unknown sk_named_gamut_t was provided.");
            return SkNamedGamut::kXYZ;
    }
}

static inline skcms_TransferFunction AsNamedTransferFn(sk_named_transfer_fn_t transfer) {
    switch (transfer)
    {
        case SRGB_SK_NAMED_TRANSFER_FN:
            return SkNamedTransferFn::kSRGB;
        case TWO_DOT_TWO_SK_NAMED_TRANSFER_FN:
            return SkNamedTransferFn::k2Dot2;
        case LINEAR_SK_NAMED_TRANSFER_FN:
            return SkNamedTransferFn::kLinear;
        case REC2020_SK_NAMED_TRANSFER_FN:
            return SkNamedTransferFn::kRec2020;
        default:
            SkASSERTF(false, "An unknown sk_named_transfer_fn_t was provided.");
            return SkNamedTransferFn::kLinear;
    }
}

#include "include/core/SkMatrix.h"
static inline SkMatrix AsMatrix(const sk_matrix_t* matrix) {
    return SkMatrix::MakeAll(
        matrix->mat[0], matrix->mat[1], matrix->mat[2],
        matrix->mat[3], matrix->mat[4], matrix->mat[5],
        matrix->mat[6], matrix->mat[7], matrix->mat[8]);
}
static inline sk_matrix_t ToMatrix(const SkMatrix* matrix) {
    sk_matrix_t m;
    matrix->get9(m.mat);
    return m;
}

#include "include/core/SkImageInfo.h"
static inline SkImageInfo AsImageInfo(const sk_imageinfo_t* info) {
    return SkImageInfo::Make(
        info->width,
        info->height,
        (SkColorType)info->colorType,
        (SkAlphaType)info->alphaType,
        sk_ref_sp(AsColorSpace(info->colorspace))); 
}
static inline sk_imageinfo_t ToImageInfo(const SkImageInfo info) {
    return {
        ToColorSpace(info.refColorSpace().release()),
        info.width(),
        info.height(),
        (sk_colortype_t)info.colorType(),
        (sk_alphatype_t)info.alphaType(),
    };
}
static inline const SkImageInfo* ToImageInfo(const sk_imageinfo_t* cinfo) {
    return reinterpret_cast<const SkImageInfo*>(cinfo);
}
static inline sk_imageinfo_t* AsImageInfo(SkImageInfo* info) {
    return reinterpret_cast<sk_imageinfo_t*>(info);
}

#include "include/core/SkTextBlob.h"
static inline SkTextBlobBuilder::RunBuffer AsTextBlobBuilderRunBuffer(const sk_textblob_builder_runbuffer_t* runbuffer) {
    return {
        (SkGlyphID*)runbuffer->glyphs,
        (SkScalar*)runbuffer->pos,
        (char*)runbuffer->utf8text,
        (uint32_t*)runbuffer->clusters,
    };
}
static inline sk_textblob_builder_runbuffer_t ToTextBlobBuilderRunBuffer(const SkTextBlobBuilder::RunBuffer runbuffer) {
    return {
        runbuffer.glyphs,
        runbuffer.pos,
        runbuffer.utf8text,
        runbuffer.clusters,
    };
}

#include "include/docs/SkPDFDocument.h"
static inline SkTime::DateTime AsDocumentOptionalTimestamp(const sk_time_datetime_t* datetime) {
    if (datetime) {
        return *AsTimeDateTime(datetime);
    } else {
        return SkTime::DateTime();
    }
}
static inline SkString AsDocumentOptionalString(const sk_string_t* skstring) {
    if (skstring) {
        return *AsString(skstring);
    } else {
        return SkString();
    }
}
static inline SkPDF::Metadata AsDocumentPDFMetadata(const sk_document_pdf_metadata_t* metadata) {
    SkPDF::Metadata md;
    md.fTitle = AsDocumentOptionalString(metadata->fTitle);
    md.fAuthor = AsDocumentOptionalString(metadata->fAuthor);
    md.fSubject = AsDocumentOptionalString(metadata->fSubject);
    md.fKeywords = AsDocumentOptionalString(metadata->fKeywords);
    md.fCreator = AsDocumentOptionalString(metadata->fCreator);
    md.fProducer = AsDocumentOptionalString(metadata->fProducer);
    md.fCreation =  AsDocumentOptionalTimestamp(metadata->fCreation),
    md.fModified =  AsDocumentOptionalTimestamp(metadata->fModified),
    md.fRasterDPI = metadata->fRasterDPI;
    md.fPDFA = metadata->fPDFA;
    md.fEncodingQuality = metadata->fEncodingQuality;
    return md;
}

#endif
