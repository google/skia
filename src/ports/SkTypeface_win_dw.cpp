/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDWriteFontFileStream.h"
#include "SkFontDescriptor.h"
#include "SkFontStream.h"
#include "SkOTTable_head.h"
#include "SkOTTable_hhea.h"
#include "SkOTTable_OS_2.h"
#include "SkOTTable_post.h"
#include "SkScalerContext.h"
#include "SkScalerContext_win_dw.h"
#include "SkTypeface_win_dw.h"
#include "SkTypes.h"
#include "SkUtils.h"

void DWriteFontTypeface::onGetFontDescriptor(SkFontDescriptor* desc,
                                             bool* isLocalStream) const {
    // Get the family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> dwFamilyNames;
    HRV(fDWriteFontFamily->GetFamilyNames(&dwFamilyNames));

    UINT32 dwFamilyNamesLength;
    HRV(dwFamilyNames->GetStringLength(0, &dwFamilyNamesLength));

    SkSMallocWCHAR dwFamilyNameChar(dwFamilyNamesLength+1);
    HRV(dwFamilyNames->GetString(0, dwFamilyNameChar.get(), dwFamilyNamesLength+1));

    SkString utf8FamilyName;
    HRV(sk_wchar_to_skstring(dwFamilyNameChar.get(), &utf8FamilyName));

    desc->setFamilyName(utf8FamilyName.c_str());
    *isLocalStream = SkToBool(fDWriteFontFileLoader.get());
}

static SkUnichar next_utf8(const void** chars) {
    return SkUTF8_NextUnichar((const char**)chars);
}

static SkUnichar next_utf16(const void** chars) {
    return SkUTF16_NextUnichar((const uint16_t**)chars);
}

static SkUnichar next_utf32(const void** chars) {
    const SkUnichar** uniChars = (const SkUnichar**)chars;
    SkUnichar uni = **uniChars;
    *uniChars += 1;
    return uni;
}

typedef SkUnichar (*EncodingProc)(const void**);

static EncodingProc find_encoding_proc(SkTypeface::Encoding enc) {
    static const EncodingProc gProcs[] = {
        next_utf8, next_utf16, next_utf32
    };
    SkASSERT((size_t)enc < SK_ARRAY_COUNT(gProcs));
    return gProcs[enc];
}

int DWriteFontTypeface::onCharsToGlyphs(const void* chars, Encoding encoding,
                                        uint16_t glyphs[], int glyphCount) const
{
    if (NULL == glyphs) {
        EncodingProc next_ucs4_proc = find_encoding_proc(encoding);
        for (int i = 0; i < glyphCount; ++i) {
            const SkUnichar c = next_ucs4_proc(&chars);
            BOOL exists;
            fDWriteFont->HasCharacter(c, &exists);
            if (!exists) {
                return i;
            }
        }
        return glyphCount;
    }

    switch (encoding) {
    case SkTypeface::kUTF8_Encoding:
    case SkTypeface::kUTF16_Encoding: {
        static const int scratchCount = 256;
        UINT32 scratch[scratchCount];
        EncodingProc next_ucs4_proc = find_encoding_proc(encoding);
        for (int baseGlyph = 0; baseGlyph < glyphCount; baseGlyph += scratchCount) {
            int glyphsLeft = glyphCount - baseGlyph;
            int limit = SkTMin(glyphsLeft, scratchCount);
            for (int i = 0; i < limit; ++i) {
                scratch[i] = next_ucs4_proc(&chars);
            }
            fDWriteFontFace->GetGlyphIndices(scratch, limit, &glyphs[baseGlyph]);
        }
        break;
    }
    case SkTypeface::kUTF32_Encoding: {
        const UINT32* utf32 = reinterpret_cast<const UINT32*>(chars);
        fDWriteFontFace->GetGlyphIndices(utf32, glyphCount, glyphs);
        break;
    }
    default:
        SK_CRASH();
    }

    for (int i = 0; i < glyphCount; ++i) {
        if (0 == glyphs[i]) {
            return i;
        }
    }
    return glyphCount;
}

