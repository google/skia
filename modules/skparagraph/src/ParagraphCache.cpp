// Copyright 2019 Google LLC
#include <memory>

#include "modules/skparagraph/include/FontArguments.h"
#include "modules/skparagraph/include/ParagraphCache.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "src/core/SkFloatBits.h"
#include "src/core/SkLRUCache.h"

using namespace skia_private;

namespace skia {
namespace textlayout {

class ParagraphCacheKey;
class ParagraphCacheValue;

struct ParagraphCache::Cache {
    Cache()
        : fLRUCacheMap(kMaxEntries)
        , fCacheIsOn(true)
        , fLastCachedValue(nullptr)
#ifdef PARAGRAPH_CACHE_STATS
        , fTotalRequests(0)
        , fCacheMisses(0)
        , fHashMisses(0)
#endif
    {}

    void printStatistics() const {
        SkDebugf("--- Paragraph Cache ---\n");
        SkDebugf("Total requests: %d\n", fTotalRequests);
        SkDebugf("Cache misses: %d\n", fCacheMisses);
        SkDebugf("Cache miss %%: %f\n", (fTotalRequests > 0) ? 100.f * fCacheMisses / fTotalRequests : 0.f);
        int cacheHits = fTotalRequests - fCacheMisses;
        SkDebugf("Hash miss %%: %f\n", (cacheHits > 0) ? 100.f * fHashMisses / cacheHits : 0.f);
        SkDebugf("---------------------\n");
    }

    struct KeyHash {
        uint32_t operator()(const ParagraphCacheKey& key) const;
    };
    static const int kMaxEntries = 128;

    SkLRUCache<ParagraphCacheKey, std::unique_ptr<Entry>, KeyHash> fLRUCacheMap;
    bool fCacheIsOn;
    ParagraphCacheValue* fLastCachedValue;

#ifdef PARAGRAPH_CACHE_STATS
    int fTotalRequests;
    int fCacheMisses;
    int fHashMisses; // cache hit but hash table missed
#endif
};

namespace {
    int32_t relax(SkScalar a) {
        // This rounding is done to match Flutter tests. Must be removed..
        if (SkIsFinite(a)) {
          auto threshold = SkIntToScalar(1 << 12);
          return SkFloat2Bits(SkScalarRoundToScalar(a * threshold)/threshold);
        } else {
          return SkFloat2Bits(a);
        }
    }

}  // namespace

class ParagraphCacheKey {
public:
    ParagraphCacheKey(const ParagraphImpl* paragraph)
        : fText(paragraph->fText.c_str(), paragraph->fText.size())
        , fPlaceholders(paragraph->fPlaceholders)
        , fTextStyles(paragraph->fTextStyles)
        , fParagraphStyle(paragraph->paragraphStyle()) {
        fHash = computeHash();
    }

    ParagraphCacheKey(const ParagraphCacheKey& other) = default;

    ParagraphCacheKey(ParagraphCacheKey&& other)
        : fText(std::move(other.fText))
        , fPlaceholders(std::move(other.fPlaceholders))
        , fTextStyles(std::move(other.fTextStyles))
        , fParagraphStyle(std::move(other.fParagraphStyle))
        , fHash(other.fHash) {
        other.fHash = 0;
    }

    bool operator==(const ParagraphCacheKey& other) const;

    uint32_t hash() const { return fHash; }

    const SkString& text() const { return fText; }

private:
    static uint32_t mix(uint32_t hash, uint32_t data);
    static uint32_t mixTextStyle(uint32_t hash, const TextStyle& style);
    uint32_t computeHash() const;

    SkString fText;
    TArray<Placeholder> fPlaceholders;
    TArray<Block> fTextStyles;
    ParagraphStyle fParagraphStyle;
    uint32_t fHash;
};

class ParagraphCacheValue {
public:
    ParagraphCacheValue(ParagraphCacheKey&& key, const ParagraphImpl* paragraph)
        : fKey(std::move(key))
        , fRuns(paragraph->fRuns)
        , fUnresolvedGlyphs(paragraph->fUnresolvedGlyphs)
        , fUnresolvedCodepoints(paragraph->fUnresolvedCodepoints)
        , fClusters(paragraph->fClusters)
        , fClustersIndexFromCodeUnit(paragraph->fClustersIndexFromCodeUnit)
        , fCodeUnitProperties(paragraph->fCodeUnitProperties)
        , fWords(paragraph->fWords)
        , fBidiRegions(paragraph->fBidiRegions)
        , fHasLineBreaks(paragraph->fHasLineBreaks)
        , fHasWhitespacesInside(paragraph->fHasWhitespacesInside)
        , fTrailingSpaces(paragraph->fTrailingSpaces)
        , fFontSwitches(paragraph->fFontSwitches) { }

