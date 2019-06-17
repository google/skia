// Copyright 2019 Google LLC.
#ifndef FontCollection_DEFINED
#define FontCollection_DEFINED

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

    ~FontCollection() = default;

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

private:
    std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const;

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
