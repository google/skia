// Copyright 2019 Google LLC.
#include <memory>

#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

namespace {
    SkScalar relax(SkScalar a) {
        // This rounding is done to match Flutter tests. Must be removed..
        if (SkScalarIsFinite(a)) {
          auto threshold = SkIntToScalar(1 << 12);
          return SkScalarRoundToScalar(a * threshold)/threshold;
        } else {
          return a;
        }
    }
}  // namespace

class ParagraphCacheKey {
public:
    ParagraphCacheKey(const ParagraphImpl* paragraph)
        : fText(paragraph->fText.c_str(), paragraph->fText.size())
        , fPlaceholders(paragraph->fPlaceholders)
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) { }

    SkString fText;
    SkTArray<Placeholder, true> fPlaceholders;
    SkTArray<Block, true> fTextStyles;
    ParagraphStyle fParagraphStyle;
};

class ParagraphCacheValue {
public:
    ParagraphCacheValue(const ParagraphImpl* paragraph)
        : fKey(ParagraphCacheKey(paragraph))
        , fRuns(paragraph->fRuns)
        , fCodeUnitProperties(paragraph->fCodeUnitProperties)
        , fWords(paragraph->fWords)
        , fBidiRegions(paragraph->fBidiRegions)
        , fUTF8IndexForUTF16Index(paragraph->fUTF8IndexForUTF16Index)
        , fUTF16IndexForUTF8Index(paragraph->fUTF16IndexForUTF8Index) { }

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results
    SkTArray<Run, false> fRuns;
    // ICU results
    SkTArray<CodeUnitFlags> fCodeUnitProperties;
    std::vector<size_t> fWords;
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
    SkTArray<TextIndex, true> fUTF8IndexForUTF16Index;
    SkTArray<size_t, true> fUTF16IndexForUTF8Index;
};

uint32_t ParagraphCache::KeyHash::mix(uint32_t hash, uint32_t data) const {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}

uint32_t ParagraphCache::KeyHash::operator()(const ParagraphCacheKey& key) const {
    uint32_t hash = 0;
    for (auto& ph : key.fPlaceholders) {
        if (ph.fRange.width() == 0) {
            continue;
        }
        hash = mix(hash, SkGoodHash()(ph.fRange.start));
        hash = mix(hash, SkGoodHash()(ph.fRange.end));
        hash = mix(hash, SkGoodHash()(relax(ph.fStyle.fHeight)));
        hash = mix(hash, SkGoodHash()(relax(ph.fStyle.fWidth)));
        hash = mix(hash, SkGoodHash()(ph.fStyle.fAlignment));
        hash = mix(hash, SkGoodHash()(ph.fStyle.fBaseline));
        if (ph.fStyle.fAlignment == PlaceholderAlignment::kBaseline) {
            hash = mix(hash, SkGoodHash()(relax(ph.fStyle.fBaselineOffset)));
        }
    }

    for (auto& ts : key.fTextStyles) {
        if (ts.fStyle.isPlaceholder()) {
            continue;
        }
        hash = mix(hash, SkGoodHash()(relax(ts.fStyle.getLetterSpacing())));
        hash = mix(hash, SkGoodHash()(relax(ts.fStyle.getWordSpacing())));
        hash = mix(hash, SkGoodHash()(ts.fStyle.getLocale()));
        hash = mix(hash, SkGoodHash()(relax(ts.fStyle.getHeight())));
        for (auto& ff : ts.fStyle.getFontFamilies()) {
            hash = mix(hash, SkGoodHash()(ff));
        }
        for (auto& ff : ts.fStyle.getFontFeatures()) {
            hash = mix(hash, SkGoodHash()(ff.fValue));
            hash = mix(hash, SkGoodHash()(ff.fName));
        }
        hash = mix(hash, SkGoodHash()(ts.fStyle.getFontStyle()));
        hash = mix(hash, SkGoodHash()(relax(ts.fStyle.getFontSize())));
        hash = mix(hash, SkGoodHash()(ts.fRange));
    }

    hash = mix(hash, SkGoodHash()(relax(key.fParagraphStyle.getHeight())));
    hash = mix(hash, SkGoodHash()(key.fParagraphStyle.getTextDirection()));

    auto& strutStyle = key.fParagraphStyle.getStrutStyle();
    if (strutStyle.getStrutEnabled()) {
        hash = mix(hash, SkGoodHash()(relax(strutStyle.getHeight())));
        hash = mix(hash, SkGoodHash()(relax(strutStyle.getLeading())));
        hash = mix(hash, SkGoodHash()(relax(strutStyle.getFontSize())));
        hash = mix(hash, SkGoodHash()(strutStyle.getHeightOverride()));
        hash = mix(hash, SkGoodHash()(strutStyle.getFontStyle()));
        hash = mix(hash, SkGoodHash()(strutStyle.getForceStrutHeight()));
        for (auto& ff : strutStyle.getFontFamilies()) {
            hash = mix(hash, SkGoodHash()(ff));
        }
    }

    hash = mix(hash, SkGoodHash()(key.fText));
    return hash;
}

