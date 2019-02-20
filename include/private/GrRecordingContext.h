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
class GrOnFlushCallbackObject;
class GrOpMemoryPool;
class GrRecordingContextPriv;
class GrStrikeCache;
class GrTextBlobCache;

class SK_API GrRecordingContext : public GrImageContext {
public:
    ~GrRecordingContext() override;

    // Provides access to functions that aren't part of the public API.
    GrRecordingContextPriv priv();
    const GrRecordingContextPriv priv() const;

protected:
    friend class GrRecordingContextPriv; // for hidden functions

    GrRecordingContext(GrBackendApi, const GrContextOptions&, uint32_t contextID);
    bool init(sk_sp<const GrCaps>, sk_sp<GrSkSLFPFactoryCache>) override;

    void abandonContext() override;

    // CONTEXT TODO: move GrDrawingManager to GrRecordingContext for real
    virtual GrDrawingManager* drawingManager() = 0;

    sk_sp<GrOpMemoryPool> refOpMemoryPool();
    GrOpMemoryPool* opMemoryPool();

    GrStrikeCache* getGlyphCache() { return fGlyphCache.get(); }
    GrTextBlobCache* getTextBlobCache();
    const GrTextBlobCache* getTextBlobCache() const;

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

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

    /*
     * This method will attempt to create a renderTargetContext that has, at least, the number of
     * channels and precision per channel as requested in 'config' (e.g., A8 and 888 can be
     * converted to 8888). It may also swizzle the channels (e.g., BGRA -> RGBA).
     * SRGB-ness will be preserved.
     */
    sk_sp<GrRenderTargetContext> makeDeferredRenderTargetContextWithFallback(
                                            const GrBackendFormat& format,
                                            SkBackingFit fit,
                                            int width, int height,
                                            GrPixelConfig config,
                                            sk_sp<SkColorSpace> colorSpace,
                                            int sampleCnt = 1,
                                            GrMipMapped = GrMipMapped::kNo,
                                            GrSurfaceOrigin origin = kBottomLeft_GrSurfaceOrigin,
                                            const SkSurfaceProps* surfaceProps = nullptr,
                                            SkBudgeted budgeted = SkBudgeted::kYes);

    GrAuditTrail* auditTrail() { return &fAuditTrail; }

    GrRecordingContext* asRecordingContext() override { return this; }

private:
    // All the GrOp-derived classes use this pool.
    sk_sp<GrOpMemoryPool>             fOpMemoryPool;

    std::unique_ptr<GrStrikeCache>    fGlyphCache;
    std::unique_ptr<GrTextBlobCache>  fTextBlobCache;

    GrAuditTrail                      fAuditTrail;

    typedef GrImageContext INHERITED;
};

#endif
