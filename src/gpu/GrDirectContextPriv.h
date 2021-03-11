/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDirectContextPriv_DEFINED
#define GrDirectContextPriv_DEFINED

#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkSpan.h"

class GrAtlasManager;
class GrBackendFormat;
class GrBackendRenderTarget;
class GrMemoryPool;
class GrOnFlushCallbackObject;
class GrRenderTargetProxy;
class GrSemaphore;
class GrSurfaceProxy;

class SkDeferredDisplayList;
class SkTaskGroup;

/** Class that adds methods to GrDirectContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrDirectContext. It should never have additional
    data members or virtual methods. */
class GrDirectContextPriv {
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
    GrRecordingContext::Arenas arenas() { return fContext->arenas(); }

    GrStrikeCache* getGrStrikeCache() { return fContext->fStrikeCache.get(); }
    GrTextBlobCache* getTextBlobCache() { return fContext->getTextBlobCache(); }

    GrThreadSafeCache* threadSafeCache() { return fContext->threadSafeCache(); }

    /**
     * Registers an object for flush-related callbacks. (See GrOnFlushCallbackObject.)
     *
     * NOTE: the drawing manager tracks this object as a raw pointer; it is up to the caller to
     * ensure its lifetime is tied to that of the context.
     */
    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

    GrAuditTrail* auditTrail() { return fContext->auditTrail(); }

    /**
     * Finalizes all pending reads and writes to the surfaces and also performs an MSAA resolves
     * if necessary. The GrSurfaceProxy array is treated as a hint. If it is supplied the context
     * will guarantee that the draws required for those proxies are flushed but it could do more.
     * If no array is provided then all current work will be flushed.
     *
     * It is not necessary to call this before reading the render target via Skia/GrContext.
     * GrContext will detect when it must perform a resolve before reading pixels back from the
     * surface or using it as a texture.
     */
    GrSemaphoresSubmitted flushSurfaces(
                SkSpan<GrSurfaceProxy*>,
                SkSurface::BackendSurfaceAccess = SkSurface::BackendSurfaceAccess::kNoAccess,
                const GrFlushInfo& = {},
                const GrBackendSurfaceMutableState* newState = nullptr);

    /** Version of above that flushes for a single proxy. Null is allowed. */
    GrSemaphoresSubmitted flushSurface(
                GrSurfaceProxy* proxy,
                SkSurface::BackendSurfaceAccess access = SkSurface::BackendSurfaceAccess::kNoAccess,
                const GrFlushInfo& info = {},
                const GrBackendSurfaceMutableState* newState = nullptr) {
        size_t size = proxy ? 1 : 0;
        return this->flushSurfaces({&proxy, size}, access, info, newState);
    }

    /**
     * Returns true if createPMToUPMEffect and createUPMToPMEffect will succeed. In other words,
     * did we find a pair of round-trip preserving conversion effects?
     */
    bool validPMUPMConversionExists();

    /**
     * These functions create premul <-> unpremul effects, using the specialized round-trip effects
     * from GrConfigConversionEffect.
     */
    std::unique_ptr<GrFragmentProcessor> createPMToUPMEffect(std::unique_ptr<GrFragmentProcessor>);
    std::unique_ptr<GrFragmentProcessor> createUPMToPMEffect(std::unique_ptr<GrFragmentProcessor>);

    SkTaskGroup* getTaskGroup() { return fContext->fTaskGroup.get(); }

    GrResourceProvider* resourceProvider() { return fContext->fResourceProvider.get(); }
    const GrResourceProvider* resourceProvider() const { return fContext->fResourceProvider.get(); }

    GrResourceCache* getResourceCache() { return fContext->fResourceCache.get(); }

    GrGpu* getGpu() { return fContext->fGpu.get(); }
    const GrGpu* getGpu() const { return fContext->fGpu.get(); }

    // This accessor should only ever be called by the GrOpFlushState.
    GrAtlasManager* getAtlasManager() {
        return fContext->onGetAtlasManager();
    }

    // This accessor should only ever be called by the GrOpFlushState.
    GrSmallPathAtlasMgr* getSmallPathAtlasMgr() {
        return fContext->onGetSmallPathAtlasMgr();
    }

    void createDDLTask(sk_sp<const SkDeferredDisplayList>,
                       sk_sp<GrRenderTargetProxy> newDest,
                       SkIPoint offset);

    bool compile(const GrProgramDesc&, const GrProgramInfo&);

    GrContextOptions::PersistentCache* getPersistentCache() { return fContext->fPersistentCache; }
    GrContextOptions::ShaderErrorHandler* getShaderErrorHandler() const {
        return fContext->fShaderErrorHandler;
    }

    GrClientMappedBufferManager* clientMappedBufferManager() {
        return fContext->fMappedBufferManager.get();
    }

#if GR_TEST_UTILS
    /** Reset GPU stats */
    void resetGpuStats() const;

    /** Prints cache stats to the string if GR_CACHE_STATS == 1. */
    void dumpCacheStats(SkString*) const;
    void dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) const;
    void printCacheStats() const;

    /** Prints GPU stats to the string if GR_GPU_STATS == 1. */
    void dumpGpuStats(SkString*) const;
    void dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) const;
    void printGpuStats() const;

    /** These are only active if GR_GPU_STATS == 1. */
    void resetContextStats() const;
    void dumpContextStats(SkString*) const;
    void dumpContextStatsKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) const;
    void printContextStats() const;

    /** Get pointer to atlas texture for given mask format. Note that this wraps an
        actively mutating texture in an SkImage. This could yield unexpected results
        if it gets cached or used more generally. */
    sk_sp<SkImage> testingOnly_getFontAtlasImage(GrMaskFormat format, unsigned int index = 0);

    /**
     * Purge all the unlocked resources from the cache.
     * This entry point is mainly meant for timing texture uploads
     * and is not defined in normal builds of Skia.
     */
    void testingOnly_purgeAllUnlockedResources();

    void testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject*);
#endif

private:
    explicit GrDirectContextPriv(GrDirectContext* context) : fContext(context) {}
    GrDirectContextPriv(const GrDirectContextPriv&) = delete;
    GrDirectContextPriv& operator=(const GrDirectContextPriv&) = delete;

    // No taking addresses of this type.
    const GrDirectContextPriv* operator&() const;
    GrDirectContextPriv* operator&();

    GrDirectContext* fContext;

    friend class GrDirectContext; // to construct/copy this type.
};

inline GrDirectContextPriv GrDirectContext::priv() { return GrDirectContextPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const GrDirectContextPriv GrDirectContext::priv() const {
    return GrDirectContextPriv(const_cast<GrDirectContext*>(this));
}

#endif
