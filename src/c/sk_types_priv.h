/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_types_priv_DEFINED
#define sk_types_priv_DEFINED

#include "sk_types.h"


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
DEF_CLASS_MAP(SkDynamicMemoryWStream, sk_wstream_dynamicmemorystream_t, DynamicMemoryWStream)
DEF_CLASS_MAP(SkFILEStream, sk_stream_filestream_t, FileStream)
DEF_CLASS_MAP(SkFILEWStream, sk_wstream_filestream_t, FileWStream)
DEF_CLASS_MAP(SkFontMgr, sk_fontmgr_t, FontMgr)
DEF_CLASS_MAP(SkImage, sk_image_t, Image)
DEF_CLASS_MAP(SkImageFilter, sk_imagefilter_t, ImageFilter)
DEF_CLASS_MAP(SkMaskFilter, sk_maskfilter_t, MaskFilter)
DEF_CLASS_MAP(SkMatrix44, sk_matrix44_t, Matrix44)
DEF_CLASS_MAP(SkMemoryStream, sk_stream_memorystream_t, MemoryStream)
DEF_CLASS_MAP(SkNWayCanvas, sk_nway_canvas_t, NWayCanvas)
DEF_CLASS_MAP(SkNoDrawCanvas, sk_nodraw_canvas_t, NoDrawCanvas)
DEF_CLASS_MAP(SkOpBuilder, sk_opbuilder_t, OpBuilder)
DEF_CLASS_MAP(SkPaint, sk_paint_t, Paint)
DEF_CLASS_MAP(SkPath, sk_path_t, Path)
DEF_CLASS_MAP(SkPathEffect, sk_path_effect_t, PathEffect)
DEF_CLASS_MAP(SkPathMeasure, sk_pathmeasure_t, PathMeasure)
DEF_CLASS_MAP(SkPicture, sk_picture_t, Picture)
DEF_CLASS_MAP(SkPictureRecorder, sk_picture_recorder_t, PictureRecorder)
DEF_CLASS_MAP(SkPixelSerializer, sk_pixelserializer_t, PixelSerializer)
DEF_CLASS_MAP(SkPixmap, sk_pixmap_t, Pixmap)
DEF_CLASS_MAP(SkRegion, sk_region_t, Region)
DEF_CLASS_MAP(SkShader, sk_shader_t, Shader)
DEF_CLASS_MAP(SkStream, sk_stream_t, Stream)
DEF_CLASS_MAP(SkStreamAsset, sk_stream_asset_t, StreamAsset)
DEF_CLASS_MAP(SkStreamRewindable, sk_stream_streamrewindable_t, StreamRewindable)
DEF_CLASS_MAP(SkString, sk_string_t, String)
DEF_CLASS_MAP(SkSurface, sk_surface_t, Surface)
DEF_CLASS_MAP(SkTypeface, sk_typeface_t, Typeface)
DEF_CLASS_MAP(SkVertices, sk_vertices_t, Vertices)
DEF_CLASS_MAP(SkWStream, sk_wstream_t, WStream)
DEF_CLASS_MAP(SkXMLStreamWriter, sk_xmlstreamwriter_t, XMLStreamWriter)
DEF_CLASS_MAP(SkXMLWriter, sk_xmlwriter_t, XMLWriter)

DEF_CLASS_MAP(GrContext, gr_context_t, GrContext)

DEF_STRUCT_MAP(SkColorSpacePrimaries, sk_colorspaceprimaries_t, ColorSpacePrimaries)
DEF_STRUCT_MAP(SkColorSpaceTransferFn, sk_colorspace_transfer_fn_t, ColorSpaceTransferFn)
DEF_STRUCT_MAP(SkEncodedInfo, sk_encodedinfo_t, EncodedInfo)
DEF_STRUCT_MAP(SkHighContrastConfig, sk_highcontrastconfig_t, HighContrastConfig)
DEF_STRUCT_MAP(SkIPoint, sk_ipoint_t, IPoint)
DEF_STRUCT_MAP(SkIRect, sk_irect_t, IRect)
DEF_STRUCT_MAP(SkISize, sk_isize_t, ISize)
DEF_STRUCT_MAP(SkMask, sk_mask_t, Mask)
DEF_STRUCT_MAP(SkPoint, sk_point_t, Point)
DEF_STRUCT_MAP(SkPoint3, sk_point3_t, Point3)
DEF_STRUCT_MAP(SkRect, sk_rect_t, Rect)
DEF_STRUCT_MAP(SkSize, sk_size_t, Size)

