/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTest.h"
#include <algorithm>
#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrContextPriv.h"
#include "GrDrawOpAtlas.h"
#include "GrDrawingManager.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrPipelineBuilder.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrSemaphore.h"
#include "GrSurfaceContextPriv.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkMathPriv.h"
#include "SkString.h"
#include "ops/GrMeshDrawOp.h"
#include "text/GrAtlasGlyphCache.h"
#include "text/GrTextBlobCache.h"

namespace GrTest {
void SetupAlwaysEvictAtlas(GrContext* context) {
    // These sizes were selected because they allow each atlas to hold a single plot and will thus
    // stress the atlas
    int dim = GrDrawOpAtlas::kGlyphMaxDim;
    GrDrawOpAtlasConfig configs[3];
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

GrBackendTexture CreateBackendTexture(GrBackend backend, int width, int height,
                                      GrPixelConfig config, GrBackendObject handle) {
    if (kOpenGL_GrBackend == backend) {
        GrGLTextureInfo* glInfo = (GrGLTextureInfo*)(handle);
        return GrBackendTexture(width, height, config, *glInfo);
    } else {
        SkASSERT(kVulkan_GrBackend == backend);
        GrVkImageInfo* vkInfo = (GrVkImageInfo*)(handle);
        return GrBackendTexture(width, height, *vkInfo);
    }
}
};

bool GrSurfaceProxy::isWrapped_ForTesting() const {
    return SkToBool(fTarget);
}

bool GrRenderTargetContext::isWrapped_ForTesting() const {
    return fRenderTargetProxy->isWrapped_ForTesting();
}

void GrContext::setTextBlobCacheLimit_ForTesting(size_t bytes) {
    fTextBlobCache->setBudget(bytes);
}

void GrContext::setTextContextAtlasSizes_ForTesting(const GrDrawOpAtlasConfig* configs) {
    fAtlasGlyphCache->setAtlasSizes_ForTesting(configs);
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

sk_sp<SkImage> GrContext::getFontAtlasImage_ForTesting(GrMaskFormat format) {
    GrAtlasGlyphCache* cache = this->getAtlasGlyphCache();

    sk_sp<GrTextureProxy> proxy = cache->getProxy(format);
    if (!proxy) {
        return nullptr;
    }

    SkASSERT(proxy->priv().isExact());
    sk_sp<SkImage> image(new SkImage_Gpu(this, kNeedNewImageUniqueID, kPremul_SkAlphaType,
                                         std::move(proxy), nullptr, SkBudgeted::kNo));
    return image;
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

#ifdef SK_DEBUG
int GrResourceCache::countUniqueKeysWithTag(const char* tag) const {
    int count = 0;
    UniqueHash::ConstIter iter(&fUniqueHash);
    while (!iter.done()) {
        if (0 == strcmp(tag, (*iter).getUniqueKey().tag())) {
            ++count;
        }
        ++iter;
    }
    return count;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fRenderTargetContext->singleOwner());)

uint32_t GrRenderTargetContextPriv::testingOnly_addLegacyMeshDrawOp(
        GrPaint&& paint,
        GrAAType aaType,
        std::unique_ptr<GrLegacyMeshDrawOp> op,
        const GrUserStencilSettings* uss,
        bool snapToCenters) {
    ASSERT_SINGLE_OWNER
    if (fRenderTargetContext->drawingManager()->wasAbandoned()) {
        return SK_InvalidUniqueID;
    }
    SkDEBUGCODE(fRenderTargetContext->validate());
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::testingOnly_addLegacyMeshDrawOp");

    GrPipelineBuilder pipelineBuilder(std::move(paint), aaType);
    if (uss) {
        pipelineBuilder.setUserStencil(uss);
    }
    pipelineBuilder.setSnapVerticesToPixelCenters(snapToCenters);

    return fRenderTargetContext->addLegacyMeshDrawOp(std::move(pipelineBuilder), GrNoClip(),
                                                     std::move(op));
}

uint32_t GrRenderTargetContextPriv::testingOnly_addDrawOp(std::unique_ptr<GrDrawOp> op) {
    ASSERT_SINGLE_OWNER
    if (fRenderTargetContext->drawingManager()->wasAbandoned()) {
        return SK_InvalidUniqueID;
    }
    SkDEBUGCODE(fRenderTargetContext->validate());
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::testingOnly_addDrawOp");
    return fRenderTargetContext->addDrawOp(GrNoClip(), std::move(op));
}

#undef ASSERT_SINGLE_OWNER

///////////////////////////////////////////////////////////////////////////////

GrRenderTarget::Flags GrRenderTargetProxy::testingOnly_getFlags() const {
    return fRenderTargetFlags;
}

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
    bool canConfigBeImageStorage(GrPixelConfig) const override { return false; }
    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

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
                       const SkIPoint& dstPoint) override { return false; }

    void onQueryMultisampleSpecs(GrRenderTarget* rt, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override {
        *effectiveSampleCnt = rt->numStencilSamples();
    }

    GrGpuCommandBuffer* createCommandBuffer(const GrGpuCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuCommandBuffer::LoadAndStoreInfo&) override {
        return nullptr;
    }

    void drawDebugWireRect(GrRenderTarget*, const SkIRect&, GrColor) override {}

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
    void deleteFence(GrFence) const override {}

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore() override { return nullptr; }
    void insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override {}
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override {}
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

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

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                          GrSurfaceOrigin,
                                          GrBackendTextureFlags,
                                          int sampleCnt,
                                          GrWrapOwnership) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrSurfaceOrigin) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             GrSurfaceOrigin,
                                                             int sampleCnt) override {
        return nullptr;
    }

    GrBuffer* onCreateBuffer(size_t, GrBufferType, GrAccessPattern, const void*) override {
        return nullptr;
    }

    gr_instanced::InstancedRendering* onCreateInstancedRendering() override { return nullptr; }

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