bool operator==(const ParagraphCacheKey& a, const ParagraphCacheKey& b) {
    if (a.fText.size() != b.fText.size()) {
        return false;
    }
    if (a.fPlaceholders.count() != b.fPlaceholders.count()) {
        return false;
    }
    if (a.fText != b.fText) {
        return false;
    }
    if (a.fTextStyles.size() != b.fTextStyles.size()) {
        return false;
    }

    // There is no need to compare default paragraph styles - they are included into fTextStyles
    if (!nearlyEqual(a.fParagraphStyle.getHeight(), b.fParagraphStyle.getHeight())) {
        return false;
    }
    if (a.fParagraphStyle.getTextDirection() != b.fParagraphStyle.getTextDirection()) {
        return false;
    }

    if (!(a.fParagraphStyle.getStrutStyle() == b.fParagraphStyle.getStrutStyle())) {
        return false;
    }

    for (size_t i = 0; i < a.fTextStyles.size(); ++i) {
        auto& tsa = a.fTextStyles[i];
        auto& tsb = b.fTextStyles[i];
        if (tsa.fStyle.isPlaceholder()) {
            continue;
        }
        if (!(tsa.fStyle.equalsByFonts(tsb.fStyle))) {
            return false;
        }
        if (tsa.fRange.width() != tsb.fRange.width()) {
            return false;
        }
        if (tsa.fRange.start != tsb.fRange.start) {
            return false;
        }
    }
    for (size_t i = 0; i < a.fPlaceholders.size(); ++i) {
        auto& tsa = a.fPlaceholders[i];
        auto& tsb = b.fPlaceholders[i];
        if (tsa.fRange.width() == 0 && tsb.fRange.width() == 0) {
            continue;
        }
        if (!(tsa.fStyle.equals(tsb.fStyle))) {
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
    std::unique_ptr<ParagraphCacheValue> fValue;
};

ParagraphCache::ParagraphCache()
    : fChecker([](ParagraphImpl* impl, const char*, bool){ })
    , fLRUCacheMap(kMaxEntries)
    , fCacheIsOn(true)
    , fLastCachedValue(nullptr)
#ifdef PARAGRAPH_CACHE_STATS
    , fTotalRequests(0)
    , fCacheMisses(0)
    , fHashMisses(0)
#endif
{ }

ParagraphCache::~ParagraphCache() { }

void ParagraphCache::updateTo(ParagraphImpl* paragraph, const Entry* entry) {

    paragraph->fRuns.reset();
    paragraph->fRuns = entry->fValue->fRuns;
    paragraph->fCodeUnitProperties = entry->fValue->fCodeUnitProperties;
    paragraph->fWords = entry->fValue->fWords;
    paragraph->fBidiRegions = entry->fValue->fBidiRegions;
    paragraph->fUTF8IndexForUTF16Index = entry->fValue->fUTF8IndexForUTF16Index;
    paragraph->fUTF16IndexForUTF8Index = entry->fValue->fUTF16IndexForUTF8Index;
    for (auto& run : paragraph->fRuns) {
      run.setOwner(paragraph);
    }
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
    fLastCachedValue = nullptr;
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
        // isTooMuchMemoryWasted(paragraph) not needed for now
        if (isPossiblyTextEditing(paragraph)) {
            // Skip this paragraph
            return false;
        }
        ParagraphCacheValue* value = new ParagraphCacheValue(paragraph);
        fLRUCacheMap.insert(key, std::make_unique<Entry>(value));
        fChecker(paragraph, "addedParagraph", true);
        fLastCachedValue = value;
        return true;
    } else {
        // We do not have to update the paragraph
        return false;
    }
}

// Special situation: (very) long paragraph that is close to the last formatted paragraph
#define NOCACHE_PREFIX_LENGTH 40
bool ParagraphCache::isPossiblyTextEditing(ParagraphImpl* paragraph) {
    if (fLastCachedValue == nullptr) {
        return false;
    }

    auto& lastText = fLastCachedValue->fKey.fText;
    auto& text = paragraph->fText;

    if ((lastText.size() < NOCACHE_PREFIX_LENGTH) || (text.size() < NOCACHE_PREFIX_LENGTH)) {
        // Either last text or the current are too short
        return false;
    }

    if (std::strncmp(lastText.c_str(), text.c_str(), NOCACHE_PREFIX_LENGTH) == 0) {
        // Texts have the same starts
        return true;
    }

    if (std::strncmp(&lastText[lastText.size() - NOCACHE_PREFIX_LENGTH], &text[text.size() - NOCACHE_PREFIX_LENGTH], NOCACHE_PREFIX_LENGTH) == 0) {
        // Texts have the same ends
        return true;
    }

    // It does not look like editing the text
    return false;
}
}  // namespace textlayout
}  // namespace skia