DEF_STRUCT_MAP(GrBackendRenderTargetDesc, gr_backend_rendertarget_desc_t, GrBackendRenderTargetDesc)
DEF_STRUCT_MAP(GrBackendTextureDesc, gr_backend_texture_desc_t, GrBackendTextureDesc)
DEF_STRUCT_MAP(GrContextOptions, gr_context_options_t, GrContextOptions)
DEF_STRUCT_MAP(GrGLInterface, gr_glinterface_t, GrGLInterface)

#include "SkCanvas.h"
DEF_MAP(SkCanvas::Lattice, sk_lattice_t, Lattice)

#include "SkCodec.h"
DEF_MAP(SkCodec::FrameInfo, sk_codec_frameinfo_t, FrameInfo)
DEF_MAP(SkCodec::Options, sk_codec_options_t, CodecOptions)

#include "SkImageFilter.h"
DEF_MAP(SkImageFilter::CropRect, sk_imagefilter_croprect_t, ImageFilterCropRect)

#include "SkJpegEncoder.h"
DEF_MAP(SkJpegEncoder::Options, sk_jpegencoder_options_t, JpegEncoderOptions)

#include "SkPaint.h"
DEF_MAP(SkPaint::FontMetrics, sk_fontmetrics_t, FontMetrics)

#include "SkPath.h"
DEF_MAP(SkPath::Iter, sk_path_iterator_t, PathIter)
DEF_MAP(SkPath::RawIter, sk_path_rawiterator_t, PathRawIter)

#include "SkPngEncoder.h"
DEF_MAP(SkPngEncoder::Options, sk_pngencoder_options_t, PngEncoderOptions)

#include "SkTime.h"
DEF_MAP(SkTime::DateTime, sk_time_datetime_t, TimeDateTime)

#include "SkWebpEncoder.h"
DEF_MAP(SkWebpEncoder::Options, sk_webpencoder_options_t, WebpEncoderOptions)


#include "SkMatrix.h"
static inline void from_c(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(
        cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
        cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
        cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}
static inline void from_sk(const SkMatrix* matrix, sk_matrix_t* cmatrix) {
    matrix->get9(cmatrix->mat);
}

#include "SkImageInfo.h"
static inline void from_c(const sk_imageinfo_t& cinfo, SkImageInfo* info) {
    if (info) { 
        *info = SkImageInfo::Make(
            cinfo.width,
            cinfo.height,
            (SkColorType)cinfo.colorType,
            (SkAlphaType)cinfo.alphaType,
            sk_ref_sp(AsColorSpace(cinfo.colorspace))); 
    } 
} 
static inline void from_sk(const SkImageInfo& info, sk_imageinfo_t* cinfo) {
    if (cinfo) { 
        *cinfo = {
            ToColorSpace(info.refColorSpace().release()),
            info.width(),
            info.height(),
            (sk_colortype_t)info.colorType(),
            (sk_alphatype_t)info.alphaType(),
        }; 
    } 
} 

#include "SkDocument.h"
static inline void from_c(const sk_document_pdf_metadata_t& cmetadata, SkDocument::PDFMetadata* metadata) {
    if (metadata) {
        SkDocument::PDFMetadata md;
        if (cmetadata.fTitle) md.fTitle = AsString(*cmetadata.fTitle);
        if (cmetadata.fAuthor) md.fAuthor = AsString(*cmetadata.fAuthor);
        if (cmetadata.fSubject) md.fSubject = AsString(*cmetadata.fSubject);
        if (cmetadata.fKeywords) md.fKeywords = AsString(*cmetadata.fKeywords);
        if (cmetadata.fCreator) md.fCreator = AsString(*cmetadata.fCreator);
        if (cmetadata.fProducer) md.fProducer = AsString(*cmetadata.fProducer);
        if (cmetadata.fCreation) {
            md.fCreation.fEnabled = true;
            md.fCreation.fDateTime = AsTimeDateTime(*cmetadata.fCreation);
        }
        if (cmetadata.fModified) {
            md.fModified.fEnabled = true;
            md.fModified.fDateTime = AsTimeDateTime(*cmetadata.fModified);
        }
        *metadata = md;
    } 
} 

#include "SkSurfaceProps.h"
static inline void from_c(const sk_surfaceprops_t* cprops, SkSurfaceProps* props) {
    *props = SkSurfaceProps(cprops->flags, (SkPixelGeometry)cprops->pixelGeometry);
}
static inline void from_sk(const SkSurfaceProps* props, sk_surfaceprops_t* cprops) {
    *cprops = {
        (sk_pixelgeometry_t)props->pixelGeometry(),
        (sk_surfaceprops_flags_t)props->flags()
    };
}

#endif
