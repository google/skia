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
    SkPDFType0Font(sk_sp<const SkAdvancedTypefaceMetrics> info,
                   sk_sp<SkTypeface> typeface,
                   SkAdvancedTypefaceMetrics::FontType type);
    virtual ~SkPDFType0Font();
    sk_sp<SkPDFObject> getFontSubset(const SkPDFGlyphSet* usage) override;
#ifdef SK_DEBUG
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) const override;
#endif

private:
#ifdef SK_DEBUG
    bool fPopulated;
#endif
    bool populate(const SkPDFGlyphSet* subset);
    typedef SkPDFDict INHERITED;
};

class SkPDFType1Font final : public SkPDFFont {
public:
    SkPDFType1Font(sk_sp<const SkAdvancedTypefaceMetrics> info,
                   sk_sp<SkTypeface> typeface,
                   uint16_t glyphID,
                   sk_sp<SkPDFDict> relatedFontDescriptor);
    virtual ~SkPDFType1Font();

private:
    bool populate(int16_t glyphID);
    bool addFontDescriptor(int16_t defaultWidth);
};

class SkPDFType3Font final : public SkPDFFont {
public:
    SkPDFType3Font(sk_sp<const SkAdvancedTypefaceMetrics> info,
                   sk_sp<SkTypeface> typeface,
                   SkAdvancedTypefaceMetrics::FontType fontType,
                   uint16_t glyphID);
    virtual ~SkPDFType3Font() {}
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) const override {
        SkDEBUGFAIL("should call getFontSubset!");
    }
    sk_sp<SkPDFObject> getFontSubset(const SkPDFGlyphSet* usage) override;
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

bool SkPDFFont::canEmbed() const {
    if (!fFontInfo.get()) {
        SkASSERT(fFontType == SkAdvancedTypefaceMetrics::kOther_Font);
        return true;
    }
    return (fFontInfo->fFlags &
            SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag) == 0;
}

bool SkPDFFont::canSubset() const {
    if (!fFontInfo.get()) {
        SkASSERT(fFontType == SkAdvancedTypefaceMetrics::kOther_Font);
        return true;
    }
    return (fFontInfo->fFlags &
            SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag) == 0;
}