int DWriteFontTypeface::onCountGlyphs() const {
    return fDWriteFontFace->GetGlyphCount();
}

int DWriteFontTypeface::onGetUPEM() const {
    DWRITE_FONT_METRICS metrics;
    fDWriteFontFace->GetMetrics(&metrics);
    return metrics.designUnitsPerEm;
}

class LocalizedStrings_IDWriteLocalizedStrings : public SkTypeface::LocalizedStrings {
public:
    /** Takes ownership of the IDWriteLocalizedStrings. */
    explicit LocalizedStrings_IDWriteLocalizedStrings(IDWriteLocalizedStrings* strings)
        : fIndex(0), fStrings(strings)
    { }

    virtual bool next(SkTypeface::LocalizedString* localizedString) SK_OVERRIDE {
        if (fIndex >= fStrings->GetCount()) {
            return false;
        }

        // String
        UINT32 stringLength;
        HRBM(fStrings->GetStringLength(fIndex, &stringLength), "Could not get string length.");
        stringLength += 1;

        SkSMallocWCHAR wString(stringLength);
        HRBM(fStrings->GetString(fIndex, wString.get(), stringLength), "Could not get string.");

        HRB(sk_wchar_to_skstring(wString.get(), &localizedString->fString));

        // Locale
        UINT32 localeLength;
        HRBM(fStrings->GetLocaleNameLength(fIndex, &localeLength), "Could not get locale length.");
        localeLength += 1;

        SkSMallocWCHAR wLocale(localeLength);
        HRBM(fStrings->GetLocaleName(fIndex, wLocale.get(), localeLength), "Could not get locale.");

        HRB(sk_wchar_to_skstring(wLocale.get(), &localizedString->fLanguage));

        ++fIndex;
        return true;
    }

private:
    UINT32 fIndex;
    SkTScopedComPtr<IDWriteLocalizedStrings> fStrings;
};

SkTypeface::LocalizedStrings* DWriteFontTypeface::onCreateFamilyNameIterator() const {
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    HRNM(fDWriteFontFamily->GetFamilyNames(&familyNames), "Could not obtain family names.");

    return new LocalizedStrings_IDWriteLocalizedStrings(familyNames.release());
}

int DWriteFontTypeface::onGetTableTags(SkFontTableTag tags[]) const {
    DWRITE_FONT_FACE_TYPE type = fDWriteFontFace->GetType();
    if (type != DWRITE_FONT_FACE_TYPE_CFF &&
        type != DWRITE_FONT_FACE_TYPE_TRUETYPE &&
        type != DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION)
    {
        return 0;
    }

    int ttcIndex;
    SkAutoTUnref<SkStream> stream(this->openStream(&ttcIndex));
    return stream.get() ? SkFontStream::GetTableTags(stream, ttcIndex, tags) : 0;
}

size_t DWriteFontTypeface::onGetTableData(SkFontTableTag tag, size_t offset,
                                          size_t length, void* data) const
{
    AutoDWriteTable table(fDWriteFontFace.get(), SkEndian_SwapBE32(tag));
    if (!table.fExists) {
        return 0;
    }

    if (offset > table.fSize) {
        return 0;
    }
    size_t size = SkTMin(length, table.fSize - offset);
    if (NULL != data) {
        memcpy(data, table.fData + offset, size);
    }

    return size;
}

