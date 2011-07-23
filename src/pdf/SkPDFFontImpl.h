/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    bool addFontDescriptor(int16_t defaultWidth, const SkPDFGlyphSet* subset);
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

    SkPDFType3Font(SkAdvancedTypefaceMetrics* info, SkTypeface* typeface,
                   uint16_t glyphID, SkPDFDict* relatedFontDescriptor);

    bool populate(int16_t glyphID);
};

#endif
