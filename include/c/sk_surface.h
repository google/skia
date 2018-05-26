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

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

SK_C_API sk_colortype_t sk_colortype_get_default_8888(void);

SK_C_API sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t*, const sk_surfaceprops_t*);
SK_C_API sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t*, void* pixels, size_t rowBytes, const sk_surfaceprops_t* props);
SK_C_API void sk_surface_unref(sk_surface_t*);
SK_C_API sk_canvas_t* sk_surface_get_canvas(sk_surface_t*);
SK_C_API sk_image_t* sk_surface_new_image_snapshot(sk_surface_t*);
SK_C_API sk_surface_t* sk_surface_new_backend_render_target(gr_context_t* context, const gr_backend_rendertarget_desc_t* desc, const sk_surfaceprops_t* props);
SK_C_API sk_surface_t* sk_surface_new_backend_texture(gr_context_t* context, const gr_backend_texture_desc_t* desc, const sk_surfaceprops_t* props);
SK_C_API sk_surface_t* sk_surface_new_backend_texture_as_render_target(gr_context_t* context, const gr_backend_texture_desc_t* desc, const sk_surfaceprops_t* props);
SK_C_API sk_surface_t* sk_surface_new_render_target(gr_context_t* context, bool budgeted, const sk_imageinfo_t* info, int sampleCount, const sk_surfaceprops_t* props);
SK_C_API void sk_surface_draw(sk_surface_t* surface, sk_canvas_t* canvas, float x, float y, const sk_paint_t* paint);
SK_C_API bool sk_surface_peek_pixels(sk_surface_t* surface, sk_pixmap_t* pixmap);
SK_C_API bool sk_surface_read_pixels(sk_surface_t* surface, sk_imageinfo_t* dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY);
SK_C_API void sk_surface_get_props(sk_surface_t* surface, sk_surfaceprops_t* props);

SK_C_PLUS_PLUS_END_GUARD

#endif
