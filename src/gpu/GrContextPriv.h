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

class GrSemaphore;
class GrSurfaceProxy;
class GrPreFlushCallbackObject;

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

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy> proxy,
                                                      sk_sp<SkColorSpace>);

    sk_sp<GrSurfaceContext> makeDeferredSurfaceContext(const GrSurfaceDesc& dstDesc,
                                                       SkBackingFit dstFit,
                                                       SkBudgeted isDstBudgeted);

    // TODO: Maybe add a 'surfaceProps' param (that is ignored for non-RTs) and remove
    // makeBackendTextureRenderTargetContext & makeBackendTextureAsRenderTargetRenderTargetContext
    sk_sp<GrSurfaceContext> makeBackendSurfaceContext(const GrBackendTextureDesc& desc,
                                                      sk_sp<SkColorSpace> colorSpace);

    sk_sp<GrRenderTargetContext> makeBackendTextureRenderTargetContext(
                                                         const GrBackendTextureDesc& desc,
                                                         sk_sp<SkColorSpace> colorSpace,
                                                         const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendRenderTargetRenderTargetContext(
                                                              const GrBackendRenderTargetDesc& desc,
                                                              sk_sp<SkColorSpace> colorSpace,
                                                              const SkSurfaceProps* = nullptr);

    sk_sp<GrRenderTargetContext> makeBackendTextureAsRenderTargetRenderTargetContext(
                                                                 const GrBackendTextureDesc& desc,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 const SkSurfaceProps* = nullptr);

    bool disableGpuYUVConversion() const { return fContext->fDisableGpuYUVConversion; }

    /*
     * A ref will be taken on the preFlushCallbackObject which will be removed when the
     * context is destroyed.
     */
    void addPreFlushCallbackObject(sk_sp<GrPreFlushCallbackObject>);

private:
    explicit GrContextPriv(GrContext* context) : fContext(context) {}
    GrContextPriv(const GrContextPriv&); // unimpl
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
