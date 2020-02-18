/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrRecordingContext.h"

#include "include/gpu/GrContext.h"
#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProgramDesc.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/text/GrTextBlobCache.h"

#define ASSERT_SINGLE_OWNER_PRIV \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)

GrRecordingContext::ProgramData::ProgramData(std::unique_ptr<const GrProgramDesc> desc,
                                             const GrProgramInfo* info)
        : fDesc(std::move(desc))
        , fInfo(info) {
}

GrRecordingContext::ProgramData::ProgramData(ProgramData&& other)
        : fDesc(std::move(other.fDesc))
        , fInfo(other.fInfo) {
}

GrRecordingContext::ProgramData::~ProgramData() {}

GrRecordingContext::GrRecordingContext(GrBackendApi backend,
                                       const GrContextOptions& options,
                                       uint32_t contextID)
        : INHERITED(backend, options, contextID)
        , fAuditTrail(new GrAuditTrail()) {
}

GrRecordingContext::~GrRecordingContext() { }

/**
 * TODO: move textblob draw calls below context (see comment below)
 */
static void textblobcache_overbudget_CB(void* data) {
    SkASSERT(data);
    GrRecordingContext* context = reinterpret_cast<GrRecordingContext*>(data);

    GrContext* direct = context->priv().asDirectContext();
    if (!direct) {
        return;
    }

    // TextBlobs are drawn at the SkGpuDevice level, therefore they cannot rely on
    // GrRenderTargetContext to perform a necessary flush.  The solution is to move drawText calls
    // to below the GrContext level, but this is not trivial because they call drawPath on
    // SkGpuDevice.
    direct->flush();
}

bool GrRecordingContext::init(sk_sp<const GrCaps> caps) {

    if (!INHERITED::init(std::move(caps))) {
        return false;
    }

    fStrikeCache.reset(new GrStrikeCache{});

    fTextBlobCache.reset(new GrTextBlobCache(textblobcache_overbudget_CB, this,
                                             this->contextID()));

    return true;
}

void GrRecordingContext::setupDrawingManager(bool sortOpsTasks, bool reduceOpsTaskSplitting) {
    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = this->options().fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = this->options().fGpuPathRenderers;
#endif
    // FIXME: Once this is removed from Chrome and Android, rename to fEnable"".
    if (!this->options().fDisableCoverageCountingPaths) {
        prcOptions.fGpuPathRenderers |= GpuPathRenderers::kCoverageCounting;
    }
    if (this->options().fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    if (!this->proxyProvider()->renderingDirectly()) {
        // DDL TODO: remove this crippling of the path renderer chain
        // Disable the small path renderer bc of the proxies in the atlas. They need to be
        // unified when the opsTasks are added back to the destination drawing manager.
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    GrTextContext::Options textContextOptions;
    textContextOptions.fMaxDistanceFieldFontSize = this->options().fGlyphsAsPathsFontSize;
    textContextOptions.fMinDistanceFieldFontSize = this->options().fMinDistanceFieldFontSize;
    textContextOptions.fDistanceFieldVerticesAlwaysHaveW = false;
#if SK_SUPPORT_ATLAS_TEXT
    if (GrContextOptions::Enable::kYes == this->options().fDistanceFieldGlyphVerticesAlwaysHaveW) {
        textContextOptions.fDistanceFieldVerticesAlwaysHaveW = true;
    }
#endif

    fDrawingManager.reset(new GrDrawingManager(this,
                                               prcOptions,
                                               textContextOptions,
                                               sortOpsTasks,
                                               reduceOpsTaskSplitting));
}

void GrRecordingContext::abandonContext() {
    INHERITED::abandonContext();

    fStrikeCache->freeAll();
    fTextBlobCache->freeAll();
}

GrDrawingManager* GrRecordingContext::drawingManager() {
    return fDrawingManager.get();
}

GrRecordingContext::Arenas::Arenas(GrOpMemoryPool* opMemoryPool, SkArenaAlloc* recordTimeAllocator)
        : fOpMemoryPool(opMemoryPool)
        , fRecordTimeAllocator(recordTimeAllocator) {
    // OwnedArenas should instantiate these before passing the bare pointer off to this struct.
    SkASSERT(opMemoryPool);
    SkASSERT(recordTimeAllocator);
}

// Must be defined here so that std::unique_ptr can see the sizes of the various pools, otherwise
// it can't generate a default destructor for them.
GrRecordingContext::OwnedArenas::OwnedArenas() {}
GrRecordingContext::OwnedArenas::~OwnedArenas() {}

GrRecordingContext::OwnedArenas& GrRecordingContext::OwnedArenas::operator=(OwnedArenas&& a) {
    fOpMemoryPool = std::move(a.fOpMemoryPool);
    fRecordTimeAllocator = std::move(a.fRecordTimeAllocator);
    return *this;
}

GrRecordingContext::Arenas GrRecordingContext::OwnedArenas::get() {
    if (!fOpMemoryPool) {
        // DDL TODO: should the size of the memory pool be decreased in DDL mode? CPU-side memory
        // consumed in DDL mode vs. normal mode for a single skp might be a good metric of wasted
        // memory.
        fOpMemoryPool = GrOpMemoryPool::Make(16384, 16384);
    }

    if (!fRecordTimeAllocator) {
        // TODO: empirically determine a better number for SkArenaAlloc's firstHeapAllocation param
        fRecordTimeAllocator = std::make_unique<SkArenaAlloc>(sizeof(GrPipeline) * 100);
    }

    return {fOpMemoryPool.get(), fRecordTimeAllocator.get()};
}

GrRecordingContext::OwnedArenas&& GrRecordingContext::detachArenas() {
    return std::move(fArenas);
}

GrTextBlobCache* GrRecordingContext::getTextBlobCache() {
    return fTextBlobCache.get();
}

const GrTextBlobCache* GrRecordingContext::getTextBlobCache() const {
    return fTextBlobCache.get();
}

void GrRecordingContext::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    this->drawingManager()->addOnFlushCallbackObject(onFlushCBObject);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
sk_sp<const GrCaps> GrRecordingContextPriv::refCaps() const {
    return fContext->refCaps();
}

void GrRecordingContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

GrContext* GrRecordingContextPriv::backdoor() {
    return (GrContext*) fContext;
}

