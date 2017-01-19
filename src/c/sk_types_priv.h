/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_types_priv_DEFINED
#define sk_types_priv_DEFINED

#include "SkImageInfo.h"
#include "SkBlurTypes.h"
#include "SkDocument.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkCodec.h"
#include "SkPicture.h"
#include "SkPixmap.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMask.h"
#include "Sk1DPathEffect.h"
#include "SkFontStyle.h"
#include "SkXMLWriter.h"
#include "GrContext.h"
#include "SkPathOps.h"
#include "SkRegion.h"
#include "SkTypeface.h"
#include "SkEncodedInfo.h"
#include "SkTime.h"
#include "SkCamera.h"
#include "gl/GrGLInterface.h"

#include "sk_path.h"
#include "sk_paint.h"
#include "sk_shader.h"
#include "sk_maskfilter.h"

#include "sk_types.h"

class SkMaskFilter;
class SkPaint;
class SkShader;

static inline const SkPaint& AsPaint(const sk_paint_t& cpaint) {
    return reinterpret_cast<const SkPaint&>(cpaint);
}

static inline const SkPaint* AsPaint(const sk_paint_t* cpaint) {
    return reinterpret_cast<const SkPaint*>(cpaint);
}

static inline SkPaint* AsPaint(sk_paint_t* cpaint) {
    return reinterpret_cast<SkPaint*>(cpaint);
}

static inline SkMaskFilter* AsMaskFilter(sk_maskfilter_t* cfilter) {
    return reinterpret_cast<SkMaskFilter*>(cfilter);
}

static inline sk_maskfilter_t* ToMaskFilter(SkMaskFilter* filter) {
    return reinterpret_cast<sk_maskfilter_t*>(filter);
}

static inline SkShader* AsShader(sk_shader_t* cshader) {
    return reinterpret_cast<SkShader*>(cshader);
}

static inline SkRect* AsRect(sk_rect_t* crect) {
    return reinterpret_cast<SkRect*>(crect);
}

static inline const SkRect* AsRect(const sk_rect_t* crect) {
    return reinterpret_cast<const SkRect*>(crect);
}

static inline const SkRect& AsRect(const sk_rect_t& crect) {
    return reinterpret_cast<const SkRect&>(crect);
}

static inline sk_rect_t ToRect(const SkRect& rect) {
    return reinterpret_cast<const sk_rect_t&>(rect);
}

static inline sk_irect_t ToIRect(const SkIRect& rect) {
    return reinterpret_cast<const sk_irect_t&>(rect);
}

static inline SkIRect* AsIRect(sk_irect_t* crect) {
    return reinterpret_cast<SkIRect*>(crect);
}

static inline const SkIRect* AsIRect(const sk_irect_t* crect) {
    return reinterpret_cast<const SkIRect*>(crect);
}

static inline const SkIRect& AsIRect(const sk_irect_t& crect) {
    return reinterpret_cast<const SkIRect&>(crect);
}

static inline SkRegion* AsRegion(sk_region_t* creg) {
    return reinterpret_cast<SkRegion*>(creg);
}

static inline SkRegion& AsRegion(sk_region_t& creg) {
    return reinterpret_cast<SkRegion&>(creg);
}

static inline const SkRegion* AsRegion(const sk_region_t* creg) {
    return reinterpret_cast<const SkRegion*>(creg);
}

static inline const SkRegion& AsRegion(const sk_region_t& creg) {
    return reinterpret_cast<const SkRegion&>(creg);
}

static inline sk_region_t* ToRegion(SkRegion* reg) {
    return reinterpret_cast<sk_region_t*>(reg);
}

static inline const SkBitmap* AsBitmap(const sk_bitmap_t* cbitmap) {
    return reinterpret_cast<const SkBitmap*>(cbitmap);
}

static inline const SkBitmap& AsBitmap(const sk_bitmap_t& cbitmap) {
    return reinterpret_cast<const SkBitmap&>(cbitmap);
}

static inline SkBitmap* AsBitmap(sk_bitmap_t* cbitmap) {
    return reinterpret_cast<SkBitmap*>(cbitmap);
}

static inline const SkPixmap* AsPixmap(const sk_pixmap_t* cpixmap) {
    return reinterpret_cast<const SkPixmap*>(cpixmap);
}

static inline const SkPixmap& AsPixmap(const sk_pixmap_t& cpixmap) {
    return reinterpret_cast<const SkPixmap&>(cpixmap);
}

