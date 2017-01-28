/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL EXPERIMENTAL
// DO NOT USE -- FOR INTERNAL TESTING ONLY

#ifndef sk_surface_DEFINED
#define sk_surface_DEFINED

#include "sk_types.h"

SK_C_PLUS_PLUS_BEGIN_GUARD

/**
    Return the default sk_colortype_t; this is operating-system dependent.
*/
SK_C_API sk_colortype_t sk_colortype_get_default_8888();

/**
    Return a new surface, with the memory for the pixels automatically
    allocated.  If the requested surface cannot be created, or the
    request is not a supported configuration, NULL will be returned.

    @param sk_imageinfo_t* Specify the width, height, color type, and
                           alpha type for the surface.

    @param sk_surfaceprops_t* If not NULL, specify additional non-default
                              properties of the surface.
*/
SK_C_API sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t*, const sk_surfaceprops_t*);

/**
    Create a new surface which will draw into the specified pixels
    with the specified rowbytes.  If the requested surface cannot be
    created, or the request is not a supported configuration, NULL
    will be returned.

    @param sk_imageinfo_t* Specify the width, height, color type, and
                           alpha type for the surface.
    @param void* pixels Specify the location in memory where the
                        destination pixels are.  This memory must
                        outlast this surface.
     @param size_t rowBytes Specify the difference, in bytes, between
                           each adjacent row.  Should be at least
                           (width * sizeof(one pixel)).
    @param sk_surfaceprops_t* If not NULL, specify additional non-default
                              properties of the surface.
*/
SK_C_API sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t*, void* pixels, size_t rowBytes, const sk_surfaceprops_t* props);

/**
    Decrement the reference count. If the reference count is 1 before
    the decrement, then release both the memory holding the
    sk_surface_t and any pixel memory it may be managing.  New
    sk_surface_t are created with a reference count of 1.
*/
SK_C_API void sk_surface_unref(sk_surface_t*);

/**
 *  Return the canvas associated with this surface. Note: the canvas is owned by the surface,
 *  so the returned object is only valid while the owning surface is valid.
 */
SK_C_API sk_canvas_t* sk_surface_get_canvas(sk_surface_t*);

/**
 *  Call sk_image_unref() when the returned image is no longer used.
 */
SK_C_API sk_image_t* sk_surface_new_image_snapshot(sk_surface_t*);

/**
 *  Used to wrap a pre-existing 3D API rendering target as a SkSurface. Skia will not assume
 *  ownership of the render target and the client must ensure the render target is valid for the
 *  lifetime of the SkSurface.
 */
SK_C_API sk_surface_t* sk_surface_new_backend_render_target(gr_context_t* context, const gr_backend_rendertarget_desc_t* desc, const sk_surfaceprops_t* props);

/**
 *  Used to wrap a pre-existing backend 3D API texture as a SkSurface. The kRenderTarget flag
 *  must be set on GrBackendTextureDesc for this to succeed. Skia will not assume ownership
 *  of the texture and the client must ensure the texture is valid for the lifetime of the
 *  SkSurface.
 */
SK_C_API sk_surface_t* sk_surface_new_backend_texture(gr_context_t* context, const gr_backend_texture_desc_t* desc, const sk_surfaceprops_t* props);

/**
 *  Used to wrap a pre-existing 3D API texture as a SkSurface. Skia will treat the texture as
 *  a rendering target only, but unlike NewFromBackendRenderTarget, Skia will manage and own
 *  the associated render target objects (but not the provided texture). The kRenderTarget flag
 *  must be set on GrBackendTextureDesc for this to succeed. Skia will not assume ownership
 *  of the texture and the client must ensure the texture is valid for the lifetime of the
 *  SkSurface.
 */
SK_C_API sk_surface_t* sk_surface_new_backend_texture_as_render_target(gr_context_t* context, const gr_backend_texture_desc_t* desc, const sk_surfaceprops_t* props);

SK_C_API sk_surface_t* sk_surface_new_render_target(gr_context_t* context, bool budgeted, const sk_imageinfo_t* info, int sampleCount, const sk_surfaceprops_t* props);

SK_C_PLUS_PLUS_END_GUARD

#endif
