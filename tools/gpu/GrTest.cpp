/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"

#include "GrBatchAtlas.h"
#include "GrContextOptions.h"
#include "GrDrawContextPriv.h"
#include "GrDrawingManager.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrResourceCache.h"

#include "SkGpuDevice.h"
#include "SkGrPriv.h"
#include "SkMathPriv.h"
#include "SkString.h"

#include "text/GrBatchFontCache.h"
#include "text/GrTextBlobCache.h"

namespace GrTest {
void SetupAlwaysEvictAtlas(GrContext* context) {
    // These sizes were selected because they allow each atlas to hold a single plot and will thus
    // stress the atlas
    int dim = GrBatchAtlas::kGlyphMaxDim;
    GrBatchAtlasConfig configs[3];
    configs[kA8_GrMaskFormat].fWidth = dim;
    configs[kA8_GrMaskFormat].fHeight = dim;
    configs[kA8_GrMaskFormat].fLog2Width = SkNextLog2(dim);
    configs[kA8_GrMaskFormat].fLog2Height = SkNextLog2(dim);
    configs[kA8_GrMaskFormat].fPlotWidth = dim;
    configs[kA8_GrMaskFormat].fPlotHeight = dim;

    configs[kA565_GrMaskFormat].fWidth = dim;
    configs[kA565_GrMaskFormat].fHeight = dim;
    configs[kA565_GrMaskFormat].fLog2Width = SkNextLog2(dim);
    configs[kA565_GrMaskFormat].fLog2Height = SkNextLog2(dim);
    configs[kA565_GrMaskFormat].fPlotWidth = dim;
    configs[kA565_GrMaskFormat].fPlotHeight = dim;

    configs[kARGB_GrMaskFormat].fWidth = dim;
    configs[kARGB_GrMaskFormat].fHeight = dim;
    configs[kARGB_GrMaskFormat].fLog2Width = SkNextLog2(dim);
    configs[kARGB_GrMaskFormat].fLog2Height = SkNextLog2(dim);
    configs[kARGB_GrMaskFormat].fPlotWidth = dim;
    configs[kARGB_GrMaskFormat].fPlotHeight = dim;

    context->setTextContextAtlasSizes_ForTesting(configs);
}
};

void GrTestTarget::init(GrContext* ctx, sk_sp<GrDrawContext> drawContext) {
    SkASSERT(!fContext);

    fContext.reset(SkRef(ctx));
    fDrawContext = drawContext;
}

void GrContext::getTestTarget(GrTestTarget* tar, sk_sp<GrDrawContext> drawContext) {
    this->flush();
    SkASSERT(drawContext);
    // We could create a proxy GrDrawTarget that passes through to fGpu until ~GrTextTarget() and
    // then disconnects. This would help prevent test writers from mixing using the returned
    // GrDrawTarget and regular drawing. We could also assert or fail in GrContext drawing methods
    // until ~GrTestTarget().
    tar->init(this, std::move(drawContext));
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

GrTexture* GrContext::getFontAtlasTexture(GrMaskFormat format) {
    GrBatchFontCache* cache = this->getBatchFontCache();

    return cache->getTexture(format);
}

void SkGpuDevice::drawTexture(GrTexture* tex, const SkRect& dst, const SkPaint& paint) {
    GrPaint grPaint;
    SkMatrix mat;
    mat.reset();
    if (!SkPaintToGrPaint(this->context(), paint, mat,
                          this->surfaceProps().isGammaCorrect(), &grPaint)) {
        return;
    }
    SkMatrix textureMat;
    textureMat.reset();
    textureMat[SkMatrix::kMScaleX] = 1.0f/dst.width();
    textureMat[SkMatrix::kMScaleY] = 1.0f/dst.height();
    textureMat[SkMatrix::kMTransX] = -dst.fLeft/dst.width();
    textureMat[SkMatrix::kMTransY] = -dst.fTop/dst.height();

    grPaint.addColorTextureProcessor(tex, textureMat);

    fDrawContext->drawRect(GrNoClip(), grPaint, mat, dst);
}


#if GR_GPU_STATS
void GrGpu::Stats::dump(SkString* out) {
    out->appendf("Render Target Binds: %d\n", fRenderTargetBinds);
    out->appendf("Shader Compilations: %d\n", fShaderCompilations);
    out->appendf("Textures Created: %d\n", fTextureCreates);
    out->appendf("Texture Uploads: %d\n", fTextureUploads);
    out->appendf("Transfers to Texture: %d\n", fTransfersToTexture);
    out->appendf("Stencil Buffer Creates: %d\n", fStencilAttachmentCreates);
    out->appendf("Number of draws: %d\n", fNumDraws);
}

void GrGpu::Stats::dumpKeyValuePairs(SkTArray<SkString>* keys, SkTArray<double>* values) {
    keys->push_back(SkString("render_target_binds")); values->push_back(fRenderTargetBinds);
    keys->push_back(SkString("shader_compilations")); values->push_back(fShaderCompilations);
    keys->push_back(SkString("texture_uploads")); values->push_back(fTextureUploads);
    keys->push_back(SkString("number_of_draws")); values->push_back(fNumDraws);
    keys->push_back(SkString("number_of_failed_draws")); values->push_back(fNumFailedDraws);
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
                 " (%d budgeted, %d wrapped, %d locked, %d scratch %.2g%% full), high %d\n",
                 stats.fTotal, fBudgetedCount, stats.fWrapped, stats.fNumNonPurgeable,
                 stats.fScratch, countUtilization, fHighWaterCount);
    out->appendf("\t\tEntry Bytes: current %d (budgeted %d, %.2g%% full, %d unbudgeted) high %d\n",
                 SkToInt(fBytes), SkToInt(fBudgetedBytes), byteUtilization,
                 SkToInt(stats.fUnbudgetedSize), SkToInt(fHighWaterBytes));
}

