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
    virtual size_t getFontManagersCount() const = 0;
    virtual void setAssetFontManager(sk_sp<SkFontMgr> fontManager) = 0;
    virtual void setDynamicFontManager(sk_sp<SkFontMgr> fontManager) = 0;
    virtual void setTestFontManager(sk_sp<SkFontMgr> fontManager) = 0;
    virtual void setDefaultFontManager(sk_sp<SkFontMgr> fontManager,
                                       const char defaultFamilyName[]) = 0;
    virtual std::vector<sk_sp<SkFontMgr>> getFontManagerOrder() const = 0;
    virtual void disableFontFallback() = 0;
    virtual bool fontFallbackEnabled() = 0;

    virtual void resolveFonts(const TextStyle& style, SkSpan<const char> text) = 0;
    virtual std::pair<SkFont, SkScalar>* findFontForCodepoint(const char* ch, bool first) = 0;

    virtual sk_sp<SkTypeface> matchTypeface(const char familyName[], SkFontStyle fontStyle) = 0;
    virtual sk_sp<SkTypeface> defaultFallback(SkUnichar unicode, SkFontStyle fontStyle) = 0;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontCollection_DEFINED
