/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextPriv_DEFINED
#define GrContextPriv_DEFINED

#include "GrContext.h"
#include "GrSurfaceContext.h"

/** Class that adds methods to GrContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrContext. It should never have additional
    data members or virtual methods. */
class GrContextPriv {
public:
    GrDrawingManager* drawingManager() { return fContext->fDrawingManager.get(); }

    // Create a renderTargetContext that wraps an existing renderTarget
    sk_sp<GrRenderTargetContext> makeWrappedRenderTargetContext(sk_sp<GrRenderTarget> rt,
                                                                sk_sp<SkColorSpace> colorSpace,
                                                                const SkSurfaceProps* = nullptr);

    // Create a surfaceContext that wraps an existing texture or renderTarget
    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurface> tex);

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy);

    sk_sp<GrSurfaceContext> makeDeferredSurfaceContext(const GrSurfaceDesc& dstDesc,
                                                       SkBackingFit dstFit,
                                                       SkBudgeted isDstBudgeted);

    sk_sp<GrRenderTargetContext> makeBackendTextureRenderTargetContext(
                                                         const GrBackendTextureDesc& desc,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* = nullptr,
                                                         GrWrapOwnership = kBorrow_GrWrapOwnership);

    sk_sp<GrRenderTargetContext> makeBackendRenderTargetRenderTargetContext(
                                                              const GrBackendRenderTargetDesc& desc,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendTextureAsRenderTargetRenderTargetContext(
                                                                 const GrBackendTextureDesc& desc,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* = nullptr);

private:
    explicit GrContextPriv(GrContext* context) : fContext(context) {}
    GrContextPriv(const GrContextPriv&) {} // unimpl
    GrContextPriv& operator=(const GrContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrContextPriv* operator&() const;
    GrContextPriv* operator&();

    GrContext* fContext;

    friend class GrContext; // to construct/copy this type.
};

inline GrContextPriv GrContext::contextPriv() { return GrContextPriv(this); }

inline const GrContextPriv GrContext::contextPriv () const {
    return GrContextPriv(const_cast<GrContext*>(this));
}

#endif
