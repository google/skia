
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"

#include "GrBatchAtlas.h"
#include "GrBatchFontCache.h"
#include "GrContextOptions.h"
#include "GrDrawContext.h"
#include "GrDrawingManager.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrResourceCache.h"
#include "GrTextBlobCache.h"
#include "SkGrPriv.h"
#include "SkString.h"

namespace GrTest {
void SetupAlwaysEvictAtlas(GrContext* context) {
    // These sizes were selected because they allow each atlas to hold a single plot and will thus
    // stress the atlas
    int dim = GrBatchAtlas::kGlyphMaxDim;
    GrBatchAtlasConfig configs[3];
    configs[kA8_GrMaskFormat].fWidth = dim;
    configs[kA8_GrMaskFormat].fHeight = dim;
    configs[kA8_GrMaskFormat].fPlotWidth = dim;
    configs[kA8_GrMaskFormat].fPlotHeight = dim;

    configs[kA565_GrMaskFormat].fWidth = dim;
    configs[kA565_GrMaskFormat].fHeight = dim;
    configs[kA565_GrMaskFormat].fPlotWidth = dim;
    configs[kA565_GrMaskFormat].fPlotHeight = dim;

    configs[kARGB_GrMaskFormat].fWidth = dim;
    configs[kARGB_GrMaskFormat].fHeight = dim;
    configs[kARGB_GrMaskFormat].fPlotWidth = dim;
    configs[kARGB_GrMaskFormat].fPlotHeight = dim;

    context->setTextContextAtlasSizes_ForTesting(configs);
}
};

void GrTestTarget::init(GrContext* ctx, GrDrawTarget* target, GrRenderTarget* rt) {
    SkASSERT(!fContext);

    fContext.reset(SkRef(ctx));
    fDrawTarget.reset(SkRef(target));
    fRenderTarget.reset(SkRef(rt));
}

void GrContext::getTestTarget(GrTestTarget* tar, GrRenderTarget* rt) {
    this->flush();
    // We could create a proxy GrDrawTarget that passes through to fGpu until ~GrTextTarget() and
    // then disconnects. This would help prevent test writers from mixing using the returned
    // GrDrawTarget and regular drawing. We could also assert or fail in GrContext drawing methods
    // until ~GrTestTarget().
    if (!rt) {
        GrSurfaceDesc desc;
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        desc.fWidth = 32;
        desc.fHeight = 32;
        desc.fConfig = kRGBA_8888_GrPixelConfig;
        desc.fSampleCnt = 0;

        SkAutoTUnref<GrTexture> texture(this->textureProvider()->createTexture(desc, false,
                                                                               nullptr, 0));
        if (nullptr == texture) {
            return;
        }
        SkASSERT(nullptr != texture->asRenderTarget());
        rt = texture->asRenderTarget();
    }

    SkAutoTUnref<GrDrawTarget> dt(fDrawingManager->newDrawTarget(rt));
    tar->init(this, dt, rt);
}

void GrContext::setTextBlobCacheLimit_ForTesting(size_t bytes) {
    fTextBlobCache->setBudget(bytes);
}

void GrContext::setTextContextAtlasSizes_ForTesting(const GrBatchAtlasConfig* configs) {
    fBatchFontCache->setAtlasSizes_ForTesting(configs);
}

///////////////////////////////////////////////////////////////////////////////

void GrContext::purgeAllUnlockedResources() {
    fResourceCache->purgeAllUnlocked();
}

void GrContext::resetGpuStats() const {
#if GR_GPU_STATS
    fGpu->stats()->reset();
#endif
}

void GrContext::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    fResourceCache->dumpStats(out);
#endif
}

void GrContext::dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys,
                                            SkTArray<double>* values) const {
#if GR_CACHE_STATS
    fResourceCache->dumpStatsKeyValuePairs(keys, values);
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

void GrContext::dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys,
                                          SkTArray<double>* values) const {
#if GR_GPU_STATS
    return fGpu->stats()->dumpKeyValuePairs(keys, values);
#endif
}

void GrContext::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContext::drawFontCache(const SkRect& rect, GrMaskFormat format, const SkPaint& paint,
                              GrRenderTarget* target) {
    GrBatchFontCache* cache = this->getBatchFontCache();

    GrTexture* atlas = cache->getTexture(format);

    SkAutoTUnref<GrDrawContext> drawContext(this->drawContext(target));
    // TODO: add drawContext method to encapsulate this.

    GrPaint grPaint;
    SkMatrix mat;
    mat.reset();
    if (!SkPaintToGrPaint(this, paint, mat, &grPaint)) {
        return;
    }
    SkMatrix textureMat;
    textureMat.reset();
    // TODO: use setScaleTranslate()
    textureMat[SkMatrix::kMScaleX] = 1.0f/rect.width();
    textureMat[SkMatrix::kMScaleY] = 1.0f/rect.height();
    textureMat[SkMatrix::kMTransX] = -rect.fLeft/rect.width();
    textureMat[SkMatrix::kMTransY] = -rect.fTop/rect.height();

    grPaint.addColorTextureProcessor(atlas, textureMat);

    GrClip clip;
    drawContext->drawRect(clip, grPaint, mat, rect);
}

#if GR_GPU_STATS
void GrGpu::Stats::dump(SkString* out) {
    out->appendf("Render Target Binds: %d\n", fRenderTargetBinds);
    out->appendf("Shader Compilations: %d\n", fShaderCompilations);
    out->appendf("Textures Created: %d\n", fTextureCreates);
    out->appendf("Texture Uploads: %d\n", fTextureUploads);
    out->appendf("Stencil Buffer Creates: %d\n", fStencilAttachmentCreates);
    out->appendf("Number of draws: %d\n", fNumDraws);
}