static inline SkPixmap* AsPixmap(sk_pixmap_t* cpixmap) {
    return reinterpret_cast<SkPixmap*>(cpixmap);
}

static inline SkPixmap& AsPixmap(sk_pixmap_t& cpixmap) {
    return reinterpret_cast<SkPixmap&>(cpixmap);
}

static inline sk_pixmap_t* ToPixmap(SkPixmap* pixmap) {
    return reinterpret_cast<sk_pixmap_t*>(pixmap);
}

static inline SkMask* AsMask(sk_mask_t* cmask) {
    return reinterpret_cast<SkMask*>(cmask);
}

static inline const SkMask* AsMask(const sk_mask_t* cmask) {
    return reinterpret_cast<const SkMask*>(cmask);
}

static inline sk_mask_t* ToMask(SkMask* mask) {
    return reinterpret_cast<sk_mask_t*>(mask);
}

static inline SkData* AsData(const sk_data_t* cdata) {
    return reinterpret_cast<SkData*>(const_cast<sk_data_t*>(cdata));
}

static inline sk_data_t* ToData(SkData* data) {
    return reinterpret_cast<sk_data_t*>(data);
}

static inline sk_path_t* ToPath(SkPath* cpath) {
    return reinterpret_cast<sk_path_t*>(cpath);
}

static inline const SkPath& AsPath(const sk_path_t& cpath) {
    return reinterpret_cast<const SkPath&>(cpath);
}

static inline SkPath* AsPath(sk_path_t* cpath) {
    return reinterpret_cast<SkPath*>(cpath);
}

static inline const SkPath* AsPath(const sk_path_t* cpath) {
    return reinterpret_cast<const SkPath*>(cpath);
}

static inline const SkImage* AsImage(const sk_image_t* cimage) {
    return reinterpret_cast<const SkImage*>(cimage);
}

static inline sk_image_t* ToImage(SkImage* cimage) {
    return reinterpret_cast<sk_image_t*>(cimage);
}

static inline sk_canvas_t* ToCanvas(SkCanvas* canvas) {
    return reinterpret_cast<sk_canvas_t*>(canvas);
}

static inline SkCanvas* AsCanvas(sk_canvas_t* ccanvas) {
    return reinterpret_cast<SkCanvas*>(ccanvas);
}

static inline SkPictureRecorder* AsPictureRecorder(sk_picture_recorder_t* crec) {
    return reinterpret_cast<SkPictureRecorder*>(crec);
}

static inline sk_picture_recorder_t* ToPictureRecorder(SkPictureRecorder* rec) {
    return reinterpret_cast<sk_picture_recorder_t*>(rec);
}

static inline const SkPicture* AsPicture(const sk_picture_t* cpic) {
    return reinterpret_cast<const SkPicture*>(cpic);
}

static inline SkPicture* AsPicture(sk_picture_t* cpic) {
    return reinterpret_cast<SkPicture*>(cpic);
}

static inline sk_picture_t* ToPicture(SkPicture* pic) {
    return reinterpret_cast<sk_picture_t*>(pic);
}

static inline SkImageFilter* AsImageFilter(sk_imagefilter_t* cfilter) {
    return reinterpret_cast<SkImageFilter*>(cfilter);
}

static inline SkImageFilter** AsImageFilters(sk_imagefilter_t** cfilter) {
    return reinterpret_cast<SkImageFilter**>(cfilter);
}

static inline sk_imagefilter_t* ToImageFilter(SkImageFilter* filter) {
    return reinterpret_cast<sk_imagefilter_t*>(filter);
}

static inline SkColorFilter* AsColorFilter(sk_colorfilter_t* cfilter) {
    return reinterpret_cast<SkColorFilter*>(cfilter);
}

static inline sk_colorfilter_t* ToColorFilter(SkColorFilter* filter) {
    return reinterpret_cast<sk_colorfilter_t*>(filter);
}

static inline const SkCodec* AsCodec(const sk_codec_t* codec) {
    return reinterpret_cast<const SkCodec*>(codec);
}

static inline const SkCodec& AsCodec(const sk_codec_t& codec) {
    return reinterpret_cast<const SkCodec&>(codec);
}

static inline SkCodec* AsCodec(sk_codec_t* codec) {
    return reinterpret_cast<SkCodec*>(codec);
}

static inline sk_codec_t* ToCodec(SkCodec* codec) {
    return reinterpret_cast<sk_codec_t*>(codec);
}

static inline const SkCodec::Options* AsCodecOptions(const sk_codec_options_t* t) {
    return reinterpret_cast<const SkCodec::Options*>(t);
}

