/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkStrikeCache.h"

#include <cctype>

#include "include/core/SkGraphics.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkStrike.h"

class SkStrikeCache::Node final : public SkRefCnt, public SkStrikeForGPU {
public:
    Node(SkStrikeCache* strikeCache,
         const SkDescriptor& desc,
         std::unique_ptr<SkScalerContext> scaler,
         const SkFontMetrics* metrics,
         std::unique_ptr<SkStrikePinner> pinner)
            : fStrikeCache{strikeCache}
            , fStrike{desc, std::move(scaler), metrics}
            , fPinner{std::move(pinner)} {}

    const SkGlyphPositionRoundingSpec& roundingSpec() const override {
        return fStrike.roundingSpec();
    }

    const SkDescriptor& getDescriptor() const override {
        return fStrike.getDescriptor();
    }

    void prepareForMaskDrawing(
            SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
        fStrike.prepareForMaskDrawing(drawbles, rejects);
    }

    void prepareForSDFTDrawing(
            SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
        fStrike.prepareForSDFTDrawing(drawbles, rejects);
    }

    void prepareForPathDrawing(
            SkDrawableGlyphBuffer* drawbles, SkSourceGlyphBuffer* rejects) override {
        fStrike.prepareForPathDrawing(drawbles, rejects);
    }

    void onAboutToExitScope() override {
        fStrikeCache->attachNode(this);
    }

    SkStrikeCache* const            fStrikeCache;
    Node*                           fNext{nullptr};
    Node*                           fPrev{nullptr};
    SkStrike                        fStrike;
    std::unique_ptr<SkStrikePinner> fPinner;
};

bool gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental = false;

SkStrikeCache* SkStrikeCache::GlobalStrikeCache() {
#if !defined(SK_BUILD_FOR_IOS)
    if (gSkUseThreadLocalStrikeCaches_IAcknowledgeThisIsIncrediblyExperimental) {
        static thread_local auto* cache = new SkStrikeCache;
        return cache;
    }
#endif
    static auto* cache = new SkStrikeCache;
    return cache;
}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(SkStrikeCache::Node* node)
    : fNode{node} {}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr()
    : fNode{nullptr} {}

SkStrikeCache::ExclusiveStrikePtr::ExclusiveStrikePtr(ExclusiveStrikePtr&& o)
    : fNode{o.fNode} {
    o.fNode = nullptr;
}

SkStrikeCache::ExclusiveStrikePtr&
SkStrikeCache::ExclusiveStrikePtr::operator = (ExclusiveStrikePtr&& o) {
    if (fNode != nullptr) {
        fNode->fStrikeCache->attachNode(fNode);
    }
    fNode = o.fNode;
    o.fNode = nullptr;
    return *this;
}

SkStrikeCache::ExclusiveStrikePtr::~ExclusiveStrikePtr() {
    if (fNode != nullptr) {
        fNode->fStrikeCache->attachNode(fNode);
    }
}

SkStrike* SkStrikeCache::ExclusiveStrikePtr::get() const {
    return &fNode->fStrike;
}

SkStrike* SkStrikeCache::ExclusiveStrikePtr::operator -> () const {
    return this->get();
}

SkStrike& SkStrikeCache::ExclusiveStrikePtr::operator *  () const {
    return *this->get();
}

SkStrikeCache::ExclusiveStrikePtr::operator bool () const {
    return fNode != nullptr;
}

bool operator == (const SkStrikeCache::ExclusiveStrikePtr& lhs,
                  const SkStrikeCache::ExclusiveStrikePtr& rhs) {
    return lhs.fNode == rhs.fNode;
}

bool operator == (const SkStrikeCache::ExclusiveStrikePtr& lhs, decltype(nullptr)) {
    return lhs.fNode == nullptr;
}

bool operator == (decltype(nullptr), const SkStrikeCache::ExclusiveStrikePtr& rhs) {
    return nullptr == rhs.fNode;
}

