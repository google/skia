/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDirectContextPriv_DEFINED
#define GrDirectContextPriv_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrContextThreadSafeProxy.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"

#include <cstddef>
#include <memory>
#include <utility>

class GrAtlasManager;
class GrClientMappedBufferManager;
class GrDeferredDisplayList;
class GrFragmentProcessor;
class GrOnFlushCallbackObject;
class GrProgramDesc;
class GrProgramInfo;
class GrRenderTargetProxy;
class GrResourceCache;
class GrResourceProvider;
class GrSurfaceProxy;
class SkImage;
class SkString;
class SkTaskGroup;
enum class GrBackendApi : unsigned int;
enum class GrSemaphoresSubmitted : bool;
struct GrFlushInfo;

namespace skgpu {
class MutableTextureState;
enum class MaskFormat : int;
namespace ganesh {
class SmallPathAtlasMgr;
}
}  // namespace skgpu
namespace sktext {
namespace gpu {
class StrikeCache;
}
}  // namespace sktext

/** Class that adds methods to GrDirectContext that are only intended for use internal to Skia.
    This class is purely a privileged window into GrDirectContext. It should never have additional
    data members or virtual methods. */
class GrDirectContextPriv : public GrRecordingContextPriv {
public:
    static sk_sp<GrDirectContext> Make(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       sk_sp<GrContextThreadSafeProxy> proxy) {
        return sk_sp<GrDirectContext>(new GrDirectContext(backend, options, std::move(proxy)));
    }

    static bool Init(const sk_sp<GrDirectContext>& ctx) {
        SkASSERT(ctx);
        return ctx->init();
    }

    static void SetGpu(const sk_sp<GrDirectContext>& ctx, std::unique_ptr<GrGpu> gpu) {
        SkASSERT(ctx);
        ctx->fGpu = std::move(gpu);
    }

    GrDirectContext* context() { return static_cast<GrDirectContext*>(fContext); }
    const GrDirectContext* context() const { return static_cast<const GrDirectContext*>(fContext); }

    sktext::gpu::StrikeCache* getStrikeCache() { return this->context()->fStrikeCache.get(); }

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
            SkSurfaces::BackendSurfaceAccess = SkSurfaces::BackendSurfaceAccess::kNoAccess,
            const GrFlushInfo& = {},
            const skgpu::MutableTextureState* newState = nullptr);

    /** Version of above that flushes for a single proxy. Null is allowed. */
    GrSemaphoresSubmitted flushSurface(
            GrSurfaceProxy* proxy,
            SkSurfaces::BackendSurfaceAccess access = SkSurfaces::BackendSurfaceAccess::kNoAccess,
            const GrFlushInfo& info = {},
            const skgpu::MutableTextureState* newState = nullptr) {
        size_t size = proxy ? 1 : 0;
        return this->flushSurfaces({&proxy, size}, access, info, newState);
    }

    /**
     * Returns true if createPMToUPMEffect and createUPMToPMEffect will succeed. In other words,
     * did we find a pair of round-trip preserving conversion effects?
     */
    bool validPMUPMConversionExists();

    /**
     * These functions create premul <-> unpremul effects, using specialized round-trip effects.
     */
    std::unique_ptr<GrFragmentProcessor> createPMToUPMEffect(std::unique_ptr<GrFragmentProcessor>);
    std::unique_ptr<GrFragmentProcessor> createUPMToPMEffect(std::unique_ptr<GrFragmentProcessor>);

    SkTaskGroup* getTaskGroup() { return this->context()->fTaskGroup.get(); }

    GrResourceProvider* resourceProvider() { return this->context()->fResourceProvider.get(); }
    const GrResourceProvider* resourceProvider() const {
        return this->context()->fResourceProvider.get();
    }

    GrResourceCache* getResourceCache() { return this->context()->fResourceCache.get(); }

    GrGpu* getGpu() { return this->context()->fGpu.get(); }
    const GrGpu* getGpu() const { return this->context()->fGpu.get(); }

    // This accessor should only ever be called by the GrOpFlushState.
    GrAtlasManager* getAtlasManager() {
        return this->context()->onGetAtlasManager();
    }

    // This accessor should only ever be called by the GrOpFlushState.
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    skgpu::ganesh::SmallPathAtlasMgr* getSmallPathAtlasMgr() {
        return this->context()->onGetSmallPathAtlasMgr();
    }
#endif

    void createDDLTask(sk_sp<const GrDeferredDisplayList>,
                       sk_sp<GrRenderTargetProxy> newDest);

    bool compile(const GrProgramDesc&, const GrProgramInfo&);

    GrContextOptions::PersistentCache* getPersistentCache() {
        return this->context()->fPersistentCache;
    }

    GrClientMappedBufferManager* clientMappedBufferManager() {
        return this->context()->fMappedBufferManager.get();
    }

    void setInsideReleaseProc(bool inside) {
        if (inside) {
            this->context()->fInsideReleaseProcCnt++;
        } else {
            SkASSERT(this->context()->fInsideReleaseProcCnt > 0);
            this->context()->fInsideReleaseProcCnt--;
        }
    }

#if defined(GPU_TEST_UTILS)
    /** Reset GPU stats */
    void resetGpuStats() const;

    /** Prints cache stats to the string if GR_CACHE_STATS == 1. */
    void dumpCacheStats(SkString*) const;
    void dumpCacheStatsKeyValuePairs(
            skia_private::TArray<SkString>* keys, skia_private::TArray<double>* values) const;
    void printCacheStats() const;

    /** Prints GPU stats to the string if GR_GPU_STATS == 1. */
    void dumpGpuStats(SkString*) const;
    void dumpGpuStatsKeyValuePairs(
            skia_private::TArray<SkString>* keys, skia_private::TArray<double>* values) const;
    void printGpuStats() const;

    /** These are only active if GR_GPU_STATS == 1. */
    void resetContextStats();
    void dumpContextStats(SkString*) const;
    void dumpContextStatsKeyValuePairs(
            skia_private::TArray<SkString>* keys, skia_private::TArray<double>* values) const;
    void printContextStats() const;

    /** Get pointer to atlas texture for given mask format. Note that this wraps an
        actively mutating texture in an SkImage. This could yield unexpected results
        if it gets cached or used more generally. */
    sk_sp<SkImage> testingOnly_getFontAtlasImage(skgpu::MaskFormat format, unsigned int index = 0);

    void testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject*);
#endif

private:
    explicit GrDirectContextPriv(GrDirectContext* dContext) : GrRecordingContextPriv(dContext) {}
    GrDirectContextPriv& operator=(const GrDirectContextPriv&) = delete;

    // No taking addresses of this type.
    const GrDirectContextPriv* operator&() const;
    GrDirectContextPriv* operator&();

    friend class GrDirectContext; // to construct/copy this type.

    using INHERITED = GrRecordingContextPriv;
};

inline GrDirectContextPriv GrDirectContext::priv() { return GrDirectContextPriv(this); }

// NOLINTNEXTLINE(readability-const-return-type)
inline const GrDirectContextPriv GrDirectContext::priv() const {
    return GrDirectContextPriv(const_cast<GrDirectContext*>(this));
}

#endif
