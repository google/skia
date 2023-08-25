/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrRecordingContext.h"

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkArenaAlloc.h"
#include "src/gpu/ganesh/GrAuditTrail.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/PathRendererChain.h"
#include "src/gpu/ganesh/ops/AtlasTextOp.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

#include <utility>

using namespace skia_private;

using TextBlobRedrawCoordinator = sktext::gpu::TextBlobRedrawCoordinator;

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

GrRecordingContext::GrRecordingContext(sk_sp<GrContextThreadSafeProxy> proxy, bool ddlRecording)
        : GrImageContext(std::move(proxy))
        , fAuditTrail(new GrAuditTrail())
        , fArenas(ddlRecording) {
    fProxyProvider = std::make_unique<GrProxyProvider>(this);
}

GrRecordingContext::~GrRecordingContext() {
    skgpu::ganesh::AtlasTextOp::ClearCache();
}

bool GrRecordingContext::init() {
    if (!GrImageContext::init()) {
        return false;
    }

    skgpu::ganesh::PathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = this->options().fAllowPathMaskCaching;
#if defined(GR_TEST_UTILS)
    prcOptions.fGpuPathRenderers = this->options().fGpuPathRenderers;
#endif
    // FIXME: Once this is removed from Chrome and Android, rename to fEnable"".
    if (this->options().fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    bool reduceOpsTaskSplitting = true;
    if (this->caps()->avoidReorderingRenderTasks()) {
        reduceOpsTaskSplitting = false;
    } else if (GrContextOptions::Enable::kYes == this->options().fReduceOpsTaskSplitting) {
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
    GrImageContext::abandonContext();

    this->destroyDrawingManager();
}

GrDrawingManager* GrRecordingContext::drawingManager() {
    return fDrawingManager.get();
}

void GrRecordingContext::destroyDrawingManager() {
    fDrawingManager.reset();
}

GrRecordingContext::Arenas::Arenas(SkArenaAlloc* recordTimeAllocator,
                                   sktext::gpu::SubRunAllocator* subRunAllocator)
        : fRecordTimeAllocator(recordTimeAllocator)
        , fRecordTimeSubRunAllocator(subRunAllocator) {
    // OwnedArenas should instantiate these before passing the bare pointer off to this struct.
    SkASSERT(subRunAllocator);
}

// Must be defined here so that std::unique_ptr can see the sizes of the various pools, otherwise
// it can't generate a default destructor for them.
GrRecordingContext::OwnedArenas::OwnedArenas(bool ddlRecording) : fDDLRecording(ddlRecording) {}
GrRecordingContext::OwnedArenas::~OwnedArenas() {}

GrRecordingContext::OwnedArenas& GrRecordingContext::OwnedArenas::operator=(OwnedArenas&& a) {
    fDDLRecording = a.fDDLRecording;
    fRecordTimeAllocator = std::move(a.fRecordTimeAllocator);
    fRecordTimeSubRunAllocator = std::move(a.fRecordTimeSubRunAllocator);
    return *this;
}

GrRecordingContext::Arenas GrRecordingContext::OwnedArenas::get() {
    if (!fRecordTimeAllocator && fDDLRecording) {
        // TODO: empirically determine a better number for SkArenaAlloc's firstHeapAllocation param
        fRecordTimeAllocator = std::make_unique<SkArenaAlloc>(1024);
    }

    if (!fRecordTimeSubRunAllocator) {
        fRecordTimeSubRunAllocator = std::make_unique<sktext::gpu::SubRunAllocator>();
    }

    return {fRecordTimeAllocator.get(), fRecordTimeSubRunAllocator.get()};
}

GrRecordingContext::OwnedArenas&& GrRecordingContext::detachArenas() {
    return std::move(fArenas);
}

TextBlobRedrawCoordinator* GrRecordingContext::getTextBlobRedrawCoordinator() {
    return fThreadSafeProxy->priv().getTextBlobRedrawCoordinator();
}

const TextBlobRedrawCoordinator* GrRecordingContext::getTextBlobRedrawCoordinator() const {
    return fThreadSafeProxy->priv().getTextBlobRedrawCoordinator();
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

sk_sp<const SkCapabilities> GrRecordingContext::skCapabilities() const {
    return this->refCaps();
}

int GrRecordingContext::maxTextureSize() const { return this->caps()->maxTextureSize(); }

int GrRecordingContext::maxRenderTargetSize() const { return this->caps()->maxRenderTargetSize(); }

bool GrRecordingContext::colorTypeSupportedAsImage(SkColorType colorType) const {
    GrBackendFormat format =
            this->caps()->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                  GrRenderable::kNo);
    return format.isValid();
}

bool GrRecordingContext::supportsProtectedContent() const {
    return this->caps()->supportsProtectedContent();
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

#if defined(GR_TEST_UTILS)

#if GR_GPU_STATS

void GrRecordingContext::Stats::dump(SkString* out) const {
    out->appendf("Num Path Masks Generated: %d\n", fNumPathMasksGenerated);
    out->appendf("Num Path Mask Cache Hits: %d\n", fNumPathMaskCacheHits);
}

void GrRecordingContext::Stats::dumpKeyValuePairs(TArray<SkString>* keys,
                                                  TArray<double>* values) const {
    keys->push_back(SkString("path_masks_generated"));
    values->push_back(fNumPathMasksGenerated);

    keys->push_back(SkString("path_mask_cache_hits"));
    values->push_back(fNumPathMaskCacheHits);
}

void GrRecordingContext::DMSAAStats::dumpKeyValuePairs(TArray<SkString>* keys,
                                                       TArray<double>* values) const {
    keys->push_back(SkString("dmsaa_render_passes"));
    values->push_back(fNumRenderPasses);

    keys->push_back(SkString("dmsaa_multisample_render_passes"));
    values->push_back(fNumMultisampleRenderPasses);

    for (const auto& [name, count] : fTriggerCounts) {
        keys->push_back(SkStringPrintf("dmsaa_trigger_%s", name.c_str()));
        values->push_back(count);
    }
}

void GrRecordingContext::DMSAAStats::dump() const {
    SkDebugf("DMSAA Render Passes: %d\n", fNumRenderPasses);
    SkDebugf("DMSAA Multisample Render Passes: %d\n", fNumMultisampleRenderPasses);
    if (!fTriggerCounts.empty()) {
        SkDebugf("DMSAA Triggers:\n");
        for (const auto& [name, count] : fTriggerCounts) {
            SkDebugf("    %s: %d\n", name.c_str(), count);
        }
    }
}

void GrRecordingContext::DMSAAStats::merge(const DMSAAStats& stats) {
    fNumRenderPasses += stats.fNumRenderPasses;
    fNumMultisampleRenderPasses += stats.fNumMultisampleRenderPasses;
    for (const auto& [name, count] : stats.fTriggerCounts) {
        fTriggerCounts[name] += count;
    }
}

#endif // GR_GPU_STATS
#endif // defined(GR_TEST_UTILS)