SkStrikeCache::~SkStrikeCache() {
    Node* node = fHead;
    while (node) {
        Node* next = node->fNext;
        node->unref();
        node = next;
    }
}

SkExclusiveStrikePtr SkStrikeCache::findOrCreateStrikeExclusive(
        const SkDescriptor& desc, const SkScalerContextEffects& effects, const SkTypeface& typeface)
{
    return SkExclusiveStrikePtr(this->findOrCreateStrike(desc, effects, typeface));
}

auto SkStrikeCache::findOrCreateStrike(const SkDescriptor& desc,
                                       const SkScalerContextEffects& effects,
                                       const SkTypeface& typeface) -> Node* {
    Node* node = this->findAndDetachStrike(desc);
    if (node == nullptr) {
        auto scaler = typeface.createScalerContext(effects, &desc);
        node = this->createStrike(desc, std::move(scaler));
    }
    return node;
}

SkScopedStrikeForGPU SkStrikeCache::findOrCreateScopedStrike(const SkDescriptor& desc,
                                                             const SkScalerContextEffects& effects,
                                                             const SkTypeface& typeface) {
    return SkScopedStrikeForGPU{this->findOrCreateStrike(desc, effects, typeface)};
}

void SkStrikeCache::PurgeAll() {
    GlobalStrikeCache()->purgeAll();
}

