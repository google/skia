/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPDFCanon.h"
#include "SkPDFConvertType1FontStream.h"
#include "SkPDFDevice.h"
#include "SkPDFMakeCIDGlyphWidthsArray.h"
#include "SkPDFMakeToUnicodeCmap.h"
#include "SkPDFFont.h"
#include "SkPDFUtils.h"
#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTypefacePriv.h"
#include "SkTypes.h"
#include "SkUtils.h"

#if defined (SK_SFNTLY_SUBSETTER)
    #if defined (GOOGLE3)
        // #including #defines doesn't work with this build system.
        #include "typography/font/sfntly/src/sample/chromium/font_subsetter.h"
    #else
        #include SK_SFNTLY_SUBSETTER
    #endif
#endif

namespace {
// PDF's notion of symbolic vs non-symbolic is related to the character set, not
// symbols vs. characters.  Rarely is a font the right character set to call it
// non-symbolic, so always call it symbolic.  (PDF 1.4 spec, section 5.7.1)
static const int kPdfSymbolic = 4;

class SkPDFType0Font final : public SkPDFFont {
public:
    SkPDFType0Font(const SkAdvancedTypefaceMetrics* info,
                   sk_sp<SkTypeface> typeface,
                   SkAdvancedTypefaceMetrics::FontType type);
    virtual ~SkPDFType0Font();
    sk_sp<SkPDFObject> getFontSubset(SkPDFCanon*, const SkPDFGlyphSet*) override;
#ifdef SK_DEBUG
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) const override;
#endif

private:
#ifdef SK_DEBUG
    bool fPopulated;
#endif
    bool populate(const SkPDFGlyphSet* subset,
                  const SkAdvancedTypefaceMetrics& metrics);
    typedef SkPDFDict INHERITED;
};

class SkPDFType1Font final : public SkPDFFont {
public:
    SkPDFType1Font(const SkAdvancedTypefaceMetrics* info,
                   sk_sp<SkTypeface> typeface,
                   uint16_t glyphID,
                   SkPDFCanon* canon);
    virtual ~SkPDFType1Font() {}
};

class SkPDFType3Font final : public SkPDFFont {
public:
    SkPDFType3Font(const SkAdvancedTypefaceMetrics* info,
                   sk_sp<SkTypeface> typeface,
                   SkAdvancedTypefaceMetrics::FontType fontType,
                   uint16_t glyphID);
    virtual ~SkPDFType3Font() {}
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) const override {
        SkDEBUGFAIL("should call getFontSubset!");
    }
    sk_sp<SkPDFObject> getFontSubset(SkPDFCanon*, const SkPDFGlyphSet*) override;
};

///////////////////////////////////////////////////////////////////////////////
// File-Local Functions
///////////////////////////////////////////////////////////////////////////////

static SkAutoGlyphCache vector_cache(SkTypeface* face, SkScalar size = 0) {
    SkPaint tmpPaint;
    tmpPaint.setHinting(SkPaint::kNo_Hinting);
    tmpPaint.setTypeface(sk_ref_sp(face));
    if (0 == size) {
        SkASSERT(face);
        tmpPaint.setTextSize((SkScalar)face->getUnitsPerEm());
    } else {
        tmpPaint.setTextSize(size);
    }    
    const SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    SkAutoGlyphCache glyphCache(tmpPaint, &props, nullptr);
    SkASSERT(glyphCache.get());
    return glyphCache;
}

// scale from em-units to base-1000, returning as a SkScalar
SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    if (emSize == 1000) {
        return scaled;
    } else {
        return SkScalarMulDiv(scaled, 1000, emSize);
    }
}

SkScalar scaleFromFontUnits(int16_t val, uint16_t emSize) {
    return from_font_units(SkIntToScalar(val), emSize);
}


void setGlyphWidthAndBoundingBox(SkScalar width, SkIRect box,
                                 SkDynamicMemoryWStream* content) {
    // Specify width and bounding box for the glyph.
    SkPDFUtils::AppendScalar(width, content);
    content->writeText(" 0 ");
    content->writeDecAsText(box.fLeft);
    content->writeText(" ");
    content->writeDecAsText(box.fTop);
    content->writeText(" ");
    content->writeDecAsText(box.fRight);
    content->writeText(" ");
    content->writeDecAsText(box.fBottom);
    content->writeText(" d1\n");
}

