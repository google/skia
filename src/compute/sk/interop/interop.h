/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#pragma once

//
//
//

#ifdef __cplusplus
extern "C" {
#endif

#include <interop.h>

#ifdef __cplusplus
}
#endif

//
//
//

struct pxl_interop*
pxl_interop_create(const struct GrGLInterface* const gl, uint32_t const surface_count_max);

//
//
//

void
pxl_interop_snap_create(struct pxl_interop  * const interop,
                        pxl_interop_surface_t const surface,
                        GrGLuint            * const snap);

void
pxl_interop_snap_dispose(struct pxl_interop * const interop,
                         GrGLuint             const snap);

//
//
//


