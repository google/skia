/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurfacePriv_DEFINED
#define SkSurfacePriv_DEFINED

#include "SkSurfaceProps.h"
#include "SkSurface.h"

struct SkImageInfo;

static inline SkSurfaceProps SkSurfacePropsCopyOrDefault(const SkSurfaceProps* props) {
    if (props) {
        return *props;
    } else {
        return SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType);
    }
}

static inline SkPixelGeometry SkSurfacePropsDefaultPixelGeometry() {
    return SkSurfaceProps(SkSurfaceProps::kLegacyFontHost_InitType).pixelGeometry();
}

constexpr size_t kIgnoreRowBytesValue = static_cast<size_t>(~0);

bool SkSurfaceValidateRasterInfo(const SkImageInfo&, size_t rb = kIgnoreRowBytesValue);


#if GR_TEST_UTILS
    /** Retrieves the backend texture. If Surface has no backend texture, an invalid
        object is returned. Call GrBackendTexture::isValid to determine if the result
        is valid.

        The returned GrBackendTexture should be discarded if the Surface is drawn to or deleted.

        @param backendHandleAccess  one of:  kFlushRead_BackendHandleAccess,
                                             kFlushWrite_BackendHandleAccess,
                                             kDiscardWrite_BackendHandleAccess
        @return                     GPU texture reference; invalid on failure
    */
    GrBackendTexture SkSurfaceGetBackendTexture(SkSurface* surface,
                                                SkSurface::BackendHandleAccess backendHandleAccess);

    /** Retrieves the backend render target. If Surface has no backend render target, an invalid
        object is returned. Call GrBackendRenderTarget::isValid to determine if the result
        is valid.

        The returned GrBackendRenderTarget should be discarded if the Surface is drawn to
        or deleted.

        @param backendHandleAccess  one of:  kFlushRead_BackendHandleAccess,
                                             kFlushWrite_BackendHandleAccess,
                                             kDiscardWrite_BackendHandleAccess
        @return                     GPU render target reference; invalid on failure
    */
    GrBackendRenderTarget SkSurfaceGetBackendRenderTarget(SkSurface* surface,
                                                SkSurface::BackendHandleAccess backendHandleAccess);
#endif

#endif
