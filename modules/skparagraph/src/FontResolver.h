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
    FontResolver(sk_sp<FontCollection> fontCollection);
    ~FontResolver() = default;

    void findAllFontsForStyledBlock(const TextStyle& style, SkSpan<const char> text);
    bool findFirst(const char* codepoint, SkFont* font, SkScalar* height);
    bool findNext(const char* codepoint, SkFont* font, SkScalar* height);

private:
    std::pair<SkFont, SkScalar> makeFont(sk_sp<SkTypeface> typeface, SkScalar size,
                                         SkScalar height);

    size_t resolveAllCharactersByFont(std::pair<SkFont, SkScalar> font);
    void addResolvedWhitespacesToMapping();

    struct Hash {
        uint32_t operator()(const std::pair<SkFont, SkScalar>& key) const {
            return SkTypeface::UniqueID(key.first.getTypeface()) +
                   SkScalarCeilToInt(key.first.getSize()) + SkScalarCeilToInt(key.second);
        }
    };

    SkUnichar firstUnresolved();

    sk_sp<FontCollection> fFontCollection;

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

#endif  // FontResolver_DEFINED
