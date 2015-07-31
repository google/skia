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

sk_surface_t* sk_surface_new_raster(const sk_imageinfo_t*, const sk_surfaceprops_t*);
sk_surface_t* sk_surface_new_raster_direct(const sk_imageinfo_t*, void* pixels, size_t rowBytes,
                                           const sk_surfaceprops_t* props);
void sk_surface_unref(sk_surface_t*);

/**
 *  Return the canvas associated with this surface. Note: the canvas is owned by the surface,
 *  so the returned object is only valid while the owning surface is valid.
 */
sk_canvas_t* sk_surface_get_canvas(sk_surface_t*);

/**
 *  Call sk_image_unref() when the returned image is no longer used.
 */
sk_image_t* sk_surface_new_image_snapshot(sk_surface_t*);

SK_C_PLUS_PLUS_END_GUARD

#endif
