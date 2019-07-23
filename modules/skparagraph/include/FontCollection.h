// Copyright 2019 Google LLC.
#ifndef FontCollection_DEFINED
#define FontCollection_DEFINED

#include <memory>
#include <set>
#include <vector>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"

// TODO: Make it external so the other platforms (Android) could use it
#define DEFAULT_FONT_FAMILY "sans-serif"

namespace skia {
namespace textlayout {

class TextStyle;
class FontCollection : public SkRefCnt {
public:
    FontCollection();

    ~FontCollection() = default;

    size_t getFontManagersCount() const;

    void setAssetFontManager(sk_sp<SkFontMgr> fontManager);
    void setDynamicFontManager(sk_sp<SkFontMgr> fontManager);
    void setTestFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager);
    void setDefaultFontManager(sk_sp<SkFontMgr> fontManager, const char defaultFamilyName[]);

    sk_sp<SkFontMgr> geFallbackManager() const { return fDefaultFontManager; }

    sk_sp<SkTypeface> matchTypeface(const SkString& familyName, SkFontStyle fontStyle);
    sk_sp<SkTypeface> matchDefaultTypeface(SkFontStyle fontStyle);
    sk_sp<SkTypeface> defaultFallback(SkUnichar unicode, SkFontStyle fontStyle, const SkString& locale);

    void disableFontFallback();
    bool fontFallbackEnabled() { return fEnableFontFallback; }

private:

    friend TextStyle;

    static const SkString& getFontFamilyName(size_t index);
    static bool getFontFamilyIndex(const SkString& name, size_t* index);
    static size_t addFontFamily(const SkString& familyName);
    static size_t addLocale(const SkString& locale);

    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const;

    typedef size_t FamilyNameIndex;
    typedef size_t LocaleIndex;
    struct FamilyKey {
        FamilyKey(const SkString& family, const char* locale, SkFontStyle style)
                : fFontStyle(style) {
            fFontFamily = addFontFamily(family);
            fLocale = addLocale(SkString(locale));
        }

        FamilyKey() {}

        FamilyNameIndex fFontFamily;
        LocaleIndex fLocale;
        SkFontStyle fFontStyle;

        bool operator==(const FamilyKey& other) const;

        struct Hasher {
            size_t operator()(const FamilyKey& key) const;
        };
    };

    bool fEnableFontFallback;
    SkTHashMap<FamilyKey, sk_sp<SkTypeface>, FamilyKey::Hasher> fTypefaces;
    sk_sp<SkFontMgr> fDefaultFontManager;
    sk_sp<SkFontMgr> fAssetFontManager;
    sk_sp<SkFontMgr> fDynamicFontManager;
    sk_sp<SkFontMgr> fTestFontManager;
    SkString fDefaultFamilyName;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontCollection_DEFINED
