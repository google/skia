/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContextPriv_DEFINED
#define GrRecordingContextPriv_DEFINED

#include "GrRecordingContext.h"

/** Class that exposes methods to GrRecordingContext that are only intended for use internal to
    Skia. This class is purely a privileged window into GrRecordingContext. It should never have
    additional data members or virtual methods. */
class GrRecordingContextPriv {
public:
    // from GrContext_Base
    uint32_t contextID() const { return fContext->contextID(); }

    bool matches(GrContext_Base* candidate) const { return fContext->matches(candidate); }

    const GrContextOptions& options() const { return fContext->options(); }

    const GrCaps* caps() const { return fContext->caps(); }
    sk_sp<const GrCaps> refCaps() const;

    sk_sp<GrSkSLFPFactoryCache> fpFactoryCache();

    GrImageContext* asImageContext() { return fContext->asImageContext(); }
    GrRecordingContext* asRecordingContext() { return fContext->asRecordingContext(); }
    GrContext* asDirectContext() { return fContext->asDirectContext(); }

    // from GrImageContext
    GrProxyProvider* proxyProvider() { return fContext->proxyProvider(); }
    const GrProxyProvider* proxyProvider() const { return fContext->proxyProvider(); }

    bool abandoned() const { return fContext->abandoned(); }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* singleOwner() const { return fContext->singleOwner(); } )

    // from GrRecordingContext
    sk_sp<GrOpMemoryPool> refOpMemoryPool();
    GrOpMemoryPool* opMemoryPool() { return fContext->opMemoryPool(); }

    /*
     * Create a new render target context backed by a deferred-style
     * GrRenderTargetProxy. We guarantee that "asTextureProxy" will succeed for
     * renderTargetContexts created via this entry point.
     */
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContext(
                                            const GrBackendFormat& format,
                                            SkBackingFit fit,
                                            int width, int height,
                                            GrPixelConfig config,
                                            sk_sp<SkColorSpace> colorSpace,
                                            int sampleCnt = 1,
                                            GrMipMapped = GrMipMapped::kNo,
                                            GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                            const SkSurfaceProps* surfaceProps = nullptr,
                                            SkBudgeted = SkBudgeted::kYes);

    GrAuditTrail* auditTrail() { return fContext->auditTrail(); }

private:
    explicit GrRecordingContextPriv(GrRecordingContext* context) : fContext(context) {}
    GrRecordingContextPriv(const GrRecordingContextPriv&); // unimpl
    GrRecordingContextPriv& operator=(const GrRecordingContextPriv&); // unimpl

    // No taking addresses of this type.
    const GrRecordingContextPriv* operator&() const;
    GrRecordingContextPriv* operator&();

    GrRecordingContext* fContext;

    friend class GrRecordingContext; // to construct/copy this type.
};

inline GrRecordingContextPriv GrRecordingContext::priv() { return GrRecordingContextPriv(this); }

inline const GrRecordingContextPriv GrRecordingContext::priv () const {
    return GrRecordingContextPriv(const_cast<GrRecordingContext*>(this));
}

#endif
