/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextPriv_DEFINED
#define GrContextPriv_DEFINED

#include "GrContext.h"

/** Class that adds methods to GrContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrContext. It should never have additional
    data members or virtual methods. */
class GrContextPriv {
public:
    GrDrawingManager* drawingManager() { return fContext->fDrawingManager; }

    // Create a drawContext that wraps an existing renderTarget
    sk_sp<GrDrawContext> makeWrappedDrawContext(sk_sp<GrRenderTarget> rt,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* = nullptr);

    sk_sp<GrDrawContext> makeBackendTextureDrawContext(const GrBackendTextureDesc& desc,
                                                       sk_sp<SkColorSpace> colorSpace,
                                                       const SkSurfaceProps* = nullptr,
                                                       GrWrapOwnership = kBorrow_GrWrapOwnership);

    sk_sp<GrDrawContext> makeBackendRenderTargetDrawContext(const GrBackendRenderTargetDesc& desc,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            const SkSurfaceProps* = nullptr);

    sk_sp<GrDrawContext> makeBackendTextureAsRenderTargetDrawContext(
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