void GrResourceCache::dumpStatsKeyValuePairs(SkTArray<SkString>* keys,
                                             SkTArray<double>* values) const {
    this->validate();

    Stats stats;
    this->getStats(&stats);

    keys->push_back(SkString("gpu_cache_purgable_entries")); values->push_back(stats.fNumPurgeable);
}

#endif

///////////////////////////////////////////////////////////////////////////////

void GrResourceCache::changeTimestamp(uint32_t newTimestamp) { fTimestamp = newTimestamp; }

///////////////////////////////////////////////////////////////////////////////

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fDrawContext->fSingleOwner);)
#define RETURN_IF_ABANDONED        if (fDrawContext->fDrawingManager->wasAbandoned()) { return; }

void GrDrawContextPriv::testingOnly_drawBatch(const GrPaint& paint,
                                              GrDrawBatch* batch,
                                              const GrUserStencilSettings* uss,
                                              bool snapToCenters) {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED
    SkDEBUGCODE(fDrawContext->validate();)
    GR_AUDIT_TRAIL_AUTO_FRAME(fDrawContext->fAuditTrail, "GrDrawContext::testingOnly_drawBatch");

    GrPipelineBuilder pipelineBuilder(paint, fDrawContext->mustUseHWAA(paint));
    if (uss) {
        pipelineBuilder.setUserStencil(uss);
    }
    if (snapToCenters) {
        pipelineBuilder.setState(GrPipelineBuilder::kSnapVerticesToPixelCenters_Flag, true);
    }

    fDrawContext->getDrawTarget()->drawBatch(pipelineBuilder, fDrawContext, GrNoClip(), batch);
}

#undef ASSERT_SINGLE_OWNER
#undef RETURN_IF_ABANDONED

///////////////////////////////////////////////////////////////////////////////
// Code for the mock context. It's built on a mock GrGpu class that does nothing.
////

#include "GrGpu.h"

class GrPipeline;

class MockCaps : public GrCaps {
public:
    explicit MockCaps(const GrContextOptions& options) : INHERITED(options) {}
    bool isConfigTexturable(GrPixelConfig config) const override { return false; }
    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override { return false; }
private:
    typedef GrCaps INHERITED;
};

class MockGpu : public GrGpu {
public:
    MockGpu(GrContext* context, const GrContextOptions& options) : INHERITED(context) {
        fCaps.reset(new MockCaps(options));
    }
    ~MockGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return false; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return false; }

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override { return false; };

    void onGetMultisampleSpecs(GrRenderTarget* rt,
                               const GrStencilSettings&,
                               int* effectiveSampleCnt,
                               SkAutoTDeleteArray<SkPoint>*) override {
        *effectiveSampleCnt = rt->desc().fSampleCnt;
    }

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) const override {
        return false;
    }

    GrGpuCommandBuffer* createCommandBuffer(GrRenderTarget* target,
                                            const GrGpuCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuCommandBuffer::LoadAndStoreInfo&) override {
        return nullptr;
    }

    void drawDebugWireRect(GrRenderTarget*, const SkIRect&, GrColor) override {};

private:
    void onResetContext(uint32_t resetBits) override {}

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                               const SkTArray<GrMipLevel>& texels) override {
        return nullptr;
    }

    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                         const SkTArray<GrMipLevel>& texels) override {
        return nullptr;
    }

    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&,
                                    GrWrapOwnership) override { return nullptr; }

    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&,
                                              GrWrapOwnership) override {
        return nullptr;
    }

    GrRenderTarget* onWrapBackendTextureAsRenderTarget(const GrBackendTextureDesc&) override {
        return nullptr;
    }

    GrBuffer* onCreateBuffer(size_t, GrBufferType, GrAccessPattern, const void*) override {
        return nullptr;
    }

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        return false;
    }

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrPixelConfig config, const SkTArray<GrMipLevel>& texels) override {
        return false;
    }

    bool onTransferPixels(GrSurface* surface,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override {
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
                                                    GrPixelConfig config, bool isRT) override {
        return 0;
    }
    bool isTestingOnlyBackendTexture(GrBackendObject ) const override { return false; }
    void deleteTestingOnlyBackendTexture(GrBackendObject, bool abandonTexture) override {}

    typedef GrGpu INHERITED;
};

GrContext* GrContext::CreateMockContext() {
    GrContext* context = new GrContext;

    context->initMockContext();
    return context;
}

void GrContext::initMockContext() {
    GrContextOptions options;
    options.fBufferMapThreshold = 0;
    SkASSERT(nullptr == fGpu);
    fGpu = new MockGpu(this, options);
    SkASSERT(fGpu);
    this->initCommon(options);

    // We delete these because we want to test the cache starting with zero resources. Also, none of
    // these objects are required for any of tests that use this context. TODO: make stop allocating
    // resources in the buffer pools.
    fDrawingManager->abandon();
}
