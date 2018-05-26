/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkImage.h"
#include "SkImageEncoder.h"

#include "sk_image.h"

#include "sk_types_priv.h"

void sk_image_ref(const sk_image_t* cimage) {
    AsImage(cimage)->ref();
}

void sk_image_unref(const sk_image_t* cimage) {
    SkSafeUnref(AsImage(cimage));
}

sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t* cinfo, const void* pixels, size_t rowBytes) {
    return sk_image_new_raster_copy_with_colortable(cinfo, pixels, rowBytes, nullptr);
}

sk_image_t* sk_image_new_raster_copy_with_pixmap(const sk_pixmap_t* pixmap) {
    auto image = SkImage::MakeRasterCopy(AsPixmap(*pixmap));
    return ToImage(image.release());
}

sk_image_t* sk_image_new_raster_copy_with_colortable(const sk_imageinfo_t* cinfo, const void* pixels, size_t rowBytes, sk_colortable_t* ctable) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    auto image = SkImage::MakeRasterCopy(SkPixmap(info, pixels, rowBytes, AsColorTable(ctable)));
    return ToImage(image.release());
}

sk_image_t* sk_image_new_raster_data(const sk_imageinfo_t* cinfo, sk_data_t* pixels, size_t rowBytes) {
    SkImageInfo info;
    from_c(*cinfo, &info);
    return ToImage(SkImage::MakeRasterData(info, sk_ref_sp(AsData(pixels)), rowBytes).release());
}

sk_image_t* sk_image_new_raster(const sk_pixmap_t* pixmap, sk_image_raster_release_proc releaseProc, void* context) {
    auto image = SkImage::MakeFromRaster(AsPixmap(*pixmap), releaseProc, context);
    return ToImage(image.release());
}

sk_image_t* sk_image_new_from_bitmap(const sk_bitmap_t* cbitmap) {
    return ToImage(SkImage::MakeFromBitmap(*AsBitmap(cbitmap)).release());
}

sk_image_t* sk_image_new_from_encoded(sk_data_t* cdata, const sk_irect_t* subset) {
    return ToImage(SkImage::MakeFromEncoded(sk_ref_sp(AsData(cdata)), AsIRect(subset)).release());
}

sk_image_t* sk_image_new_from_texture(gr_context_t* context, const gr_backend_texture_desc_t* desc, sk_alphatype_t alpha, sk_colorspace_t* colorSpace, sk_image_texture_release_proc releaseProc, void* releaseContext) {
    return ToImage(SkImage::MakeFromTexture(AsGrContext(context), AsGrBackendTextureDesc(*desc), (SkAlphaType)alpha, sk_ref_sp(AsColorSpace(colorSpace)), releaseProc, releaseContext).release());
}

sk_image_t* sk_image_new_from_adopted_texture(gr_context_t* context, const gr_backend_texture_desc_t* desc, sk_alphatype_t alpha, sk_colorspace_t* colorSpace) {
    return ToImage(SkImage::MakeFromAdoptedTexture(AsGrContext(context), AsGrBackendTextureDesc(*desc), (SkAlphaType)alpha, sk_ref_sp(AsColorSpace(colorSpace))).release());
}

sk_image_t* sk_image_new_from_picture(sk_picture_t* picture, const sk_isize_t* dimensions, const sk_matrix_t* cmatrix, const sk_paint_t* paint) {
    SkMatrix matrix;
    if (cmatrix) {
        from_c(cmatrix, &matrix);
    }
    return ToImage(SkImage::MakeFromPicture(sk_ref_sp(AsPicture(picture)), AsISize(*dimensions), &matrix, AsPaint(paint), SkImage::BitDepth::kU8, SkColorSpace::MakeSRGB()).release());
}

int sk_image_get_width(const sk_image_t* cimage) {
    return AsImage(cimage)->width();
}

int sk_image_get_height(const sk_image_t* cimage) {
    return AsImage(cimage)->height();
}