    // Input == key
    ParagraphCacheKey fKey;

    // Shaped results
    TArray<Run, false> fRuns;
    size_t fUnresolvedGlyphs;
    std::unordered_set<SkUnichar> fUnresolvedCodepoints;
    TArray<Cluster, true> fClusters;
    TArray<size_t, true> fClustersIndexFromCodeUnit;
    // ICU results
    TArray<SkUnicode::CodeUnitFlags, true> fCodeUnitProperties;
    std::vector<size_t> fWords;
    std::vector<SkUnicode::BidiRegion> fBidiRegions;
    bool fHasLineBreaks;
    bool fHasWhitespacesInside;
    TextIndex fTrailingSpaces;
    // Reported through ParagraphImpl::resolvedFonts()
    TArray<ResolvedFontDescriptor> fFontSwitches;
};

uint32_t ParagraphCacheKey::mix(uint32_t hash, uint32_t data) {
    hash += data;
    hash += (hash << 10);
    hash ^= (hash >> 6);
    return hash;
}

// Must cover the fields TextStyle::equalsByFonts() compares, normalized the same way
uint32_t ParagraphCacheKey::mixTextStyle(uint32_t hash, const TextStyle& style) {
    hash = mix(hash, SkGoodHash()(relax(style.getLetterSpacing())));
    hash = mix(hash, SkGoodHash()(relax(style.getWordSpacing())));
    hash = mix(hash, SkGoodHash()(style.getLocale()));
    hash = mix(hash, SkGoodHash()(relax(style.getHeight())));
    hash = mix(hash, SkGoodHash()(style.getHalfLeading()));
    hash = mix(hash, SkGoodHash()(relax(style.getBaselineShift())));
    for (auto& ff : style.getFontFamilies()) {
        hash = mix(hash, SkGoodHash()(ff));
    }
    for (auto& ff : style.getFontFeatures()) {
        hash = mix(hash, SkGoodHash()(ff.fValue));
        hash = mix(hash, SkGoodHash()(ff.fName));
    }
    hash = mix(hash, std::hash<std::optional<FontArguments>>()(style.getFontArguments()));
    hash = mix(hash, SkGoodHash()(style.getFontStyle()));
    hash = mix(hash, SkGoodHash()(relax(style.getFontSize())));
    hash = mix(hash, SkGoodHash()(static_cast<int>(style.getFontEdging())));
    hash = mix(hash, SkGoodHash()(static_cast<int>(style.getFontHinting())));
    hash = mix(hash, SkGoodHash()(style.getSubpixel()));
    return hash;
}

uint32_t ParagraphCacheKey::computeHash() const {
    uint32_t hash = 0;
    for (auto& ph : fPlaceholders) {
        if (ph.fRange.width() == 0) {
            continue;
        }
        hash = mix(hash, SkGoodHash()(ph.fRange));
        // The placeholder run takes its font metrics from the surrounding text style
        hash = mixTextStyle(hash, ph.fTextStyle);
        hash = mix(hash, SkGoodHash()(relax(ph.fStyle.fHeight)));
        hash = mix(hash, SkGoodHash()(relax(ph.fStyle.fWidth)));
        hash = mix(hash, SkGoodHash()(ph.fStyle.fAlignment));
        hash = mix(hash, SkGoodHash()(ph.fStyle.fBaseline));
        if (ph.fStyle.fAlignment == PlaceholderAlignment::kBaseline) {
            hash = mix(hash, SkGoodHash()(relax(ph.fStyle.fBaselineOffset)));
        }
    }

    for (auto& ts : fTextStyles) {
        if (ts.fStyle.isPlaceholder()) {
            continue;
        }
        hash = mixTextStyle(hash, ts.fStyle);
        hash = mix(hash, SkGoodHash()(ts.fRange));
    }

    hash = mix(hash, SkGoodHash()(fParagraphStyle.getTextDirection()));
    hash = mix(hash, SkGoodHash()(fParagraphStyle.getReplaceTabCharacters() ? 1 : 0));
    hash = mix(hash, SkGoodHash()(fParagraphStyle.fakeMissingFontStyles() ? 1 : 0));
    hash = mix(hash, SkGoodHash()(fParagraphStyle.getLetterSpacingByCSSSpec() ? 1 : 0));

    auto& strutStyle = fParagraphStyle.getStrutStyle();
    if (strutStyle.getStrutEnabled()) {
        hash = mix(hash, SkGoodHash()(relax(strutStyle.getHeight())));
        hash = mix(hash, SkGoodHash()(relax(strutStyle.getLeading())));
        hash = mix(hash, SkGoodHash()(relax(strutStyle.getFontSize())));
        hash = mix(hash, SkGoodHash()(strutStyle.getHeightOverride()));
        hash = mix(hash, SkGoodHash()(strutStyle.getFontStyle()));
        hash = mix(hash, SkGoodHash()(strutStyle.getForceStrutHeight()));
        // StrutStyle::operator== compares fHalfLeading, so the hash must include it
        hash = mix(hash, SkGoodHash()(strutStyle.getHalfLeading()));
        for (auto& ff : strutStyle.getFontFamilies()) {
            hash = mix(hash, SkGoodHash()(ff));
        }
    }

    hash = mix(hash, SkGoodHash()(fText));
    return hash;
}

uint32_t ParagraphCache::Cache::KeyHash::operator()(const ParagraphCacheKey& key) const {
    return key.hash();
}

bool ParagraphCacheKey::operator==(const ParagraphCacheKey& other) const {
    if (fText.size() != other.fText.size()) {
        return false;
    }
    if (fPlaceholders.size() != other.fPlaceholders.size()) {
        return false;
    }
    if (fText != other.fText) {
        return false;
    }
    if (fTextStyles.size() != other.fTextStyles.size()) {
        return false;
    }

    // There is no need to compare default paragraph styles - they are included into fTextStyles
    if (fParagraphStyle.getTextDirection() != other.fParagraphStyle.getTextDirection()) {
        return false;
    }

    if (!(fParagraphStyle.getStrutStyle() == other.fParagraphStyle.getStrutStyle())) {
        return false;
    }

    if (!(fParagraphStyle.getReplaceTabCharacters() == other.fParagraphStyle.getReplaceTabCharacters())) {
        return false;
    }

    // The shaper fakes missing bold and italic in the run fonts
    if (fParagraphStyle.fakeMissingFontStyles() != other.fParagraphStyle.fakeMissingFontStyles()) {
        return false;
    }

    // Letter spacing is applied to the cached runs
    if (fParagraphStyle.getLetterSpacingByCSSSpec() !=
        other.fParagraphStyle.getLetterSpacingByCSSSpec()) {
        return false;
    }

    for (int i = 0; i < fTextStyles.size(); ++i) {
        auto& tsa = fTextStyles[i];
        auto& tsb = other.fTextStyles[i];
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
    for (int i = 0; i < fPlaceholders.size(); ++i) {
        auto& tsa = fPlaceholders[i];
        auto& tsb = other.fPlaceholders[i];
        if (tsa.fRange.width() == 0 && tsb.fRange.width() == 0) {
            continue;
        }
        if (!(tsa.fStyle.equals(tsb.fStyle))) {
            return false;
        }
        // The placeholder run takes its font metrics from the surrounding text style
        if (!tsa.fTextStyle.equalsByFonts(tsb.fTextStyle)) {
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
    , fCache(std::make_unique<Cache>())
{ }

ParagraphCache::~ParagraphCache() { }

void ParagraphCache::turnOn(bool value) { fCache->fCacheIsOn = value; }
int ParagraphCache::count() { return fCache->fLRUCacheMap.count(); }

void ParagraphCache::updateTo(ParagraphImpl* paragraph, const Entry* entry) {

    paragraph->fRuns.clear();
    paragraph->fRuns = entry->fValue->fRuns;
    paragraph->fUnresolvedGlyphs = entry->fValue->fUnresolvedGlyphs;
    paragraph->fUnresolvedCodepoints = entry->fValue->fUnresolvedCodepoints;
    paragraph->fClusters = entry->fValue->fClusters;
    paragraph->fClustersIndexFromCodeUnit = entry->fValue->fClustersIndexFromCodeUnit;
    paragraph->fCodeUnitProperties = entry->fValue->fCodeUnitProperties;
    paragraph->fWords = entry->fValue->fWords;
    paragraph->fBidiRegions = entry->fValue->fBidiRegions;
    paragraph->fHasLineBreaks = entry->fValue->fHasLineBreaks;
    paragraph->fHasWhitespacesInside = entry->fValue->fHasWhitespacesInside;
    paragraph->fTrailingSpaces = entry->fValue->fTrailingSpaces;
    paragraph->fFontSwitches = entry->fValue->fFontSwitches;
    for (auto& run : paragraph->fRuns) {
        run.setOwner(paragraph);
    }
    for (auto& cluster : paragraph->fClusters) {
        cluster.setOwner(paragraph);
    }
}

void ParagraphCache::printStatistics() {
    fCache->printStatistics();
}

void ParagraphCache::abandon() {
    this->reset();
}

void ParagraphCache::reset() {
    SkAutoMutexExclusive lock(fParagraphMutex);
#ifdef PARAGRAPH_CACHE_STATS
    fCache->fTotalRequests = 0;
    fCache->fCacheMisses = 0;
    fCache->fHashMisses = 0;
#endif
    fCache->fLRUCacheMap.reset();
    fCache->fLastCachedValue = nullptr;
}

bool ParagraphCache::findParagraph(ParagraphImpl* paragraph) {
    if (!fCache->fCacheIsOn) {
        return false;
    }
#ifdef PARAGRAPH_CACHE_STATS
    ++fCache->fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);
    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fCache->fLRUCacheMap.find(key);

    if (!entry) {
        // We have a cache miss
#ifdef PARAGRAPH_CACHE_STATS
        ++fCache->fCacheMisses;
#endif
        fChecker(paragraph, "missingParagraph", true);
        return false;
    }
    updateTo(paragraph, entry->get());
    fChecker(paragraph, "foundParagraph", true);
    return true;
}

bool ParagraphCache::updateParagraph(ParagraphImpl* paragraph) {
    if (!fCache->fCacheIsOn) {
        return false;
    }
#ifdef PARAGRAPH_CACHE_STATS
    ++fCache->fTotalRequests;
#endif
    SkAutoMutexExclusive lock(fParagraphMutex);

    ParagraphCacheKey key(paragraph);
    std::unique_ptr<Entry>* entry = fCache->fLRUCacheMap.find(key);
    if (!entry) {
        // isTooMuchMemoryWasted(paragraph) not needed for now
        if (isPossiblyTextEditing(paragraph)) {
            // Skip this paragraph
            return false;
        }
        ParagraphCacheValue* value = new ParagraphCacheValue(std::move(key), paragraph);
        fCache->fLRUCacheMap.insert(value->fKey, std::make_unique<Entry>(value));
        fChecker(paragraph, "addedParagraph", true);
        fCache->fLastCachedValue = value;
        return true;
    } else {
        // We do not have to update the paragraph
        return false;
    }
}

// Special situation: (very) long paragraph that is close to the last formatted paragraph
#define NOCACHE_PREFIX_LENGTH 40
bool ParagraphCache::isPossiblyTextEditing(ParagraphImpl* paragraph) {
    if (fCache->fLastCachedValue == nullptr) {
        return false;
    }

    auto& lastText = fCache->fLastCachedValue->fKey.text();
    auto& text = paragraph->fText;

    if ((lastText.size() < NOCACHE_PREFIX_LENGTH) || (text.size() < NOCACHE_PREFIX_LENGTH)) {
        // Either last text or the current are too short
        return false;
    }

    if (std::strncmp(lastText.c_str(), text.c_str(), NOCACHE_PREFIX_LENGTH) == 0) {
        // Texts have the same starts
        return true;
    }

    if (std::strncmp(lastText.c_str() + lastText.size() - NOCACHE_PREFIX_LENGTH, &text[text.size() - NOCACHE_PREFIX_LENGTH], NOCACHE_PREFIX_LENGTH) == 0) {
        // Texts have the same ends
        return true;
    }

    // It does not look like editing the text
    return false;
}
}  // namespace textlayout
}  // namespace skia
