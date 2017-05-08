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
#include "SkTypes.h"
#include "SkUtils.h"

#ifdef SK_PDF_USE_SFNTLY
    #include "sample/chromium/font_subsetter.h"
#endif

SkAutoGlyphCache SkPDFFont::MakeVectorCache(SkTypeface* face, int* size) {
    SkPaint tmpPaint;
    tmpPaint.setHinting(SkPaint::kNo_Hinting);
    tmpPaint.setTypeface(sk_ref_sp(face));
    int unitsPerEm = face->getUnitsPerEm();
    if (unitsPerEm <= 0) {
        unitsPerEm = 1024;
    }
    if (size) {
        *size = unitsPerEm;
    }
    tmpPaint.setTextSize((SkScalar)unitsPerEm);
    const SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    SkAutoGlyphCache glyphCache(tmpPaint, &props, nullptr);
    SkASSERT(glyphCache.get());
    return glyphCache;
}

namespace {
// PDF's notion of symbolic vs non-symbolic is related to the character set, not
// symbols vs. characters.  Rarely is a font the right character set to call it
// non-symbolic, so always call it symbolic.  (PDF 1.4 spec, section 5.7.1)
static const int32_t kPdfSymbolic = 4;

struct SkPDFType0Font final : public SkPDFFont {
    SkPDFType0Font(SkPDFFont::Info, const SkAdvancedTypefaceMetrics&);
    ~SkPDFType0Font() override;
    void getFontSubset(SkPDFCanon*) override;
#ifdef SK_DEBUG
    void emitObject(SkWStream*, const SkPDFObjNumMap&) const override;
    bool fPopulated;
#endif
    typedef SkPDFDict INHERITED;
};

struct SkPDFType1Font final : public SkPDFFont {
    SkPDFType1Font(SkPDFFont::Info, const SkAdvancedTypefaceMetrics&, SkPDFCanon*);
    ~SkPDFType1Font() override {}
    void getFontSubset(SkPDFCanon*) override {} // TODO(halcanary): implement
};

struct SkPDFType3Font final : public SkPDFFont {
    SkPDFType3Font(SkPDFFont::Info, const SkAdvancedTypefaceMetrics&);
    ~SkPDFType3Font() override {}
    void getFontSubset(SkPDFCanon*) override;
};

///////////////////////////////////////////////////////////////////////////////
// File-Local Functions
///////////////////////////////////////////////////////////////////////////////

// scale from em-units to base-1000, returning as a SkScalar
SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    if (emSize == 1000) {
        return scaled;
    } else {
        return scaled * 1000 / emSize;
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

static bool can_embed(const SkAdvancedTypefaceMetrics& metrics) {
    return !SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
}

const SkAdvancedTypefaceMetrics* SkPDFFont::GetMetrics(SkTypeface* typeface,
                                                       SkPDFCanon* canon) {
    SkASSERT(typeface);
    SkFontID id = typeface->uniqueID();
    if (SkAdvancedTypefaceMetrics** ptr = canon->fTypefaceMetrics.find(id)) {
        return *ptr;
    }
    int count = typeface->countGlyphs();
    if (count <= 0 || count > 1 + SK_MaxU16) {
        // Cache nullptr to skip this check.  Use SkSafeUnref().
        canon->fTypefaceMetrics.set(id, nullptr);
        return nullptr;
    }
    sk_sp<SkAdvancedTypefaceMetrics> metrics(
            typeface->getAdvancedTypefaceMetrics(
                    SkTypeface::kGlyphNames_PerGlyphInfo | SkTypeface::kToUnicode_PerGlyphInfo,
                    nullptr, 0));
    if (!metrics) {
        metrics = sk_make_sp<SkAdvancedTypefaceMetrics>();
    }
    return *canon->fTypefaceMetrics.set(id, metrics.release());
}

SkAdvancedTypefaceMetrics::FontType SkPDFFont::FontType(const SkAdvancedTypefaceMetrics& metrics) {
    if (SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kMultiMaster_FontFlag) ||
        SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag)) {
        // force Type3 fallback.
        return SkAdvancedTypefaceMetrics::kOther_Font;
    }
    return metrics.fType;
}

static SkGlyphID first_nonzero_glyph_for_single_byte_encoding(SkGlyphID gid) {
    return gid != 0 ? gid - (gid - 1) % 255 : 1;
}