//////////////////////////////////////////////////////////////////////////////

void GrContextPriv::testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject* cb) {
    fContext->flush();
    fContext->fDrawingManager->testingOnly_removeOnFlushCallbackObject(cb);
}

void GrDrawingManager::testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject* cb) {
    int n = std::find(fOnFlushCBObjects.begin(), fOnFlushCBObjects.end(), cb) -
            fOnFlushCBObjects.begin();
    SkASSERT(n < fOnFlushCBObjects.count());
    fOnFlushCBObjects.removeShuffle(n);
}

//////////////////////////////////////////////////////////////////////////////

#define DRAW_OP_TEST_EXTERN(Op) \
    extern std::unique_ptr<GrDrawOp> Op##__Test(GrPaint&&, SkRandom*, GrContext*, GrFSAAType);

#define LEGACY_MESH_DRAW_OP_TEST_EXTERN(Op) \
    extern std::unique_ptr<GrLegacyMeshDrawOp> Op##__Test(SkRandom*, GrContext*);

#define DRAW_OP_TEST_ENTRY(Op) Op##__Test

LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAConvexPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAFillRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAFillRectOpLocalMatrix);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAFlatteningConvexPathOp)
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAHairlineOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AAStrokeRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(AnalyticRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(DashOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(DefaultPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(GrDrawAtlasOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(NonAAStrokeRectOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(SmallPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(TesselatingPathOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(TextBlobOp);
LEGACY_MESH_DRAW_OP_TEST_EXTERN(VerticesOp);

DRAW_OP_TEST_EXTERN(CircleOp)
DRAW_OP_TEST_EXTERN(DIEllipseOp);
DRAW_OP_TEST_EXTERN(EllipseOp);
DRAW_OP_TEST_EXTERN(NonAAFillRectOp)
DRAW_OP_TEST_EXTERN(RRectOp);

void GrDrawRandomOp(SkRandom* random, GrRenderTargetContext* renderTargetContext, GrPaint&& paint) {
    GrContext* context = renderTargetContext->surfPriv().getContext();
    using MakeTestLegacyMeshDrawOpFn = std::unique_ptr<GrLegacyMeshDrawOp>(SkRandom*, GrContext*);
    static constexpr MakeTestLegacyMeshDrawOpFn* gLegacyFactories[] = {
        DRAW_OP_TEST_ENTRY(AAConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOpLocalMatrix),
        DRAW_OP_TEST_ENTRY(AAFlatteningConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAHairlineOp),
        DRAW_OP_TEST_ENTRY(AAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(AnalyticRectOp),
        DRAW_OP_TEST_ENTRY(DashOp),
        DRAW_OP_TEST_ENTRY(DefaultPathOp),
        DRAW_OP_TEST_ENTRY(GrDrawAtlasOp),
        DRAW_OP_TEST_ENTRY(NonAAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(SmallPathOp),
        DRAW_OP_TEST_ENTRY(TesselatingPathOp),
        DRAW_OP_TEST_ENTRY(TextBlobOp),
        DRAW_OP_TEST_ENTRY(VerticesOp)
    };

    using MakeDrawOpFn = std::unique_ptr<GrDrawOp>(GrPaint&&, SkRandom*, GrContext*, GrFSAAType);
    static constexpr MakeDrawOpFn* gFactories[] = {
        DRAW_OP_TEST_ENTRY(CircleOp),
        DRAW_OP_TEST_ENTRY(DIEllipseOp),
        DRAW_OP_TEST_ENTRY(EllipseOp),
        DRAW_OP_TEST_ENTRY(NonAAFillRectOp),
        DRAW_OP_TEST_ENTRY(RRectOp),
    };

    static constexpr size_t kTotal = SK_ARRAY_COUNT(gLegacyFactories) + SK_ARRAY_COUNT(gFactories);

    uint32_t index = random->nextULessThan(static_cast<uint32_t>(kTotal));
    if (index < SK_ARRAY_COUNT(gLegacyFactories)) {
        const GrUserStencilSettings* uss = GrGetRandomStencil(random, context);
        // We don't use kHW because we will hit an assertion if the render target is not
        // multisampled
        static constexpr GrAAType kAATypes[] = {GrAAType::kNone, GrAAType::kCoverage};
        GrAAType aaType = kAATypes[random->nextULessThan(SK_ARRAY_COUNT(kAATypes))];
        bool snapToCenters = random->nextBool();
        auto op = gLegacyFactories[index](random, context);
        SkASSERT(op);
        renderTargetContext->priv().testingOnly_addLegacyMeshDrawOp(
                std::move(paint), aaType, std::move(op), uss, snapToCenters);
    } else {
        auto op = gFactories[index - SK_ARRAY_COUNT(gLegacyFactories)](
                std::move(paint), random, context, renderTargetContext->fsaaType());
        SkASSERT(op);
        renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
    }
}
