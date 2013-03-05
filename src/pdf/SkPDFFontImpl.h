
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPDFFontImpl_DEFINED
#define SkPDFFontImpl_DEFINED

#include "SkPDFFont.h"

class SkPDFType0Font : public SkPDFFont {
public:
    virtual ~SkPDFType0Font();
    virtual bool multiByteGlyphs() const { return true; }
    SK_API virtual SkPDFFont* getFontSubset(const SkPDFGlyphSet* usage);
#ifdef SK_DEBUG
    virtual void emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                            bool indirect);
#endif

private:
    friend class SkPDFFont;  // to access the constructor
#ifdef SK_DEBUG
    bool fPopulated;
    typedef SkPDFDict INHERITED;
#endif

    SkPDFType0Font(SkAdvancedTypefaceMetrics* info, SkTypeface* typeface);

    bool populate(const SkPDFGlyphSet* subset);
};

class SkPDFCIDFont : public SkPDFFont {
public:
    virtual ~SkPDFCIDFont();
    virtual bool multiByteGlyphs() const { return true; }

private:
    friend class SkPDFType0Font;  // to access the constructor

    SkPDFCIDFont(SkAdvancedTypefaceMetrics* info, SkTypeface* typeface,
                 const SkPDFGlyphSet* subset);

    bool populate(const SkPDFGlyphSet* subset);
    bool addFontDescriptor(int16_t defaultWidth,
                           const SkTDArray<uint32_t>* subset);
};

class SkPDFType1Font : public SkPDFFont {
public:
    virtual ~SkPDFType1Font();
    virtual bool multiByteGlyphs() const { return false; }

private:
    friend class SkPDFFont;  // to access the constructor

    SkPDFType1Font(SkAdvancedTypefaceMetrics* info, SkTypeface* typeface,
                   uint16_t glyphID, SkPDFDict* relatedFontDescriptor);

    bool populate(int16_t glyphID);
    bool addFontDescriptor(int16_t defaultWidth);
    void addWidthInfoFromRange(int16_t defaultWidth,
        const SkAdvancedTypefaceMetrics::WidthRange* widthRangeEntry);
};

class SkPDFType3Font : public SkPDFFont {
public:
    virtual ~SkPDFType3Font();
    virtual bool multiByteGlyphs() const { return false; }

private:
    friend class SkPDFFont;  // to access the constructor

    SkPDFType3Font(SkAdvancedTypefaceMetrics* info, SkTypeface* typeface, uint16_t glyphID);

    bool populate(int16_t glyphID);
};

#endif
