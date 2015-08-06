
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"

#include "GrBufferedDrawTarget.h"
#include "GrContextOptions.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrResourceCache.h"
#include "GrTextBlobCache.h"
#include "SkString.h"

void GrTestTarget::init(GrContext* ctx, GrDrawTarget* target) {
    SkASSERT(!fContext);

    fContext.reset(SkRef(ctx));
    fDrawTarget.reset(SkRef(target));
}

void GrContext::getTestTarget(GrTestTarget* tar) {
    this->flush();
    // We could create a proxy GrDrawTarget that passes through to fGpu until ~GrTextTarget() and
    // then disconnects. This would help prevent test writers from mixing using the returned
    // GrDrawTarget and regular drawing. We could also assert or fail in GrContext drawing methods
    // until ~GrTestTarget().
    tar->init(this, fDrawingMgr.fDrawTarget);
}

void GrContext::setTextBlobCacheLimit_ForTesting(size_t bytes) {
    fTextBlobCache->setBudget(bytes);
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::purgeAllUnlockedResources() {
    fResourceCache->purgeAllUnlocked();
}

void GrContext::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    fResourceCache->dumpStats(out);
#endif
}

void GrContext::printCacheStats() const {
    SkString out;
    this->dumpCacheStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContext::dumpGpuStats(SkString* out) const {
#if GR_GPU_STATS
    return fGpu->stats()->dump(out);
#endif
}

void GrContext::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

#if GR_GPU_STATS
void GrGpu::Stats::dump(SkString* out) {
    out->appendf("Render Target Binds: %d\n", fRenderTargetBinds);
    out->appendf("Shader Compilations: %d\n", fShaderCompilations);
    out->appendf("Textures Created: %d\n", fTextureCreates);
    out->appendf("Texture Uploads: %d\n", fTextureUploads);
    out->appendf("Stencil Buffer Creates: %d\n", fStencilAttachmentCreates);
}
#endif

#if GR_CACHE_STATS
void GrResourceCache::dumpStats(SkString* out) const {
    this->validate();

    int locked = fNonpurgeableResources.count();

    struct Stats {
        int fScratch;
        int fExternal;
        int fBorrowed;
        int fAdopted;
        size_t fUnbudgetedSize;

        Stats() : fScratch(0), fExternal(0), fBorrowed(0), fAdopted(0), fUnbudgetedSize(0) {}

        void update(GrGpuResource* resource) {
            if (resource->cacheAccess().isScratch()) {
                ++fScratch;
            }
            if (resource->cacheAccess().isExternal()) {
                ++fExternal;
            }
            if (resource->cacheAccess().isBorrowed()) {
                ++fBorrowed;
            }
            if (resource->cacheAccess().isAdopted()) {
                ++fAdopted;
            }
            if (!resource->resourcePriv().isBudgeted()) {
                fUnbudgetedSize += resource->gpuMemorySize();
            }
        }
    };

    Stats stats;

    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        stats.update(fNonpurgeableResources[i]);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        stats.update(fPurgeableQueue.at(i));
    }

    float countUtilization = (100.f * fBudgetedCount) / fMaxCount;
    float byteUtilization = (100.f * fBudgetedBytes) / fMaxBytes;

    out->appendf("Budget: %d items %d bytes\n", fMaxCount, (int)fMaxBytes);
    out->appendf("\t\tEntry Count: current %d"
                 " (%d budgeted, %d external(%d borrowed, %d adopted), %d locked, %d scratch %.2g%% full), high %d\n",
                 this->getResourceCount(), fBudgetedCount, stats.fExternal, stats.fBorrowed,
                 stats.fAdopted, locked, stats.fScratch, countUtilization, fHighWaterCount);
    out->appendf("\t\tEntry Bytes: current %d (budgeted %d, %.2g%% full, %d unbudgeted) high %d\n",
                 SkToInt(fBytes), SkToInt(fBudgetedBytes), byteUtilization,
                 SkToInt(stats.fUnbudgetedSize), SkToInt(fHighWaterBytes));
}

#endif

///////////////////////////////////////////////////////////////////////////////

void GrResourceCache::changeTimestamp(uint32_t newTimestamp) { fTimestamp = newTimestamp; }

///////////////////////////////////////////////////////////////////////////////
// Code for the mock context. It's built on a mock GrGpu class that does nothing.
////

#include "GrGpu.h"

class GrPipeline;

class MockGpu : public GrGpu {
public:
    MockGpu(GrContext* context, const GrContextOptions& options) : INHERITED(context) {
        fCaps.reset(SkNEW_ARGS(GrCaps, (options)));
    }
    ~MockGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return false; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height, size_t rowBytes,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return false; }

    void buildProgramDesc(GrProgramDesc*,const GrPrimitiveProcessor&,
                          const GrPipeline&,
                          const GrBatchTracker&) const override {}

    void discard(GrRenderTarget*) override {}

    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) override { return false; };

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const override {
        return false;
    }

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

private:
    void onResetContext(uint32_t resetBits) override {}

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle lifeCycle,
                               const void* srcData, size_t rowBytes) override {
        return NULL;
    }

    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle,
                                         const void* srcData) override {
        return NULL;
    }

    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&,
                                    GrWrapOwnership) override { return NULL; }

    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&,
                                              GrWrapOwnership) override {
        return NULL;
    }

    GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) override { return NULL; }

    GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) override { return NULL; }

    void onClear(GrRenderTarget*, const SkIRect& rect, GrColor color) override {}

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) override {}

    void onDraw(const DrawArgs&, const GrNonInstancedVertices&) override {}

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        return false;
    }

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrPixelConfig config, const void* buffer,
                       size_t rowBytes) override {
        return false;
    }

    void onResolveRenderTarget(GrRenderTarget* target) override { return; }

    bool createStencilAttachmentForRenderTarget(GrRenderTarget*, int width, int height) override {
        return false;
    }

    bool attachStencilAttachmentToRenderTarget(GrStencilAttachment*, GrRenderTarget*) override {
        return false;
    }

    void clearStencil(GrRenderTarget* target) override  {}

    void didAddGpuTraceMarker() override {}

    void didRemoveGpuTraceMarker() override {}

    GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                    GrPixelConfig config) const override {
        return 0; 
    }
    bool isTestingOnlyBackendTexture(GrBackendObject id) const override { return false; }
    void deleteTestingOnlyBackendTexture(GrBackendObject id) const override {}

    typedef GrGpu INHERITED;
};

GrContext* GrContext::CreateMockContext() {
    GrContext* context = SkNEW(GrContext);

    context->initMockContext();
    return context;
}

void GrContext::initMockContext() {
    GrContextOptions options;
    options.fGeometryBufferMapThreshold = 0;
    SkASSERT(NULL == fGpu);
    fGpu = SkNEW_ARGS(MockGpu, (this, options));
    SkASSERT(fGpu);
    this->initCommon();

    // We delete these because we want to test the cache starting with zero resources. Also, none of
    // these objects are required for any of tests that use this context. TODO: make stop allocating
    // resources in the buffer pools.
    fDrawingMgr.abandon();
}