bool SkPDFFont::hasGlyph(uint16_t id) {
    return (id >= fFirstGlyphID && id <= fLastGlyphID) || id == 0;
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

// static
SkPDFFont* SkPDFFont::GetFontResource(SkPDFCanon* canon,
                                      SkTypeface* face,
                                      uint16_t glyphID) {
    SkASSERT(canon);
    const uint32_t fontID = SkTypeface::UniqueID(face);
    SkPDFFont* relatedFont;
    if (SkPDFFont* pdfFont = canon->findFont(fontID, glyphID, &relatedFont)) {
        return SkRef(pdfFont);
    }
    sk_sp<SkTypeface> typeface(face ? sk_ref_sp(face) : SkTypeface::MakeDefault());
    SkASSERT(typeface);
    int glyphCount = typeface->countGlyphs();
    if (glyphCount < 1 ||  // typeface lacks even a NOTDEF glyph.
        glyphCount > 1 + SK_MaxU16 ||  // invalid glyphCount
        glyphID >= glyphCount) {  // invalid glyph
        return nullptr;
    }
    sk_sp<const SkAdvancedTypefaceMetrics> fontMetrics;
    sk_sp<SkPDFDict> relatedFontDescriptor;
    if (relatedFont) {
        fontMetrics = relatedFont->refFontInfo();
        relatedFontDescriptor = relatedFont->refFontDescriptor();

        // This only is to catch callers who pass invalid glyph ids.
        // If glyph id is invalid, then we will create duplicate entries
        // for TrueType fonts.
        SkDEBUGCODE(SkAdvancedTypefaceMetrics::FontType fontType = relatedFont->getType());
        SkASSERT(fontType != SkAdvancedTypefaceMetrics::kType1CID_Font);
        SkASSERT(fontType != SkAdvancedTypefaceMetrics::kTrueType_Font);
    } else {
        SkTypeface::PerGlyphInfo info = SkTypeface::kGlyphNames_PerGlyphInfo |
                                        SkTypeface::kToUnicode_PerGlyphInfo;
        fontMetrics.reset(
            typeface->getAdvancedTypefaceMetrics(info, nullptr, 0));
    }

    SkAdvancedTypefaceMetrics::FontType type =
        fontMetrics ? fontMetrics->fType : SkAdvancedTypefaceMetrics::kOther_Font;
    if (fontMetrics &&
        SkToBool(fontMetrics->fFlags &
                 SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag)) {
        // force Type3 fallback.
        type = SkAdvancedTypefaceMetrics::kOther_Font;
    }

    sk_sp<SkPDFFont> font;
    switch (type) {
        case SkAdvancedTypefaceMetrics::kType1CID_Font:
        case SkAdvancedTypefaceMetrics::kTrueType_Font:
            SkASSERT(relatedFontDescriptor == nullptr);
            SkASSERT(fontMetrics != nullptr);
            font = sk_make_sp<SkPDFType0Font>(std::move(fontMetrics),
                                              std::move(typeface),
                                              type);
            break;
        case SkAdvancedTypefaceMetrics::kType1_Font:
            SkASSERT(fontMetrics != nullptr);
            font = sk_make_sp<SkPDFType1Font>(std::move(fontMetrics),
                                              std::move(typeface),
                                              glyphID,
                                              std::move(relatedFontDescriptor));
            break;
        case SkAdvancedTypefaceMetrics::kCFF_Font:
            SkASSERT(fontMetrics != nullptr);
            // fallthrough
        case SkAdvancedTypefaceMetrics::kOther_Font:
            font = sk_make_sp<SkPDFType3Font>(std::move(fontMetrics),
                                              std::move(typeface),
                                              type,
                                              glyphID);
            break;
        default:
            SkDEBUGFAIL("invalid SkAdvancedTypefaceMetrics::FontType");
            return nullptr;
    }
    // When firstGlyphID==0, SkFont::IsMatch() matches all glyphs in font.
    SkGlyphID firstGlyphID = font->multiByteGlyphs() ? 0 : font->fFirstGlyphID;
    // TODO(halcanary) Make SkCanon::addFont take sk_sp<SkPDFFont>.
    canon->addFont(font.get(), fontID, firstGlyphID);
    return font.release();  // TODO(halcanary) return sk_sp<SkPDFFont>.
}

sk_sp<SkPDFObject> SkPDFFont::getFontSubset(const SkPDFGlyphSet*) {
    return nullptr;  // Default: no support.
}

// TODO: take a sk_sp<SkAdvancedTypefaceMetrics> and sk_sp<SkTypeface>
SkPDFFont::SkPDFFont(sk_sp<const SkAdvancedTypefaceMetrics> info,
                     sk_sp<SkTypeface> typeface,
                     sk_sp<SkPDFDict> relatedFontDescriptor,
                     SkAdvancedTypefaceMetrics::FontType fontType,
                     bool multiByteGlyphs)
    : SkPDFDict("Font")
    , fTypeface(std::move(typeface))
    , fFontInfo(std::move(info))
    , fDescriptor(std::move(relatedFontDescriptor))
    , fFirstGlyphID(1)
    , fFontType(fontType)
    , fMultiByteGlyphs(multiByteGlyphs) {
    SkASSERT(fTypeface);
    fLastGlyphID = fFontInfo ? fFontInfo->fLastGlyphID : 0;
    if (0 == fLastGlyphID) {
        fLastGlyphID = SkToU16(fTypeface->countGlyphs() - 1);
    }
}

void SkPDFFont::setFontInfo(sk_sp<const SkAdvancedTypefaceMetrics> info) {
    if (info) {
        fFontInfo = std::move(info);
    }
}

void SkPDFFont::setLastGlyphID(uint16_t glyphID) {
    fLastGlyphID = glyphID;
}

void SkPDFFont::setFontDescriptor(sk_sp<SkPDFDict> descriptor) {
    fDescriptor = std::move(descriptor);
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

void SkPDFFont::adjustGlyphRangeForSingleByteEncoding(uint16_t glyphID) {
    // Single byte glyph encoding supports a max of 255 glyphs.
    fFirstGlyphID = glyphID - (glyphID - 1) % 255;
    if (fLastGlyphID > fFirstGlyphID + 255 - 1) {
        fLastGlyphID = fFirstGlyphID + 255 - 1;
    }
}

void SkPDFFont::populateToUnicodeTable(const SkPDFGlyphSet* subset) {
    if (fFontInfo == nullptr || fFontInfo->fGlyphToUnicode.begin() == nullptr) {
        return;
    }
    this->insertObjRef("ToUnicode",
                       SkPDFMakeToUnicodeCmap(fFontInfo->fGlyphToUnicode,
                                              subset,
                                              multiByteGlyphs(),
                                              firstGlyphID(),
                                              lastGlyphID()));
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType0Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType0Font::SkPDFType0Font(sk_sp<const SkAdvancedTypefaceMetrics> info,
                               sk_sp<SkTypeface> typeface,
                               SkAdvancedTypefaceMetrics::FontType fontType)
    : SkPDFFont(std::move(info), std::move(typeface), nullptr, fontType, true) {
    SkDEBUGCODE(fPopulated = false);
    if (!canSubset()) {
        this->populate(nullptr);
    }
}

SkPDFType0Font::~SkPDFType0Font() {}

sk_sp<SkPDFObject>  SkPDFType0Font::getFontSubset(const SkPDFGlyphSet* subset) {
    if (!canSubset()) {
        return nullptr;
    }
    auto newSubset = sk_make_sp<SkPDFType0Font>(refFontInfo(), refTypeface(), getType());
    newSubset->populate(subset);
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

bool SkPDFType0Font::populate(const SkPDFGlyphSet* subset) {
    SkASSERT(this->canEmbed());
    SkASSERT(this->getFontInfo());
    const SkAdvancedTypefaceMetrics& metrics = *(this->getFontInfo());
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
            if (this->canSubset() && subset) {
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

    uint16_t emSize = this->getFontInfo()->fEmSize;
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
    this->populateToUnicodeTable(subset);
    SkDEBUGCODE(fPopulated = true);
    return true;
}

sk_sp<const SkAdvancedTypefaceMetrics> SkPDFFont::GetFontMetricsWithToUnicode(
        SkTypeface* typeface, uint32_t* glyphs, uint32_t glyphsCount) {
    return sk_sp<const SkAdvancedTypefaceMetrics>(
            typeface->getAdvancedTypefaceMetrics(
                    SkTypeface::kToUnicode_PerGlyphInfo, glyphs, glyphsCount));
}


///////////////////////////////////////////////////////////////////////////////
// class SkPDFType1Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType1Font::SkPDFType1Font(sk_sp<const SkAdvancedTypefaceMetrics> info,
                               sk_sp<SkTypeface> typeface,
                               uint16_t glyphID,
                               sk_sp<SkPDFDict> relatedFontDescriptor)
    : SkPDFFont(std::move(info),
                std::move(typeface),
                std::move(relatedFontDescriptor),
                SkAdvancedTypefaceMetrics::kType1_Font,
                /* multiByteGlyphs = */ false) {
    this->populate(glyphID);  // TODO(halcanary): subset this.
}

SkPDFType1Font::~SkPDFType1Font() {}

bool SkPDFType1Font::addFontDescriptor(int16_t defaultWidth) {
    if (sk_sp<SkPDFDict> descriptor = this->refFontDescriptor()) {
        this->insertObjRef("FontDescriptor", std::move(descriptor));
        return true;
    }

    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    setFontDescriptor(descriptor);

    int ttcIndex;
    size_t header SK_INIT_TO_AVOID_WARNING;
    size_t data SK_INIT_TO_AVOID_WARNING;
    size_t trailer SK_INIT_TO_AVOID_WARNING;
    std::unique_ptr<SkStreamAsset> rawFontData(typeface()->openStream(&ttcIndex));
    sk_sp<SkData> fontData = SkPDFConvertType1FontStream(std::move(rawFontData),
                                                         &header, &data, &trailer);
    if (!fontData) {
        return false;
    }
    SkASSERT(this->canEmbed());
    auto fontStream = sk_make_sp<SkPDFStream>(std::move(fontData));
    fontStream->dict()->insertInt("Length1", header);
    fontStream->dict()->insertInt("Length2", data);
    fontStream->dict()->insertInt("Length3", trailer);
    descriptor->insertObjRef("FontFile", std::move(fontStream));

    SkASSERT(this->getFontInfo());
    add_common_font_descriptor_entries(descriptor.get(),
                                       *this->getFontInfo(),
                                       defaultWidth);
    this->insertObjRef("FontDescriptor", std::move(descriptor));
    return true;
}

bool SkPDFType1Font::populate(int16_t glyphID) {
    this->insertName("Subtype", "Type1");
    this->insertName("BaseFont", this->getFontInfo()->fFontName);
    adjustGlyphRangeForSingleByteEncoding(glyphID);
    SkGlyphID firstGlyphID = this->firstGlyphID();
    SkGlyphID lastGlyphID = this->lastGlyphID();

    // glyphCount not including glyph 0
    unsigned glyphCount = 1 + lastGlyphID - firstGlyphID;
    SkASSERT(glyphCount > 0 && glyphCount <= 255);
    this->insertInt("FirstChar", (size_t)0);
    this->insertInt("LastChar", (size_t)glyphCount);
    {
        SkAutoGlyphCache glyphCache = vector_cache(this->typeface());
        auto widths = sk_make_sp<SkPDFArray>();
        SkScalar advance = glyphCache->getGlyphIDAdvance(0).fAdvanceX;
        const uint16_t emSize = this->getFontInfo()->fEmSize;
        widths->appendScalar(from_font_units(advance, emSize));
        for (unsigned gID = firstGlyphID; gID <= lastGlyphID; gID++) {
            advance = glyphCache->getGlyphIDAdvance(gID).fAdvanceX;
            widths->appendScalar(from_font_units(advance, emSize));
        }
        this->insertObject("Widths", std::move(widths));
    }
    if (!addFontDescriptor(0)) {
        return false;
    }
    auto encDiffs = sk_make_sp<SkPDFArray>();
    encDiffs->reserve(lastGlyphID - firstGlyphID + 3);
    encDiffs->appendInt(0);
    const SkTArray<SkString>& glyphNames = this->getFontInfo()->fGlyphNames;
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
    this->insertObject("Encoding", std::move(encoding));
    return true;
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

static void add_type3_font_info(SkPDFDict* font,
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
    sk_sp<const SkAdvancedTypefaceMetrics> metrics;
    if (subset) {
        SkTDArray<uint32_t> subsetList;
        for (SkGlyphID gID : SingleByteGlyphIdIterator(firstGlyphID, lastGlyphID)) {
            if (gID == 0 || subset->has(gID)) {  // Always include glyph 0.
                subsetList.push(0);
            }
        }
        subset->exportTo(&subsetList);
        metrics = SkPDFFont::GetFontMetricsWithToUnicode(typeface, subsetList.begin(),
                                                         subsetList.count());
    } else {
        metrics = SkPDFFont::GetFontMetricsWithToUnicode(typeface, nullptr, 0);
    }
    if (metrics) {
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

SkPDFType3Font::SkPDFType3Font(sk_sp<const SkAdvancedTypefaceMetrics> info,
                               sk_sp<SkTypeface> typeface,
                               SkAdvancedTypefaceMetrics::FontType fontType,
                               uint16_t glyphID)
    : SkPDFFont(std::move(info), std::move(typeface), nullptr,
                fontType, /* multiByteGlyphs = */ false) {
    // If fLastGlyphID isn't set (because there is not fFontInfo), look it up.
    this->setLastGlyphID(SkToU16(this->typeface()->countGlyphs() - 1));
    this->adjustGlyphRangeForSingleByteEncoding(glyphID);
}

sk_sp<SkPDFObject> SkPDFType3Font::getFontSubset(const SkPDFGlyphSet* usage) {
    // All fonts are subset before serialization.
    // TODO(halcanary): all fonts should follow this pattern.
    auto font = sk_make_sp<SkPDFDict>("Font");
    const SkAdvancedTypefaceMetrics* info = this->getFontInfo();
    uint16_t emSize = info && info->fEmSize > 0 ? info->fEmSize : 1000;
    add_type3_font_info(font.get(), this->typeface(), (SkScalar)emSize, usage,
                        this->firstGlyphID(), this->lastGlyphID());
    return font;
}


////////////////////////////////////////////////////////////////////////////////

SkPDFFont::Match SkPDFFont::IsMatch(SkPDFFont* existingFont,
                                    uint32_t existingFontID,
                                    uint16_t existingGlyphID,
                                    uint32_t searchFontID,
                                    uint16_t searchGlyphID) {
    if (existingFontID != searchFontID) {
        return SkPDFFont::kNot_Match;
    }
    if (existingGlyphID == 0 || searchGlyphID == 0) {
        return SkPDFFont::kExact_Match;
    }
    if (existingFont != nullptr) {
        return (existingFont->fFirstGlyphID <= searchGlyphID &&
                searchGlyphID <= existingFont->fLastGlyphID)
                       ? SkPDFFont::kExact_Match
                       : SkPDFFont::kRelated_Match;
    }
    return (existingGlyphID == searchGlyphID) ? SkPDFFont::kExact_Match
                                              : SkPDFFont::kRelated_Match;
}

//  Since getAdvancedTypefaceMetrics is expensive, cache the result.
bool SkPDFFont::CanEmbedTypeface(SkTypeface* typeface, SkPDFCanon* canon) {
    SkFontID id = SkTypeface::UniqueID(typeface);
    if (bool* value = canon->fCanEmbedTypeface.find(id)) {
        return *value;
    }
    SkAutoResolveDefaultTypeface face(typeface);
    bool canEmbed = true;
    sk_sp<const SkAdvancedTypefaceMetrics> fontMetrics(
            face->getAdvancedTypefaceMetrics(
                    SkTypeface::kNo_PerGlyphInfo, nullptr, 0));
    if (fontMetrics) {
        canEmbed = !SkToBool(
                fontMetrics->fFlags &
                SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
    }
    return *canon->fCanEmbedTypeface.set(id, canEmbed);
}

void SkPDFFont::drop() {
    fTypeface = nullptr;
    fFontInfo = nullptr;
    fDescriptor = nullptr;
    this->SkPDFDict::drop();
}