static inline SkTypeface* AsTypeface(sk_typeface_t* typeface) {
    return reinterpret_cast<SkTypeface*>(typeface);
}

static inline sk_typeface_t* ToTypeface(SkTypeface* typeface) {
    return reinterpret_cast<sk_typeface_t*>(typeface);
}

static inline sk_colorspace_t* ToColorSpace(SkColorSpace* colorspace) {
    return reinterpret_cast<sk_colorspace_t*>(colorspace);
}

static inline sk_shader_t* ToShader(SkShader* shader) {
    return reinterpret_cast<sk_shader_t*>(shader);
}

static inline const SkFILEStream* AsFileStream(const sk_stream_filestream_t* cfilestream) {
    return reinterpret_cast<const SkFILEStream*>(cfilestream);
}

static inline SkFILEStream* AsFileStream(sk_stream_filestream_t* cfilestream) {
    return reinterpret_cast<SkFILEStream*>(cfilestream);
}

static inline sk_stream_filestream_t* ToFileStream(SkFILEStream* stream) {
    return reinterpret_cast<sk_stream_filestream_t*>(stream);
}

static inline const SkMemoryStream* AsMemoryStream(const sk_stream_memorystream_t* cmemorystream) {
    return reinterpret_cast<const SkMemoryStream*>(cmemorystream);
}

static inline SkMemoryStream* AsMemoryStream(sk_stream_memorystream_t* cmemorystream) {
    return reinterpret_cast<SkMemoryStream*>(cmemorystream);
}

static inline sk_stream_memorystream_t* ToMemoryStream(SkMemoryStream* stream) {
    return reinterpret_cast<sk_stream_memorystream_t*>(stream);
}

static inline SkStreamRewindable* AsStreamRewindable(sk_stream_streamrewindable_t* cstreamrewindable) {
    return reinterpret_cast<SkStreamRewindable*>(cstreamrewindable);
}

static inline const SkStream* AsStream(const sk_stream_t* cstream) {
    return reinterpret_cast<const SkStream*>(cstream);
}

static inline SkStream* AsStream(sk_stream_t* cstream) {
    return reinterpret_cast<SkStream*>(cstream);
}

static inline sk_stream_t* ToStream(SkStream* cstream) {
    return reinterpret_cast<sk_stream_t*>(cstream);
}

static inline sk_stream_asset_t* ToStreamAsset(SkStreamAsset* cstream) {
    return reinterpret_cast<sk_stream_asset_t*>(cstream);
}

static inline SkStreamAsset* AsStreamAsset(sk_stream_asset_t* cstream) {
    return reinterpret_cast<SkStreamAsset*>(cstream);
}

static inline SkFILEWStream* AsFileWStream(sk_wstream_filestream_t* cfilestream) {
    return reinterpret_cast<SkFILEWStream*>(cfilestream);
}

static inline SkDynamicMemoryWStream* AsDynamicMemoryWStream(sk_wstream_dynamicmemorystream_t* cmemorystream) {
    return reinterpret_cast<SkDynamicMemoryWStream*>(cmemorystream);
}

static inline SkWStream* AsWStream(sk_wstream_t* cstream) {
    return reinterpret_cast<SkWStream*>(cstream);
}

static inline sk_wstream_filestream_t* ToFileWStream(SkFILEWStream* filestream) {
    return reinterpret_cast<sk_wstream_filestream_t*>(filestream);
}

static inline sk_wstream_dynamicmemorystream_t* ToDynamicMemoryWStream(SkDynamicMemoryWStream* memorystream) {
    return reinterpret_cast<sk_wstream_dynamicmemorystream_t*>(memorystream);
}

static inline sk_wstream_t* ToWStream(SkWStream* stream) {
    return reinterpret_cast<sk_wstream_t*>(stream);
}

static inline const SkPoint& AsPoint(const sk_point_t& p) {
    return reinterpret_cast<const SkPoint&>(p);
}

static inline const SkPoint* AsPoint(const sk_point_t* p) {
    return reinterpret_cast<const SkPoint*>(p);
}

static inline SkPoint* AsPoint(sk_point_t* p) {
    return reinterpret_cast<SkPoint*>(p);
}

static inline sk_point_t* ToPoint(SkPoint *p) {
    return reinterpret_cast<sk_point_t*>(p);
}

static inline sk_point_t& ToPoint(SkPoint &p) {
    return reinterpret_cast<sk_point_t&>(p);
}

