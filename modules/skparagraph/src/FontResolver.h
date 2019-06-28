// Copyright 2019 Google LLC.
#ifndef FontResolver_DEFINED
#define FontResolver_DEFINED

#include "src/core/SkSpan.h"
#include <memory>
#include <set>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"

namespace skia {
namespace textlayout {

class FontResolver {
public:
    FontResolver(sk_sp<FontCollection> fontCollection, SkSpan<const char> fullText);
    ~FontResolver() = default;

    void findAllFontsForStyledBlock(const TextStyle& style, SkSpan<const char> text);
    bool findFirst(const char* codepoint, SkFont* font, SkScalar* height);
    bool findNext(const char* codepoint, SkFont* font, SkScalar* height);

private:
    std::pair<SkFont, SkScalar> makeFont(sk_sp<SkTypeface> typeface, SkScalar size,
                                         SkScalar height);

    struct Hash {
        uint32_t operator()(const std::pair<SkFont, SkScalar>& key) const {
            return SkTypeface::UniqueID(key.first.getTypeface()) +
                   SkScalarCeilToInt(key.first.getSize()) + SkScalarCeilToInt(key.second);
        }
    };

    sk_sp<FontCollection> fFontCollection;
    SkSpan<const char> fFullText;

    SkTHashMap<const char*, std::pair<SkFont, SkScalar>> fFontMapping;
    SkTHashSet<std::pair<SkFont, SkScalar>, Hash> fResolvedFonts;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontResolver_DEFINED