void SkStrikeCache::Dump() {
    SkDebugf("GlyphCache [     used    budget ]\n");
    SkDebugf("    bytes  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheUsed(), SkGraphics::GetFontCacheLimit());
    SkDebugf("    count  [ %8zu  %8zu ]\n",
             SkGraphics::GetFontCacheCountUsed(), SkGraphics::GetFontCacheCountLimit());

    int counter = 0;

    auto visitor = [&counter](const SkStrike& cache) {
        const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

        SkDebugf("index %d\n", counter);
        SkDebugf("%s", rec.dump().c_str());
        counter += 1;
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}

namespace {
    const char gGlyphCacheDumpName[] = "skia/sk_glyph_cache";
}  // namespace

void SkStrikeCache::DumpMemoryStatistics(SkTraceMemoryDump* dump) {
    dump->dumpNumericValue(gGlyphCacheDumpName, "size", "bytes", SkGraphics::GetFontCacheUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_size", "bytes",
                           SkGraphics::GetFontCacheLimit());
    dump->dumpNumericValue(gGlyphCacheDumpName, "glyph_count", "objects",
                           SkGraphics::GetFontCacheCountUsed());
    dump->dumpNumericValue(gGlyphCacheDumpName, "budget_glyph_count", "objects",
                           SkGraphics::GetFontCacheCountLimit());

    if (dump->getRequestedDetails() == SkTraceMemoryDump::kLight_LevelOfDetail) {
        dump->setMemoryBacking(gGlyphCacheDumpName, "malloc", nullptr);
        return;
    }

    auto visitor = [&dump](const SkStrike& cache) {
        const SkTypeface* face = cache.getScalerContext()->getTypeface();
        const SkScalerContextRec& rec = cache.getScalerContext()->getRec();

        SkString fontName;
        face->getFamilyName(&fontName);
        // Replace all special characters with '_'.
        for (size_t index = 0; index < fontName.size(); ++index) {
            if (!std::isalnum(fontName[index])) {
                fontName[index] = '_';
            }
        }

        SkString dumpName = SkStringPrintf(
                "%s/%s_%d/%p", gGlyphCacheDumpName, fontName.c_str(), rec.fFontID, &cache);

        dump->dumpNumericValue(dumpName.c_str(),
                               "size", "bytes", cache.getMemoryUsed());
        dump->dumpNumericValue(dumpName.c_str(),
                               "glyph_count", "objects", cache.countCachedGlyphs());
        dump->setMemoryBacking(dumpName.c_str(), "malloc", nullptr);
    };

    GlobalStrikeCache()->forEachStrike(visitor);
}


void SkStrikeCache::attachNode(Node* node) {
    if (node == nullptr) {
        return;
    }
    SkAutoSpinlock ac(fLock);

    this->validate();
    node->fStrike.validate();

    this->internalAttachToHead(node);
    this->internalPurge();
}

SkExclusiveStrikePtr SkStrikeCache::findStrikeExclusive(const SkDescriptor& desc) {
    return SkExclusiveStrikePtr(this->findAndDetachStrike(desc));
}

auto SkStrikeCache::findAndDetachStrike(const SkDescriptor& desc) -> Node* {
    SkAutoSpinlock ac(fLock);

    for (Node* node = fHead; node != nullptr; node = node->fNext) {
        if (node->fStrike.getDescriptor() == desc) {
            this->internalDetachCache(node);
            return node;
        }
    }

    return nullptr;
}

SkExclusiveStrikePtr SkStrikeCache::createStrikeExclusive(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner)
{
    return SkExclusiveStrikePtr(
            this->createStrike(desc, std::move(scaler), maybeMetrics, std::move(pinner)));
}

auto SkStrikeCache::createStrike(
        const SkDescriptor& desc,
        std::unique_ptr<SkScalerContext> scaler,
        SkFontMetrics* maybeMetrics,
        std::unique_ptr<SkStrikePinner> pinner) -> Node* {
    return new Node{this, desc, std::move(scaler), maybeMetrics, std::move(pinner)};
}

void SkStrikeCache::purgeAll() {
    SkAutoSpinlock ac(fLock);
    this->internalPurge(fTotalMemoryUsed);
}

size_t SkStrikeCache::getTotalMemoryUsed() const {
    SkAutoSpinlock ac(fLock);
    return fTotalMemoryUsed;
}

int SkStrikeCache::getCacheCountUsed() const {
    SkAutoSpinlock ac(fLock);
    return fCacheCount;
}

int SkStrikeCache::getCacheCountLimit() const {
    SkAutoSpinlock ac(fLock);
    return fCacheCountLimit;
}

size_t SkStrikeCache::setCacheSizeLimit(size_t newLimit) {
    static const size_t minLimit = 256 * 1024;
    if (newLimit < minLimit) {
        newLimit = minLimit;
    }

    SkAutoSpinlock ac(fLock);

    size_t prevLimit = fCacheSizeLimit;
    fCacheSizeLimit = newLimit;
    this->internalPurge();
    return prevLimit;
}

size_t  SkStrikeCache::getCacheSizeLimit() const {
    SkAutoSpinlock ac(fLock);
    return fCacheSizeLimit;
}

int SkStrikeCache::setCacheCountLimit(int newCount) {
    if (newCount < 0) {
        newCount = 0;
    }

    SkAutoSpinlock ac(fLock);

    int prevCount = fCacheCountLimit;
    fCacheCountLimit = newCount;
    this->internalPurge();
    return prevCount;
}

int SkStrikeCache::getCachePointSizeLimit() const {
    SkAutoSpinlock ac(fLock);
    return fPointSizeLimit;
}

int SkStrikeCache::setCachePointSizeLimit(int newLimit) {
    if (newLimit < 0) {
        newLimit = 0;
    }

    SkAutoSpinlock ac(fLock);

    int prevLimit = fPointSizeLimit;
    fPointSizeLimit = newLimit;
    return prevLimit;
}

void SkStrikeCache::forEachStrike(std::function<void(const SkStrike&)> visitor) const {
    SkAutoSpinlock ac(fLock);

    this->validate();

    for (Node* node = fHead; node != nullptr; node = node->fNext) {
        visitor(node->fStrike);
    }
}

size_t SkStrikeCache::internalPurge(size_t minBytesNeeded) {
    this->validate();

    size_t bytesNeeded = 0;
    if (fTotalMemoryUsed > fCacheSizeLimit) {
        bytesNeeded = fTotalMemoryUsed - fCacheSizeLimit;
    }
    bytesNeeded = std::max(bytesNeeded, minBytesNeeded);
    if (bytesNeeded) {
        // no small purges!
        bytesNeeded = std::max(bytesNeeded, fTotalMemoryUsed >> 2);
    }

    int countNeeded = 0;
    if (fCacheCount > fCacheCountLimit) {
        countNeeded = fCacheCount - fCacheCountLimit;
        // no small purges!
        countNeeded = std::max(countNeeded, fCacheCount >> 2);
    }

    // early exit
    if (!countNeeded && !bytesNeeded) {
        return 0;
    }

    size_t  bytesFreed = 0;
    int     countFreed = 0;

    // Start at the tail and proceed backwards deleting; the list is in LRU
    // order, with unimportant entries at the tail.
    Node* node = fTail;
    while (node != nullptr && (bytesFreed < bytesNeeded || countFreed < countNeeded)) {
        Node* prev = node->fPrev;

        // Only delete if the strike is not pinned.
        if (node->fPinner == nullptr || node->fPinner->canDelete()) {
            bytesFreed += node->fStrike.getMemoryUsed();
            countFreed += 1;
            this->internalDetachCache(node);
            node->unref();
        }
        node = prev;
    }

    this->validate();

#ifdef SPEW_PURGE_STATUS
    if (countFreed) {
        SkDebugf("purging %dK from font cache [%d entries]\n",
                 (int)(bytesFreed >> 10), countFreed);
    }
#endif

    return bytesFreed;
}

void SkStrikeCache::internalAttachToHead(Node* node) {
    SkASSERT(nullptr == node->fPrev && nullptr == node->fNext);
    if (fHead) {
        fHead->fPrev = node;
        node->fNext = fHead;
    }
    fHead = node;

    if (fTail == nullptr) {
        fTail = node;
    }

    fCacheCount += 1;
    fTotalMemoryUsed += node->fStrike.getMemoryUsed();
}

void SkStrikeCache::internalDetachCache(Node* node) {
    SkASSERT(fCacheCount > 0);
    fCacheCount -= 1;
    fTotalMemoryUsed -= node->fStrike.getMemoryUsed();

    if (node->fPrev) {
        node->fPrev->fNext = node->fNext;
    } else {
        fHead = node->fNext;
    }
    if (node->fNext) {
        node->fNext->fPrev = node->fPrev;
    } else {
        fTail = node->fPrev;
    }
    node->fPrev = node->fNext = nullptr;
}

void SkStrikeCache::ValidateGlyphCacheDataSize() {
#ifdef SK_DEBUG
    GlobalStrikeCache()->validateGlyphCacheDataSize();
#endif
}

#ifdef SK_DEBUG
void SkStrikeCache::validateGlyphCacheDataSize() const {
    this->forEachStrike(
            [](const SkStrike& cache) { cache.forceValidate();
    });
}
#endif

#ifdef SK_DEBUG
void SkStrikeCache::validate() const {
    size_t computedBytes = 0;
    int computedCount = 0;

    const Node* node = fHead;
    while (node != nullptr) {
        computedBytes += node->fStrike.getMemoryUsed();
        computedCount += 1;
        node = node->fNext;
    }

    // Can't use SkASSERTF because it looses thread annotations.
    if (fCacheCount != computedCount) {
        SkDebugf("fCacheCount: %d, computedCount: %d", fCacheCount, computedCount);
        SK_ABORT("fCacheCount != computedCount");
    }
    if (fTotalMemoryUsed != computedBytes) {
        SkDebugf("fTotalMemoryUsed: %d, computedBytes: %d", fTotalMemoryUsed, computedBytes);
        SK_ABORT("fTotalMemoryUsed == computedBytes");
    }
}
#endif