static inline const SkIPoint& AsIPoint(const sk_ipoint_t& p) {
    return reinterpret_cast<const SkIPoint&>(p);
}

static inline const SkIPoint* AsIPoint(const sk_ipoint_t* p) {
    return reinterpret_cast<const SkIPoint*>(p);
}

static inline const SkSize& AsSize(const sk_size_t& p) {
    return reinterpret_cast<const SkSize&>(p);
}

static inline const SkSize* AsSize(const sk_size_t* p) {
    return reinterpret_cast<const SkSize*>(p);
}

static inline const SkISize& AsISize(const sk_isize_t& p) {
    return reinterpret_cast<const SkISize&>(p);
}

static inline const SkISize* AsISize(const sk_isize_t* p) {
    return reinterpret_cast<const SkISize*>(p);
}

static inline SkISize* AsISize(sk_isize_t* p) {
    return reinterpret_cast<SkISize*>(p);
}

static inline const sk_isize_t& ToISize(const SkISize& p) {
    return reinterpret_cast<const sk_isize_t&>(p);
}

static inline const sk_isize_t* ToISize(const SkISize* p) {
    return reinterpret_cast<const sk_isize_t*>(p);
}

static inline const SkPoint3& AsPoint3(const sk_point3_t& p) {
    return reinterpret_cast<const SkPoint3&>(p);
}

static inline const SkPoint3* AsPoint3(const sk_point3_t* p) {
    return reinterpret_cast<const SkPoint3*>(p);
}

static inline const SkImageFilter::CropRect& AsImageFilterCropRect(const sk_imagefilter_croprect_t& p) {
    return reinterpret_cast<const SkImageFilter::CropRect&>(p);
}

static inline const SkImageFilter::CropRect* AsImageFilterCropRect(const sk_imagefilter_croprect_t* p) {
    return reinterpret_cast<const SkImageFilter::CropRect*>(p);
}

static inline SkPaint::FontMetrics* AsFontMetrics(sk_fontmetrics_t* p) {
    return reinterpret_cast<SkPaint::FontMetrics*>(p);
}

static inline sk_fontmetrics_t* ToFontMetrics(SkPaint::FontMetrics* p) {
    return reinterpret_cast<sk_fontmetrics_t*>(p);
}

static inline const SkString* AsString(const sk_string_t* str) {
    return reinterpret_cast<const SkString*>(str);
}

static inline SkString* AsString(sk_string_t* str) {
    return reinterpret_cast<SkString*>(str);
}

static inline SkString& AsString(sk_string_t& str) {
    return reinterpret_cast<SkString&>(str);
}

static inline sk_string_t* ToString(SkString* data) {
    return reinterpret_cast<sk_string_t*>(data);
}

static inline SkDocument* AsDocument(sk_document_t* cdocument) {
    return reinterpret_cast<SkDocument*>(cdocument);
}

static inline sk_document_t* ToDocument(SkDocument* document) {
    return reinterpret_cast<sk_document_t*>(document);
}

static inline SkPath::Iter* AsPathIter(sk_path_iterator_t* iter) {
    return reinterpret_cast<SkPath::Iter*>(iter);
}

static inline sk_path_iterator_t* ToPathIter(SkPath::Iter* iter) {
    return reinterpret_cast<sk_path_iterator_t*>(iter);
}

static inline SkPath::RawIter* AsPathRawIter(sk_path_rawiterator_t* iter) {
    return reinterpret_cast<SkPath::RawIter*>(iter);
}

static inline sk_path_rawiterator_t* ToPathRawIter(SkPath::RawIter* iter) {
    return reinterpret_cast<sk_path_rawiterator_t*>(iter);
}

static inline const SkPathEffect* AsPathEffect(const sk_path_effect_t* p) {
    return reinterpret_cast<const SkPathEffect*>(p);
}

static inline SkPathEffect* AsPathEffect(sk_path_effect_t* p) {
    return reinterpret_cast<SkPathEffect*>(p);
}

static inline sk_path_effect_t* ToPathEffect(SkPathEffect* p) {
    return reinterpret_cast<sk_path_effect_t*>(p);
}

static inline const sk_path_effect_t* ToPathEffect(const SkPathEffect* p) {
    return reinterpret_cast<const sk_path_effect_t*>(p);
}

static inline const SkColorTable* AsColorTable(const sk_colortable_t* p) {
    return reinterpret_cast<const SkColorTable*>(p);
}