uint32_t sk_image_get_unique_id(const sk_image_t* cimage) {
    return AsImage(cimage)->uniqueID();
}

sk_alphatype_t sk_image_get_alpha_type(const sk_image_t* image) {
    return (sk_alphatype_t) AsImage(image)->alphaType();
}

bool sk_image_is_alpha_only(const sk_image_t* image) {
    return AsImage(image)->isAlphaOnly();
}

sk_shader_t* sk_image_make_shader(const sk_image_t* image, sk_shader_tilemode_t tileX, sk_shader_tilemode_t tileY, const sk_matrix_t* cmatrix) {
    sk_sp<SkShader> shader;
    if (cmatrix) {
        SkMatrix matrix;
        from_c(cmatrix, &matrix);
        shader = AsImage(image)->makeShader((SkShader::TileMode)tileX, (SkShader::TileMode)tileY, &matrix);
    } else {
        shader = AsImage(image)->makeShader((SkShader::TileMode)tileX, (SkShader::TileMode)tileY, nullptr);
    }
    return ToShader(shader.release());
}

bool sk_image_peek_pixels(const sk_image_t* image, sk_pixmap_t* pixmap) {
    return AsImage(image)->peekPixels(AsPixmap(pixmap));
}

bool sk_image_is_texture_backed(const sk_image_t* image) {
    return AsImage(image)->isTextureBacked();
}

bool sk_image_is_lazy_generated(const sk_image_t* image) {
    return AsImage(image)->isLazyGenerated();
}

bool sk_image_read_pixels(const sk_image_t* image, const sk_imageinfo_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY, sk_image_caching_hint_t cachingHint) {
    SkImageInfo info;
    from_c(*dstInfo, &info);
    return AsImage(image)->readPixels(info, dstPixels, dstRowBytes, srcX, srcY, (SkImage::CachingHint)cachingHint);
}

bool sk_image_read_pixels_into_pixmap(const sk_image_t* image, const sk_pixmap_t* dst, int srcX, int srcY, sk_image_caching_hint_t cachingHint) {
    return AsImage(image)->readPixels(AsPixmap(*dst), srcX, srcY, (SkImage::CachingHint)cachingHint);
}

bool sk_image_scale_pixels(const sk_image_t* image, const sk_pixmap_t* dst, sk_filter_quality_t quality, sk_image_caching_hint_t cachingHint) {
    return AsImage(image)->scalePixels(AsPixmap(*dst), (SkFilterQuality)quality, (SkImage::CachingHint)cachingHint);
}

sk_data_t* sk_image_encode(const sk_image_t* cimage) {
    return ToData(AsImage(cimage)->encode());
}

sk_data_t* sk_image_encode_with_serializer(const sk_image_t* cimage, sk_pixelserializer_t* serializer) {
    return ToData(AsImage(cimage)->encode(AsPixelSerializer(serializer)));
}

sk_data_t* sk_image_encode_specific(const sk_image_t* cimage, sk_encoded_image_format_t encoder, int quality) {
    return ToData(AsImage(cimage)->encode((SkEncodedImageFormat)encoder, quality));
}

sk_image_t* sk_image_make_subset(const sk_image_t* cimage, const sk_irect_t* subset) {
    return ToImage(AsImage(cimage)->makeSubset(AsIRect(*subset)).release());
}

sk_image_t* sk_image_make_non_texture_image(const sk_image_t* cimage) {
    return ToImage(AsImage(cimage)->makeNonTextureImage().release());
}

sk_image_t* sk_image_make_with_filter(const sk_image_t* cimage, const sk_imagefilter_t* filter, const sk_irect_t* subset, const sk_irect_t* clipBounds, sk_irect_t* outSubset, sk_ipoint_t* outOffset) {
    return ToImage(AsImage(cimage)->makeWithFilter(AsImageFilter(filter), AsIRect(*subset), AsIRect(*clipBounds), AsIRect(outSubset), AsIPoint(outOffset)).release());
}
