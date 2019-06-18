// Copyright 2019 Google LLC.
#ifndef FontCollection_DEFINED
#define FontCollection_DEFINED

#include <src/core/SkSpan.h>
#include <memory>
#include <set>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/TextStyle.h"

namespace skia {
namespace textlayout {

class FontCollection : public SkRefCnt {
public:
    FontCollection();

    ~FontCollection();

    size_t getFontManagersCount() const;

    void setAssetFontManager(sk_sp<SkFontMgr> fontManager);
    void setDynamicFontManager(sk_sp<SkFontMgr> fontManager);
    void setTestFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const char defaultFamilyName[]);

    sk_sp<SkFontMgr> geFallbackManager() const { return fDefaultFontManager; }

    sk_sp<SkTypeface> matchTypeface(const char familyName[], SkFontStyle fontStyle);
    sk_sp<SkTypeface> matchDefaultTypeface(SkFontStyle fontStyle);
    sk_sp<SkTypeface> defaultFallback(SkUnichar unicode, SkFontStyle fontStyle);

    void disableFontFallback();
    bool fontFallbackEnabled() { return fEnableFontFallback; }

    void findAllFontsForStyledBlock(const TextStyle& style, SkSpan<const char> text);
    bool findFirst(const char* codepoint, SkFont* font, SkScalar* height);
    bool findNext(const char* codepoint, SkFont* font, SkScalar* height);
    void resetFontResolution();

private:
    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const;
    std::pair<SkFont, SkScalar> makeFont(sk_sp<SkTypeface> typeface, SkScalar size,
                                         SkScalar height);

    size_t resolveAllCharactersByFont(std::pair<SkFont, SkScalar> font);
    void addResolvedWhitespacesToMapping();

    struct FamilyKey {
        FamilyKey(const char family[], const char loc[], SkFontStyle style)
                : fFontFamily(family), fLocale(loc), fFontStyle(style) {}

        FamilyKey() {}

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

    SkUnichar firstUnresolved();

    bool fEnableFontFallback;
    SkTHashMap<FamilyKey, sk_sp<SkTypeface>, FamilyKey::Hasher> fTypefaces;
    sk_sp<SkFontMgr> fDefaultFontManager;
    sk_sp<SkFontMgr> fAssetFontManager;
    sk_sp<SkFontMgr> fDynamicFontManager;
    sk_sp<SkFontMgr> fTestFontManager;
    SkString fDefaultFamilyName;

    SkTHashMap<const char*, std::pair<SkFont, SkScalar>> fFontMapping;
    SkTHashSet<std::pair<SkFont, SkScalar>, Hash> fResolvedFonts;
    bool fHintingOn;
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

#endif  // FontCollection_DEFINED
