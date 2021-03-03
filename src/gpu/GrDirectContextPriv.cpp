/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDirectContextPriv.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/generated/GrConfigConversionEffect.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"

#define ASSERT_OWNED_PROXY(P) \
    SkASSERT(!(P) || !((P)->peekTexture()) || (P)->peekTexture()->getContext() == fContext)
#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fContext->singleOwner())
#define RETURN_VALUE_IF_ABANDONED(value) if (fContext->abandoned()) { return (value); }

sk_sp<const GrCaps> GrDirectContextPriv::refCaps() const {
    return fContext->refCaps();
}

void GrDirectContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

GrSemaphoresSubmitted GrDirectContextPriv::flushSurfaces(
                                                    SkSpan<GrSurfaceProxy*> proxies,
                                                    SkSurface::BackendSurfaceAccess access,
                                                    const GrFlushInfo& info,
                                                    const GrBackendSurfaceMutableState* newState) {
    ASSERT_SINGLE_OWNER
    GR_CREATE_TRACE_MARKER_CONTEXT("GrDirectContextPriv", "flushSurfaces", fContext);

    if (fContext->abandoned()) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo;
    }

#ifdef SK_DEBUG
    for (GrSurfaceProxy* proxy : proxies) {
        SkASSERT(proxy);
        ASSERT_OWNED_PROXY(proxy);
    }
#endif
    return fContext->drawingManager()->flushSurfaces(proxies, access, info, newState);
}

void GrDirectContextPriv::createDDLTask(sk_sp<const SkDeferredDisplayList> ddl,
                                        sk_sp<GrRenderTargetProxy> newDest,
                                        SkIPoint offset) {
    fContext->drawingManager()->createDDLTask(std::move(ddl), std::move(newDest), offset);
}

bool GrDirectContextPriv::compile(const GrProgramDesc& desc, const GrProgramInfo& info) {
    GrGpu* gpu = this->getGpu();
    if (!gpu) {
        return false;
    }

    return gpu->compile(desc, info);
}


//////////////////////////////////////////////////////////////////////////////
#if GR_TEST_UTILS

void GrDirectContextPriv::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStats(out);
#endif
}

void GrDirectContextPriv::dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                      SkTArray<double>* values) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStatsKeyValuePairs(keys, values);
#endif
}

void GrDirectContextPriv::printCacheStats() const {
    SkString out;
    this->dumpCacheStats(&out);
    SkDebugf("%s", out.c_str());
}

/////////////////////////////////////////////////
void GrDirectContextPriv::resetGpuStats() const {
#if GR_GPU_STATS
    fContext->fGpu->stats()->reset();
#endif
}

void GrDirectContextPriv::dumpGpuStats(SkString* out) const {
#if GR_GPU_STATS
    fContext->fGpu->stats()->dump(out);
    if (auto builder = fContext->fGpu->pipelineBuilder()) {
        builder->stats()->dump(out);
    }
#endif
}

void GrDirectContextPriv::dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                    SkTArray<double>* values) const {
#if GR_GPU_STATS
    fContext->fGpu->stats()->dumpKeyValuePairs(keys, values);
    if (auto builder = fContext->fGpu->pipelineBuilder()) {
        builder->stats()->dumpKeyValuePairs(keys, values);
    }
#endif
}

void GrDirectContextPriv::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

/////////////////////////////////////////////////
void GrDirectContextPriv::resetContextStats() const {
#if GR_GPU_STATS
    fContext->stats()->reset();
#endif
}

void GrDirectContextPriv::dumpContextStats(SkString* out) const {
#if GR_GPU_STATS
    return fContext->stats()->dump(out);
#endif
}

void GrDirectContextPriv::dumpContextStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                        SkTArray<double>* values) const {
#if GR_GPU_STATS
    return fContext->stats()->dumpKeyValuePairs(keys, values);
#endif
}

void GrDirectContextPriv::printContextStats() const {
    SkString out;
    this->dumpContextStats(&out);
    SkDebugf("%s", out.c_str());
}

/////////////////////////////////////////////////
sk_sp<SkImage> GrDirectContextPriv::testingOnly_getFontAtlasImage(GrMaskFormat format,
                                                                  unsigned int index) {
    auto atlasManager = this->getAtlasManager();
    if (!atlasManager) {
        return nullptr;
    }

    unsigned int numActiveProxies;
    const GrSurfaceProxyView* views = atlasManager->getViews(format, &numActiveProxies);
    if (index >= numActiveProxies || !views || !views[index].proxy()) {
        return nullptr;
    }

    SkColorType colorType = GrColorTypeToSkColorType(GrMaskFormatToColorType(format));
    SkASSERT(views[index].proxy()->priv().isExact());
    sk_sp<SkImage> image(new SkImage_Gpu(sk_ref_sp(fContext), kNeedNewImageUniqueID,
                                         views[index], colorType, kPremul_SkAlphaType, nullptr));
    return image;
}

void GrDirectContextPriv::testingOnly_purgeAllUnlockedResources() {
    fContext->fResourceCache->purgeAllUnlocked();
}

void GrDirectContextPriv::testingOnly_flushAndRemoveOnFlushCallbackObject(
        GrOnFlushCallbackObject* cb) {
    fContext->flushAndSubmit();
    fContext->drawingManager()->testingOnly_removeOnFlushCallbackObject(cb);
}
#endif

bool GrDirectContextPriv::validPMUPMConversionExists() {
    ASSERT_SINGLE_OWNER

    if (!fContext->fDidTestPMConversions) {
        fContext->fPMUPMConversionsRoundTrip =
                GrConfigConversionEffect::TestForPreservingPMConversions(fContext);
        fContext->fDidTestPMConversions = true;
    }

    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return fContext->fPMUPMConversionsRoundTrip;
}

std::unique_ptr<GrFragmentProcessor> GrDirectContextPriv::createPMToUPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->priv().validPMUPMConversionExists() in this case
    SkASSERT(fContext->fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToUnpremul);
}

std::unique_ptr<GrFragmentProcessor> GrDirectContextPriv::createUPMToPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->priv().validPMUPMConversionExists() in this case
    SkASSERT(fContext->fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToPremul);
}