SkPDFFont* SkPDFFont::GetFontResource(SkPDFCanon* canon,
                                      SkTypeface* face,
                                      SkGlyphID glyphID) {
    SkASSERT(canon);
    SkASSERT(face);  // All SkPDFDevice::internalDrawText ensures this.
    const SkAdvancedTypefaceMetrics* fontMetrics = SkPDFFont::GetMetrics(face, canon);
    SkASSERT(fontMetrics);  // SkPDFDevice::internalDrawText ensures the typeface is good.
                            // GetMetrics only returns null to signify a bad typeface.
    const SkAdvancedTypefaceMetrics& metrics = *fontMetrics;
    SkAdvancedTypefaceMetrics::FontType type = SkPDFFont::FontType(metrics);
    bool multibyte = SkPDFFont::IsMultiByte(type);
    SkGlyphID subsetCode = multibyte ? 0 : first_nonzero_glyph_for_single_byte_encoding(glyphID);
    uint64_t fontID = (SkTypeface::UniqueID(face) << 16) | subsetCode;

    if (SkPDFFont** found = canon->fFontMap.find(fontID)) {
        SkPDFFont* foundFont = *found;
        SkASSERT(foundFont && multibyte == foundFont->multiByteGlyphs());
        return SkRef(foundFont);
    }

    sk_sp<SkTypeface> typeface(sk_ref_sp(face));
    SkASSERT(typeface);

    SkGlyphID lastGlyph = SkToU16(typeface->countGlyphs() - 1);

    // should be caught by SkPDFDevice::internalDrawText
    SkASSERT(glyphID <= lastGlyph);

    SkGlyphID firstNonZeroGlyph;
    if (multibyte) {
        firstNonZeroGlyph = 1;
    } else {
        firstNonZeroGlyph = subsetCode;
        lastGlyph = SkToU16(SkTMin<int>((int)lastGlyph, 254 + (int)subsetCode));
    }
    SkPDFFont::Info info = {std::move(typeface), firstNonZeroGlyph, lastGlyph, type};
    sk_sp<SkPDFFont> font;
    switch (type) {
        case SkAdvancedTypefaceMetrics::kType1CID_Font:
        case SkAdvancedTypefaceMetrics::kTrueType_Font:
            SkASSERT(multibyte);
            font = sk_make_sp<SkPDFType0Font>(std::move(info), metrics);
            break;
        case SkAdvancedTypefaceMetrics::kType1_Font:
            SkASSERT(!multibyte);
            font = sk_make_sp<SkPDFType1Font>(std::move(info), metrics, canon);
            break;
        default:
            SkASSERT(!multibyte);
            // Type3 is our fallback font.
            font = sk_make_sp<SkPDFType3Font>(std::move(info), metrics);
            break;
    }
    canon->fFontMap.set(fontID, SkRef(font.get()));
    return font.release();  // TODO(halcanary) return sk_sp<SkPDFFont>.
}

SkPDFFont::SkPDFFont(SkPDFFont::Info info)
    : SkPDFDict("Font")
    , fTypeface(std::move(info.fTypeface))
    , fGlyphUsage(info.fLastGlyphID + 1)  // TODO(halcanary): Adjust mapping?
    , fFirstGlyphID(info.fFirstGlyphID)
    , fLastGlyphID(info.fLastGlyphID)
    , fFontType(info.fFontType) {
    SkASSERT(fTypeface);
}

static void  add_common_font_descriptor_entries(SkPDFDict* descriptor,
                                                const SkAdvancedTypefaceMetrics& metrics,
                                                uint16_t emSize,
                                                int16_t defaultWidth) {
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
            "FontBBox", makeFontBBox(metrics.fBBox, emSize));
    if (defaultWidth > 0) {
        descriptor->insertScalar("MissingWidth",
                scaleFromFontUnits(defaultWidth, emSize));
    }
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType0Font
///////////////////////////////////////////////////////////////////////////////

SkPDFType0Font::SkPDFType0Font(
        SkPDFFont::Info info,
        const SkAdvancedTypefaceMetrics& metrics)
    : SkPDFFont(std::move(info)) {
    SkDEBUGCODE(fPopulated = false);
}

SkPDFType0Font::~SkPDFType0Font() {}


#ifdef SK_DEBUG
void SkPDFType0Font::emitObject(SkWStream* stream,
                                const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(fPopulated);
    return INHERITED::emitObject(stream, objNumMap);
}
#endif