SkStream* DWriteFontTypeface::onOpenStream(int* ttcIndex) const {
    *ttcIndex = fDWriteFontFace->GetIndex();

    UINT32 numFiles;
    HRNM(fDWriteFontFace->GetFiles(&numFiles, NULL),
         "Could not get number of font files.");
    if (numFiles != 1) {
        return NULL;
    }

    SkTScopedComPtr<IDWriteFontFile> fontFile;
    HRNM(fDWriteFontFace->GetFiles(&numFiles, &fontFile), "Could not get font files.");

    const void* fontFileKey;
    UINT32 fontFileKeySize;
    HRNM(fontFile->GetReferenceKey(&fontFileKey, &fontFileKeySize),
         "Could not get font file reference key.");

    SkTScopedComPtr<IDWriteFontFileLoader> fontFileLoader;
    HRNM(fontFile->GetLoader(&fontFileLoader), "Could not get font file loader.");

    SkTScopedComPtr<IDWriteFontFileStream> fontFileStream;
    HRNM(fontFileLoader->CreateStreamFromKey(fontFileKey, fontFileKeySize,
                                             &fontFileStream),
         "Could not create font file stream.");

    return SkNEW_ARGS(SkDWriteFontFileStream, (fontFileStream.get()));
}

SkScalerContext* DWriteFontTypeface::onCreateScalerContext(const SkDescriptor* desc) const {
    return SkNEW_ARGS(SkScalerContext_DW, (const_cast<DWriteFontTypeface*>(this), desc));
}

