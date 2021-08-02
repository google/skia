/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRecordingContextPriv_DEFINED
#define GrRecordingContextPriv_DEFINED

#include "include/core/SkPaint.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/BaseDevice.h"
#include "src/gpu/text/GrSDFTControl.h"

class GrImageInfo;
class GrSwizzle;
class SkDeferredDisplayList;

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

    GrImageContext* asImageContext() { return fContext->asImageContext(); }
    GrRecordingContext* asRecordingContext() { return fContext->asRecordingContext(); }

    // from GrRecordingContext
    GrProxyProvider* proxyProvider() { return fContext->proxyProvider(); }
    const GrProxyProvider* proxyProvider() const { return fContext->proxyProvider(); }

    /** This is only useful for debug purposes */
    SkDEBUGCODE(GrSingleOwner* singleOwner() const { return fContext->singleOwner(); } )

    // from GrRecordingContext
    GrDrawingManager* drawingManager() { return fContext->drawingManager(); }

    SkArenaAlloc* recordTimeAllocator() { return fContext->arenas().recordTimeAllocator(); }
    GrSubRunAllocator* recordTimeSubRunAllocator() {
        return fContext->arenas().recordTimeSubRunAllocator();
    }
    GrRecordingContext::Arenas arenas() { return fContext->arenas(); }

    GrRecordingContext::OwnedArenas&& detachArenas() { return fContext->detachArenas(); }

    void recordProgramInfo(const GrProgramInfo* programInfo) {
        fContext->recordProgramInfo(programInfo);
    }

    void detachProgramData(SkTArray<GrRecordingContext::ProgramData>* dst) {
        fContext->detachProgramData(dst);
    }

    GrTextBlobCache* getTextBlobCache() { return fContext->getTextBlobCache(); }

    GrThreadSafeCache* threadSafeCache() { return fContext->threadSafeCache(); }

    void moveRenderTasksToDDL(SkDeferredDisplayList*);

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

    GrAuditTrail* auditTrail() { return fContext->fAuditTrail.get(); }

#if GR_TEST_UTILS
    // Used by tests that intentionally exercise codepaths that print warning messages, in order to
    // not confuse users with output that looks like a testing failure.
    class AutoSuppressWarningMessages {
    public:
        AutoSuppressWarningMessages(GrRecordingContext* context) : fContext(context) {
            ++fContext->fSuppressWarningMessages;
        }
        ~AutoSuppressWarningMessages() {
            --fContext->fSuppressWarningMessages;
        }
    private:
        GrRecordingContext* fContext;
    };
    void incrSuppressWarningMessages() { ++fContext->fSuppressWarningMessages; }
    void decrSuppressWarningMessages() { --fContext->fSuppressWarningMessages; }
#endif

    void printWarningMessage(const char* msg) const {
#if GR_TEST_UTILS
        if (fContext->fSuppressWarningMessages > 0) {
            return;
        }
#endif
        SkDebugf("%s", msg);
    }

    GrRecordingContext::Stats* stats() {
        return &fContext->fStats;
    }

#if GR_GPU_STATS && GR_TEST_UTILS
    using DMSAAStats = GrRecordingContext::DMSAAStats;
    DMSAAStats& dmsaaStats() { return fContext->fDMSAAStats; }
#endif

    GrSDFTControl getSDFTControl(bool useSDFTForSmallText) const;

    /**
     * Create a GrRecordingContext without a resource cache
     */
    static sk_sp<GrRecordingContext> MakeDDL(sk_sp<GrContextThreadSafeProxy>);

    sk_sp<skgpu::BaseDevice> createDevice(GrColorType,
                                          sk_sp<GrSurfaceProxy>,
                                          sk_sp<SkColorSpace>,
                                          GrSurfaceOrigin,
                                          const SkSurfaceProps&,
                                          skgpu::BaseDevice::InitContents);
    sk_sp<skgpu::BaseDevice> createDevice(SkBudgeted,
                                          const SkImageInfo&,
                                          SkBackingFit,
                                          int sampleCount,
                                          GrMipmapped,
                                          GrProtected,
                                          GrSurfaceOrigin,
                                          const SkSurfaceProps&,
                                          skgpu::BaseDevice::InitContents);

    // If the passed in GrSurfaceProxy is renderable this will return a SurfaceDrawContext,
    // otherwise it will return a GrSurfaceContext.
    std::unique_ptr<GrSurfaceContext> makeSC(GrSurfaceProxyView readView,
                                             const GrColorInfo&);

    /**
     * Uses GrImageInfo's color type to pick the default texture format. Will return a
     * SurfaceDrawContext if possible.
     */
    std::unique_ptr<GrSurfaceFillContext> makeSFC(GrImageInfo,
                                                  SkBackingFit = SkBackingFit::kExact,
                                                  int sampleCount = 1,
                                                  GrMipmapped = GrMipmapped::kNo,
                                                  GrProtected = GrProtected::kNo,
                                                  GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
                                                  SkBudgeted = SkBudgeted::kYes);

    /**
     * Makes a custom configured GrSurfaceFillContext where the caller specifies the specific
     * texture format and swizzles. The color type will be kUnknown. Returns a SurfaceDrawContext
     * if possible.
     */
    std::unique_ptr<GrSurfaceFillContext> makeSFC(SkAlphaType,
                                                  sk_sp<SkColorSpace>,
                                                  SkISize dimensions,
                                                  SkBackingFit,
                                                  const GrBackendFormat&,
                                                  int sampleCount,
                                                  GrMipmapped,
                                                  GrProtected,
                                                  GrSwizzle readSwizzle,
                                                  GrSwizzle writeSwizzle,
                                                  GrSurfaceOrigin,
                                                  SkBudgeted);

    /**
     * Like the above but uses GetFallbackColorTypeAndFormat to find a fallback color type (and
     * compatible format) if the passed GrImageInfo's color type is not renderable.
     */
    std::unique_ptr<GrSurfaceFillContext> makeSFCWithFallback(
            GrImageInfo,
            SkBackingFit = SkBackingFit::kExact,
            int sampleCount = 1,
            GrMipmapped = GrMipmapped::kNo,
            GrProtected = GrProtected::kNo,
            GrSurfaceOrigin = kTopLeft_GrSurfaceOrigin,
            SkBudgeted = SkBudgeted::kYes);

    /**
     * Creates a GrSurfaceFillContext from an existing GrBackendTexture. The GrColorInfo's color
     * type must be compatible with backend texture's format or this will fail. All formats are
     * considered compatible with kUnknown. Returns a SurfaceDrawContext if possible.
     */
    std::unique_ptr<GrSurfaceFillContext> makeSFCFromBackendTexture(
            GrColorInfo,
            const GrBackendTexture&,
            int sampleCount,
            GrSurfaceOrigin,
            sk_sp<GrRefCntedCallback> releaseHelper);

private:
    explicit GrRecordingContextPriv(GrRecordingContext* context) : fContext(context) {}
    GrRecordingContextPriv(const GrRecordingContextPriv&) = delete;
    GrRecordingContextPriv& operator=(const GrRecordingContextPriv&) = delete;

    // No taking addresses of this type.
    const GrRecordingContextPriv* operator&() const;
    GrRecordingContextPriv* operator&();

    GrRecordingContext* fContext;

    friend class GrRecordingContext; // to construct/copy this type.
};

inline GrRecordingContextPriv GrRecordingContext::priv() { return GrRecordingContextPriv(this); }

inline const GrRecordingContextPriv GrRecordingContext::priv () const {  // NOLINT(readability-const-return-type)
    return GrRecordingContextPriv(const_cast<GrRecordingContext*>(this));
}

#endif
