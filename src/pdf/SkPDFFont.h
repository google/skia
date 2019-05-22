/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPDFFont_DEFINED
#define SkPDFFont_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkAdvancedTypefaceMetrics.h"
#include "src/core/SkStrikeCache.h"
#include "src/pdf/SkPDFGlyphUse.h"
#include "src/pdf/SkPDFTypes.h"

#include <vector>

class SkPDFDocument;
class SkStrike;
class SkString;

/** \class SkPDFFont
    A PDF Object class representing a font.  The font may have resources
    attached to it in order to embed the font.  SkPDFFonts are canonicalized
    so that resource deduplication will only include one copy of a font.
    This class uses the same pattern as SkPDFGraphicState, a static weak
    reference to each instantiated class.
*/
class SkPDFFont {
public:
    SkPDFFont() {}
    ~SkPDFFont();
    SkPDFFont(SkPDFFont&&);
    SkPDFFont& operator=(SkPDFFont&&);

    /** Returns the typeface represented by this class. Returns nullptr for the
     *  default typeface.
     */
    SkTypeface* typeface() const { return fTypeface.get(); }

    /** Returns the font type represented in this font.  For Type0 fonts,
     *  returns the type of the descendant font.
     */
    SkAdvancedTypefaceMetrics::FontType getType() const { return fFontType; }

    static SkAdvancedTypefaceMetrics::FontType FontType(const SkAdvancedTypefaceMetrics&);
    static void GetType1GlyphNames(const SkTypeface&, SkString*);

    static bool IsMultiByte(SkAdvancedTypefaceMetrics::FontType type) {
        return type == SkAdvancedTypefaceMetrics::kType1CID_Font ||
               type == SkAdvancedTypefaceMetrics::kTrueType_Font;
    }

    /** Returns true if this font encoding supports glyph IDs above 255.
     */
    bool multiByteGlyphs() const { return SkPDFFont::IsMultiByte(this->getType()); }

    /** Return true if this font has an encoding for the passed glyph id.
     */
    bool hasGlyph(SkGlyphID gid) {
        return (gid >= this->firstGlyphID() && gid <= this->lastGlyphID()) || gid == 0;
    }

    /** Convert the input glyph ID into the font encoding.  */
    SkGlyphID glyphToPDFFontEncoding(SkGlyphID gid) const {
        if (this->multiByteGlyphs() || gid == 0) {
            return gid;
        }
        SkASSERT(gid >= this->firstGlyphID() && gid <= this->lastGlyphID());
        SkASSERT(this->firstGlyphID() > 0);
        return gid - this->firstGlyphID() + 1;
    }

    void noteGlyphUsage(SkGlyphID glyph) {
        SkASSERT(this->hasGlyph(glyph));
        fGlyphUsage.set(glyph);
    }

    SkPDFIndirectReference indirectReference() const { return fIndirectReference; }

    /** Get the font resource for the passed typeface and glyphID. The
     *  reference count of the object is incremented and it is the caller's
     *  responsibility to unreference it when done.  This is needed to
     *  accommodate the weak reference pattern used when the returned object
     *  is new and has no other references.
     *  @param typeface  The typeface to find, not nullptr.
     *  @param glyphID   Specify which section of a large font is of interest.
     */
    static SkPDFFont* GetFontResource(SkPDFDocument* doc,
                                      SkStrike* cache,
                                      SkTypeface* typeface,
                                      SkGlyphID glyphID);

    /** Gets SkAdvancedTypefaceMetrics, and caches the result.
     *  @param typeface can not be nullptr.
     *  @return nullptr only when typeface is bad.
     */
    static const SkAdvancedTypefaceMetrics* GetMetrics(const SkTypeface* typeface,
                                                       SkPDFDocument* canon);

    static const std::vector<SkUnichar>& GetUnicodeMap(const SkTypeface* typeface,
                                                       SkPDFDocument* canon);

    static void PopulateCommonFontDescriptor(SkPDFDict* descriptor,
                                             const SkAdvancedTypefaceMetrics&,
                                             uint16_t emSize,
                                             int16_t defaultWidth);

    void emitSubset(SkPDFDocument*) const;

    /**
     *  Return false iff the typeface has its NotEmbeddable flag set.
     *  typeface is not nullptr
     */
    static bool CanEmbedTypeface(SkTypeface*, SkPDFDocument*);

    SkGlyphID firstGlyphID() const { return fGlyphUsage.firstNonZero(); }
    SkGlyphID lastGlyphID() const { return fGlyphUsage.lastGlyph(); }
    const SkPDFGlyphUse& glyphUsage() const { return fGlyphUsage; }
    sk_sp<SkTypeface> refTypeface() const { return fTypeface; }

private:
    sk_sp<SkTypeface> fTypeface;
    SkPDFGlyphUse fGlyphUsage;
    SkPDFIndirectReference fIndirectReference;
    SkAdvancedTypefaceMetrics::FontType fFontType = (SkAdvancedTypefaceMetrics::FontType)(-1);

    SkPDFFont(sk_sp<SkTypeface>,
              SkGlyphID firstGlyphID,
              SkGlyphID lastGlyphID,
              SkAdvancedTypefaceMetrics::FontType fontType,
              SkPDFIndirectReference indirectReference);
    // The glyph IDs accessible with this font.  For Type1 (non CID) fonts,
    // this will be a subset if the font has more than 255 glyphs.

    SkPDFFont(const SkPDFFont&) = delete;
    SkPDFFont& operator=(const SkPDFFont&) = delete;
};

#endif
