/*
 * Copyright 2014 Google Inc.
 * Copyright 2015 Xamarin Inc.
 * Copyright 2017 Microsoft Corporation. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sk_surface_DEFINED
#define sk_surface_DEFINED

#include "include/c/sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

// surface

SK_C_API sk_surface_t* sk_surface_new_null(int width, int height);
SK_C_API sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t*, size_t rowBytes, const sk_surfaceprops_t*);
SK_C_API sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t*, void* pixels, size_t rowBytes, const sk_surface_raster_release_proc releaseProc, void* context, const sk_surfaceprops_t* props);

SK_C_API sk_surface_t* sk_surface_new_backend_texture(gr_recording_context_t* context, const gr_backendtexture_t* texture, gr_surfaceorigin_t origin, int samples, sk_colortype_t colorType, sk_colorspace_t* colorspace, const sk_surfaceprops_t* props);
SK_C_API sk_surface_t* sk_surface_new_backend_render_target(gr_recording_context_t* context, const gr_backendrendertarget_t* target, gr_surfaceorigin_t origin, sk_colortype_t colorType, sk_colorspace_t* colorspace, const sk_surfaceprops_t* props);
SK_C_API sk_surface_t* sk_surface_new_render_target(gr_recording_context_t* context, bool budgeted, const sk_imageinfo_t* cinfo, int sampleCount, gr_surfaceorigin_t origin, const sk_surfaceprops_t* props, bool shouldCreateWithMips);

SK_C_API sk_surface_t* sk_surface_new_metal_layer(gr_recording_context_t* context, gr_mtl_handle_t layer, gr_surfaceorigin_t origin, int sampleCount, sk_colortype_t colorType, sk_colorspace_t* colorspace, const sk_surfaceprops_t* props, gr_mtl_handle_t* drawable);
SK_C_API sk_surface_t* sk_surface_new_metal_view(gr_recording_context_t* context, gr_mtl_handle_t mtkView, gr_surfaceorigin_t origin, int sampleCount, sk_colortype_t colorType, sk_colorspace_t* colorspace, const sk_surfaceprops_t* props);

SK_C_API void sk_surface_unref(sk_surface_t*);
SK_C_API sk_canvas_t* sk_surface_get_canvas(sk_surface_t*);
SK_C_API sk_image_t* sk_surface_new_image_snapshot(sk_surface_t*);
SK_C_API sk_image_t* sk_surface_new_image_snapshot_with_crop(sk_surface_t* surface, const sk_irect_t* bounds);
SK_C_API void sk_surface_draw(sk_surface_t* surface, sk_canvas_t* canvas, float x, float y, const sk_paint_t* paint);
SK_C_API bool sk_surface_peek_pixels(sk_surface_t* surface, sk_pixmap_t* pixmap);
SK_C_API bool sk_surface_read_pixels(sk_surface_t* surface, sk_imageinfo_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY);
SK_C_API const sk_surfaceprops_t* sk_surface_get_props(sk_surface_t* surface);
SK_C_API void sk_surface_flush(sk_surface_t* surface);
SK_C_API void sk_surface_flush_and_submit(sk_surface_t* surface, bool syncCpu);
SK_C_API gr_recording_context_t* sk_surface_get_recording_context(sk_surface_t* surface);

// surface props

SK_C_API sk_surfaceprops_t* sk_surfaceprops_new(uint32_t flags, sk_pixelgeometry_t geometry);
SK_C_API void sk_surfaceprops_delete(sk_surfaceprops_t* props);
SK_C_API uint32_t sk_surfaceprops_get_flags(sk_surfaceprops_t* props);
SK_C_API sk_pixelgeometry_t sk_surfaceprops_get_pixel_geometry(sk_surfaceprops_t* props);

SK_C_PLUS_PLUS_END_GUARD

#endif
