// Copyright 2019 Google LLC.
#ifndef FontResolver_DEFINED
#define FontResolver_DEFINED

#include <memory>
#include <set>
#include "modules/skparagraph/src/TextLine.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

class FontResolver {
public:
    struct FontDescr {
        FontDescr() {}
        FontDescr(SkFont font, SkScalar height)
                : fFont(font), fHeight(height) {}
        bool operator==(const FontDescr& a) const {
            return this->fFont == a.fFont && this->fHeight == a.fHeight;
        }
        SkFont fFont;
        SkScalar fHeight;
    };

    FontResolver() = default;
    ~FontResolver() = default;

    void findAllFontsForAllStyledBlocks(SkSpan<const char> utf8,
                                        SkSpan<TextBlock> styles,
                                        sk_sp<FontCollection> fontCollection);
    bool findFirst(const char* codepoint, SkFont* font, SkScalar* height);
    bool findNext(const char* codepoint, SkFont* font, SkScalar* height);

private:
    void findAllFontsForStyledBlock(const TextStyle& style, SkSpan<const char> text);
    FontDescr makeFont(sk_sp<SkTypeface> typeface, SkScalar size, SkScalar height);
    size_t resolveAllCharactersByFont(const FontDescr& fontDescr);
    void addResolvedWhitespacesToMapping();

    struct Hash {
        uint32_t operator()(const FontDescr& key) const {
            return SkTypeface::UniqueID(key.fFont.getTypeface()) +
                   SkScalarCeilToInt(key.fFont.getSize()) +
                   SkScalarCeilToInt(key.fHeight);
        }
    };

    SkUnichar firstUnresolved();

    sk_sp<FontCollection> fFontCollection;
    SkSpan<const char> fText;
    SkSpan<TextBlock> fStyles;

    SkTHashMap<const char*, FontDescr> fFontMapping;
    SkTHashSet<FontDescr, Hash> fResolvedFonts;
    FontDescr fFirstResolvedFont;

    SkTArray<SkUnichar> fCodepoints;
    SkTArray<const char*> fCharacters;
    SkTArray<size_t> fUnresolvedIndexes;
    SkTArray<SkUnichar> fUnresolvedCodepoints;
    SkTHashMap<size_t, FontDescr> fWhitespaces;
    size_t fUnresolved;
};
}  // namespace textlayout
}  // namespace skia

#endif  // FontResolver_DEFINED
