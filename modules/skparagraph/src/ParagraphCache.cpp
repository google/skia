// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/ParagraphCache.h"

namespace skia {
namespace textlayout {

ParagraphCacheKey::ParagraphCacheKey(ParagraphImpl* paragraph)
        : fText(paragraph->fText.c_str(), paragraph->fText.size())
        , fFontSwitches(paragraph->switches())
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) { }

ParagraphCacheValue::ParagraphCacheValue(ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fInternalState(paragraph->state())
        , fRuns(paragraph->fRuns)
        , fClusters(paragraph->fClusters)
        , fRunShifts(paragraph->fRunShifts) { }

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
        // TODO: this is too strong, but at least we will not lose lines
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

    void updateFrom(ParagraphImpl* paragraph) {

        this->fValue->fInternalState = paragraph->fState;
        this->fValue->fRunShifts = paragraph->fRunShifts;
        for (size_t i = 0; i < paragraph->fRuns.size(); ++i) {
            auto& run = paragraph->fRuns[i];
            if (run.fSpaced) {
                this->fValue->fRuns[i] = run;
            }
        }
    }

    void updateTo(ParagraphImpl* paragraph) {
        paragraph->fRuns.reset();
        paragraph->fRuns = this->fValue->fRuns;
        for (auto& run : paragraph->fRuns) {
            run.setMaster(paragraph);
        }

        paragraph->fClusters.reset();
        paragraph->fClusters = this->fValue->fClusters;
        for (auto& cluster : paragraph->fClusters) {
            cluster.setMaster(paragraph);
        }

        paragraph->fRunShifts.reset();
        for (auto& runShift : this->fValue->fRunShifts) {
            paragraph->fRunShifts.push_back(runShift);
        }

        paragraph->fState = this->fValue->fInternalState;
    }

    ParagraphCacheValue* fValue;
};

ParagraphCache::ParagraphCache()
    : fChecker([](ParagraphImpl* impl, const char*, bool){ })
    , fMap(kMaxEntries)
#ifdef PARAGRAPH_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{ }

ParagraphCache::~ParagraphCache() {
#ifdef PARAGRAPH_CACHE_STATS
    printStatistics();
#endif
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
    fMap.foreach([](std::unique_ptr<Entry>* e) {
    });

    this->reset();
}

void ParagraphCache::reset() {
#ifdef PARAGRAPH_CACHE_STATS
    fTotalRequests = 0;
    fCacheMisses = 0;
    fHashMisses = 0;
#endif
    fMap.reset();
}

bool ParagraphCache::findParagraph(ParagraphImpl* paragraph) {
#ifdef PARAGRAPH_CACHE_STATS
    ++fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fMap.find(key);
    if (!entry) {
        // We have a cache miss
#ifdef PARAGRAPH_CACHE_STATS
        ++fCacheMisses;
#endif
        fChecker(paragraph, "missingParagraph", true);
        return false;
    }
    (*entry)->updateTo(paragraph);
    fChecker(paragraph, "foundParagraph", true);
    return true;
}

bool ParagraphCache::updateParagraph(ParagraphImpl* paragraph) {
#ifdef PARAGRAPH_CACHE_STATS
    ++fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fMap.find(key);
    if (!entry) {
        ParagraphCacheValue* value = new ParagraphCacheValue(paragraph);
        fMap.insert(key, std::unique_ptr<Entry>(new Entry(value)));
        fChecker(paragraph, "addedParagraph", true);
        return true;
    } else {
        (*entry)->updateFrom(paragraph);
        fChecker(paragraph, "updatedParagraph", true);
        return false;
    }
}
}
}
