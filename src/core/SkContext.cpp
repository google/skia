/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkContext.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/core/SkTypes.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkSharedContext.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkSynchronizedResourceCache.h"
#include "src/core/SkTypefaceCache.h"

SkContext::SkContext(sk_sp<SkSharedContext> sharedContext)
        : fSharedContext(std::move(sharedContext)) {}

SkContext::~SkContext() = default;

SkResourceCache* SkContext::resourceCache() const {
    return fSharedContext->synchronizedResourceCache();
}

SkStrikeCache* SkContext::fontCache() const {
    return fSharedContext->fontCache();
}

SkTypefaceCache* SkContext::typefaceCache() const {
    return fSharedContext->typefaceCache();
}

size_t SkContext::fontCacheLimit() const {
    return this->fontCache()->getCacheSizeLimit();
}

size_t SkContext::setFontCacheLimit(size_t bytes) {
    return this->fontCache()->setCacheSizeLimit(bytes);
}

size_t SkContext::fontCacheUsed() const {
    return this->fontCache()->getTotalMemoryUsed();
}

int SkContext::fontCacheCountLimit() const {
    return this->fontCache()->getCacheCountLimit();
}

int SkContext::setFontCacheCountLimit(int count) {
    return this->fontCache()->setCacheCountLimit(count);
}

int SkContext::fontCacheCountUsed() const {
    return this->fontCache()->getCacheCountUsed();
}

void SkContext::purgeFontCache() { this->fontCache()->purgeAll(); }

void SkContext::purgePinnedFontCache() { this->fontCache()->purgePinned(); }

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
    this->resourceCache()->visitAll(sk_trace_dump_visitor, dump);
    SkStrikeCache::DumpMemoryStatistics(dump);
}
