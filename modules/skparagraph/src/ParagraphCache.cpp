// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

class ParagraphCacheKey {
public:
    ParagraphCacheKey(const ParagraphImpl* paragraph)
        : fText(paragraph->fText.c_str(), paragraph->fText.size())
        , fFontSwitches(paragraph->switches())
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) { }

    SkString fText;
    SkTArray<FontDescr> fFontSwitches;
    SkTArray<Block, true> fTextStyles;
    ParagraphStyle fParagraphStyle;
};

class ParagraphCacheValue {
public:
    ParagraphCacheValue(const ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fInternalState(paragraph->state())
        , fRuns(paragraph->fRuns)
        , fClusters(paragraph->fClusters) { }

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results:
    InternalState fInternalState;
    SkTArray<Run> fRuns;
    SkTArray<Cluster, true> fClusters;
    SkTArray<RunShifts, true> fRunShifts;
};


uint32_t ParagraphCache::KeyHash::mix(uint32_t hash, uint32_t data) const {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}
uint32_t ParagraphCache::KeyHash::operator()(const ParagraphCacheKey& key) const {
    uint32_t hash = 0;
    for (auto& fd : key.fFontSwitches) {
        hash = mix(hash, SkGoodHash()(fd.fStart));
        hash = mix(hash, SkGoodHash()(fd.fFont.getSize()));

        if (fd.fFont.getTypeface() != nullptr) {
            SkString name;
            fd.fFont.getTypeface()->getFamilyName(&name);
            hash = mix(hash, SkGoodHash()(name));
            hash = mix(hash, SkGoodHash()(fd.fFont.getTypeface()->fontStyle()));
        }
    }
    for (auto& ts : key.fTextStyles) {
        hash = mix(hash, SkGoodHash()(ts.fStyle.getLetterSpacing()));
        hash = mix(hash, SkGoodHash()(ts.fStyle.getWordSpacing()));
        hash = mix(hash, SkGoodHash()(ts.fRange));
    }
    hash = mix(hash, SkGoodHash()(key.fText));
    return hash;
}

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b) {
    if (a.fText.size() != b.fText.size()) {
        return false;
    }
    if (a.fFontSwitches.count() != b.fFontSwitches.count()) {
        return false;
    }
    if (a.fText != b.fText) {
        return false;
    }
    if (a.fTextStyles.size() != b.fTextStyles.size()) {
        return false;
    }

    if (a.fParagraphStyle.getMaxLines() != b.fParagraphStyle.getMaxLines()) {
        // This is too strong, but at least we will not lose lines
        return false;
    }

    for (size_t i = 0; i < a.fFontSwitches.size(); ++i) {
        auto& fda = a.fFontSwitches[i];
        auto& fdb = b.fFontSwitches[i];
        if (fda.fStart != fdb.fStart) {
            return false;
        }
        if (fda.fFont != fdb.fFont) {
            return false;
        }
    }

    for (size_t i = 0; i < a.fTextStyles.size(); ++i) {
        auto& tsa = a.fTextStyles[i];
        auto& tsb = b.fTextStyles[i];
        if (tsa.fStyle.getLetterSpacing() != tsb.fStyle.getLetterSpacing()) {
            return false;
        }
        if (tsa.fStyle.getWordSpacing() != tsb.fStyle.getWordSpacing()) {
            return false;
        }
        if (tsa.fRange.width() != tsb.fRange.width()) {
            return false;
        }
        if (tsa.fRange.start != tsb.fRange.start) {
            return false;
        }
    }

    return true;
}

struct ParagraphCache::Entry {

    Entry(ParagraphCacheValue* value) : fValue(value) {}
    ParagraphCacheValue* fValue;
};

ParagraphCache::ParagraphCache()
    : fChecker([](ParagraphImpl* impl, const char*, bool){ })
    , fLRUCacheMap(kMaxEntries)
    , fCacheIsOn(true)
#ifdef PARAGRAPH_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{ }

ParagraphCache::~ParagraphCache() { }

void ParagraphCache::updateFrom(const ParagraphImpl* paragraph, Entry* entry) {

    entry->fValue->fInternalState = paragraph->state();
    entry->fValue->fRunShifts = paragraph->fRunShifts;
    for (size_t i = 0; i < paragraph->fRuns.size(); ++i) {
        auto& run = paragraph->fRuns[i];
        if (run.fSpaced) {
            entry->fValue->fRuns[i] = run;
        }
    }
}

void ParagraphCache::updateTo(ParagraphImpl* paragraph, const Entry* entry) {
    paragraph->fRuns.reset();
    paragraph->fRuns = entry->fValue->fRuns;
    for (auto& run : paragraph->fRuns) {
        run.setMaster(paragraph);
    }

    paragraph->fClusters.reset();
    paragraph->fClusters = entry->fValue->fClusters;
    for (auto& cluster : paragraph->fClusters) {
        cluster.setMaster(paragraph);
    }

    paragraph->fRunShifts.reset();
    for (auto& runShift : entry->fValue->fRunShifts) {
        paragraph->fRunShifts.push_back(runShift);
    }

    paragraph->fState = entry->fValue->fInternalState;
}

void ParagraphCache::printStatistics() {
    SkDebugf("--- Paragraph Cache ---\n");
    SkDebugf("Total requests: %d\n", fTotalRequests);
    SkDebugf("Cache misses: %d\n", fCacheMisses);
    SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ? 100.f * fCacheMisses / fTotalRequests : 0.f);
    int cacheHits = fTotalRequests - fCacheMisses;
    SkDebugf("Hash miss %%: %f\n", (cacheHits > 0) ? 100.f * fHashMisses / cacheHits : 0.f);
    SkDebugf("---------------------\n");
}

void ParagraphCache::abandon() {
    SkAutoMutexExclusive lock(fParagraphMutex);
    fLRUCacheMap.foreach([](std::unique_ptr<Entry>* e) {
    });

    this->reset();
}

void ParagraphCache::reset() {
    SkAutoMutexExclusive lock(fParagraphMutex);
#ifdef PARAGRAPH_CACHE_STATS
    fTotalRequests = 0;
    fCacheMisses = 0;
    fHashMisses = 0;
#endif
    fLRUCacheMap.reset();
}

bool ParagraphCache::findParagraph(ParagraphImpl* paragraph) {
    if (!fCacheIsOn) {
        return false;
    }
#ifdef PARAGRAPH_CACHE_STATS
    ++fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fLRUCacheMap.find(key);
    if (!entry) {
        // We have a cache miss
#ifdef PARAGRAPH_CACHE_STATS
        ++fCacheMisses;
#endif
        fChecker(paragraph, "missingParagraph", true);
        return false;
    }
    updateTo(paragraph, entry->get());
    fChecker(paragraph, "foundParagraph", true);
    return true;
}

bool ParagraphCache::updateParagraph(ParagraphImpl* paragraph) {
    if (!fCacheIsOn) {
        return false;
    }
#ifdef PARAGRAPH_CACHE_STATS
    ++fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fLRUCacheMap.find(key);
    if (!entry) {
        ParagraphCacheValue* value = new ParagraphCacheValue(paragraph);
        fLRUCacheMap.insert(key, std::unique_ptr<Entry>(new Entry(value)));
        fChecker(paragraph, "addedParagraph", true);
        return true;
    } else {
        updateFrom(paragraph, entry->get());
        fChecker(paragraph, "updatedParagraph", true);
        return false;
    }
}
}
}
