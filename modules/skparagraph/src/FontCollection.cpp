// Copyright 2019 Google LLC
#include "modules/skparagraph/include/FontCollection.h"

#include "include/core/SkTypeface.h"
#include "include/private/SkMutex.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skshaper/include/SkShaper_harfbuzz.h"
#include "src/core/SkLRUCache.h"
#include "src/core/SkTHash.h"

namespace {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    const char* kColorEmojiFontMac = "Apple Color Emoji";
#else
    const char* kColorEmojiLocale = "und-Zsye";
#endif
}
namespace skia {
namespace textlayout {

struct FontCollection::FaceCache {
    struct FamilyKey {
        FamilyKey(const std::vector<SkString>& familyNames, SkFontStyle style, const std::optional<FontArguments>& args)
                : fFamilyNames(familyNames), fFontStyle(style), fFontArguments(args) {}

        FamilyKey() {}

        std::vector<SkString> fFamilyNames;
        SkFontStyle fFontStyle;
        std::optional<FontArguments> fFontArguments;

        bool operator==(const FamilyKey& other) const {
            return fFamilyNames == other.fFamilyNames &&
                   fFontStyle == other.fFontStyle &&
                   fFontArguments == other.fFontArguments;
        }

        struct Hasher {
            size_t operator()(const FamilyKey& key) const {
                size_t hash = 0;
                for (const SkString& family : key.fFamilyNames) {
                    hash ^= std::hash<std::string>()(family.c_str());
                }
                return hash ^
                       std::hash<uint32_t>()(key.fFontStyle.weight()) ^
                       std::hash<uint32_t>()(key.fFontStyle.slant()) ^
                       std::hash<std::optional<FontArguments>>()(key.fFontArguments);
            }
        };
    };
    // Synchronize fTypefaces inside the pimpl struct to preserve FontCollection's ABI layout
    mutable SkMutex fMutex;
    skia_private::THashMap<FamilyKey, std::vector<sk_sp<SkTypeface>>, FamilyKey::Hasher> fTypefaces
            SK_GUARDED_BY(fMutex);
};

struct FontCollection::VariationCache {
    struct Key {
        Key(SkTypefaceID typefaceID, const FontArguments& args)
                : fTypefaceID(typefaceID), fFontArguments(args) {}

        Key() : fFontArguments(SkFontArguments()) {}

        SkTypefaceID fTypefaceID;
        FontArguments fFontArguments;

        bool operator==(const Key& other) const {
            return fTypefaceID == other.fTypefaceID &&
                   fFontArguments == other.fFontArguments;
        }

        struct Hasher {
            size_t operator()(const Key& key) const {
                return std::hash<SkTypefaceID>()(key.fTypefaceID) ^
                       std::hash<FontArguments>()(key.fFontArguments);
            }
        };
    };
    // Synchronize fTypefaces inside the pimpl struct to preserve FontCollection's ABI layout
    mutable SkMutex fMutex;
    skia_private::THashMap<Key, sk_sp<SkTypeface>, Key::Hasher> fTypefaces SK_GUARDED_BY(fMutex);
};

// System fallback results for a codepoint (or an emoji start), shared by all paragraphs.
// Each miss is a full system query (CoreText, fontconfig), so misses are cached too.
struct FontCollection::FallbackCache {
    // Keeps the hit rate for chat-like text with 2000 different ideographs and emoji.
    // An entry uses about 100 bytes, so a full cache uses about 400 KB.
    static constexpr int kMaxEntries = 4096;

    struct Key {
        Key(SkUnichar unicode,
            bool emoji,
            const std::vector<SkString>& families,
            SkFontStyle style,
            const SkString& locale)
                : fUnicode(unicode), fEmoji(emoji), fFontStyle(style), fLocale(locale) {
            if (!families.empty()) {
                fFamily = families[0];
            }
        }

        SkUnichar fUnicode;
        bool fEmoji;
        // Only the first family is passed to the font manager as a hint
        std::optional<SkString> fFamily;
        SkFontStyle fFontStyle;
        SkString fLocale;

        bool operator==(const Key&) const = default;

        struct Hasher {
            uint32_t operator()(const Key& key) const {
                return SkGoodHash()(key.fUnicode) ^ SkGoodHash()(key.fEmoji) ^
                       (key.fFamily ? SkGoodHash()(*key.fFamily) : 0) ^
                       SkGoodHash()(key.fFontStyle) ^ SkGoodHash()(key.fLocale);
            }
        };
    };

