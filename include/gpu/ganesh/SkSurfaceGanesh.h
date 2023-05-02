/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfaceGanesh_DEFINED
#define SkSurfaceGanesh_DEFINED

#include "include/core/SkSurface.h"
#include "include/private/base/SkAPI.h"

class GrBackendRenderTarget;
class GrBackendTexture;

namespace SkSurfaces {
using BackendHandleAccess = SkSurface::BackendHandleAccess;

/** Retrieves the back-end texture. If SkSurface has no back-end texture, an invalid
    object is returned. Call GrBackendTexture::isValid to determine if the result
    is valid.

    The returned GrBackendTexture should be discarded if the SkSurface is drawn to or deleted.

    @return                     GPU texture reference; invalid on failure
*/
SK_API GrBackendTexture GetBackendTexture(SkSurface*, BackendHandleAccess);

/** Retrieves the back-end render target. If SkSurface has no back-end render target, an invalid
    object is returned. Call GrBackendRenderTarget::isValid to determine if the result
    is valid.

    The returned GrBackendRenderTarget should be discarded if the SkSurface is drawn to
    or deleted.

    @return                     GPU render target reference; invalid on failure
*/
SK_API GrBackendRenderTarget GetBackendRenderTarget(SkSurface*, BackendHandleAccess);

}  // namespace SkSurfaces

#endif
