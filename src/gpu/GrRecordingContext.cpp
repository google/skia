/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrRecordingContext.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/ops/GrAtlasTextOp.h"
#include "src/gpu/text/GrTextBlob.h"
#include "src/gpu/text/GrTextBlobCache.h"

GrRecordingContext::ProgramData::ProgramData(std::unique_ptr<const GrProgramDesc> desc,
                                             const GrProgramInfo* info)
        : fDesc(std::move(desc))
        , fInfo(info) {
}

GrRecordingContext::ProgramData::ProgramData(ProgramData&& other)
        : fDesc(std::move(other.fDesc))
        , fInfo(other.fInfo) {
}

GrRecordingContext::ProgramData::~ProgramData() = default;

GrRecordingContext::GrRecordingContext(sk_sp<GrContextThreadSafeProxy> proxy)
        : INHERITED(std::move(proxy))
        , fAuditTrail(new GrAuditTrail()) {
    fProxyProvider = std::make_unique<GrProxyProvider>(this);
}

GrRecordingContext::~GrRecordingContext() {
    GrAtlasTextOp::ClearCache();
}

int GrRecordingContext::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    GrBackendFormat format =
            this->caps()->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                  GrRenderable::kYes);
    return this->caps()->maxRenderTargetSampleCount(format);
}

bool GrRecordingContext::init() {
    if (!INHERITED::init()) {
        return false;
    }

    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = this->options().fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = this->options().fGpuPathRenderers;
#endif
    // FIXME: Once this is removed from Chrome and Android, rename to fEnable"".
    if (this->options().fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    bool reduceOpsTaskSplitting = false;
    if (GrContextOptions::Enable::kYes == this->options().fReduceOpsTaskSplitting) {
        reduceOpsTaskSplitting = true;
    } else if (GrContextOptions::Enable::kNo == this->options().fReduceOpsTaskSplitting) {
        reduceOpsTaskSplitting = false;
    }
    fDrawingManager.reset(new GrDrawingManager(this,
                                               prcOptions,
                                               reduceOpsTaskSplitting));
    return true;
}

void GrRecordingContext::abandonContext() {
    INHERITED::abandonContext();

    this->destroyDrawingManager();
}

GrDrawingManager* GrRecordingContext::drawingManager() {
    return fDrawingManager.get();
}

void GrRecordingContext::destroyDrawingManager() {
    fDrawingManager.reset();
}

GrRecordingContext::Arenas::Arenas(SkArenaAlloc* recordTimeAllocator,
                                   GrSubRunAllocator* subRunAllocator)
        : fRecordTimeAllocator(recordTimeAllocator)
        , fRecordTimeSubRunAllocator(subRunAllocator) {
    // OwnedArenas should instantiate these before passing the bare pointer off to this struct.
    SkASSERT(recordTimeAllocator);
    SkASSERT(subRunAllocator);
}

// Must be defined here so that std::unique_ptr can see the sizes of the various pools, otherwise
// it can't generate a default destructor for them.
GrRecordingContext::OwnedArenas::OwnedArenas() {}
GrRecordingContext::OwnedArenas::~OwnedArenas() {}

GrRecordingContext::OwnedArenas& GrRecordingContext::OwnedArenas::operator=(OwnedArenas&& a) {
    fRecordTimeAllocator = std::move(a.fRecordTimeAllocator);
    fRecordTimeSubRunAllocator = std::move(a.fRecordTimeSubRunAllocator);
    return *this;
}

GrRecordingContext::Arenas GrRecordingContext::OwnedArenas::get() {
    if (!fRecordTimeAllocator) {
        // TODO: empirically determine a better number for SkArenaAlloc's firstHeapAllocation param
        fRecordTimeAllocator = std::make_unique<SkArenaAlloc>(1024);
    }

    if (!fRecordTimeSubRunAllocator) {
        fRecordTimeSubRunAllocator = std::make_unique<GrSubRunAllocator>();
    }

    return {fRecordTimeAllocator.get(), fRecordTimeSubRunAllocator.get()};
}

GrRecordingContext::OwnedArenas&& GrRecordingContext::detachArenas() {
    return std::move(fArenas);
}

GrTextBlobCache* GrRecordingContext::getTextBlobCache() {
    return fThreadSafeProxy->priv().getTextBlobCache();
}

const GrTextBlobCache* GrRecordingContext::getTextBlobCache() const {
    return fThreadSafeProxy->priv().getTextBlobCache();
}

GrThreadSafeCache* GrRecordingContext::threadSafeCache() {
    return fThreadSafeProxy->priv().threadSafeCache();
}

const GrThreadSafeCache* GrRecordingContext::threadSafeCache() const {
    return fThreadSafeProxy->priv().threadSafeCache();
}

void GrRecordingContext::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    this->drawingManager()->addOnFlushCallbackObject(onFlushCBObject);
}

////////////////////////////////////////////////////////////////////////////////

int GrRecordingContext::maxTextureSize() const { return this->caps()->maxTextureSize(); }

int GrRecordingContext::maxRenderTargetSize() const { return this->caps()->maxRenderTargetSize(); }

bool GrRecordingContext::colorTypeSupportedAsImage(SkColorType colorType) const {
    GrBackendFormat format =
            this->caps()->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                  GrRenderable::kNo);
    return format.isValid();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrRecordingContextPriv::refCaps() const {
    return fContext->refCaps();
}

void GrRecordingContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"

void GrRecordingContext::dumpJSON(SkJSONWriter* writer) const {
    writer->beginObject();

#if GR_GPU_STATS
    writer->appendS32("path_masks_generated", this->stats()->numPathMasksGenerated());
    writer->appendS32("path_mask_cache_hits", this->stats()->numPathMaskCacheHits());
#endif

    writer->endObject();
}
#else
void GrRecordingContext::dumpJSON(SkJSONWriter*) const { }
#endif

#if GR_TEST_UTILS

#if GR_GPU_STATS

void GrRecordingContext::Stats::dump(SkString* out) {
    out->appendf("Num Path Masks Generated: %d\n", fNumPathMasksGenerated);
    out->appendf("Num Path Mask Cache Hits: %d\n", fNumPathMaskCacheHits);
}

void GrRecordingContext::Stats::dumpKeyValuePairs(SkTArray<SkString>* keys,
                                                  SkTArray<double>* values) {
    keys->push_back(SkString("path_masks_generated"));
    values->push_back(fNumPathMasksGenerated);

    keys->push_back(SkString("path_mask_cache_hits"));
    values->push_back(fNumPathMaskCacheHits);
}

#endif // GR_GPU_STATS
#endif // GR_TEST_UTILS