#ifdef SK_PDF_USE_SFNTLY
// if possible, make no copy.
static sk_sp<SkData> stream_to_data(std::unique_ptr<SkStreamAsset> stream) {
    SkASSERT(stream);
    (void)stream->rewind();
    SkASSERT(stream->hasLength());
    size_t size = stream->getLength();
    if (const void* base = stream->getMemoryBase()) {
        SkData::ReleaseProc proc =
            [](const void*, void* ctx) { delete (SkStreamAsset*)ctx; };
        return SkData::MakeWithProc(base, size, proc, stream.release());
    }
    return SkData::MakeFromStream(stream.get(), size);
}

static sk_sp<SkPDFStream> get_subset_font_stream(
        std::unique_ptr<SkStreamAsset> fontAsset,
        const SkBitSet& glyphUsage,
        const char* fontName,
        int ttcIndex) {
    // Generate glyph id array in format needed by sfntly.
    // TODO(halcanary): sfntly should take a more compact format.
    SkTDArray<unsigned> subset;
    if (!glyphUsage.has(0)) {
        subset.push(0);  // Always include glyph 0.
    }
    glyphUsage.exportTo(&subset);

    unsigned char* subsetFont{nullptr};
    sk_sp<SkData> fontData(stream_to_data(std::move(fontAsset)));
#if defined(GOOGLE3)
    // TODO(halcanary): update GOOGLE3 to newest version of Sfntly.
    (void)ttcIndex;
    int subsetFontSize = SfntlyWrapper::SubsetFont(fontName,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.begin(),
                                                   subset.count(),
                                                   &subsetFont);
#else
    (void)fontName;
    int subsetFontSize = SfntlyWrapper::SubsetFont(ttcIndex,
                                                   fontData->bytes(),
                                                   fontData->size(),
                                                   subset.begin(),
                                                   subset.count(),
                                                   &subsetFont);
#endif
    fontData.reset();
    subset.reset();
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
#endif  // SK_PDF_USE_SFNTLY

void SkPDFType0Font::getFontSubset(SkPDFCanon* canon) {
    const SkAdvancedTypefaceMetrics* metricsPtr =
        SkPDFFont::GetMetrics(this->typeface(), canon);
    SkASSERT(metricsPtr);
    if (!metricsPtr) { return; }
    const SkAdvancedTypefaceMetrics& metrics = *metricsPtr;
    SkASSERT(can_embed(metrics));
    SkAdvancedTypefaceMetrics::FontType type = this->getType();
    SkTypeface* face = this->typeface();
    SkASSERT(face);

    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    uint16_t emSize = SkToU16(this->typeface()->getUnitsPerEm());
    add_common_font_descriptor_entries(descriptor.get(), metrics, emSize , 0);

    int ttcIndex;
    std::unique_ptr<SkStreamAsset> fontAsset(face->openStream(&ttcIndex));
    size_t fontSize = fontAsset ? fontAsset->getLength() : 0;
    if (0 == fontSize) {
        SkDebugf("Error: (SkTypeface)(%p)::openStream() returned "
                 "empty stream (%p) when identified as kType1CID_Font "
                 "or kTrueType_Font.\n", face, fontAsset.get());
    } else {
        switch (type) {
            case SkAdvancedTypefaceMetrics::kTrueType_Font: {
                #ifdef SK_PDF_USE_SFNTLY
                if (!SkToBool(metrics.fFlags &
                              SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag)) {
                    sk_sp<SkPDFStream> subsetStream = get_subset_font_stream(
                            std::move(fontAsset), this->glyphUsage(),
                            metrics.fFontName.c_str(), ttcIndex);
                    if (subsetStream) {
                        descriptor->insertObjRef("FontFile2", std::move(subsetStream));
                        break;
                    }
                    // If subsetting fails, fall back to original font data.
                    fontAsset.reset(face->openStream(&ttcIndex));
                    SkASSERT(fontAsset);
                    SkASSERT(fontAsset->getLength() == fontSize);
                    if (!fontAsset || fontAsset->getLength() == 0) { break; }
                }
                #endif  // SK_PDF_USE_SFNTLY
                auto fontStream = sk_make_sp<SkPDFSharedStream>(std::move(fontAsset));
                fontStream->dict()->insertInt("Length1", fontSize);
                descriptor->insertObjRef("FontFile2", std::move(fontStream));
                break;
            }
            case SkAdvancedTypefaceMetrics::kType1CID_Font: {
                auto fontStream = sk_make_sp<SkPDFSharedStream>(std::move(fontAsset));
                fontStream->dict()->insertName("Subtype", "CIDFontType0C");
                descriptor->insertObjRef("FontFile3", std::move(fontStream));
                break;
            }
            default:
                SkASSERT(false);
        }
    }

    auto newCIDFont = sk_make_sp<SkPDFDict>("Font");
    newCIDFont->insertObjRef("FontDescriptor", std::move(descriptor));
    newCIDFont->insertName("BaseFont", metrics.fFontName);

    switch (type) {
        case SkAdvancedTypefaceMetrics::kType1CID_Font:
            newCIDFont->insertName("Subtype", "CIDFontType0");
            break;
        case SkAdvancedTypefaceMetrics::kTrueType_Font:
            newCIDFont->insertName("Subtype", "CIDFontType2");
            newCIDFont->insertName("CIDToGIDMap", "Identity");
            break;
        default:
            SkASSERT(false);
    }
    auto sysInfo = sk_make_sp<SkPDFDict>();
    sysInfo->insertString("Registry", "Adobe");
    sysInfo->insertString("Ordering", "Identity");
    sysInfo->insertInt("Supplement", 0);
    newCIDFont->insertObject("CIDSystemInfo", std::move(sysInfo));

    int16_t defaultWidth = 0;
    {
        int emSize;
        SkAutoGlyphCache glyphCache = SkPDFFont::MakeVectorCache(face, &emSize);
        sk_sp<SkPDFArray> widths = SkPDFMakeCIDGlyphWidthsArray(
                glyphCache.get(), &this->glyphUsage(), SkToS16(emSize), &defaultWidth);
        if (widths && widths->size() > 0) {
            newCIDFont->insertObject("W", std::move(widths));
        }
        newCIDFont->insertScalar(
                "DW", scaleFromFontUnits(defaultWidth, SkToS16(emSize)));
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
                                                  &this->glyphUsage(),
                                                  multiByteGlyphs(),
                                                  firstGlyphID(),
                                                  lastGlyphID()));
    }
    SkDEBUGCODE(fPopulated = true);
    return;
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFType1Font
///////////////////////////////////////////////////////////////////////////////