void GrGpu::Stats::dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) {
    keys->push_back(SkString("render_target_binds")); values->push_back(fRenderTargetBinds);
    keys->push_back(SkString("shader_compilations")); values->push_back(fShaderCompilations);
    keys->push_back(SkString("textures_created")); values->push_back(fTextureCreates);
    keys->push_back(SkString("texture_uploads")); values->push_back(fTextureUploads);
    keys->push_back(SkString("stencil_buffer_creates")); values->push_back(fStencilAttachmentCreates);
    keys->push_back(SkString("number_of_draws")); values->push_back(fNumDraws);
}

#endif

#if GR_CACHE_STATS
void GrResourceCache::getStats(Stats* stats) const {
    stats->reset();

    stats->fTotal = this->getResourceCount();
    stats->fNumNonPurgeable = fNonpurgeableResources.count();
    stats->fNumPurgeable = fPurgeableQueue.count();

    for (int i = 0; i < fNonpurgeableResources.count(); ++i) {
        stats->update(fNonpurgeableResources[i]);
    }
    for (int i = 0; i < fPurgeableQueue.count(); ++i) {
        stats->update(fPurgeableQueue.at(i));
    }
}

void GrResourceCache::dumpStats(SkString* out) const {
    this->validate();

    Stats stats;

    this->getStats(&stats);

    float countUtilization = (100.f * fBudgetedCount) / fMaxCount;
    float byteUtilization = (100.f * fBudgetedBytes) / fMaxBytes;

    out->appendf("Budget: %d items %d bytes\n", fMaxCount, (int)fMaxBytes);
    out->appendf("\t\tEntry Count: current %d"
                 " (%d budgeted, %d external(%d borrowed, %d adopted), %d locked, %d scratch %.2g%% full), high %d\n",
                 stats.fTotal, fBudgetedCount, stats.fExternal, stats.fBorrowed,
                 stats.fAdopted, stats.fNumNonPurgeable, stats.fScratch, countUtilization,
                 fHighWaterCount);
    out->appendf("\t\tEntry Bytes: current %d (budgeted %d, %.2g%% full, %d unbudgeted) high %d\n",
                 SkToInt(fBytes), SkToInt(fBudgetedBytes), byteUtilization,
                 SkToInt(stats.fUnbudgetedSize), SkToInt(fHighWaterBytes));
}

void GrResourceCache::dumpStatsKeyValuePairs(SkTArray<SkString>* keys,
                                             SkTArray<double>* values) const {
    this->validate();

    Stats stats;
    this->getStats(&stats);

    keys->push_back(SkString("gpu_cache_total_entries")); values->push_back(stats.fTotal);
    keys->push_back(SkString("gpu_cache_external_entries")); values->push_back(stats.fExternal);
    keys->push_back(SkString("gpu_cache_borrowed_entries")); values->push_back(stats.fBorrowed);
    keys->push_back(SkString("gpu_cache_adopted_entries")); values->push_back(stats.fAdopted);
    keys->push_back(SkString("gpu_cache_purgable_entries")); values->push_back(stats.fNumPurgeable);
    keys->push_back(SkString("gpu_cache_non_purgable_entries")); values->push_back(stats.fNumNonPurgeable);
    keys->push_back(SkString("gpu_cache_scratch_entries")); values->push_back(stats.fScratch);
    keys->push_back(SkString("gpu_cache_unbudgeted_size")); values->push_back((double)stats.fUnbudgetedSize);
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
        fCaps.reset(new GrCaps(options));
    }
    ~MockGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return false; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height, size_t rowBytes,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return false; }

    void buildProgramDesc(GrProgramDesc*, const GrPrimitiveProcessor&,
                          const GrPipeline&) const override {}

    void discard(GrRenderTarget*) override {}

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override { return false; };

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const override {
        return false;
    }

private:
    void onResetContext(uint32_t resetBits) override {}

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle lifeCycle,
                               const void* srcData, size_t rowBytes) override {
        return nullptr;
    }

    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle,
                                         const void* srcData) override {
        return nullptr;
    }

    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&,
                                    GrWrapOwnership) override { return nullptr; }

    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&,
                                              GrWrapOwnership) override {
        return nullptr;
    }

    GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) override { return nullptr; }

    GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) override { return nullptr; }

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

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override {
        return nullptr;
    }

    void clearStencil(GrRenderTarget* target) override  {}

    GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                    GrPixelConfig config) const override {
        return 0; 
    }
    bool isTestingOnlyBackendTexture(GrBackendObject ) const override { return false; }
    void deleteTestingOnlyBackendTexture(GrBackendObject, bool abandonTexture) const override {}

    typedef GrGpu INHERITED;
};

GrContext* GrContext::CreateMockContext() {
    GrContext* context = new GrContext;

    context->initMockContext();
    return context;
}

void GrContext::initMockContext() {
    GrContextOptions options;
    options.fGeometryBufferMapThreshold = 0;
    SkASSERT(nullptr == fGpu);
    fGpu = new MockGpu(this, options);
    SkASSERT(fGpu);
    this->initCommon(options);

    // We delete these because we want to test the cache starting with zero resources. Also, none of
    // these objects are required for any of tests that use this context. TODO: make stop allocating
    // resources in the buffer pools.
    fDrawingManager->abandon();
}
