/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrContextPriv.h"
#include "GrDrawOpAtlas.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrMemoryPool.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrSemaphore.h"
#include "GrSurfaceContextPriv.h"
#include "GrTest.h"
#include "GrTexture.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkMathPriv.h"
#include "SkString.h"
#include "SkTo.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "ops/GrMeshDrawOp.h"
#include "text/GrGlyphCache.h"
#include "text/GrTextBlobCache.h"
#include <algorithm>

namespace GrTest {

void SetupAlwaysEvictAtlas(GrContext* context, int dim) {
    // These sizes were selected because they allow each atlas to hold a single plot and will thus
    // stress the atlas
    GrDrawOpAtlasConfig configs[3];
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

    context->contextPriv().setTextContextAtlasSizes_ForTesting(configs);
}

}  // namespace GrTest

bool GrSurfaceProxy::isWrapped_ForTesting() const {
    return SkToBool(fTarget);
}

bool GrRenderTargetContext::isWrapped_ForTesting() const {
    return fRenderTargetProxy->isWrapped_ForTesting();
}

void GrContextPriv::setTextBlobCacheLimit_ForTesting(size_t bytes) {
    fContext->fTextBlobCache->setBudget(bytes);
}

void GrContextPriv::setTextContextAtlasSizes_ForTesting(const GrDrawOpAtlasConfig* configs) {
    GrAtlasManager* atlasManager = this->getAtlasManager();
    if (atlasManager) {
        atlasManager->setAtlasSizes_ForTesting(configs);
    }
}

///////////////////////////////////////////////////////////////////////////////

void GrContextPriv::purgeAllUnlockedResources_ForTesting() {
    fContext->fResourceCache->purgeAllUnlocked();
}

void GrContextPriv::resetGpuStats() const {
#if GR_GPU_STATS
    fContext->fGpu->stats()->reset();
#endif
}

void GrContextPriv::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStats(out);
#endif
}

void GrContextPriv::dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                SkTArray<double>* values) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStatsKeyValuePairs(keys, values);
#endif
}

void GrContextPriv::printCacheStats() const {
    SkString out;
    this->dumpCacheStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContextPriv::dumpGpuStats(SkString* out) const {
#if GR_GPU_STATS
    return fContext->fGpu->stats()->dump(out);
#endif
}

void GrContextPriv::dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys,
                                              SkTArray<double>* values) const {
#if GR_GPU_STATS
    return fContext->fGpu->stats()->dumpKeyValuePairs(keys, values);
#endif
}

void GrContextPriv::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

sk_sp<SkImage> GrContextPriv::getFontAtlasImage_ForTesting(GrMaskFormat format, unsigned int index) {
    auto atlasManager = this->getAtlasManager();
    if (!atlasManager) {
        return nullptr;
    }

    unsigned int numActiveProxies;
    const sk_sp<GrTextureProxy>* proxies = atlasManager->getProxies(format, &numActiveProxies);
    if (index >= numActiveProxies || !proxies || !proxies[index]) {
        return nullptr;
    }

    SkASSERT(proxies[index]->priv().isExact());
    sk_sp<SkImage> image(new SkImage_Gpu(sk_ref_sp(fContext), kNeedNewImageUniqueID,
                                         kPremul_SkAlphaType, proxies[index], nullptr,
                                         SkBudgeted::kNo));
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

GrBackendTexture GrGpu::createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                        SkColorType colorType, bool isRenderTarget,
                                                        GrMipMapped mipMapped) {
    GrPixelConfig config = SkColorType2GrPixelConfig(colorType);
    if (kUnknown_GrPixelConfig == config) {
        return GrBackendTexture();
    }
    return this->createTestingOnlyBackendTexture(pixels, w, h, config, isRenderTarget, mipMapped);
}

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


uint32_t GrRenderTargetContextPriv::testingOnly_getOpListID() {
    return fRenderTargetContext->getOpList()->uniqueID();
}

uint32_t GrRenderTargetContextPriv::testingOnly_addDrawOp(std::unique_ptr<GrDrawOp> op) {
    return this->testingOnly_addDrawOp(GrNoClip(), std::move(op));
}

uint32_t GrRenderTargetContextPriv::testingOnly_addDrawOp(const GrClip& clip,
                                                          std::unique_ptr<GrDrawOp> op) {
    ASSERT_SINGLE_OWNER
    if (fRenderTargetContext->drawingManager()->wasAbandoned()) {
        fRenderTargetContext->fContext->contextPriv().opMemoryPool()->release(std::move(op));
        return SK_InvalidUniqueID;
    }
    SkDEBUGCODE(fRenderTargetContext->validate());
    GR_AUDIT_TRAIL_AUTO_FRAME(fRenderTargetContext->fAuditTrail,
                              "GrRenderTargetContext::testingOnly_addDrawOp");
    return fRenderTargetContext->addDrawOp(clip, std::move(op));
}

#undef ASSERT_SINGLE_OWNER

///////////////////////////////////////////////////////////////////////////////

GrInternalSurfaceFlags GrSurfaceProxy::testingOnly_getFlags() const {
    return fSurfaceFlags;
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

void GrCoverageCountingPathRenderer::testingOnly_drawPathDirectly(const DrawPathArgs& args) {
    // Call onDrawPath() directly: We want to test paths that might fail onCanDrawPath() simply for
    // performance reasons, and GrPathRenderer::drawPath() assert that this call returns true.
    // The test is responsible to not draw any paths that CCPR is not actually capable of.
    this->onDrawPath(args);
}

const GrUniqueKey& GrCoverageCountingPathRenderer::testingOnly_getStashedAtlasKey() const {
    return fStashedAtlasKey;
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
DRAW_OP_TEST_EXTERN(TextureOp);

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
            DRAW_OP_TEST_ENTRY(TextureOp),
    };

    static constexpr size_t kTotal = SK_ARRAY_COUNT(gFactories);
    uint32_t index = random->nextULessThan(static_cast<uint32_t>(kTotal));
    auto op = gFactories[index](
            std::move(paint), random, context, renderTargetContext->fsaaType());
    SkASSERT(op);
    renderTargetContext->priv().testingOnly_addDrawOp(std::move(op));
}