static sk_sp<SkPDFArray> makeFontBBox(SkIRect glyphBBox, uint16_t emSize) {
    auto bbox = sk_make_sp<SkPDFArray>();
    bbox->reserve(4);
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fLeft, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fBottom, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fRight, emSize));
    bbox->appendScalar(scaleFromFontUnits(glyphBBox.fTop, emSize));
    return bbox;
}
}  // namespace


///////////////////////////////////////////////////////////////////////////////
// class SkPDFGlyphSet
///////////////////////////////////////////////////////////////////////////////

SkPDFGlyphSet::SkPDFGlyphSet() : fBitSet(SK_MaxU16 + 1) {
}

void SkPDFGlyphSet::set(const uint16_t* glyphIDs, int numGlyphs) {
    for (int i = 0; i < numGlyphs; ++i) {
        fBitSet.setBit(glyphIDs[i], true);
    }
}

bool SkPDFGlyphSet::has(uint16_t glyphID) const {
    return fBitSet.isBitSet(glyphID);
}

void SkPDFGlyphSet::exportTo(SkTDArray<unsigned int>* glyphIDs) const {
    fBitSet.exportTo(glyphIDs);
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFGlyphSetMap
///////////////////////////////////////////////////////////////////////////////

SkPDFGlyphSetMap::SkPDFGlyphSetMap() {}

SkPDFGlyphSetMap::~SkPDFGlyphSetMap() {
    fMap.reset();
}

void SkPDFGlyphSetMap::noteGlyphUsage(SkPDFFont* font, const uint16_t* glyphIDs,
                                      int numGlyphs) {
    SkPDFGlyphSet* subset = getGlyphSetForFont(font);
    if (subset) {
        subset->set(glyphIDs, numGlyphs);
    }
}

SkPDFGlyphSet* SkPDFGlyphSetMap::getGlyphSetForFont(SkPDFFont* font) {
    int index = fMap.count();
    for (int i = 0; i < index; ++i) {
        if (fMap[i].fFont == font) {
            return &fMap[i].fGlyphSet;
        }
    }
    FontGlyphSetPair& pair = fMap.push_back();
    pair.fFont = font;
    return &pair.fGlyphSet;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFFont
///////////////////////////////////////////////////////////////////////////////

/* Font subset design: It would be nice to be able to subset fonts
 * (particularly type 3 fonts), but it's a lot of work and not a priority.
 *
 * Resources are canonicalized and uniqueified by pointer so there has to be
 * some additional state indicating which subset of the font is used.  It
 * must be maintained at the page granularity and then combined at the document
 * granularity. a) change SkPDFFont to fill in its state on demand, kind of
 * like SkPDFGraphicState.  b) maintain a per font glyph usage class in each
 * page/pdf device. c) in the document, retrieve the per font glyph usage
 * from each page and combine it and ask for a resource with that subset.
 */

SkPDFFont::~SkPDFFont() {}

static bool can_embed(const SkAdvancedTypefaceMetrics* metrics) {
    return !metrics ||
           !SkToBool(metrics->fFlags & SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
}

static bool can_subset(const SkAdvancedTypefaceMetrics* metrics) {
    return !metrics ||
           !SkToBool(metrics->fFlags & SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag);
}

int SkPDFFont::glyphsToPDFFontEncoding(SkGlyphID* glyphIDs, int numGlyphs) const {
    // A font with multibyte glyphs will support all glyph IDs in a single font.
    if (this->multiByteGlyphs()) {
        return numGlyphs;
    }

    for (int i = 0; i < numGlyphs; i++) {
        if (glyphIDs[i] == 0) {
            continue;
        }
        if (glyphIDs[i] < fFirstGlyphID || glyphIDs[i] > fLastGlyphID) {
            return i;
        }
        glyphIDs[i] -= (fFirstGlyphID - 1);
    }

    return numGlyphs;
}

int SkPDFFont::glyphsToPDFFontEncodingCount(const SkGlyphID* glyphIDs,
                                            int numGlyphs) const {
    if (this->multiByteGlyphs()) {    // A font with multibyte glyphs will
        return numGlyphs;             // support all glyph IDs in a single font.
    }
    for (int i = 0; i < numGlyphs; i++) {
        if (glyphIDs[i] != 0 &&
            (glyphIDs[i] < fFirstGlyphID || glyphIDs[i] > fLastGlyphID)) {
            return i;
        }
    }
    return numGlyphs;
}


const SkAdvancedTypefaceMetrics* SkPDFFont::GetMetrics(SkTypeface* typeface,
                                                       SkPDFCanon* canon) {
    SkFontID id = SkTypeface::UniqueID(typeface);
    if (SkAdvancedTypefaceMetrics** ptr = canon->fTypefaceMetrics.find(id)) {
        return *ptr;
    }
    sk_sp<SkTypeface> defaultFace;
    if (!typeface) {
        defaultFace = SkTypeface::MakeDefault();
        typeface = defaultFace.get();
    }
    sk_sp<SkAdvancedTypefaceMetrics> metrics(
            typeface->getAdvancedTypefaceMetrics(
                    SkTypeface::kGlyphNames_PerGlyphInfo | SkTypeface::kToUnicode_PerGlyphInfo,
                    nullptr, 0));
    return *canon->fTypefaceMetrics.set(id, metrics.release());
}

SkAdvancedTypefaceMetrics::FontType font_type(const SkAdvancedTypefaceMetrics* metrics) {
    if (!metrics || SkToBool(metrics->fFlags &
                             SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag)) {
        // force Type3 fallback.
        return SkAdvancedTypefaceMetrics::kOther_Font;
    }
    return metrics->fType;
}

static SkGlyphID first_glyph_for_single_byte_encoding(SkGlyphID gid) {
    return gid != 0 ? gid - (gid - 1) % 255 : 1;
}

SkPDFFont* SkPDFFont::GetFontResource(SkPDFCanon* canon,
                                      SkTypeface* face,
                                      SkGlyphID glyphID) {
    SkASSERT(canon);
    const SkAdvancedTypefaceMetrics* fontMetrics = SkPDFFont::GetMetrics(face, canon);
    SkAdvancedTypefaceMetrics::FontType type = font_type(fontMetrics);
    bool multibyte = SkPDFFont::IsMultiByte(type);
    SkGlyphID firstGlyph = multibyte ? 0 : first_glyph_for_single_byte_encoding(glyphID);
    uint64_t fontID = (SkTypeface::UniqueID(face) << 16) | firstGlyph;

    if (SkPDFFont** found = canon->fFontMap.find(fontID)) {
        SkASSERT(multibyte == (*found)->multiByteGlyphs());
        return SkRef(*found);
    }

    sk_sp<SkTypeface> typeface(face ? sk_ref_sp(face) : SkTypeface::MakeDefault());
    SkASSERT(typeface);
    int glyphCount = typeface->countGlyphs();
    // Validate typeface + glyph;
    if (glyphCount < 1 ||  // typeface lacks even a NOTDEF glyph.
        glyphCount > 1 + SK_MaxU16 ||  // invalid glyphCount
        glyphID >= glyphCount) {  // invalid glyph
        return nullptr;
    }

    sk_sp<SkPDFFont> font;
    switch (type) {
        case SkAdvancedTypefaceMetrics::kType1CID_Font:
        case SkAdvancedTypefaceMetrics::kTrueType_Font:
            SkASSERT(multibyte);
            SkASSERT(fontMetrics != nullptr);
            font = sk_make_sp<SkPDFType0Font>(fontMetrics,
                                              std::move(typeface),
                                              type);
            break;
        case SkAdvancedTypefaceMetrics::kType1_Font:
            SkASSERT(!multibyte);
            SkASSERT(fontMetrics != nullptr);
            font = sk_make_sp<SkPDFType1Font>(fontMetrics,
                                              std::move(typeface),
                                              glyphID,
                                              canon);
            break;
        default:
            SkASSERT(!multibyte);
            font = sk_make_sp<SkPDFType3Font>(fontMetrics,
                                              std::move(typeface),
                                              type,
                                              glyphID);
            break;
    }
    canon->fFontMap.set(fontID, SkRef(font.get()));
    return font.release();  // TODO(halcanary) return sk_sp<SkPDFFont>.
}

sk_sp<SkPDFObject> SkPDFFont::getFontSubset(SkPDFCanon*, const SkPDFGlyphSet*) {
    return nullptr;  // Default: no support.
}

SkPDFFont::SkPDFFont(sk_sp<SkTypeface> typeface,
                     SkAdvancedTypefaceMetrics::FontType fontType)
    : SkPDFDict("Font")
    , fTypeface(std::move(typeface))
    , fFirstGlyphID(1)
    , fFontType(fontType) {
    SkASSERT(fTypeface);
    fLastGlyphID = SkToU16(fTypeface->countGlyphs() - 1);
}

static void  add_common_font_descriptor_entries(SkPDFDict* descriptor,
                                                const SkAdvancedTypefaceMetrics& metrics,
                                                int16_t defaultWidth) {
    const uint16_t emSize = metrics.fEmSize;
    descriptor->insertName("FontName", metrics.fFontName);
    descriptor->insertInt("Flags", (size_t)(metrics.fStyle | kPdfSymbolic));
    descriptor->insertScalar("Ascent",
            scaleFromFontUnits(metrics.fAscent, emSize));
    descriptor->insertScalar("Descent",
            scaleFromFontUnits(metrics.fDescent, emSize));
    descriptor->insertScalar("StemV",
            scaleFromFontUnits(metrics.fStemV, emSize));
    descriptor->insertScalar("CapHeight",
            scaleFromFontUnits(metrics.fCapHeight, emSize));
    descriptor->insertInt("ItalicAngle", metrics.fItalicAngle);
    descriptor->insertObject(
            "FontBBox", makeFontBBox(metrics.fBBox, metrics.fEmSize));
    if (defaultWidth > 0) {
        descriptor->insertScalar("MissingWidth",
                scaleFromFontUnits(defaultWidth, emSize));
    }
}

void SkPDFFont::adjustGlyphRangeForSingleByteEncoding(SkGlyphID glyphID) {
    // Single byte glyph encoding supports a max of 255 glyphs.
    fFirstGlyphID = first_glyph_for_single_byte_encoding(glyphID);
    if (fLastGlyphID > fFirstGlyphID + 255 - 1) {
        fLastGlyphID = fFirstGlyphID + 255 - 1;
    }
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType0Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType0Font::SkPDFType0Font(const SkAdvancedTypefaceMetrics* info,
                               sk_sp<SkTypeface> typeface,
                               SkAdvancedTypefaceMetrics::FontType fontType)
    : SkPDFFont(std::move(typeface), fontType) {
    SkDEBUGCODE(fPopulated = false);
    if (!can_subset(info)) {
        this->populate(nullptr, *info);
    }
}

SkPDFType0Font::~SkPDFType0Font() {}

sk_sp<SkPDFObject>  SkPDFType0Font::getFontSubset(SkPDFCanon* canon,
                                                  const SkPDFGlyphSet* subset) {
    const SkAdvancedTypefaceMetrics* metrics =
        SkPDFFont::GetMetrics(this->typeface(), canon);
    SkASSERT(metrics);
    if (!can_subset(metrics)) {
        return nullptr;
    }
    auto newSubset = sk_make_sp<SkPDFType0Font>(
            metrics, this->refTypeface(), this->getType());
    newSubset->populate(subset, *metrics);
    return newSubset;
}

#ifdef SK_DEBUG
void SkPDFType0Font::emitObject(SkWStream* stream,
                                const SkPDFObjNumMap& objNumMap,
                                const SkPDFSubstituteMap& substitutes) const {
    SkASSERT(fPopulated);
    return INHERITED::emitObject(stream, objNumMap, substitutes);
}
#endif

#ifdef SK_SFNTLY_SUBSETTER
// if possible, make no copy.
static sk_sp<SkData> stream_to_data(std::unique_ptr<SkStreamAsset> stream) {
    SkASSERT(stream);
    (void)stream->rewind();
    SkASSERT(stream->hasLength());
    size_t size = stream->getLength();
    if (const void* base = stream->getMemoryBase()) {
        SkData::ReleaseProc proc =
            [](const void*, void* ctx) { delete (SkStream*)ctx; };
        return SkData::MakeWithProc(base, size, proc, stream.release());
    }
    return SkData::MakeFromStream(stream.get(), size);
}

static sk_sp<SkPDFObject> get_subset_font_stream(
        std::unique_ptr<SkStreamAsset> fontAsset,
        const SkTDArray<uint32_t>& subset,
        const char* fontName) {
    // sfntly requires unsigned int* to be passed in,
    // as far as we know, unsigned int is equivalent
    // to uint32_t on all platforms.
    static_assert(sizeof(unsigned) == sizeof(uint32_t), "");

    // TODO(halcanary): Use ttcIndex, not fontName.

    unsigned char* subsetFont{nullptr};
    int subsetFontSize{0};
    {
        sk_sp<SkData> fontData(stream_to_data(std::move(fontAsset)));
        subsetFontSize =
            SfntlyWrapper::SubsetFont(fontName,
                                      fontData->bytes(),
                                      fontData->size(),
                                      subset.begin(),
                                      subset.count(),
                                      &subsetFont);
    }
    SkASSERT(subsetFontSize > 0 || subsetFont == nullptr);
    if (subsetFontSize < 1) {
        return nullptr;
    }
    SkASSERT(subsetFont != nullptr);
    auto subsetStream = sk_make_sp<SkPDFStream>(
            SkData::MakeWithProc(
                    subsetFont, subsetFontSize,
                    [](const void* p, void*) { delete[] (unsigned char*)p; },
                    nullptr));
    subsetStream->dict()->insertInt("Length1", subsetFontSize);
    return subsetStream;
}
#endif  // SK_SFNTLY_SUBSETTER

bool SkPDFType0Font::populate(const SkPDFGlyphSet* subset,
                              const SkAdvancedTypefaceMetrics& metrics) {
    SkASSERT(can_embed(&metrics));
    SkAdvancedTypefaceMetrics::FontType type = this->getType();
    SkTypeface* face = this->typeface();
    SkASSERT(face);
    const SkString& name = metrics.fFontName;

    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    add_common_font_descriptor_entries(descriptor.get(), metrics, 0);
    switch (type) {
        case SkAdvancedTypefaceMetrics::kTrueType_Font: {
            int ttcIndex;
            std::unique_ptr<SkStreamAsset> fontAsset(face->openStream(&ttcIndex));
            SkASSERT(fontAsset);
            if (!fontAsset) {
                return false;
            }
            size_t fontSize = fontAsset->getLength();
            SkASSERT(fontSize > 0);
            if (fontSize == 0) {
                return false;
            }

            #ifdef SK_SFNTLY_SUBSETTER
            if (can_subset(&metrics) && subset) {
                // Generate glyph id array. in format needed by sfntly
                SkTDArray<uint32_t> glyphIDs;
                if (subset) {
                    if (!subset->has(0)) {
                        glyphIDs.push(0);  // Always include glyph 0.
                    }
                    subset->exportTo(&glyphIDs);
                }
                sk_sp<SkPDFObject> subsetStream = get_subset_font_stream(
                        std::move(fontAsset), glyphIDs, name.c_str());
                if (subsetStream) {
                    descriptor->insertObjRef("FontFile2", std::move(subsetStream));
                    break;
                }
                // If subsetting fails, fall back to original font data.
                fontAsset.reset(face->openStream(&ttcIndex));
            }
            #endif  // SK_SFNTLY_SUBSETTER
            auto fontStream = sk_make_sp<SkPDFSharedStream>(std::move(fontAsset));
            fontStream->dict()->insertInt("Length1", fontSize);
            descriptor->insertObjRef("FontFile2", std::move(fontStream));
            break;
        }
        case SkAdvancedTypefaceMetrics::kType1CID_Font: {
            std::unique_ptr<SkStreamAsset> fontData(face->openStream(nullptr));
            SkASSERT(fontData);
            SkASSERT(fontData->getLength() > 0);
            if (!fontData || 0 == fontData->getLength()) {
                return false;
            }
            auto fontStream = sk_make_sp<SkPDFSharedStream>(std::move(fontData));
            fontStream->dict()->insertName("Subtype", "CIDFontType0c");
            descriptor->insertObjRef("FontFile3", std::move(fontStream));
            break;
        }
        default:
            SkASSERT(false);
    }

    auto newCIDFont = sk_make_sp<SkPDFDict>("Font");
    newCIDFont->insertObjRef("FontDescriptor", std::move(descriptor));
    newCIDFont->insertName("BaseFont", name);

    if (type == SkAdvancedTypefaceMetrics::kType1CID_Font) {
        newCIDFont->insertName("Subtype", "CIDFontType0");
    } else if (type == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        newCIDFont->insertName("Subtype", "CIDFontType2");
        newCIDFont->insertName("CIDToGIDMap", "Identity");
    } else {
        SkASSERT(false);
    }

    auto sysInfo = sk_make_sp<SkPDFDict>();
    sysInfo->insertString("Registry", "Adobe");
    sysInfo->insertString("Ordering", "Identity");
    sysInfo->insertInt("Supplement", 0);
    newCIDFont->insertObject("CIDSystemInfo", std::move(sysInfo));

    uint16_t emSize = metrics.fEmSize;
    int16_t defaultWidth = 0;
    const SkBitSet* bitSet = subset ? &subset->bitSet() : nullptr;
    {
        SkAutoGlyphCache glyphCache = vector_cache(face);
        sk_sp<SkPDFArray> widths = SkPDFMakeCIDGlyphWidthsArray(
                glyphCache.get(), bitSet, emSize, &defaultWidth);
        if (widths && widths->size() > 0) {
            newCIDFont->insertObject("W", std::move(widths));
        }
        newCIDFont->insertScalar(
                "DW", scaleFromFontUnits(defaultWidth, emSize));
    }

    ////////////////////////////////////////////////////////////////////////////

    this->insertName("Subtype", "Type0");
    this->insertName("BaseFont", metrics.fFontName);
    this->insertName("Encoding", "Identity-H");
    auto descendantFonts = sk_make_sp<SkPDFArray>();
    descendantFonts->appendObjRef(std::move(newCIDFont));
    this->insertObject("DescendantFonts", std::move(descendantFonts));

    if (metrics.fGlyphToUnicode.count() > 0) {
        this->insertObjRef("ToUnicode",
                           SkPDFMakeToUnicodeCmap(metrics.fGlyphToUnicode,
                                                  subset,
                                                  multiByteGlyphs(),
                                                  firstGlyphID(),
                                                  lastGlyphID()));
    }
    SkDEBUGCODE(fPopulated = true);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType1Font
///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkPDFDict> make_type1_font_descriptor(
        SkTypeface* typeface,
        const SkAdvancedTypefaceMetrics* info) {
    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    SkASSERT(info);
    add_common_font_descriptor_entries(descriptor.get(), *info, 0);

    int ttcIndex;
    size_t header SK_INIT_TO_AVOID_WARNING;
    size_t data SK_INIT_TO_AVOID_WARNING;
    size_t trailer SK_INIT_TO_AVOID_WARNING;
    std::unique_ptr<SkStreamAsset> rawFontData(typeface->openStream(&ttcIndex));
    sk_sp<SkData> fontData = SkPDFConvertType1FontStream(std::move(rawFontData),
                                                         &header, &data, &trailer);
    if (fontData && can_embed(info)) {
        auto fontStream = sk_make_sp<SkPDFStream>(std::move(fontData));
        fontStream->dict()->insertInt("Length1", header);
        fontStream->dict()->insertInt("Length2", data);
        fontStream->dict()->insertInt("Length3", trailer);
        descriptor->insertObjRef("FontFile", std::move(fontStream));
    }
    return descriptor;
}

static void populate_type_1_font(SkPDFDict* font,
                                 const SkAdvancedTypefaceMetrics* info,
                                 SkTypeface* typeface,
                                 SkGlyphID firstGlyphID,
                                 SkGlyphID lastGlyphID) {
    SkASSERT(info);
    font->insertName("Subtype", "Type1");
    font->insertName("BaseFont", info->fFontName);

    // glyphCount not including glyph 0
    unsigned glyphCount = 1 + lastGlyphID - firstGlyphID;
    SkASSERT(glyphCount > 0 && glyphCount <= 255);
    font->insertInt("FirstChar", (size_t)0);
    font->insertInt("LastChar", (size_t)glyphCount);
    {
        SkAutoGlyphCache glyphCache = vector_cache(typeface);
        auto widths = sk_make_sp<SkPDFArray>();
        SkScalar advance = glyphCache->getGlyphIDAdvance(0).fAdvanceX;
        const uint16_t emSize = info->fEmSize;
        widths->appendScalar(from_font_units(advance, emSize));
        for (unsigned gID = firstGlyphID; gID <= lastGlyphID; gID++) {
            advance = glyphCache->getGlyphIDAdvance(gID).fAdvanceX;
            widths->appendScalar(from_font_units(advance, emSize));
        }
        font->insertObject("Widths", std::move(widths));
    }
    auto encDiffs = sk_make_sp<SkPDFArray>();
    encDiffs->reserve(lastGlyphID - firstGlyphID + 3);
    encDiffs->appendInt(0);
    const SkTArray<SkString>& glyphNames = info->fGlyphNames;
    SkASSERT(glyphNames.count() > lastGlyphID);
    encDiffs->appendName(glyphNames[0].c_str());
    const SkString unknown("UNKNOWN");
    for (int gID = firstGlyphID; gID <= lastGlyphID; gID++) {
        const bool valid = gID < glyphNames.count() && !glyphNames[gID].isEmpty();
        const SkString& name = valid ? glyphNames[gID] : unknown;
        encDiffs->appendName(name);
    }

    auto encoding = sk_make_sp<SkPDFDict>("Encoding");
    encoding->insertObject("Differences", std::move(encDiffs));
    font->insertObject("Encoding", std::move(encoding));
}

SkPDFType1Font::SkPDFType1Font(const SkAdvancedTypefaceMetrics* info,
                               sk_sp<SkTypeface> typeface,
                               uint16_t glyphID,
                               SkPDFCanon* canon)
    : SkPDFFont(std::move(typeface), SkAdvancedTypefaceMetrics::kType1_Font)
{
    SkFontID fontID = this->typeface()->uniqueID();
    sk_sp<SkPDFDict> fontDescriptor;
    if (SkPDFDict** ptr = canon->fFontDescriptors.find(fontID)) {
        fontDescriptor = sk_ref_sp(*ptr);
    } else {
        fontDescriptor = make_type1_font_descriptor(this->typeface(), info);
        canon->fFontDescriptors.set(fontID, SkRef(fontDescriptor.get()));
    }
    this->insertObjRef("FontDescriptor", std::move(fontDescriptor));
    this->adjustGlyphRangeForSingleByteEncoding(glyphID);
    // TODO(halcanary): subset this (advances and names).
    populate_type_1_font(this, info, this->typeface(),
                         this->firstGlyphID(), this->lastGlyphID());
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType3Font
///////////////////////////////////////////////////////////////////////////////

namespace {
// returns [0, first, first+1, ... last-1,  last]
struct SingleByteGlyphIdIterator {
    SingleByteGlyphIdIterator(SkGlyphID first, SkGlyphID last)
        : fFirst(first), fLast(last) {
        SkASSERT(fFirst > 0);
        SkASSERT(fLast >= first);
    }
    struct Iter {
        void operator++() {
            fCurrent = (0 == fCurrent) ? fFirst : fCurrent + 1;
        }
        // This is an input_iterator
        SkGlyphID operator*() const { return (SkGlyphID)fCurrent; }
        bool operator!=(const Iter& rhs) const {
            return fCurrent != rhs.fCurrent;
        }
        Iter(SkGlyphID f, int c) : fFirst(f), fCurrent(c) {}
    private:
        const SkGlyphID fFirst;
        int fCurrent; // must be int to make fLast+1 to fit
    };
    Iter begin() const { return Iter(fFirst, 0); }
    Iter end() const { return Iter(fFirst, (int)fLast + 1); }
private:
    const SkGlyphID fFirst;
    const SkGlyphID fLast;
};
}

static void add_type3_font_info(SkPDFCanon* canon,
                                SkPDFDict* font,
                                SkTypeface* typeface,
                                SkScalar emSize,
                                const SkPDFGlyphSet* subset,
                                SkGlyphID firstGlyphID,
                                SkGlyphID lastGlyphID) {
    SkASSERT(lastGlyphID >= firstGlyphID);
    SkASSERT(emSize > 0.0f);
    SkAutoGlyphCache cache = vector_cache(typeface, emSize);
    font->insertName("Subtype", "Type3");
    // Flip about the x-axis and scale by 1/emSize.
    SkMatrix fontMatrix;
    fontMatrix.setScale(SkScalarInvert(emSize), -SkScalarInvert(emSize));
    font->insertObject("FontMatrix", SkPDFUtils::MatrixToArray(fontMatrix));

    auto charProcs = sk_make_sp<SkPDFDict>();
    auto encoding = sk_make_sp<SkPDFDict>("Encoding");

    auto encDiffs = sk_make_sp<SkPDFArray>();
    // length(firstGlyphID .. lastGlyphID) ==  lastGlyphID - firstGlyphID + 1
    // plus 1 for glyph 0;
    SkASSERT(firstGlyphID > 0);
    SkASSERT(lastGlyphID >= firstGlyphID);
    int glyphCount = lastGlyphID - firstGlyphID + 2;
    // one other entry for the index of first glyph.
    encDiffs->reserve(glyphCount + 1);
    encDiffs->appendInt(0);  // index of first glyph

    auto widthArray = sk_make_sp<SkPDFArray>();
    widthArray->reserve(glyphCount);

    SkIRect bbox = SkIRect::MakeEmpty();

    sk_sp<SkPDFStream> emptyStream;
    for (SkGlyphID gID : SingleByteGlyphIdIterator(firstGlyphID, lastGlyphID)) {
        bool skipGlyph = subset && gID != 0 && !subset->has(gID);
        SkString characterName;
        SkScalar advance = 0.0f;
        SkIRect glyphBBox;
        if (skipGlyph) {
            characterName.set("g0");
        } else {
            characterName.printf("g%X", gID);
            const SkGlyph& glyph = cache->getGlyphIDMetrics(gID);
            advance = SkFloatToScalar(glyph.fAdvanceX);
            glyphBBox = SkIRect::MakeXYWH(glyph.fLeft, glyph.fTop,
                                          glyph.fWidth, glyph.fHeight);
            bbox.join(glyphBBox);
            const SkPath* path = cache->findPath(glyph);
            if (path && !path->isEmpty()) {
                SkDynamicMemoryWStream content;
                setGlyphWidthAndBoundingBox(SkFloatToScalar(glyph.fAdvanceX), glyphBBox,
                                            &content);
                SkPDFUtils::EmitPath(*path, SkPaint::kFill_Style, &content);
                SkPDFUtils::PaintPath(SkPaint::kFill_Style, path->getFillType(),
                                      &content);
                charProcs->insertObjRef(
                    characterName, sk_make_sp<SkPDFStream>(
                            std::unique_ptr<SkStreamAsset>(content.detachAsStream())));
            } else {
                if (!emptyStream) {
                    emptyStream = sk_make_sp<SkPDFStream>(
                            std::unique_ptr<SkStreamAsset>(
                                    new SkMemoryStream((size_t)0)));
                }
                charProcs->insertObjRef(characterName, emptyStream);
            }
        }
        encDiffs->appendName(characterName.c_str());
        widthArray->appendScalar(advance);
    }

    encoding->insertObject("Differences", std::move(encDiffs));
    font->insertInt("FirstChar", 0);
    font->insertInt("LastChar", lastGlyphID - firstGlyphID + 1);
    /* FontBBox: "A rectangle expressed in the glyph coordinate
      system, specifying the font bounding box. This is the smallest
      rectangle enclosing the shape that would result if all of the
      glyphs of the font were placed with their origins coincident and
      then filled." */
    auto fontBBox = sk_make_sp<SkPDFArray>();
    fontBBox->reserve(4);
    fontBBox->appendInt(bbox.left());
    fontBBox->appendInt(bbox.bottom());
    fontBBox->appendInt(bbox.right());
    fontBBox->appendInt(bbox.top());
    font->insertObject("FontBBox", std::move(fontBBox));
    font->insertName("CIDToGIDMap", "Identity");
    const SkAdvancedTypefaceMetrics* metrics = SkPDFFont::GetMetrics(typeface, canon);
    if (metrics /* && metrics->fGlyphToUnicode.count() > 0 */) {
        font->insertObjRef("ToUnicode",
                           SkPDFMakeToUnicodeCmap(metrics->fGlyphToUnicode,
                                                  subset,
                                                  false,
                                                  firstGlyphID,
                                                  lastGlyphID));
    }
    font->insertObject("Widths", std::move(widthArray));
    font->insertObject("Encoding", std::move(encoding));
    font->insertObject("CharProcs", std::move(charProcs));
}

SkPDFType3Font::SkPDFType3Font(const SkAdvancedTypefaceMetrics* info,
                               sk_sp<SkTypeface> typeface,
                               SkAdvancedTypefaceMetrics::FontType fontType,
                               uint16_t glyphID)
    : SkPDFFont(std::move(typeface), fontType) {
    this->adjustGlyphRangeForSingleByteEncoding(glyphID);
}

sk_sp<SkPDFObject> SkPDFType3Font::getFontSubset(SkPDFCanon* canon,
                                                 const SkPDFGlyphSet* usage) {
    // All fonts are subset before serialization.
    // TODO(halcanary): all fonts should follow this pattern.
    const SkAdvancedTypefaceMetrics* info =
        SkPDFFont::GetMetrics(this->typeface(), canon);
    uint16_t emSize = info && info->fEmSize > 0 ? info->fEmSize : 1000;
    auto font = sk_make_sp<SkPDFDict>("Font");
    add_type3_font_info(canon, font.get(), this->typeface(), (SkScalar)emSize, usage,
                        this->firstGlyphID(), this->lastGlyphID());
    return font;
}


////////////////////////////////////////////////////////////////////////////////

bool SkPDFFont::CanEmbedTypeface(SkTypeface* typeface, SkPDFCanon* canon) {
    return can_embed(SkPDFFont::GetMetrics(typeface, canon));
}

void SkPDFFont::drop() {
    fTypeface = nullptr;
    this->SkPDFDict::drop();
}
