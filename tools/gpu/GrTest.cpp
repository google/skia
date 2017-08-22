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
#include "GrGpu.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrSemaphore.h"
#include "GrSurfaceContextPriv.h"
#include "GrTexture.h"
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
    switch (backend) {
#ifdef SK_VULKAN
        case kVulkan_GrBackend: {
            GrVkImageInfo* vkInfo = (GrVkImageInfo*)(handle);
            return GrBackendTexture(width, height, *vkInfo);
        }
#endif
        case kOpenGL_GrBackend: {
            GrGLTextureInfo* glInfo = (GrGLTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *glInfo);
        }
        case kMock_GrBackend: {
            GrMockTextureInfo* mockInfo = (GrMockTextureInfo*)(handle);
            return GrBackendTexture(width, height, config, *mockInfo);
        }
        default:
            return GrBackendTexture();
    }
}

}  // namespace GrTest

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

GrRenderTargetFlags GrRenderTargetProxy::testingOnly_getFlags() const {
    return fRenderTargetFlags;
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
    extern std::unique_ptr<GrDrawOp> Op##__Test(GrPaint&&, SkRandom*, GrContext*, GrFSAAType)
#define DRAW_OP_TEST_ENTRY(Op) Op##__Test

DRAW_OP_TEST_EXTERN(AAConvexPathOp);
DRAW_OP_TEST_EXTERN(AAFillRectOp);
DRAW_OP_TEST_EXTERN(AAFlatteningConvexPathOp);
DRAW_OP_TEST_EXTERN(AAHairlineOp);
DRAW_OP_TEST_EXTERN(AAStrokeRectOp);
DRAW_OP_TEST_EXTERN(CircleOp);
DRAW_OP_TEST_EXTERN(DashOp);
DRAW_OP_TEST_EXTERN(DefaultPathOp);
DRAW_OP_TEST_EXTERN(DIEllipseOp);
DRAW_OP_TEST_EXTERN(EllipseOp);
DRAW_OP_TEST_EXTERN(GrAtlasTextOp);
DRAW_OP_TEST_EXTERN(GrDrawAtlasOp);
DRAW_OP_TEST_EXTERN(GrDrawVerticesOp);
DRAW_OP_TEST_EXTERN(NonAAFillRectOp);
DRAW_OP_TEST_EXTERN(NonAALatticeOp);
DRAW_OP_TEST_EXTERN(NonAAStrokeRectOp);
DRAW_OP_TEST_EXTERN(ShadowRRectOp);
DRAW_OP_TEST_EXTERN(SmallPathOp);
DRAW_OP_TEST_EXTERN(RegionOp);
DRAW_OP_TEST_EXTERN(RRectOp);
DRAW_OP_TEST_EXTERN(TesselatingPathOp);

void GrDrawRandomOp(SkRandom* random, GrRenderTargetContext* renderTargetContext, GrPaint&& paint) {
    GrContext* context = renderTargetContext->surfPriv().getContext();
    using MakeDrawOpFn = std::unique_ptr<GrDrawOp>(GrPaint&&, SkRandom*, GrContext*, GrFSAAType);
    static constexpr MakeDrawOpFn* gFactories[] = {
        DRAW_OP_TEST_ENTRY(AAConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAFillRectOp),
        DRAW_OP_TEST_ENTRY(AAFlatteningConvexPathOp),
        DRAW_OP_TEST_ENTRY(AAHairlineOp),
        DRAW_OP_TEST_ENTRY(AAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(CircleOp),
        DRAW_OP_TEST_ENTRY(DashOp),
        DRAW_OP_TEST_ENTRY(DefaultPathOp),
        DRAW_OP_TEST_ENTRY(DIEllipseOp),
        DRAW_OP_TEST_ENTRY(EllipseOp),
        DRAW_OP_TEST_ENTRY(GrAtlasTextOp),
        DRAW_OP_TEST_ENTRY(GrDrawAtlasOp),
        DRAW_OP_TEST_ENTRY(GrDrawVerticesOp),
        DRAW_OP_TEST_ENTRY(NonAAFillRectOp),
        DRAW_OP_TEST_ENTRY(NonAALatticeOp),
        DRAW_OP_TEST_ENTRY(NonAAStrokeRectOp),
        DRAW_OP_TEST_ENTRY(ShadowRRectOp),
        DRAW_OP_TEST_ENTRY(SmallPathOp),
        DRAW_OP_TEST_ENTRY(RegionOp),
        DRAW_OP_TEST_ENTRY(RRectOp),
        DRAW_OP_TEST_ENTRY(TesselatingPathOp),
    };

    static constexpr size_t kTotal = SK_ARRAY_COUNT(gFactories);
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(kTotal));
    auto op = gFactories[index](
            std::move(paint), random, context, renderTargetContext->fsaaType());
    SkASSERT(op);
    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
}
