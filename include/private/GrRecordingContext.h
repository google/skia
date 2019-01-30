/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContext_DEFINED
#define GrRecordingContext_DEFINED

#include "GrImageContext.h"

#include "../private/GrAuditTrail.h"

class GrOpMemoryPool;
class GrRecordingContextPriv;

// This context can record ops (and thus has a drawing manager) - recording/DDL context
class SK_API GrRecordingContext : public GrImageContext {
public:
    ~GrRecordingContext() override;

    GrRecordingContext* asRecordingContext() override { return this; }

    // Provides access to functions that aren't part of the public API.
    GrRecordingContextPriv priv();
    const GrRecordingContextPriv priv() const;

protected:
    friend class GrRecordingContextPriv; // for hidden functions

    bool abandoned1() const override;
    void abandon1() override;

    sk_sp<GrOpMemoryPool> refOpMemoryPool();
    GrOpMemoryPool* opMemoryPool() { return fOpMemoryPool.get(); }

    GrAuditTrail* auditTrail() { return &fAuditTrail; }

    sk_sp<GrSurfaceContext> makeWrappedSurfaceContext(sk_sp<GrSurfaceProxy>,
                                                      sk_sp<SkColorSpace> = nullptr,
                                                      const SkSurfaceProps* = nullptr);

    sk_sp<GrSurfaceContext> makeDeferredSurfaceContext(const GrBackendFormat&,
                                                       const GrSurfaceDesc&,
                                                       GrSurfaceOrigin,
                                                       GrMipMapped,
                                                       SkBackingFit,
                                                       SkBudgeted,
                                                       sk_sp<SkColorSpace> colorSpace = nullptr,
                                                       const SkSurfaceProps* = nullptr);

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

    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
                                                                 const GrBackendFormat& format,
                                                                 SkBackingFit fit,
                                                                 int width, int height,
                                                                 GrPixelConfig config,
                                                                 sk_sp<SkColorSpace> colorSpace,
                                                                 int sampleCnt,
                                                                 GrMipMapped mipMapped,
                                                                 GrSurfaceOrigin origin,
                                                                 const SkSurfaceProps* surfaceProps,
                                                                 SkBudgeted budgeted);
private:
    friend class GrContext;    // for ctor
    friend class GrDDLContext; // for ctor

    GrRecordingContext(GrBackendApi backend, const GrContextOptions& options, uint32_t uniqueID);

    // All the GrOp-derived classes use this pool.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    GrAuditTrail          fAuditTrail;

    typedef GrImageContext INHERITED;
};

#endif