    // Return a copy, because another thread can remove the entry after the unlock
    std::optional<sk_sp<SkTypeface>> find(const Key& key) {
        // find() also moves the entry to the front of the LRU list, so a hit also needs the lock
        SkAutoMutexExclusive lock(fMutex);
        if (auto found = fTypefaces.find(key)) {
            return *found;
        }
        return std::nullopt;
    }

    // Keep the first result if another thread inserted the key while unlocked
    sk_sp<SkTypeface> insert(Key key, sk_sp<SkTypeface> typeface) {
        SkAutoMutexExclusive lock(fMutex);
        if (auto found = fTypefaces.find(key)) {
            return *found;
        }
        return *fTypefaces.insert(std::move(key), std::move(typeface));
    }

    void reset() {
        SkAutoMutexExclusive lock(fMutex);
        fTypefaces.reset();
    }

private:
    // Leaf lock: callers query the font managers and clone typefaces without it
    SkMutex fMutex;
    SkLRUCache<Key, sk_sp<SkTypeface>, Key::Hasher> fTypefaces SK_GUARDED_BY(fMutex){kMaxEntries};
};

FontCollection::FontCollection()
        : fFaceCache(std::make_unique<FaceCache>())
        , fVariationCache(std::make_unique<VariationCache>())
        , fFallbackCache(std::make_unique<FallbackCache>())
        , fEnableFontFallback(true)
        , fDefaultFamilyNames({SkString(DEFAULT_FONT_FAMILY)}) { }

FontCollection::~FontCollection() {}

size_t FontCollection::getFontManagersCount() const { return this->getFontManagerOrder().size(); }

// Cached typefaces, fallback results and shaped paragraphs were resolved against the previous
// managers, default families and fallback setting; the variation cache is keyed by typeface
// and stays valid
void FontCollection::resetResolutionCaches() {
    fParagraphCache.reset();
    {
        SkAutoMutexExclusive lock(fFaceCache->fMutex);
        fFaceCache->fTypefaces.reset();
    }
    fFallbackCache->reset();
}

void FontCollection::setAssetFontManager(sk_sp<SkFontMgr> font_manager) {
    if (fAssetFontManager != font_manager) {
        fAssetFontManager = std::move(font_manager);
        this->resetResolutionCaches();
    }
}

void FontCollection::setDynamicFontManager(sk_sp<SkFontMgr> font_manager) {
    if (fDynamicFontManager != font_manager) {
        fDynamicFontManager = std::move(font_manager);
        this->resetResolutionCaches();
    }
}

void FontCollection::setTestFontManager(sk_sp<SkFontMgr> font_manager) {
    if (fTestFontManager != font_manager) {
        fTestFontManager = std::move(font_manager);
        this->resetResolutionCaches();
    }
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                                           const char defaultFamilyName[]) {
    fDefaultFontManager = std::move(fontManager);
    fDefaultFamilyNames.emplace_back(defaultFamilyName);
    // Every call appends a default family, so the resolution order always changes
    this->resetResolutionCaches();
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                                           const std::vector<SkString>& defaultFamilyNames) {
    if (fDefaultFontManager != fontManager || fDefaultFamilyNames != defaultFamilyNames) {
        fDefaultFontManager = std::move(fontManager);
        fDefaultFamilyNames = defaultFamilyNames;
        this->resetResolutionCaches();
    }
}

void FontCollection::setDefaultFontManager(sk_sp<SkFontMgr> fontManager) {
    if (fDefaultFontManager != fontManager) {
        fDefaultFontManager = std::move(fontManager);
        this->resetResolutionCaches();
    }
}

// Return the available font managers in the order they should be queried.
std::vector<sk_sp<SkFontMgr>> FontCollection::getFontManagerOrder() const {
    std::vector<sk_sp<SkFontMgr>> order;
    if (fDynamicFontManager) {
        order.push_back(fDynamicFontManager);
    }
    if (fAssetFontManager) {
        order.push_back(fAssetFontManager);
    }
    if (fTestFontManager) {
        order.push_back(fTestFontManager);
    }
    if (fDefaultFontManager && fEnableFontFallback) {
        order.push_back(fDefaultFontManager);
    }
    return order;
}

std::vector<sk_sp<SkTypeface>> FontCollection::findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle) {
    return findTypefaces(familyNames, fontStyle, std::nullopt);
}

std::vector<sk_sp<SkTypeface>> FontCollection::findTypefaces(const std::vector<SkString>& familyNames, SkFontStyle fontStyle, const std::optional<FontArguments>& fontArgs) {
    // Look inside the font collections cache first
    FaceCache::FamilyKey familyKey(familyNames, fontStyle, fontArgs);
    {
        // Release fFaceCache->fMutex before matchTypeface and cloneTypeface so font manager
        // queries do not block cache hits and cloneTypeface can lock fVariationCache safely
        SkAutoMutexExclusive lock(fFaceCache->fMutex);
        auto found = fFaceCache->fTypefaces.find(familyKey);
        if (found) {
            return *found;
        }
    }

    std::vector<sk_sp<SkTypeface>> typefaces;
    for (const SkString& familyName : familyNames) {
        sk_sp<SkTypeface> match = matchTypeface(familyName, fontStyle);
        if (match && fontArgs) {
            match = cloneTypeface(match, fontArgs.value());
        }
        if (match) {
            typefaces.emplace_back(std::move(match));
        }
    }

    if (typefaces.empty()) {
        sk_sp<SkTypeface> match;
        for (const SkString& familyName : fDefaultFamilyNames) {
            match = matchTypeface(familyName, fontStyle);
            if (match) {
                break;
            }
        }
        if (!match) {
            for (const auto& manager : this->getFontManagerOrder()) {
                match = manager->legacyMakeTypeface(nullptr, fontStyle);
                if (match) {
                    break;
                }
            }
        }
        if (match) {
            if (fontArgs) {
                match = cloneTypeface(match, fontArgs.value());
            }
            typefaces.emplace_back(std::move(match));
        }
    }

    // Reuse the cached vector if another thread populated familyKey while unlocked
    SkAutoMutexExclusive lock(fFaceCache->fMutex);
    if (auto* found = fFaceCache->fTypefaces.find(familyKey)) {
        return *found;
    }
    fFaceCache->fTypefaces.set(familyKey, typefaces);
    return typefaces;
}

sk_sp<SkTypeface> FontCollection::matchTypeface(const SkString& familyName, SkFontStyle fontStyle) {
    for (const auto& manager : this->getFontManagerOrder()) {
        sk_sp<SkFontStyleSet> set(manager->matchFamily(familyName.c_str()));
        if (!set || set->count() == 0) {
            continue;
        }

        sk_sp<SkTypeface> match(set->matchStyle(fontStyle));
        if (match) {
            return match;
        }
    }

    return nullptr;
}

// Find ANY font in available font managers that resolves the unicode codepoint
sk_sp<SkTypeface> FontCollection::defaultFallback(SkUnichar unicode,
                                                  const std::vector<SkString>& families,
                                                  SkFontStyle fontStyle,
                                                  const SkString& locale,
                                                  const std::optional<FontArguments>& fontArgs) {
    FallbackCache::Key key(unicode, false, families, fontStyle, locale);
    sk_sp<SkTypeface> typeface;
    if (auto cached = fFallbackCache->find(key)) {
        typeface = std::move(*cached);
    } else {
        for (const auto& manager : this->getFontManagerOrder()) {
            std::vector<const char*> bcp47;
            if (!locale.isEmpty()) {
                bcp47.push_back(locale.c_str());
            }
            const char* familyName = families.empty() ? nullptr : families[0].c_str();
            typeface = manager->matchFamilyStyleCharacter(
                    familyName, fontStyle, bcp47.data(), bcp47.size(), unicode);
            if (typeface != nullptr) {
                break;
            }
        }
        typeface = fFallbackCache->insert(std::move(key), std::move(typeface));
    }

    if (typeface && fontArgs) {
        typeface = cloneTypeface(typeface, fontArgs.value());
    }
    return typeface;
}

// Find ANY font in available font managers that resolves this emojiStart
sk_sp<SkTypeface> FontCollection::defaultEmojiFallback(SkUnichar emojiStart,
                                                       SkFontStyle fontStyle,
                                                       const SkString& locale) {
#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
    // Apple Color Emoji is matched by family name independent of codepoint, style, or locale
    FallbackCache::Key macKey(-1, true, {}, SkFontStyle(), SkString());
    sk_sp<SkTypeface> emojiTypeface;
    if (auto cached = fFallbackCache->find(macKey)) {
        emojiTypeface = *cached;
    } else {
        // Query active font managers in priority order rather than assuming fDefaultFontManager
        for (const auto& manager : this->getFontManagerOrder()) {
            if ((emojiTypeface = manager->matchFamilyStyle(kColorEmojiFontMac, SkFontStyle()))) {
                break;
            }
        }
        emojiTypeface = fFallbackCache->insert(std::move(macKey), std::move(emojiTypeface));
    }
    // Fall back to per-codepoint character matching only when Apple Color Emoji is unavailable
    if (emojiTypeface != nullptr) {
        return emojiTypeface;
    }
#endif

    FallbackCache::Key key(emojiStart, true, {}, fontStyle, locale);
    if (auto cached = fFallbackCache->find(key)) {
        return *cached;
    }

    sk_sp<SkTypeface> typeface;
    for (const auto& manager : this->getFontManagerOrder()) {
        std::vector<const char*> bcp47;
#if !defined(SK_BUILD_FOR_MAC) && !defined(SK_BUILD_FOR_IOS)
        // Non-Apple platforms select color emoji fonts via the und-Zsye BCP47 locale tag
        bcp47.push_back(kColorEmojiLocale);
#endif
        if (!locale.isEmpty()) {
            bcp47.push_back(locale.c_str());
        }

        // Not really ideal since the first codepoint may not be the best one
        // but we start from a good colored emoji at least
        typeface = manager->matchFamilyStyleCharacter(
                nullptr, fontStyle, bcp47.data(), bcp47.size(), emojiStart);
        if (typeface != nullptr) {
            // ... and stop as soon as we find something in hope it will work for all of them
            break;
        }
    }
    return fFallbackCache->insert(std::move(key), std::move(typeface));
}

sk_sp<SkTypeface> FontCollection::defaultFallback() {
    if (fDefaultFontManager == nullptr) {
        return nullptr;
    }
    for (const SkString& familyName : fDefaultFamilyNames) {
        sk_sp<SkTypeface> match = fDefaultFontManager->matchFamilyStyle(familyName.c_str(),
                                                                        SkFontStyle());
        if (match) {
            return match;
        }
    }
    return nullptr;
}

sk_sp<SkTypeface> FontCollection::cloneTypeface(const sk_sp<SkTypeface>& typeface,
                                                const FontArguments& args) {
    VariationCache::Key variationKey(typeface->uniqueID(), args);
    {
        // Clone outside the lock so typeface variation instantiation does not block cache hits
        SkAutoMutexExclusive lock(fVariationCache->fMutex);
        auto found = fVariationCache->fTypefaces.find(variationKey);
        if (found) {
            return *found;
        }
    }
    sk_sp<SkTypeface> clone = args.CloneTypeface(typeface);
    // Return the cached clone if a concurrent thread inserted variationKey first so callers
    // observe a single canonical SkTypefaceID
    SkAutoMutexExclusive lock(fVariationCache->fMutex);
    if (auto* found = fVariationCache->fTypefaces.find(variationKey)) {
        return *found;
    }
    fVariationCache->fTypefaces.set(variationKey, clone);
    return clone;
}

void FontCollection::disableFontFallback() {
    if (fEnableFontFallback) {
        fEnableFontFallback = false;
        this->resetResolutionCaches();
    }
}

void FontCollection::enableFontFallback() {
    if (!fEnableFontFallback) {
        fEnableFontFallback = true;
        this->resetResolutionCaches();
    }
}

void FontCollection::clearCaches() {
    fParagraphCache.reset();
    {
        // Acquire each cache mutex independently to match the lock order in findTypefaces
        SkAutoMutexExclusive lock(fFaceCache->fMutex);
        fFaceCache->fTypefaces.reset();
    }
    {
        // Reset variation clones after face cache entries that reference them
        SkAutoMutexExclusive lock(fVariationCache->fMutex);
        fVariationCache->fTypefaces.reset();
    }
    fFallbackCache->reset();
    SkShapers::HB::PurgeCaches();
}

}  // namespace textlayout
}  // namespace skia