static sk_sp<SkPDFDict> make_type1_font_descriptor(
        SkTypeface* typeface,
        const SkAdvancedTypefaceMetrics& info) {
    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    uint16_t emSize = SkToU16(typeface->getUnitsPerEm());
    add_common_font_descriptor_entries(descriptor.get(), info, emSize, 0);
    if (!can_embed(info)) {
        return descriptor;
    }
    int ttcIndex;
    size_t header SK_INIT_TO_AVOID_WARNING;
    size_t data SK_INIT_TO_AVOID_WARNING;
    size_t trailer SK_INIT_TO_AVOID_WARNING;
    std::unique_ptr<SkStreamAsset> rawFontData(typeface->openStream(&ttcIndex));
    sk_sp<SkData> fontData = SkPDFConvertType1FontStream(std::move(rawFontData),
                                                         &header, &data, &trailer);
    if (fontData) {
        auto fontStream = sk_make_sp<SkPDFStream>(std::move(fontData));
        fontStream->dict()->insertInt("Length1", header);
        fontStream->dict()->insertInt("Length2", data);
        fontStream->dict()->insertInt("Length3", trailer);
        descriptor->insertObjRef("FontFile", std::move(fontStream));
    }
    return descriptor;
}

static void populate_type_1_font(SkPDFDict* font,
                                 const SkAdvancedTypefaceMetrics& info,
                                 SkTypeface* typeface,
                                 SkGlyphID firstGlyphID,
                                 SkGlyphID lastGlyphID) {
    font->insertName("Subtype", "Type1");
    font->insertName("BaseFont", info.fFontName);

    // glyphCount not including glyph 0
    unsigned glyphCount = 1 + lastGlyphID - firstGlyphID;
    SkASSERT(glyphCount > 0 && glyphCount <= 255);
    font->insertInt("FirstChar", (size_t)0);
    font->insertInt("LastChar", (size_t)glyphCount);
    {
        int emSize;
        SkAutoGlyphCache glyphCache = SkPDFFont::MakeVectorCache(typeface, &emSize);
        auto widths = sk_make_sp<SkPDFArray>();
        SkScalar advance = glyphCache->getGlyphIDAdvance(0).fAdvanceX;
        widths->appendScalar(from_font_units(advance, SkToU16(emSize)));
        for (unsigned gID = firstGlyphID; gID <= lastGlyphID; gID++) {
            advance = glyphCache->getGlyphIDAdvance(gID).fAdvanceX;
            widths->appendScalar(from_font_units(advance, SkToU16(emSize)));
        }
        font->insertObject("Widths", std::move(widths));
    }
    auto encDiffs = sk_make_sp<SkPDFArray>();
    encDiffs->reserve(lastGlyphID - firstGlyphID + 3);
    encDiffs->appendInt(0);
    const SkTArray<SkString>& glyphNames = info.fGlyphNames;
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

SkPDFType1Font::SkPDFType1Font(SkPDFFont::Info info,
                               const SkAdvancedTypefaceMetrics& metrics,
                               SkPDFCanon* canon)
    : SkPDFFont(std::move(info))
{
    SkFontID fontID = this->typeface()->uniqueID();
    sk_sp<SkPDFDict> fontDescriptor;
    if (SkPDFDict** ptr = canon->fFontDescriptors.find(fontID)) {
        fontDescriptor = sk_ref_sp(*ptr);
    } else {
        fontDescriptor = make_type1_font_descriptor(this->typeface(), metrics);
        canon->fFontDescriptors.set(fontID, SkRef(fontDescriptor.get()));
    }
    this->insertObjRef("FontDescriptor", std::move(fontDescriptor));
    // TODO(halcanary): subset this (advances and names).
    populate_type_1_font(this, metrics, this->typeface(),
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
                                const SkBitSet& subset,
                                SkGlyphID firstGlyphID,
                                SkGlyphID lastGlyphID) {
    const SkAdvancedTypefaceMetrics* metrics = SkPDFFont::GetMetrics(typeface, canon);
    SkASSERT(lastGlyphID >= firstGlyphID);
    // Remove unused glyphs at the end of the range.
    // Keep the lastGlyphID >= firstGlyphID invariant true.
    while (lastGlyphID > firstGlyphID && !subset.has(lastGlyphID)) {
        --lastGlyphID;
    }
    int unitsPerEm;
    SkAutoGlyphCache cache = SkPDFFont::MakeVectorCache(typeface, &unitsPerEm);
    SkScalar emSize = (SkScalar)unitsPerEm;
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
        bool skipGlyph = gID != 0 && !subset.has(gID);
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
    if (metrics && metrics->fGlyphToUnicode.count() > 0) {
        font->insertObjRef("ToUnicode",
                           SkPDFMakeToUnicodeCmap(metrics->fGlyphToUnicode,
                                                  &subset,
                                                  false,
                                                  firstGlyphID,
                                                  lastGlyphID));
    }
    auto descriptor = sk_make_sp<SkPDFDict>("FontDescriptor");
    int32_t fontDescriptorFlags = kPdfSymbolic;
    if (metrics) {
        // Type3 FontDescriptor does not require all the same fields.
        descriptor->insertName("FontName", metrics->fFontName);
        descriptor->insertInt("ItalicAngle", metrics->fItalicAngle);
        fontDescriptorFlags |= (int32_t)metrics->fStyle;
    }
    descriptor->insertInt("Flags", fontDescriptorFlags);
    font->insertObjRef("FontDescriptor", std::move(descriptor));
    font->insertObject("Widths", std::move(widthArray));
    font->insertObject("Encoding", std::move(encoding));
    font->insertObject("CharProcs", std::move(charProcs));
}

SkPDFType3Font::SkPDFType3Font(SkPDFFont::Info info,
                               const SkAdvancedTypefaceMetrics& metrics)
    : SkPDFFont(std::move(info)) {}

void SkPDFType3Font::getFontSubset(SkPDFCanon* canon) {
    add_type3_font_info(canon, this, this->typeface(), this->glyphUsage(),
                        this->firstGlyphID(), this->lastGlyphID());
}

////////////////////////////////////////////////////////////////////////////////

bool SkPDFFont::CanEmbedTypeface(SkTypeface* typeface, SkPDFCanon* canon) {
    const SkAdvancedTypefaceMetrics* metrics = SkPDFFont::GetMetrics(typeface, canon);
    return metrics && can_embed(*metrics);
}

void SkPDFFont::drop() {
    fTypeface = nullptr;
    fGlyphUsage.~SkBitSet();
    new (&fGlyphUsage) SkBitSet(0);
    this->SkPDFDict::drop();
}