static inline SkColorTable* AsColorTable(sk_colortable_t* p) {
    return reinterpret_cast<SkColorTable*>(p);
}

static inline sk_colortable_t* ToColorTable(SkColorTable* p) {
    return reinterpret_cast<sk_colortable_t*>(p);
}

static inline const sk_colortable_t* ToColorTable(const SkColorTable* p) {
    return reinterpret_cast<const sk_colortable_t*>(p);
}

static inline const SkPixelRefFactory* AsPixelRefFactory(const sk_pixelref_factory_t* p) {
    return reinterpret_cast<const SkPixelRefFactory*>(p);
}

static inline SkPixelRefFactory* AsPixelRefFactory(sk_pixelref_factory_t* p) {
    return reinterpret_cast<SkPixelRefFactory*>(p);
}

static inline sk_pixelref_factory_t* ToColorTable(SkPixelRefFactory* p) {
    return reinterpret_cast<sk_pixelref_factory_t*>(p);
}

static inline const sk_pixelref_factory_t* ToColorTable(const SkPixelRefFactory* p) {
    return reinterpret_cast<const sk_pixelref_factory_t*>(p);
}

static inline sk_surface_t* ToSurface(SkSurface* p) {
    return reinterpret_cast<sk_surface_t*>(p);
}

static inline SkSurface* AsSurface(sk_surface_t* p) {
    return reinterpret_cast<SkSurface*>(p);
}

static inline gr_context_t* ToGrContext(GrContext* p) {
    return reinterpret_cast<gr_context_t*>(p);
}

static inline GrContext* AsGrContext(gr_context_t* p) {
    return reinterpret_cast<GrContext*>(p);
}
static inline const GrContextOptions& AsGrContextOptions(const gr_context_options_t& p) {
    return reinterpret_cast<const GrContextOptions&>(p);
}

static inline const GrBackendRenderTargetDesc& AsGrBackendRenderTargetDesc(const gr_backend_rendertarget_desc_t& p) {
    return reinterpret_cast<const GrBackendRenderTargetDesc&>(p);
}

static inline const GrBackendTextureDesc& AsGrBackendTextureDesc(const gr_backend_texture_desc_t& p) {
    return reinterpret_cast<const GrBackendTextureDesc&>(p);
}


static inline gr_glinterface_t* ToGrGLInterface(GrGLInterface* p) {
    return reinterpret_cast<gr_glinterface_t*>(p);
}

static inline GrGLInterface* AsGrGLInterface(gr_glinterface_t* p) {
    return reinterpret_cast<GrGLInterface*>(p);
}

static inline const gr_glinterface_t* ToGrGLInterface(const GrGLInterface* p) {
    return reinterpret_cast<const gr_glinterface_t*>(p);
}

static inline const GrGLInterface* AsGrGLInterface(const gr_glinterface_t* p) {
    return reinterpret_cast<const GrGLInterface*>(p);
}

static inline sk_opbuilder_t* ToOpBuilder(SkOpBuilder* p) {
    return reinterpret_cast<sk_opbuilder_t*>(p);
}

static inline SkOpBuilder* AsOpBuilder(sk_opbuilder_t* p) {
    return reinterpret_cast<SkOpBuilder*>(p);
}

static inline const SkCanvas::Lattice& AsLattice(const sk_lattice_t& p) {
    return reinterpret_cast<const SkCanvas::Lattice&>(p);
}

static inline const SkTime::DateTime& AsTimeDateTime(const sk_time_datetime_t& p) {
    return reinterpret_cast<const SkTime::DateTime&>(p);
}

static inline sk_pathmeasure_t* ToPathMeasure(SkPathMeasure* p) {
    return reinterpret_cast<sk_pathmeasure_t*>(p);
}

static inline SkPathMeasure* AsPathMeasure(sk_pathmeasure_t* p) {
    return reinterpret_cast<SkPathMeasure*>(p);
}

static inline sk_encodedinfo_t* ToEncodedInfo(SkEncodedInfo *p) {
    return reinterpret_cast<sk_encodedinfo_t*>(p);
}

static inline sk_encodedinfo_t& ToEncodedInfo(SkEncodedInfo &p) {
    return reinterpret_cast<sk_encodedinfo_t&>(p);
}

static inline const sk_encodedinfo_t* ToEncodedInfo(const SkEncodedInfo *p) {
    return reinterpret_cast<const sk_encodedinfo_t*>(p);
}

static inline const sk_encodedinfo_t& ToEncodedInfo(const SkEncodedInfo &p) {
    return reinterpret_cast<const sk_encodedinfo_t&>(p);
}

