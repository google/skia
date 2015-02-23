
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"

#include "GrGpuResourceCacheAccess.h"
#include "GrInOrderDrawBuffer.h"
#include "GrResourceCache.h"
#include "SkString.h"

void GrTestTarget::init(GrContext* ctx, GrDrawTarget* target) {
    SkASSERT(!fContext);

    fContext.reset(SkRef(ctx));
    fDrawTarget.reset(SkRef(target));

    SkNEW_IN_TLAZY(&fAGP, GrDrawTarget::AutoGeometryPush, (target));
}

void GrContext::getTestTarget(GrTestTarget* tar) {
    this->flush();
    // We could create a proxy GrDrawTarget that passes through to fGpu until ~GrTextTarget() and
    // then disconnects. This would help prevent test writers from mixing using the returned
    // GrDrawTarget and regular drawing. We could also assert or fail in GrContext drawing methods
    // until ~GrTestTarget().
    tar->init(this, fDrawBuffer);
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::setMaxTextureSizeOverride(int maxTextureSizeOverride) {
    fMaxTextureSizeOverride = maxTextureSizeOverride;
}

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
    out->appendf("Stencil Buffer Creates: %d\n", fStencilBufferCreates);
}
#endif

#if GR_CACHE_STATS
void GrResourceCache::dumpStats(SkString* out) const {
    this->validate();

    int locked = fNonpurgeableResources.count();

    struct Stats {
        int fScratch;
        int fWrapped;
        size_t fUnbudgetedSize;

        Stats() : fScratch(0), fWrapped(0), fUnbudgetedSize(0) {}

        void update(GrGpuResource* resource) {
            if (resource->cacheAccess().isScratch()) {
                ++fScratch;
            }
            if (resource->cacheAccess().isWrapped()) {
                ++fWrapped;
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
                 " (%d budgeted, %d wrapped, %d locked, %d scratch %.2g%% full), high %d\n",
                 this->getResourceCount(), fBudgetedCount, stats.fWrapped, locked, stats.fScratch,
                 countUtilization, fHighWaterCount);
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

#include "GrBufferAllocPool.h"
#include "GrInOrderDrawBuffer.h"
#include "GrGpu.h"

class GrPipeline;

class MockGpu : public GrGpu {
public:
    MockGpu(GrContext* context) : INHERITED(context) { fCaps.reset(SkNEW(GrDrawTargetCaps)); }
    ~MockGpu() SK_OVERRIDE {}
    bool canWriteTexturePixels(const GrTexture*, GrPixelConfig srcConfig) const SK_OVERRIDE {
        return true;
    }

    bool readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                   int left, int top,
                                   int width, int height,
                                   GrPixelConfig config,
                                   size_t rowBytes) const SK_OVERRIDE { return false; }
    void buildProgramDesc(GrProgramDesc*,const GrPrimitiveProcessor&,
                          const GrPipeline&,
                          const GrBatchTracker&) const SK_OVERRIDE {}

    void discard(GrRenderTarget*) SK_OVERRIDE {}

    bool canCopySurface(const GrSurface* dst,
                        const GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint) SK_OVERRIDE { return false; };

    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) SK_OVERRIDE { return false; };

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) SK_OVERRIDE {
        return false;
    }

private:
    void onResetContext(uint32_t resetBits) SK_OVERRIDE {}

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, bool budgeted, const void* srcData,
                               size_t rowBytes) SK_OVERRIDE {
        return NULL;
    }

    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, bool budgeted,
                                         const void* srcData) SK_OVERRIDE {
        return NULL;
    }

    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&) SK_OVERRIDE { return NULL; }

    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) SK_OVERRIDE {
        return NULL;
    }

    GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) SK_OVERRIDE { return NULL; }

    GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) SK_OVERRIDE { return NULL; }

    void onClear(GrRenderTarget*, const SkIRect* rect, GrColor color,
                         bool canIgnoreRect) SK_OVERRIDE {}

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) SK_OVERRIDE {}

    void onDraw(const DrawArgs&, const GrDrawTarget::DrawInfo&) SK_OVERRIDE {}

    void onStencilPath(const GrPath* path, const StencilPathState& state) SK_OVERRIDE {}

    void onDrawPath(const DrawArgs&, const GrPath*, const GrStencilSettings&) SK_OVERRIDE {}

    void onDrawPaths(const DrawArgs&,
                     const GrPathRange*,
                     const void* indices,
                     GrDrawTarget::PathIndexType,
                     const float transformValues[],
                     GrDrawTarget::PathTransformType,
                     int count,
                     const GrStencilSettings&) SK_OVERRIDE {}

    bool onReadPixels(GrRenderTarget* target,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) SK_OVERRIDE {
        return false;
    }

    bool onWriteTexturePixels(GrTexture* texture,
                              int left, int top, int width, int height,
                              GrPixelConfig config, const void* buffer,
                              size_t rowBytes) SK_OVERRIDE {
        return false;
    }

    void onResolveRenderTarget(GrRenderTarget* target) SK_OVERRIDE { return; }

    bool createStencilBufferForRenderTarget(GrRenderTarget*, int width, int height) SK_OVERRIDE {
        return false;
    }

    bool attachStencilBufferToRenderTarget(GrStencilBuffer*, GrRenderTarget*) SK_OVERRIDE {
        return false;
    }

    void clearStencil(GrRenderTarget* target) SK_OVERRIDE  {}

    void didAddGpuTraceMarker() SK_OVERRIDE {}

    void didRemoveGpuTraceMarker() SK_OVERRIDE {}

    typedef GrGpu INHERITED;
};

GrContext* GrContext::CreateMockContext() {
    GrContext* context = SkNEW_ARGS(GrContext, (Options()));

    context->initMockContext();
    return context;
}

void GrContext::initMockContext() {
    SkASSERT(NULL == fGpu);
    fGpu = SkNEW_ARGS(MockGpu, (this));
    SkASSERT(fGpu);
    this->initCommon();

    // We delete these because we want to test the cache starting with zero resources. Also, none of
    // these objects are required for any of tests that use this context. TODO: make stop allocating
    // resources in the buffer pools.
    SkDELETE(fDrawBuffer);
    SkDELETE(fDrawBufferVBAllocPool);
    SkDELETE(fDrawBufferIBAllocPool);

    fDrawBuffer = NULL;
    fDrawBufferVBAllocPool = NULL;
    fDrawBufferIBAllocPool = NULL;
}