void DWriteFontTypeface::onFilterRec(SkScalerContext::Rec* rec) const {
    if (rec->fFlags & SkScalerContext::kLCD_BGROrder_Flag ||
        rec->fFlags & SkScalerContext::kLCD_Vertical_Flag)
    {
        rec->fMaskFormat = SkMask::kA8_Format;
    }

    unsigned flagsWeDontSupport = SkScalerContext::kVertical_Flag |
                                  SkScalerContext::kDevKernText_Flag |
                                  SkScalerContext::kForceAutohinting_Flag |
                                  SkScalerContext::kEmbolden_Flag |
                                  SkScalerContext::kLCD_BGROrder_Flag |
                                  SkScalerContext::kLCD_Vertical_Flag;
    rec->fFlags &= ~flagsWeDontSupport;

    SkPaint::Hinting h = rec->getHinting();
    // DirectWrite does not provide for hinting hints.
    h = SkPaint::kSlight_Hinting;
    rec->setHinting(h);

#if SK_FONT_HOST_USE_SYSTEM_SETTINGS
    IDWriteFactory* factory = get_dwrite_factory();
    if (factory != NULL) {
        SkTScopedComPtr<IDWriteRenderingParams> defaultRenderingParams;
        if (SUCCEEDED(factory->CreateRenderingParams(&defaultRenderingParams))) {
            float gamma = defaultRenderingParams->GetGamma();
            rec->setDeviceGamma(gamma);
            rec->setPaintGamma(gamma);

            rec->setContrast(defaultRenderingParams->GetEnhancedContrast());
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
//PDF Support

using namespace skia_advanced_typeface_metrics_utils;

// Construct Glyph to Unicode table.
// Unicode code points that require conjugate pairs in utf16 are not
// supported.
// TODO(arthurhsu): Add support for conjugate pairs. It looks like that may
// require parsing the TTF cmap table (platform 4, encoding 12) directly instead
// of calling GetFontUnicodeRange().
// TODO(bungeman): This never does what anyone wants.
// What is really wanted is the text to glyphs mapping
static void populate_glyph_to_unicode(IDWriteFontFace* fontFace,
                                      const unsigned glyphCount,
                                      SkTDArray<SkUnichar>* glyphToUnicode) {
    HRESULT hr = S_OK;

    //Do this like free type instead
    UINT32 count = 0;
    for (UINT32 c = 0; c < 0x10FFFF; ++c) {
        UINT16 glyph;
        hr = fontFace->GetGlyphIndices(&c, 1, &glyph);
        if (glyph > 0) {
            ++count;
        }
    }

    SkAutoTArray<UINT32> chars(count);
    count = 0;
    for (UINT32 c = 0; c < 0x10FFFF; ++c) {
        UINT16 glyph;
        hr = fontFace->GetGlyphIndices(&c, 1, &glyph);
        if (glyph > 0) {
            chars[count] = c;
            ++count;
        }
    }

    SkAutoTArray<UINT16> glyph(count);
    fontFace->GetGlyphIndices(chars.get(), count, glyph.get());

    USHORT maxGlyph = 0;
    for (USHORT j = 0; j < count; ++j) {
        if (glyph[j] > maxGlyph) maxGlyph = glyph[j];
    }

    glyphToUnicode->setCount(maxGlyph+1);
    for (USHORT j = 0; j < maxGlyph+1u; ++j) {
        (*glyphToUnicode)[j] = 0;
    }

    //'invert'
    for (USHORT j = 0; j < count; ++j) {
        if (glyph[j] < glyphCount && (*glyphToUnicode)[glyph[j]] == 0) {
            (*glyphToUnicode)[glyph[j]] = chars[j];
        }
    }
}

static bool getWidthAdvance(IDWriteFontFace* fontFace, int gId, int16_t* advance) {
    SkASSERT(advance);

    UINT16 glyphId = gId;
    DWRITE_GLYPH_METRICS gm;
    HRESULT hr = fontFace->GetDesignGlyphMetrics(&glyphId, 1, &gm);

    if (FAILED(hr)) {
        *advance = 0;
        return false;
    }

    *advance = gm.advanceWidth;
    return true;
}

SkAdvancedTypefaceMetrics* DWriteFontTypeface::onGetAdvancedTypefaceMetrics(
        SkAdvancedTypefaceMetrics::PerGlyphInfo perGlyphInfo,
        const uint32_t* glyphIDs,
        uint32_t glyphIDsCount) const {

    SkAdvancedTypefaceMetrics* info = NULL;

    HRESULT hr = S_OK;

    const unsigned glyphCount = fDWriteFontFace->GetGlyphCount();

    DWRITE_FONT_METRICS dwfm;
    fDWriteFontFace->GetMetrics(&dwfm);

    info = new SkAdvancedTypefaceMetrics;
    info->fEmSize = dwfm.designUnitsPerEm;
    info->fLastGlyphID = SkToU16(glyphCount - 1);
    info->fStyle = 0;
    info->fFlags = SkAdvancedTypefaceMetrics::kEmpty_FontFlag;

    // SkAdvancedTypefaceMetrics::fFontName is in theory supposed to be
    // the PostScript name of the font. However, due to the way it is currently
    // used, it must actually be a family name.
    SkTScopedComPtr<IDWriteLocalizedStrings> familyNames;
    hr = fDWriteFontFamily->GetFamilyNames(&familyNames);

    UINT32 familyNameLength;
    hr = familyNames->GetStringLength(0, &familyNameLength);

    UINT32 size = familyNameLength+1;
    SkSMallocWCHAR wFamilyName(size);
    hr = familyNames->GetString(0, wFamilyName.get(), size);

    hr = sk_wchar_to_skstring(wFamilyName.get(), &info->fFontName);

    if (perGlyphInfo & SkAdvancedTypefaceMetrics::kToUnicode_PerGlyphInfo) {
        populate_glyph_to_unicode(fDWriteFontFace.get(), glyphCount, &(info->fGlyphToUnicode));
    }

    DWRITE_FONT_FACE_TYPE fontType = fDWriteFontFace->GetType();
    if (fontType == DWRITE_FONT_FACE_TYPE_TRUETYPE ||
        fontType == DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION) {
        info->fType = SkAdvancedTypefaceMetrics::kTrueType_Font;
    } else {
        info->fType = SkAdvancedTypefaceMetrics::kOther_Font;
        info->fItalicAngle = 0;
        info->fAscent = dwfm.ascent;;
        info->fDescent = dwfm.descent;
        info->fStemV = 0;
        info->fCapHeight = dwfm.capHeight;
        info->fBBox = SkIRect::MakeEmpty();
        return info;
    }

    AutoTDWriteTable<SkOTTableHead> headTable(fDWriteFontFace.get());
    AutoTDWriteTable<SkOTTablePostScript> postTable(fDWriteFontFace.get());
    AutoTDWriteTable<SkOTTableHorizontalHeader> hheaTable(fDWriteFontFace.get());
    AutoTDWriteTable<SkOTTableOS2> os2Table(fDWriteFontFace.get());
    if (!headTable.fExists || !postTable.fExists || !hheaTable.fExists || !os2Table.fExists) {
        info->fItalicAngle = 0;
        info->fAscent = dwfm.ascent;;
        info->fDescent = dwfm.descent;
        info->fStemV = 0;
        info->fCapHeight = dwfm.capHeight;
        info->fBBox = SkIRect::MakeEmpty();
        return info;
    }

    //There exist CJK fonts which set the IsFixedPitch and Monospace bits,
    //but have full width, latin half-width, and half-width kana.
    bool fixedWidth = (postTable->isFixedPitch &&
                      (1 == SkEndian_SwapBE16(hheaTable->numberOfHMetrics)));
    //Monospace
    if (fixedWidth) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kFixedPitch_Style;
    }
    //Italic
    if (os2Table->version.v0.fsSelection.field.Italic) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kItalic_Style;
    }
    //Script
    if (SkPanose::FamilyType::Script == os2Table->version.v0.panose.bFamilyType.value) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kScript_Style;
    //Serif
    } else if (SkPanose::FamilyType::TextAndDisplay == os2Table->version.v0.panose.bFamilyType.value &&
               SkPanose::Data::TextAndDisplay::SerifStyle::Triangle <= os2Table->version.v0.panose.data.textAndDisplay.bSerifStyle.value &&
               SkPanose::Data::TextAndDisplay::SerifStyle::NoFit != os2Table->version.v0.panose.data.textAndDisplay.bSerifStyle.value) {
        info->fStyle |= SkAdvancedTypefaceMetrics::kSerif_Style;
    }

    info->fItalicAngle = SkEndian_SwapBE32(postTable->italicAngle) >> 16;

    info->fAscent = SkToS16(dwfm.ascent);
    info->fDescent = SkToS16(dwfm.descent);
    info->fCapHeight = SkToS16(dwfm.capHeight);

    info->fBBox = SkIRect::MakeLTRB((int32_t)SkEndian_SwapBE16((uint16_t)headTable->xMin),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->yMax),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->xMax),
                                    (int32_t)SkEndian_SwapBE16((uint16_t)headTable->yMin));

    //TODO: is this even desired? It seems PDF only wants this value for Type1
    //fonts, and we only get here for TrueType fonts.
    info->fStemV = 0;
    /*
    // Figure out a good guess for StemV - Min width of i, I, !, 1.
    // This probably isn't very good with an italic font.
    int16_t min_width = SHRT_MAX;
    info->fStemV = 0;
    char stem_chars[] = {'i', 'I', '!', '1'};
    for (size_t i = 0; i < SK_ARRAY_COUNT(stem_chars); i++) {
        ABC abcWidths;
        if (GetCharABCWidths(hdc, stem_chars[i], stem_chars[i], &abcWidths)) {
            int16_t width = abcWidths.abcB;
            if (width > 0 && width < min_width) {
                min_width = width;
                info->fStemV = min_width;
            }
        }
    }
    */

    if (perGlyphInfo & SkAdvancedTypefaceMetrics::kHAdvance_PerGlyphInfo) {
        if (fixedWidth) {
            appendRange(&info->fGlyphWidths, 0);
            int16_t advance;
            getWidthAdvance(fDWriteFontFace.get(), 1, &advance);
            info->fGlyphWidths->fAdvance.append(1, &advance);
            finishRange(info->fGlyphWidths.get(), 0,
                        SkAdvancedTypefaceMetrics::WidthRange::kDefault);
        } else {
            info->fGlyphWidths.reset(
                getAdvanceData(fDWriteFontFace.get(),
                               glyphCount,
                               glyphIDs,
                               glyphIDsCount,
                               getWidthAdvance));
        }
    }

    return info;
}
