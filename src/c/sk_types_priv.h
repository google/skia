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
#include "SkCodec.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "Sk1DPathEffect.h"
#include "SkFontStyle.h"

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

static inline SkIRect* AsIRect(sk_irect_t* crect) {
    return reinterpret_cast<SkIRect*>(crect);
}

static inline const SkIRect* AsIRect(const sk_irect_t* crect) {
    return reinterpret_cast<const SkIRect*>(crect);
}

static inline const SkIRect& AsIRect(const sk_irect_t& crect) {
    return reinterpret_cast<const SkIRect&>(crect);
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

static inline SkTypeface* AsTypeface(sk_typeface_t* typeface) {
    return reinterpret_cast<SkTypeface*>(typeface);
}

static inline sk_typeface_t* ToTypeface(SkTypeface* typeface) {
    return reinterpret_cast<sk_typeface_t*>(typeface);
}

static inline sk_colorspace_t* ToColorSpace(SkColorSpace* colorspace) {
    return reinterpret_cast<sk_colorspace_t*>(colorspace);
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

static inline SkString* AsString(const sk_string_t* cdata) {
    return reinterpret_cast<SkString*>(const_cast<sk_string_t*>(cdata));
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

static inline SkImageInfo* AsImageInfo(sk_imageinfo_t* cinfo) {
    return reinterpret_cast<SkImageInfo*>(cinfo);
}
static inline SkPath::Iter* AsPathIter(sk_path_iterator_t* iter) {
    return reinterpret_cast<SkPath::Iter*>(iter);
}

static inline const SkImageInfo* AsImageInfo(const sk_imageinfo_t* cinfo) {
    return reinterpret_cast<const SkImageInfo*>(cinfo);
}

static inline sk_imageinfo_t* ToImageInfo(SkImageInfo* info) {
    return reinterpret_cast<sk_imageinfo_t*>(info);
}

static inline sk_imageinfo_t& ToImageInfo(SkImageInfo& info) {
    return reinterpret_cast<sk_imageinfo_t&>(info);
}

static inline const sk_imageinfo_t* ToImageInfo(const SkImageInfo* info) {
    return reinterpret_cast<const sk_imageinfo_t*>(info);
}

static inline const sk_imageinfo_t& ToImageInfo(const SkImageInfo& info) {
    return reinterpret_cast<const sk_imageinfo_t&>(info);
}

static inline bool find_sk(const sk_codec_options_t& coptions, SkCodec::Options* options) {
    if (options) {
        *options = SkCodec::Options();
        options->fZeroInitialized = (SkCodec::ZeroInitialized)coptions.fZeroInitialized;
        if (coptions.fHasSubset) {
            options->fSubset = AsIRect((sk_irect_t*)&coptions.fSubset);
        }
    }
    return true;
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

#endif
