/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_image_DEFINED
#define sk_image_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_image_t* sk_image_new_raster_copy(const sk_imageinfo_t*, const void* pixels, size_t rowBytes);
SK_C_API sk_image_t* sk_image_new_raster_copy_with_pixmap(const sk_pixmap_t* pixmap);
SK_C_API sk_image_t* sk_image_new_raster_data(const sk_imageinfo_t* cinfo, sk_data_t* pixels, size_t rowBytes);
SK_C_API sk_image_t* sk_image_new_raster(const sk_pixmap_t* pixmap, sk_image_raster_release_proc releaseProc, void* context);
SK_C_API sk_image_t* sk_image_new_from_bitmap(const sk_bitmap_t *cbitmap);
SK_C_API sk_image_t* sk_image_new_from_encoded(sk_data_t* encoded, const sk_irect_t* subset);
SK_C_API sk_image_t* sk_image_new_from_texture(gr_context_t* context, const gr_backendtexture_t* texture, gr_surfaceorigin_t origin, sk_colortype_t colorType, sk_alphatype_t alpha, sk_colorspace_t* colorSpace, sk_image_texture_release_proc releaseProc, void* releaseContext);
SK_C_API sk_image_t* sk_image_new_from_adopted_texture(gr_context_t* context, const gr_backendtexture_t* texture, gr_surfaceorigin_t origin, sk_colortype_t colorType, sk_alphatype_t alpha, sk_colorspace_t* colorSpace);
SK_C_API sk_image_t* sk_image_new_from_picture(sk_picture_t* picture, const sk_isize_t* dimensions, const sk_matrix_t* matrix, const sk_paint_t* paint);

SK_C_API sk_image_t* sk_image_make_subset(const sk_image_t* cimage, const sk_irect_t* subset);
SK_C_API sk_image_t* sk_image_make_non_texture_image(const sk_image_t* cimage);
SK_C_API sk_image_t* sk_image_make_with_filter(const sk_image_t* cimage, const sk_imagefilter_t* filter, const sk_irect_t* subset, const sk_irect_t* clipBounds, sk_irect_t* outSubset, sk_ipoint_t* outOffset);

SK_C_API void sk_image_ref(const sk_image_t*);
SK_C_API void sk_image_unref(const sk_image_t*);
SK_C_API int sk_image_get_width(const sk_image_t*);
SK_C_API int sk_image_get_height(const sk_image_t*);
SK_C_API uint32_t sk_image_get_unique_id(const sk_image_t*);
SK_C_API sk_alphatype_t sk_image_get_alpha_type(const sk_image_t*);
SK_C_API sk_colortype_t sk_image_get_color_type(const sk_image_t*);
SK_C_API sk_colorspace_t* sk_image_get_colorspace(const sk_image_t*);
SK_C_API bool sk_image_is_alpha_only(const sk_image_t*);
SK_C_API sk_shader_t* sk_image_make_shader(const sk_image_t*, sk_shader_tilemode_t tileX, sk_shader_tilemode_t tileY, const sk_matrix_t* localMatrix);
SK_C_API bool sk_image_peek_pixels(const sk_image_t* image, sk_pixmap_t* pixmap);
SK_C_API bool sk_image_is_texture_backed(const sk_image_t* image);
SK_C_API bool sk_image_is_lazy_generated(const sk_image_t* image);
SK_C_API bool sk_image_read_pixels(const sk_image_t* image, const sk_imageinfo_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY, sk_image_caching_hint_t cachingHint);
SK_C_API bool sk_image_read_pixels_into_pixmap(const sk_image_t* image, const sk_pixmap_t* dst, int srcX, int srcY, sk_image_caching_hint_t cachingHint);
SK_C_API bool sk_image_scale_pixels(const sk_image_t* image, const sk_pixmap_t* dst, sk_filter_quality_t quality, sk_image_caching_hint_t cachingHint);
SK_C_API sk_data_t* sk_image_ref_encoded(const sk_image_t*);
SK_C_API sk_data_t* sk_image_encode(const sk_image_t*);
SK_C_API sk_data_t* sk_image_encode_specific(const sk_image_t* cimage, sk_encoded_image_format_t encoder, int quality);

SK_C_PLUS_PLUS_END_GUARD

#endif
