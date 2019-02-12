/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContext_DEFINED
#define GrRecordingContext_DEFINED

#include "GrAuditTrail.h"
#include "GrImageContext.h"

class GrDrawingManager;
class GrOpMemoryPool;
class GrRecordingContextPriv;

class SK_API GrRecordingContext : public GrImageContext {
public:
    ~GrRecordingContext() override;

    // Provides access to functions that aren't part of the public API.
    GrRecordingContextPriv priv();
    const GrRecordingContextPriv priv() const;

protected:
    friend class GrRecordingContextPriv; // for hidden functions

    GrRecordingContext(GrBackendApi, const GrContextOptions&, uint32_t contextID);

    void abandonContext() override;

    // CONTEXT TODO: move GrDrawingManager to GrRecordingContext for real
    virtual GrDrawingManager* drawingManager() = 0;

    sk_sp<GrOpMemoryPool> refOpMemoryPool();
    GrOpMemoryPool* opMemoryPool();

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

    GrAuditTrail* auditTrail() { return &fAuditTrail; }

    GrRecordingContext* asRecordingContext() override { return this; }

private:
    // All the GrOp-derived classes use this pool.
    sk_sp<GrOpMemoryPool> fOpMemoryPool;
    GrAuditTrail          fAuditTrail;

    typedef GrImageContext INHERITED;
};

#endif
