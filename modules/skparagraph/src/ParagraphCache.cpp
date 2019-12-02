// Copyright 2019 Google LLC.
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

class ParagraphCacheKey {
public:
    ParagraphCacheKey(const ParagraphImpl* paragraph)
        : fText(paragraph->fText.c_str(), paragraph->fText.size())
        , fResolvedFonts(paragraph->resolvedFonts())
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) { }

    SkString fText;
    SkTArray<ResolvedFontDescriptor> fResolvedFonts;
    SkTArray<Block, true> fTextStyles;
    ParagraphStyle fParagraphStyle;
};

class ParagraphCacheValue {
public:
    ParagraphCacheValue(const ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fInternalState(paragraph->fState)
        , fRuns(paragraph->fRuns)
        , fClusters(paragraph->fClusters)
        , fUnresolvedGlyphs(paragraph->fUnresolvedGlyphs){ }

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results:
    InternalState fInternalState;
    SkTArray<Run, false> fRuns;
    SkTArray<Cluster, true> fClusters;
    size_t fUnresolvedGlyphs;
};


uint32_t ParagraphCache::KeyHash::mix(uint32_t hash, uint32_t data) const {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}
uint32_t ParagraphCache::KeyHash::operator()(const ParagraphCacheKey& key) const {
    uint32_t hash = 0;
    for (auto& fd : key.fResolvedFonts) {
        hash = mix(hash, SkGoodHash()(fd.fTextStart));
        hash = mix(hash, SkGoodHash()(fd.fFont.getSize()));

        if (fd.fFont.getTypeface() != nullptr) {
            SkString name;
            fd.fFont.getTypeface()->getFamilyName(&name);
            hash = mix(hash, SkGoodHash()(name));
            hash = mix(hash, SkGoodHash()(fd.fFont.getTypeface()->fontStyle()));
        }
    }
    for (auto& ts : key.fTextStyles) {
        if (!ts.fStyle.isPlaceholder()) {
            hash = mix(hash, SkGoodHash()(ts.fStyle.getLetterSpacing()));
            hash = mix(hash, SkGoodHash()(ts.fStyle.getWordSpacing()));
            hash = mix(hash, SkGoodHash()(ts.fRange));
        } else {
            // TODO: cache placeholders
        }
    }
    hash = mix(hash, SkGoodHash()(key.fText));
    return hash;
}

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b) {
    if (a.fText.size() != b.fText.size()) {
        return false;
    }
    if (a.fResolvedFonts.count() != b.fResolvedFonts.count()) {
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

    for (size_t i = 0; i < a.fResolvedFonts.size(); ++i) {
        auto& fda = a.fResolvedFonts[i];
        auto& fdb = b.fResolvedFonts[i];
        if (fda.fTextStart != fdb.fTextStart) {
            return false;
        }
        if (fda.fFont != fdb.fFont) {
            return false;
        }
    }

    for (size_t i = 0; i < a.fTextStyles.size(); ++i) {
        auto& tsa = a.fTextStyles[i];
        auto& tsb = b.fTextStyles[i];
        if (!(tsa.fStyle == tsb.fStyle)) {
            return false;
        }
        if (!tsa.fStyle.isPlaceholder()) {
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
        } else {
            // TODO: compare placeholders
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

    paragraph->fState = entry->fValue->fInternalState;
    paragraph->fUnresolvedGlyphs = entry->fValue->fUnresolvedGlyphs;
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