static inline SkCodec::FrameInfo* AsFrameInfo(sk_codec_frameinfo_t *p) {
    return reinterpret_cast<SkCodec::FrameInfo*>(p);
}

static inline sk_codec_frameinfo_t* ToFrameInfo(SkCodec::FrameInfo *p) {
    return reinterpret_cast<sk_codec_frameinfo_t*>(p);
}

static inline sk_codec_frameinfo_t& ToFrameInfo(SkCodec::FrameInfo &p) {
    return reinterpret_cast<sk_codec_frameinfo_t&>(p);
}

static inline const sk_codec_frameinfo_t* ToFrameInfo(const SkCodec::FrameInfo *p) {
    return reinterpret_cast<const sk_codec_frameinfo_t*>(p);
}

static inline const sk_codec_frameinfo_t& ToFrameInfo(const SkCodec::FrameInfo &p) {
    return reinterpret_cast<const sk_codec_frameinfo_t&>(p);
}

static inline const SkXMLStreamWriter* AsXMLStreamWriter(const sk_xmlstreamwriter_t* p) {
    return reinterpret_cast<const SkXMLStreamWriter*>(p);
}

static inline SkXMLStreamWriter* AsXMLStreamWriter(sk_xmlstreamwriter_t* p) {
    return reinterpret_cast<SkXMLStreamWriter*>(p);
}

static inline sk_xmlstreamwriter_t* ToXMLStreamWriter(SkXMLStreamWriter* p) {
    return reinterpret_cast<sk_xmlstreamwriter_t*>(p);
}

static inline const sk_xmlstreamwriter_t* ToXMLStreamWriter(const SkXMLStreamWriter* p) {
    return reinterpret_cast<const sk_xmlstreamwriter_t*>(p);
}

static inline const SkXMLWriter* AsXMLWriter(const sk_xmlwriter_t* p) {
    return reinterpret_cast<const SkXMLWriter*>(p);
}

static inline SkXMLWriter* AsXMLWriter(sk_xmlwriter_t* p) {
    return reinterpret_cast<SkXMLWriter*>(p);
}

static inline sk_xmlwriter_t* ToXMLWriter(SkXMLWriter* p) {
    return reinterpret_cast<sk_xmlwriter_t*>(p);
}

static inline const sk_xmlwriter_t* ToXMLWriter(const SkXMLWriter* p) {
    return reinterpret_cast<const sk_xmlwriter_t*>(p);
}

static inline const Sk3DView* As3DView(const sk_3dview_t* p) {
    return reinterpret_cast<const Sk3DView*>(p);
}

static inline Sk3DView* As3DView(sk_3dview_t* p) {
    return reinterpret_cast<Sk3DView*>(p);
}

static inline sk_3dview_t* To3DView(Sk3DView* p) {
    return reinterpret_cast<sk_3dview_t*>(p);
}

static inline const sk_3dview_t* To3DView(const Sk3DView* p) {
    return reinterpret_cast<const sk_3dview_t*>(p);
}

static inline void from_c(const sk_matrix_t* cmatrix, SkMatrix* matrix) {
    matrix->setAll(
        cmatrix->mat[0], cmatrix->mat[1], cmatrix->mat[2],
        cmatrix->mat[3], cmatrix->mat[4], cmatrix->mat[5],
        cmatrix->mat[6], cmatrix->mat[7], cmatrix->mat[8]);
}

static inline void from_sk(const SkMatrix* matrix, sk_matrix_t* cmatrix) {
    matrix->get9(cmatrix->mat);
}

static inline bool from_c(const sk_imageinfo_t& cinfo, SkImageInfo* info) {
    if (info) { 
        *info = SkImageInfo::Make(
            cinfo.width,
            cinfo.height,
            (SkColorType)cinfo.colorType,
            (SkAlphaType)cinfo.alphaType); 
    } 
    return true; 
} 

static inline bool from_sk(const SkImageInfo& info, sk_imageinfo_t* cinfo) {
    if (cinfo) { 
        *cinfo = {
            info.width(),
            info.height(),
            (sk_colortype_t)info.colorType(),
            (sk_alphatype_t)info.alphaType()
        }; 
    } 
    return true; 
} 

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

static inline void from_c(const sk_surfaceprops_t* cprops, SkSurfaceProps* props) {
    *props = SkSurfaceProps(cprops->flags, (SkPixelGeometry)cprops->pixelGeometry);
}

#endif
