// Copyright 2019 Google LLC.
#ifndef FontResolver_DEFINED
#define FontResolver_DEFINED

#include <memory>
#include <set>
#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/FontCollection.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/TextLine.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

struct FontDescr {
    FontDescr() {}
    FontDescr(SkFont font, SkScalar height)
            : fFont(font), fHeight(height), fStart(EMPTY_INDEX) {}
    bool operator==(const FontDescr& a) const {
        return this->fFont == a.fFont && this->fHeight == a.fHeight;
    }
    SkFont fFont;
    SkScalar fHeight;
    TextIndex fStart;
};

class FontResolver {
public:

    FontResolver() = default;
    ~FontResolver() = default;

    void findAllFontsForAllStyledBlocks(ParagraphImpl* master);
    bool findNext(const char* codepoint, SkFont* font, SkScalar* height);

    const SkTArray<FontDescr>& switches() const { return fFontSwitches; }

private:
    void findAllFontsForStyledBlock(const TextStyle& style, TextRange textRange);
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
    TextRange fTextRange;
    SkSpan<Block> fStyles;

    SkTArray<FontDescr> fFontSwitches;
    FontDescr* fFontIterator;
    SkTHashSet<FontDescr, Hash> fResolvedFonts;
    FontDescr fFirstResolvedFont;

    SkTHashMap<TextIndex, FontDescr> fFontMapping;
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
