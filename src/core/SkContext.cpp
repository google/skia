/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContext.h"

#include "include/core/RasterContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/core/SkTypes.h"
#include "include/cpu/Recorder.h"
#include "include/private/SkGPUContext.h"
#include "src/capture/SkCapture.h"
#include "src/capture/SkCaptureManager.h"
#include "src/core/SkContextPriv.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkSharedContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

SkContext::SkContext(const SkContextOptions& options) : SkContext(options, nullptr) {}

SkContext::SkContext(const SkContextOptions& options, std::unique_ptr<SkGPUContext> gpuContext)
        : fGPUContext(std::move(gpuContext)) {
    if (options.fEnableCapture) {
        fCaptureManager = sk_make_sp<SkCaptureManager>();
    }
    fSharedContext = sk_make_sp<SkSharedContext>(options);
}

SkContext::~SkContext() = default;

SkContextPriv SkContext::priv() { return SkContextPriv(this); }
SkContextPrivConst SkContext::priv() const { return SkContextPrivConst(this); }

void SkContext::startCapture() {
    if (!fCaptureManager) {
        return;
    }

    fCaptureManager->toggleCapture(true);
}

sk_sp<SkCapture> SkContext::endCapture() {
    if (!fCaptureManager) {
        return nullptr;
    }

    fCaptureManager->toggleCapture(false);
    return fCaptureManager->getLastCapture();
}

std::unique_ptr<SkRecorder> SkContext::makeRecorder(
        BackendSelection backend, const skgpu::graphite::RecorderOptions* graphiteOptions) {
    switch (backend) {
        case BackendSelection::kAutomatic:
            if (fGPUContext) {
                return fGPUContext->makeRecorder(graphiteOptions);
            }
            return std::make_unique<skcpu::Recorder>();
        case BackendSelection::kCPU:
            return std::make_unique<skcpu::Recorder>();
        case BackendSelection::kGPU:
            if (fGPUContext) {
                return fGPUContext->makeRecorder(graphiteOptions);
            }
            return nullptr;
    }
    SkUNREACHABLE;
}

size_t SkContext::fontCacheLimit() const { return fSharedContext->fontCacheLimit(); }

size_t SkContext::setFontCacheLimit(size_t bytes) {
    return fSharedContext->setFontCacheLimit(bytes);
}

size_t SkContext::fontCacheUsed() const { return fSharedContext->fontCacheUsed(); }

int SkContext::fontCacheCountLimit() const { return fSharedContext->fontCacheCountLimit(); }

int SkContext::setFontCacheCountLimit(int count) {
    return fSharedContext->setFontCacheCountLimit(count);
}

int SkContext::fontCacheCountUsed() const { return fSharedContext->fontCacheCountUsed(); }

void SkContext::purgeFontCache() { fSharedContext->purgeFontCache(); }

void SkContext::purgePinnedFontCache() { fSharedContext->purgePinnedFontCache(); }

size_t SkContext::resourceCacheTotalBytesUsed() const {
    return fSharedContext->resourceCacheTotalBytesUsed();
}

size_t SkContext::resourceCacheTotalByteLimit() const {
    return fSharedContext->resourceCacheTotalByteLimit();
}

size_t SkContext::setResourceCacheTotalByteLimit(size_t newLimit) {
    return fSharedContext->setResourceCacheTotalByteLimit(newLimit);
}

size_t SkContext::resourceCacheSingleAllocationByteLimit() const {
    return fSharedContext->resourceCacheSingleAllocationByteLimit();
}

size_t SkContext::setResourceCacheSingleAllocationByteLimit(size_t newLimit) {
    return fSharedContext->setResourceCacheSingleAllocationByteLimit(newLimit);
}

void SkContext::purgeResourceCache() { fSharedContext->purgeResourceCache(); }

int SkContext::typefaceCacheCountLimit() const {
    return fSharedContext->typefaceCacheCountLimit();
}

int SkContext::setTypefaceCacheCountLimit(int count) {
    return fSharedContext->setTypefaceCacheCountLimit(count);
}

namespace {
void sk_trace_dump_visitor(const SkResourceCache::Rec& rec, void* context) {
    SkTraceMemoryDump* dump = static_cast<SkTraceMemoryDump*>(context);
    SkString dumpName = SkStringPrintf("skia/sk_resource_cache/%s_%p", rec.getCategory(), &rec);
    SkDiscardableMemory* discardable = rec.diagnostic_only_getDiscardable();
    if (discardable) {
        dump->setDiscardableMemoryBacking(dumpName.c_str(), *discardable);

        // The discardable memory size will be calculated by dumper, but we also dump what we think
        // the size of object in memory is irrespective of whether object is live or dead.
        dump->dumpNumericValue(dumpName.c_str(), "discardable_size", "bytes", rec.bytesUsed());
    } else {
        dump->dumpNumericValue(dumpName.c_str(), "size", "bytes", rec.bytesUsed());
        dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
    }
}
}  // namespace

void SkContext::dumpMemoryStatistics(SkTraceMemoryDump* dump) {
    fSharedContext->synchronizedResourceCache()->visitAll(sk_trace_dump_visitor, dump);
    fSharedContext->strikeCache()->dumpMemoryStatistics(dump);
}

std::unique_ptr<SkContext> SkContextCtorAccessor::MakeContext(const SkContextOptions& options) {
    return std::unique_ptr<SkContext>(new SkContext(options));
}

std::unique_ptr<SkContext> SkContextCtorAccessor::MakeContext(
        const SkContextOptions& options, std::unique_ptr<SkGPUContext> gpuContext) {
    return std::unique_ptr<SkContext>(new SkContext(options, std::move(gpuContext)));
}

namespace SkContexts {

// GetGlobalSkContext creates a global SkContext object with only Raster settings. GPU settings can
// be configured later. This is an intermediary step in the SkContext project, whereby we use this
// global SkContext object to remove or take away functionality from SkGraphics.
SkContext* GetGlobalSkContext() {
    static SkContext* gSkContext = SkContexts::MakeRaster({}).release();
    return gSkContext;
}

}  // namespace SkContexts
