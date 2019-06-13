// Copyright 2019 Google LLC.
#ifndef FontCollectionImpl_DEFINED
#define FontCollectionImpl_DEFINED

#include <memory>
#include <set>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"

namespace skia {
namespace textlayout {

class FontCollectionImpl : public FontCollection {
public:
    FontCollectionImpl();

    ~FontCollectionImpl() override = default;

    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const override;
    size_t getFontManagersCount() const override { return getFontManagerOrder().size(); }

    void setAssetFontManager(sk_sp<SkFontMgr> fontManager) override {
        fAssetFontManager = fontManager;
    }
    void setDynamicFontManager(sk_sp<SkFontMgr> fontManager) override {
        fDynamicFontManager = fontManager;
    }
    void setTestFontManager(sk_sp<SkFontMgr> fontManager) override {
        fTestFontManager = fontManager;
    }
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                               const char defaultFamilyName[]) override {
        fDefaultFamilyName = defaultFamilyName;
        fDefaultFontManager = fontManager;
    }

    void disableFontFallback() override { fEnableFontFallback = false; }
    bool fontFallbackEnabled() override { return fEnableFontFallback; }

    sk_sp<SkTypeface> matchTypeface(const char familyName[], SkFontStyle fontStyle) override;
    sk_sp<SkTypeface> defaultFallback(SkUnichar unicode, SkFontStyle fontStyle) override;

    void resolveFonts(const TextStyle& style, SkSpan<const char> text) override;
    std::pair<SkFont, SkScalar>* findFontForCodepoint(const char* ch, bool first) override;

private:
    std::pair<SkFont, SkScalar> makeFont(sk_sp<SkTypeface> typeface, SkScalar size,
                                         SkScalar height);

    size_t resolveAllCharactersByFont(std::pair<SkFont, SkScalar> font);
    void addResolvedWhitespacesToMapping();
    SkUnichar firstUnresolved() {
        if (fUnresolved == 0) return 0;

        bool firstTry = fUnresolved == fCodepoints.size();
        auto index = firstTry ? 0 : fUnresolvedIndexes[0];
        return fCodepoints[index];
    }

    struct FamilyKey {
        FamilyKey(const char family[], const char loc[], SkFontStyle style)
                : fFontFamily(family), fLocale(loc), fFontStyle(style) {}

        FamilyKey() = default;

        SkString fFontFamily;
        SkString fLocale;
        SkFontStyle fFontStyle;

        bool operator==(const FamilyKey& other) const;

        struct Hasher {
            size_t operator()(const FamilyKey& key) const;
        };
    };

    struct Hash {
        uint32_t operator()(const std::pair<SkFont, SkScalar>& key) const {
            return SkTypeface::UniqueID(key.first.getTypeface()) +
                   SkScalarCeilToInt(key.first.getSize()) + SkScalarCeilToInt(key.second);
        }
    };

    bool fEnableFontFallback;
    SkTHashMap<FamilyKey, sk_sp<SkTypeface>, FamilyKey::Hasher> fTypefaces;
    sk_sp<SkFontMgr> fDefaultFontManager;
    sk_sp<SkFontMgr> fAssetFontManager;
    sk_sp<SkFontMgr> fDynamicFontManager;
    sk_sp<SkFontMgr> fTestFontManager;
    SkString fDefaultFamilyName;

    bool fHintingOn;

    SkTHashMap<const char*, std::pair<SkFont, SkScalar>> fFontMapping;
    SkTHashSet<std::pair<SkFont, SkScalar>, Hash> fResolvedFonts;
    std::pair<SkFont, SkScalar> fFirstResolvedFont;

    SkTArray<SkUnichar> fCodepoints;
    SkTArray<const char*> fCharacters;
    SkTArray<size_t> fUnresolvedIndexes;
    SkTArray<SkUnichar> fUnresolvedCodepoints;
    SkTHashMap<size_t, std::pair<SkFont, SkScalar>> fWhitespaces;
    size_t fUnresolved;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontCollectionImpl_DEFINED
