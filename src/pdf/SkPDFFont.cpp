/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFFont.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkBitmaskEnum.h"
#include "src/core/SkDescriptor.h"
#include "src/core/SkDevice.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTHash.h"
#include "src/pdf/SkPDFBitmap.h"
#include "src/pdf/SkPDFDevice.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFFormXObject.h"
#include "src/pdf/SkPDFGraphicState.h"
#include "src/pdf/SkPDFMakeCIDGlyphWidthsArray.h"
#include "src/pdf/SkPDFMakeToUnicodeCmap.h"
#include "src/pdf/SkPDFSubsetFont.h"
#include "src/pdf/SkPDFType1Font.h"
#include "src/pdf/SkPDFUtils.h"

#include <limits.h>
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>

using namespace skia_private;

void SkPDFFont::GetType1GlyphNames(const SkTypeface& face, SkString* dst) {
    face.getPostScriptGlyphNames(dst);
}

namespace {
// PDF's notion of symbolic vs non-symbolic is related to the character set, not
// symbols vs. characters.  Rarely is a font the right character set to call it
// non-symbolic, so always call it symbolic.  (PDF 1.4 spec, section 5.7.1)
static const int32_t kPdfSymbolic = 4;

// scale from em-units to base-1000, returning as a SkScalar
inline SkScalar from_font_units(SkScalar scaled, uint16_t emSize) {
    return emSize == 1000 ? scaled : scaled * 1000 / emSize;
}

inline SkScalar scaleFromFontUnits(int16_t val, uint16_t emSize) {
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

}  // namespace

static bool scale_paint(SkPaint& paint, SkScalar fontToEMScale) {
    // What we really want here is a way ask the path effect or mask filter for a scaled
    // version of itself (if it is linearly scalable).

    if (SkMaskFilterBase* mfb = as_MFB(paint.getMaskFilter())) {
        SkMaskFilterBase::BlurRec blurRec;
        if (mfb->asABlur(&blurRec)) {
            // asABlur returns false if ignoring the CTM
            blurRec.fSigma *= fontToEMScale;
            paint.setMaskFilter(SkMaskFilter::MakeBlur(blurRec.fStyle, blurRec.fSigma, true));
        } else {
            return false;
        }
    }
    if (SkPathEffectBase* peb = as_PEB(paint.getPathEffect())) {
        AutoSTMalloc<4, SkScalar> intervals;
        SkPathEffectBase::DashInfo dashInfo(intervals, 4, 0);
        if (peb->asADash(&dashInfo) == SkPathEffectBase::DashType::kDash) {
            if (dashInfo.fCount > 4) {
                intervals.realloc(dashInfo.fCount);
                peb->asADash(&dashInfo);
            }
            for (int32_t i = 0; i < dashInfo.fCount; ++i) {
                dashInfo.fIntervals[i] *= fontToEMScale;
            }
            dashInfo.fPhase *= fontToEMScale;
            paint.setPathEffect(
                SkDashPathEffect::Make(dashInfo.fIntervals, dashInfo.fCount, dashInfo.fPhase));
        } else {
            return false;
        }
    }

    if (paint.getStyle() != SkPaint::kFill_Style && paint.getStrokeWidth() > 0) {
        paint.setStrokeMiter(paint.getStrokeMiter() * fontToEMScale);
        paint.setStrokeWidth(paint.getStrokeWidth() * fontToEMScale);
    }

    return true;
}


SkPDFStrikeSpec::SkPDFStrikeSpec(SkStrikeSpec strikeSpec, SkScalar em)
    : fStrikeSpec(std::move(strikeSpec))
    , fUnitsPerEM(em)
{}

sk_sp<SkPDFStrike> SkPDFStrike::Make(SkPDFDocument* doc, const SkFont& font, const SkPaint& paint) {
#ifdef SK_PDF_BITMAP_GLYPH_RASTER_SIZE
    static constexpr float kBitmapFontSize = SK_PDF_BITMAP_GLYPH_RASTER_SIZE;
#else
    static constexpr float kBitmapFontSize = 64;
#endif

    SkScalar unitsPerEm = static_cast<SkScalar>(font.getTypeface()->getUnitsPerEm());
    SkASSERT(0 < unitsPerEm);

    SkFont canonFont(font);
    canonFont.setBaselineSnap(false);  // canonicalize
    canonFont.setEdging(SkFont::Edging::kAntiAlias); // canonicalize
    canonFont.setEmbeddedBitmaps(false); // canonicalize
    //canonFont.setEmbolden(); // applied by scaler context, sets glyph path to modified
    canonFont.setForceAutoHinting(false); // canonicalize
    canonFont.setHinting(SkFontHinting::kNone); // canonicalize
    canonFont.setLinearMetrics(true); // canonicalize
    canonFont.setScaleX(1.0f); // original value applied by SkPDFDevice
    //canonFont.setSize(unitsPerEm);  // canonicalize below, adjusted by SkPDFDevice
    canonFont.setSkewX(0.0f); // original value applied by SkPDFDevice
    canonFont.setSubpixel(false); // canonicalize
    //canonFont.setTypeface();

    SkPaint pathPaint(paint);
    if (scale_paint(pathPaint, unitsPerEm / font.getSize())) {
        canonFont.setSize(unitsPerEm);
    } else {
        canonFont.setSize(font.getSize());
    }
    SkScalar pathStrikeEM = canonFont.getSize();
    SkStrikeSpec pathStrikeSpec = SkStrikeSpec::MakeWithNoDevice(canonFont, &pathPaint);

    if (sk_sp<SkPDFStrike>* strike = doc->fStrikes.find(pathStrikeSpec.descriptor())) {
        return *strike;
    }

    if (kBitmapFontSize <= 0) {
        // old code path compatibility
        sk_sp<SkPDFStrike> strike(new SkPDFStrike(SkPDFStrikeSpec(pathStrikeSpec, pathStrikeEM),
                                                  SkPDFStrikeSpec(pathStrikeSpec, pathStrikeEM),
                                                  pathPaint.getMaskFilter(), doc));
        doc->fStrikes.set(strike);
        return strike;
    }

    SkPaint imagePaint(paint);
    if (scale_paint(imagePaint, kBitmapFontSize / font.getSize())) {
        canonFont.setSize(kBitmapFontSize);
    } else {
        canonFont.setSize(font.getSize());
    }
    SkScalar imageStrikeEM = canonFont.getSize();
    SkStrikeSpec imageStrikeSpec = SkStrikeSpec::MakeWithNoDevice(canonFont, &imagePaint);

    sk_sp<SkPDFStrike> strike(new SkPDFStrike(SkPDFStrikeSpec(pathStrikeSpec, pathStrikeEM),
                                              SkPDFStrikeSpec(imageStrikeSpec, imageStrikeEM),
                                              pathPaint.getMaskFilter(), doc));
    doc->fStrikes.set(strike);
    return strike;

}

SkPDFStrike::SkPDFStrike(SkPDFStrikeSpec path, SkPDFStrikeSpec image, bool hasMaskFilter,
                         SkPDFDocument* doc)
    : fPath(std::move(path))
    , fImage(std::move(image))
    , fHasMaskFilter(hasMaskFilter)
    , fDoc(doc)
{
    SkASSERT(fDoc);
}

const SkDescriptor& SkPDFStrike::Traits::GetKey(const sk_sp<SkPDFStrike>& strike) {
    return strike->fPath.fStrikeSpec.descriptor();
}
uint32_t SkPDFStrike::Traits::Hash(const SkDescriptor& descriptor) {
    return descriptor.getChecksum();
}

///////////////////////////////////////////////////////////////////////////////
// class SkPDFFont
///////////////////////////////////////////////////////////////////////////////

/* Resources are canonicalized and uniqueified by pointer so there has to be
 * some additional state indicating which subset of the font is used.  It
 * must be maintained at the document granularity.
 */

SkPDFFont::~SkPDFFont() = default;

SkPDFFont::SkPDFFont(SkPDFFont&&) = default;

static bool can_embed(const SkAdvancedTypefaceMetrics& metrics) {
    return !SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag);
}

static bool can_subset(const SkAdvancedTypefaceMetrics& metrics) {
    return !SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kNotSubsettable_FontFlag);
}

const SkAdvancedTypefaceMetrics* SkPDFFont::GetMetrics(const SkTypeface& typeface,
                                                       SkPDFDocument* canon) {
    SkTypefaceID id = typeface.uniqueID();
    if (std::unique_ptr<SkAdvancedTypefaceMetrics>* ptr = canon->fTypefaceMetrics.find(id)) {
        return ptr->get();  // canon retains ownership.
    }

    int count = typeface.countGlyphs();
    if (count <= 0 || count > 1 + SkTo<int>(UINT16_MAX)) {
        // Cache nullptr to skip this check.  Use SkSafeUnref().
        canon->fTypefaceMetrics.set(id, nullptr);
        return nullptr;
    }

    std::unique_ptr<SkAdvancedTypefaceMetrics> metrics = typeface.getAdvancedMetrics();
    if (!metrics) {
        metrics = std::make_unique<SkAdvancedTypefaceMetrics>();
    }
    if (0 == metrics->fStemV || 0 == metrics->fCapHeight) {
        SkFont font;
        font.setHinting(SkFontHinting::kNone);
        font.setTypeface(sk_ref_sp(&typeface));
        font.setSize(1000);  // glyph coordinate system
        if (0 == metrics->fStemV) {
            // Figure out a good guess for StemV - Min width of i, I, !, 1.
            // This probably isn't very good with an italic font.
            int16_t stemV = SHRT_MAX;
            for (char c : {'i', 'I', '!', '1'}) {
                uint16_t g = font.unicharToGlyph(c);
                SkRect bounds;
                font.getBounds(&g, 1, &bounds, nullptr);
                stemV = std::min(stemV, SkToS16(SkScalarRoundToInt(bounds.width())));
            }
            metrics->fStemV = stemV;
        }
        if (0 == metrics->fCapHeight) {
            // Figure out a good guess for CapHeight: average the height of M and X.
            SkScalar capHeight = 0;
            for (char c : {'M', 'X'}) {
                uint16_t g = font.unicharToGlyph(c);
                SkRect bounds;
                font.getBounds(&g, 1, &bounds, nullptr);
                capHeight += bounds.height();
            }
            metrics->fCapHeight = SkToS16(SkScalarRoundToInt(capHeight / 2));
        }
    }
    // Fonts are always subset, so always prepend the subset tag.
    metrics->fPostScriptName.prepend(canon->nextFontSubsetTag());
    return canon->fTypefaceMetrics.set(id, std::move(metrics))->get();
}

const std::vector<SkUnichar>& SkPDFFont::GetUnicodeMap(const SkTypeface& typeface,
                                                       SkPDFDocument* canon) {
    SkASSERT(canon);
    SkTypefaceID id = typeface.uniqueID();
    if (std::vector<SkUnichar>* ptr = canon->fToUnicodeMap.find(id)) {
        return *ptr;
    }
    std::vector<SkUnichar> buffer(typeface.countGlyphs());
    typeface.getGlyphToUnicodeMap(buffer.data());
    return *canon->fToUnicodeMap.set(id, std::move(buffer));
}

THashMap<SkGlyphID, SkString>& SkPDFFont::GetUnicodeMapEx(const SkTypeface& typeface,
                                                          SkPDFDocument* canon) {
    SkASSERT(canon);
    SkTypefaceID id = typeface.uniqueID();
    if (THashMap<SkGlyphID, SkString>* ptr = canon->fToUnicodeMapEx.find(id)) {
        return *ptr;
    }
    return *canon->fToUnicodeMapEx.set(id, THashMap<SkGlyphID, SkString>());
}

SkAdvancedTypefaceMetrics::FontType SkPDFFont::FontType(const SkPDFStrike& pdfStrike,
                                                        const SkAdvancedTypefaceMetrics& metrics) {
    if (SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kVariable_FontFlag) ||
        // PDF is actually interested in the encoding of the data, not just the logical format.
        // If the TrueType is actually wOFF or wOF2 then it should not be directly embedded in PDF.
        // For now export these as Type3 until the subsetter can handle table based fonts.
        // See https://github.com/harfbuzz/harfbuzz/issues/3609 and
        // https://skia-review.googlesource.com/c/skia/+/543485
        SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kAltDataFormat_FontFlag) ||
        SkToBool(metrics.fFlags & SkAdvancedTypefaceMetrics::kNotEmbeddable_FontFlag) ||
        // Something like 45eeeddb00741493 and 7c86e7641b348ca7b0 to output OpenType should work,
        // but requires PDF 1.6 which is still not supported by all printers. One could fix this by
        // using bare CFF like 31a170226c22244cbd00497b67f6ae181f0f3e76 which is only PDF 1.3,
        // but this only works when the CFF CIDs == CFF index == GlyphID as PDF bare CFF prefers
        // CFF CIDs instead of GlyphIDs and Skia doesn't know the CIDs.
        metrics.fType == SkAdvancedTypefaceMetrics::kCFF_Font ||
        pdfStrike.fHasMaskFilter)
    {
        // force Type3 fallback.
        return SkAdvancedTypefaceMetrics::kOther_Font;
    }
    return metrics.fType;
}

static SkGlyphID first_nonzero_glyph_for_single_byte_encoding(SkGlyphID gid) {
    return gid != 0 ? gid - (gid - 1) % 255 : 1;
}

SkPDFFont* SkPDFStrike::getFontResource(const SkGlyph* glyph) {
    const SkTypeface& typeface = fPath.fStrikeSpec.typeface();
    const SkAdvancedTypefaceMetrics* fontMetrics = SkPDFFont::GetMetrics(typeface, fDoc);
    SkASSERT(fontMetrics);  // SkPDFDevice::internalDrawText ensures the typeface is good.
                            // GetMetrics only returns null to signify a bad typeface.
    const SkAdvancedTypefaceMetrics& metrics = *fontMetrics;

    // Determine the FontType.
    // 1. Can the "original" font data be used directly
    // (simple OpenType, no non-default variations, not WOFF, etc).
    // 2. Is the glyph to be drawn unmodified from the font data
    // (no path effect, stroking, fake bolding, extra matrix, mask filter).
    // 3. Will PDF viewers draw this glyph the way we want
    // (at the moment this means an unmodified glyph path).
    SkAdvancedTypefaceMetrics::FontType type = SkPDFFont::FontType(*this, metrics);
    // Keep the type (and original data) if the glyph is empty or the glyph has an unmodified path.
    // Otherwise, fall back to Type3.
    if (!(glyph->isEmpty() || (glyph->path() && !glyph->pathIsModified()))) {
        type = SkAdvancedTypefaceMetrics::kOther_Font;
    }

    bool multibyte = SkPDFFont::IsMultiByte(type);
    SkGlyphID subsetCode =
            multibyte ? 0 : first_nonzero_glyph_for_single_byte_encoding(glyph->getGlyphID());
    if (SkPDFFont* font = fFontMap.find(subsetCode)) {
        SkASSERT(multibyte == font->multiByteGlyphs());
        return font;
    }

    SkGlyphID lastGlyph = SkToU16(typeface.countGlyphs() - 1);
    SkASSERT(glyph->getGlyphID() <= lastGlyph); // should be caught by SkPDFDevice::internalDrawText

    SkGlyphID firstNonZeroGlyph;
    if (multibyte) {
        firstNonZeroGlyph = 1;
    } else {
        firstNonZeroGlyph = subsetCode;
        lastGlyph = SkToU16(std::min<int>((int)lastGlyph, 254 + (int)subsetCode));
    }
    auto ref = fDoc->reserveRef();
    return fFontMap.set(subsetCode, SkPDFFont(this, firstNonZeroGlyph, lastGlyph, type, ref));
}

SkPDFFont::SkPDFFont(const SkPDFStrike* strike,
                     SkGlyphID firstGlyphID,
                     SkGlyphID lastGlyphID,
                     SkAdvancedTypefaceMetrics::FontType fontType,
                     SkPDFIndirectReference indirectReference)
    : fStrike(strike)
    , fGlyphUsage(firstGlyphID, lastGlyphID)
    , fIndirectReference(indirectReference)
    , fFontType(fontType)
{
    // Always include glyph 0
    this->noteGlyphUsage(0);
}

void SkPDFFont::PopulateCommonFontDescriptor(SkPDFDict* descriptor,
                                             const SkAdvancedTypefaceMetrics& metrics,
                                             uint16_t emSize,
                                             int16_t defaultWidth) {
    descriptor->insertName("FontName", metrics.fPostScriptName);
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
    descriptor->insertObject("FontBBox",
                             SkPDFMakeArray(scaleFromFontUnits(metrics.fBBox.left(), emSize),
                                            scaleFromFontUnits(metrics.fBBox.bottom(), emSize),
                                            scaleFromFontUnits(metrics.fBBox.right(), emSize),
                                            scaleFromFontUnits(metrics.fBBox.top(), emSize)));
    if (defaultWidth > 0) {
        descriptor->insertScalar("MissingWidth",
                scaleFromFontUnits(defaultWidth, emSize));
    }
}

///////////////////////////////////////////////////////////////////////////////
//  Type0Font
///////////////////////////////////////////////////////////////////////////////

static void emit_subset_type0(const SkPDFFont& font, SkPDFDocument* doc) {
    const SkTypeface& typeface = font.strike().fPath.fStrikeSpec.typeface();
    const SkAdvancedTypefaceMetrics* metricsPtr = SkPDFFont::GetMetrics(typeface, doc);
    SkASSERT(metricsPtr);
    if (!metricsPtr) {
        return;
    }
    const SkAdvancedTypefaceMetrics& metrics = *metricsPtr;
    SkASSERT(can_embed(metrics));
    SkAdvancedTypefaceMetrics::FontType type = font.getType();

    auto descriptor = SkPDFMakeDict("FontDescriptor");
    uint16_t emSize = SkToU16(SkScalarRoundToInt(font.strike().fPath.fUnitsPerEM));
    SkPDFFont::PopulateCommonFontDescriptor(descriptor.get(), metrics, emSize, 0);

    int ttcIndex;
    std::unique_ptr<SkStreamAsset> fontAsset = typeface.openStream(&ttcIndex);
    size_t fontSize = fontAsset ? fontAsset->getLength() : 0;
    if (0 == fontSize) {
        SkDebugf("Error: (SkTypeface)(%p)::openStream() returned "
                 "empty stream (%p) when identified as kType1CID_Font "
                 "or kTrueType_Font.\n", &typeface, fontAsset.get());
    } else if (type == SkAdvancedTypefaceMetrics::kTrueType_Font) {
        sk_sp<SkData> subsetFontData;
        if (can_subset(metrics)) {
            SkASSERT(font.firstGlyphID() == 1);
            subsetFontData = SkPDFSubsetFont(typeface, font.glyphUsage());
        }
        std::unique_ptr<SkStreamAsset> subsetFontAsset;
        if (subsetFontData) {
            subsetFontAsset = SkMemoryStream::Make(std::move(subsetFontData));
        } else {
            // If subsetting fails, fall back to original font data.
            subsetFontAsset = std::move(fontAsset);
        }
        std::unique_ptr<SkPDFDict> streamDict = SkPDFMakeDict();
        streamDict->insertInt("Length1", subsetFontAsset->getLength());
        descriptor->insertRef("FontFile2",
                              SkPDFStreamOut(std::move(streamDict), std::move(subsetFontAsset),
                                             doc, SkPDFSteamCompressionEnabled::Yes));
    } else if (type == SkAdvancedTypefaceMetrics::kType1CID_Font) {
        std::unique_ptr<SkPDFDict> streamDict = SkPDFMakeDict();
        streamDict->insertName("Subtype", "CIDFontType0C");
        descriptor->insertRef("FontFile3",
                              SkPDFStreamOut(std::move(streamDict), std::move(fontAsset),
                                             doc, SkPDFSteamCompressionEnabled::Yes));
    } else {
        SkASSERT(false);
    }

    auto newCIDFont = SkPDFMakeDict("Font");
    newCIDFont->insertRef("FontDescriptor", doc->emit(*descriptor));
    newCIDFont->insertName("BaseFont", metrics.fPostScriptName);

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
    auto sysInfo = SkPDFMakeDict();
    // These are actually ASCII strings.
    sysInfo->insertByteString("Registry", "Adobe");
    sysInfo->insertByteString("Ordering", "Identity");
    sysInfo->insertInt("Supplement", 0);
    newCIDFont->insertObject("CIDSystemInfo", std::move(sysInfo));

    // Unfortunately, poppler enforces DW (default width) must be an integer.
    int32_t defaultWidth = 0;
    {
        std::unique_ptr<SkPDFArray> widths = SkPDFMakeCIDGlyphWidthsArray(
                font.strike().fPath, font.glyphUsage(), &defaultWidth);
        if (widths && widths->size() > 0) {
            newCIDFont->insertObject("W", std::move(widths));
        }
        newCIDFont->insertInt("DW", defaultWidth);
    }

    ////////////////////////////////////////////////////////////////////////////

    SkPDFDict fontDict("Font");
    fontDict.insertName("Subtype", "Type0");
    fontDict.insertName("BaseFont", metrics.fPostScriptName);
    fontDict.insertName("Encoding", "Identity-H");
    auto descendantFonts = SkPDFMakeArray();
    descendantFonts->appendRef(doc->emit(*newCIDFont));
    fontDict.insertObject("DescendantFonts", std::move(descendantFonts));

    const std::vector<SkUnichar>& glyphToUnicode =
        SkPDFFont::GetUnicodeMap(typeface, doc);
    SkASSERT(SkToSizeT(typeface.countGlyphs()) == glyphToUnicode.size());
    std::unique_ptr<SkStreamAsset> toUnicode =
            SkPDFMakeToUnicodeCmap(glyphToUnicode.data(),
                                   SkPDFFont::GetUnicodeMapEx(typeface, doc),
                                   &font.glyphUsage(),
                                   font.multiByteGlyphs(),
                                   font.firstGlyphID(),
                                   font.lastGlyphID());
    fontDict.insertRef("ToUnicode", SkPDFStreamOut(nullptr, std::move(toUnicode), doc));

    doc->emit(fontDict, font.indirectReference());
}

///////////////////////////////////////////////////////////////////////////////
// PDFType3Font
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
}  // namespace

struct ImageAndOffset {
    sk_sp<SkImage> fImage;
    SkIPoint fOffset;
};
static ImageAndOffset to_image(SkGlyphID gid, SkBulkGlyphMetricsAndImages* smallGlyphs) {
    const SkGlyph* glyph = smallGlyphs->glyph(SkPackedGlyphID{gid});
    SkMask mask = glyph->mask();
    if (!mask.fImage) {
        return {nullptr, {0, 0}};
    }
    SkIRect bounds = mask.fBounds;
    SkBitmap bm;
    switch (mask.fFormat) {
        case SkMask::kBW_Format: {
            // Make a gray image, used to smask a rectangle.
            // TODO: emit as MaskImage?
            const SkISize size = bounds.size();
            bm.allocPixels(SkImageInfo::Make(size, kGray_8_SkColorType, kUnknown_SkAlphaType));
            for (int y = 0; y < bm.height(); ++y) {
                for (int x8 = 0; x8 < bm.width(); x8 += 8) {
                    uint8_t v = *mask.getAddr1(x8 + bounds.x(), y + bounds.y());
                    int e = std::min(x8 + 8, bm.width());
                    for (int x = x8; x < e; ++x) {
                        *bm.getAddr8(x, y) = (v >> (x & 0x7)) & 0x1 ? 0xFF : 0x00;
                    }
                }
            }
            bm.setImmutable();
            return {bm.asImage(), {bounds.x(), bounds.y()}};
        }
        case SkMask::kA8_Format:
        case SkMask::k3D_Format:  // just do the A8 part
            // Make a gray image, used to smask a rectangle.
            return {SkImages::RasterFromData(
                        SkImageInfo::Make(bounds.size(), kGray_8_SkColorType, kUnknown_SkAlphaType),
                        SkData::MakeWithCopy(mask.fImage, mask.computeImageSize()),
                        mask.fRowBytes),
                    {bounds.x(), bounds.y()}};
        case SkMask::kARGB32_Format:
            // These will be drawn as images directly.
            return {SkImages::RasterFromData(
                        SkImageInfo::MakeN32Premul(bounds.size()),
                        SkData::MakeWithCopy(mask.fImage, mask.computeTotalImageSize()),
                        mask.fRowBytes),
                    {bounds.x(), bounds.y()}};
        case SkMask::kLCD16_Format:
        default:
            SkASSERT(false);
            return {nullptr, {0, 0}};
    }
}

static SkPDFIndirectReference type3_descriptor(SkPDFDocument* doc,
                                               const SkTypeface& typeface,
                                               SkScalar xHeight) {
    if (SkPDFIndirectReference* ptr = doc->fType3FontDescriptors.find(typeface.uniqueID())) {
        return *ptr;
    }

    SkPDFDict descriptor("FontDescriptor");
    int32_t fontDescriptorFlags = kPdfSymbolic;

    /** PDF32000_2008: FontFamily should be used for Type3 fonts in Tagged PDF documents. */
    SkString familyName;
    typeface.getFamilyName(&familyName);
    if (!familyName.isEmpty()) {
        descriptor.insertByteString("FontFamily", familyName);
    }

    /** PDF32000_2008: FontStretch should be used for Type3 fonts in Tagged PDF documents. */
    static constexpr const char* stretchNames[9] = {
        "UltraCondensed",
        "ExtraCondensed",
        "Condensed",
        "SemiCondensed",
        "Normal",
        "SemiExpanded",
        "Expanded",
        "ExtraExpanded",
        "UltraExpanded",
    };
    const char* stretchName = stretchNames[typeface.fontStyle().width() - 1];
    descriptor.insertName("FontStretch", stretchName);

    /** PDF32000_2008: FontWeight should be used for Type3 fonts in Tagged PDF documents. */
    int weight = (typeface.fontStyle().weight() + 50) / 100;
    descriptor.insertInt("FontWeight", SkTPin(weight, 1, 9) * 100);

    if (const SkAdvancedTypefaceMetrics* metrics = SkPDFFont::GetMetrics(typeface, doc)) {
        // Type3 FontDescriptor does not require all the same fields.
        descriptor.insertName("FontName", metrics->fPostScriptName);
        descriptor.insertInt("ItalicAngle", metrics->fItalicAngle);
        fontDescriptorFlags |= (int32_t)metrics->fStyle;
        // Adobe requests CapHeight, XHeight, and StemV be added
        // to "greatly help our workflow downstream".
        if (metrics->fCapHeight != 0) { descriptor.insertInt("CapHeight", metrics->fCapHeight); }
        if (metrics->fStemV     != 0) { descriptor.insertInt("StemV",     metrics->fStemV);     }
        if (xHeight != 0) {
            descriptor.insertScalar("XHeight", xHeight);
        }
    }
    descriptor.insertInt("Flags", fontDescriptorFlags);
    SkPDFIndirectReference ref = doc->emit(descriptor);
    doc->fType3FontDescriptors.set(typeface.uniqueID(), ref);
    return ref;
}

static void emit_subset_type3(const SkPDFFont& pdfFont, SkPDFDocument* doc) {
    const SkPDFStrike& pdfStrike = pdfFont.strike();
    SkGlyphID firstGlyphID = pdfFont.firstGlyphID();
    SkGlyphID lastGlyphID = pdfFont.lastGlyphID();
    const SkPDFGlyphUse& subset = pdfFont.glyphUsage();
    SkASSERT(lastGlyphID >= firstGlyphID);
    // Remove unused glyphs at the end of the range.
    // Keep the lastGlyphID >= firstGlyphID invariant true.
    while (lastGlyphID > firstGlyphID && !subset.has(lastGlyphID)) {
        --lastGlyphID;
    }
    SkScalar emSize = pdfStrike.fPath.fUnitsPerEM;
    sk_sp<SkStrike> strike = pdfStrike.fPath.fStrikeSpec.findOrCreateStrike();
    SkASSERT(strike);
    SkScalar xHeight = strike->getFontMetrics().fXHeight;
    SkBulkGlyphMetricsAndPaths metricsAndPaths((sk_sp<SkStrike>(strike)));
    SkBulkGlyphMetricsAndDrawables metricsAndDrawables(std::move(strike));

    SkBulkGlyphMetricsAndImages smallGlyphs(pdfFont.strike().fImage.fStrikeSpec);
    float bitmapScale = emSize / pdfStrike.fImage.fUnitsPerEM;

    SkPDFDict font("Font");
    font.insertName("Subtype", "Type3");
    // Flip about the x-axis and scale by 1/emSize.
    SkMatrix fontMatrix;
    fontMatrix.setScale(SkScalarInvert(emSize), -SkScalarInvert(emSize));
    font.insertObject("FontMatrix", SkPDFUtils::MatrixToArray(fontMatrix));

    auto charProcs = SkPDFMakeDict();
    auto encoding = SkPDFMakeDict("Encoding");

    auto encDiffs = SkPDFMakeArray();
    // length(firstGlyphID .. lastGlyphID) ==  lastGlyphID - firstGlyphID + 1
    // plus 1 for glyph 0;
    SkASSERT(firstGlyphID > 0);
    SkASSERT(lastGlyphID >= firstGlyphID);
    int glyphCount = lastGlyphID - firstGlyphID + 2;
    // one other entry for the index of first glyph.
    encDiffs->reserve(glyphCount + 1);
    encDiffs->appendInt(0);  // index of first glyph

    auto widthArray = SkPDFMakeArray();
    widthArray->reserve(glyphCount);

    SkIRect bbox = SkIRect::MakeEmpty();

    std::unique_ptr<SkPDFDict> xobjects = SkPDFMakeDict();
    std::unique_ptr<SkPDFDict> graphicStates = SkPDFMakeDict();
    for (SkGlyphID gID : SingleByteGlyphIdIterator(firstGlyphID, lastGlyphID)) {
        SkString characterName;
        SkScalar advance = 0.0f;

        if (gID != 0 && !subset.has(gID)) {
            characterName.set("g0");
            advance = 0.0f;
            encDiffs->appendName(std::move(characterName));
            widthArray->appendScalar(advance);
            continue;
        }

        const SkGlyph* pathGlyph = metricsAndPaths.glyph(gID);
        const SkGlyph* drawableGlyph = metricsAndDrawables.glyph(gID);

        characterName.printf("g%X", gID);
        advance = pathGlyph->advanceX();
        encDiffs->appendName(characterName);
        widthArray->appendScalar(advance);

        SkIRect glyphBBox = pathGlyph->iRect();
        bbox.join(glyphBBox);
        const SkPath* path = pathGlyph->path();
        SkDrawable* drawable = drawableGlyph->drawable();
        SkDynamicMemoryWStream content;
        if (drawable && !drawable->getBounds().isEmpty()) {
            sk_sp<SkPDFDevice> glyphDevice = sk_make_sp<SkPDFDevice>(glyphBBox.size(), doc);
            SkCanvas canvas(glyphDevice);
            canvas.translate(-glyphBBox.fLeft, -glyphBBox.fTop);
            canvas.drawDrawable(drawable);
            SkPDFIndirectReference xobject = SkPDFMakeFormXObject(
                    doc, glyphDevice->content(),
                    SkPDFMakeArray(0, 0, glyphBBox.width(), glyphBBox.height()),
                    glyphDevice->makeResourceDict(),
                    SkMatrix::Translate(glyphBBox.fLeft, glyphBBox.fTop), nullptr);
            xobjects->insertRef(SkStringPrintf("Xg%X", gID), xobject);
            SkPDFUtils::AppendScalar(drawableGlyph->advanceX(), &content);
            content.writeText(" 0 d0\n1 0 0 1 0 0 cm\n/X");
            content.write(characterName.c_str(), characterName.size());
            content.writeText(" Do\n");
        } else if (path && !path->isEmpty() && !pdfStrike.fHasMaskFilter) {
            setGlyphWidthAndBoundingBox(pathGlyph->advanceX(), glyphBBox, &content);
            SkPaint::Style style = pathGlyph->pathIsHairline() ? SkPaint::kStroke_Style
                                                               : SkPaint::kFill_Style;
            SkPDFUtils::EmitPath(*path, style, &content);
            SkPDFUtils::PaintPath(style, path->getFillType(), &content);
        } else if (auto pimg = to_image(gID, &smallGlyphs); pimg.fImage) {
            using SkPDFUtils::AppendScalar;
            if (pimg.fImage->colorType() != kGray_8_SkColorType) {
                AppendScalar(pathGlyph->advanceX(), &content);
                content.writeText(" 0 d0\n");
                AppendScalar(pimg.fImage->width() * bitmapScale, &content);
                content.writeText(" 0 0 ");
                AppendScalar(-pimg.fImage->height() * bitmapScale, &content);
                content.writeText(" ");
                AppendScalar(pimg.fOffset.x() * bitmapScale, &content);
                content.writeText(" ");
                AppendScalar((pimg.fImage->height() + pimg.fOffset.y()) * bitmapScale,&content);
                content.writeText(" cm\n");
                content.writeText("/X");
                content.write(characterName.c_str(), characterName.size());
                content.writeText(" Do\n");
                SkPDFIndirectReference image = SkPDFSerializeImage(pimg.fImage.get(), doc);
                xobjects->insertRef(SkStringPrintf("Xg%X", gID), image);
            } else {
                // TODO: For A1, put ImageMask on the PDF image and draw the image?
                // The A8 mask has been converted to a Gray image

                // This is a `d1` glyph (shaded with the current fill)
                const SkGlyph* smallGlyph = smallGlyphs.glyph(SkPackedGlyphID{gID});
                SkRect smallBBox = smallGlyph->rect();
                SkIRect smallIBox;
                SkMatrix::Scale(bitmapScale, bitmapScale).mapRect(smallBBox).roundOut(&smallIBox);
                bbox.join(smallIBox);
                setGlyphWidthAndBoundingBox(pathGlyph->advanceX(), smallIBox, &content);

                AppendScalar(bitmapScale, &content);
                content.writeText(" 0 0 ");
                AppendScalar(bitmapScale, &content);
                content.writeText(" ");
                AppendScalar(pimg.fOffset.x() * bitmapScale, &content);
                content.writeText(" ");
                AppendScalar(pimg.fOffset.y() * bitmapScale, &content);
                content.writeText(" cm\n");

                // Convert Gray image to jpeg if needed
                if (pdfStrike.fHasMaskFilter) {
                    SkJpegEncoder::Options jpegOptions;
                    jpegOptions.fQuality = 50; // SK_PDF_MASK_QUALITY
                    SkImage* image = pimg.fImage.get();
                    sk_sp<SkData> jpegData = SkJpegEncoder::Encode(nullptr, image, jpegOptions);
                    if (jpegData) {
                        sk_sp<SkImage> jpegImage = SkImages::DeferredFromEncodedData(jpegData);
                        SkASSERT(jpegImage);
                        if (jpegImage) {
                            pimg.fImage = jpegImage;
                        }
                    }
                }

                // Draw image into a Form XObject
                const SkISize imageSize = pimg.fImage->dimensions();
                sk_sp<SkPDFDevice> glyphDevice = sk_sp(new SkPDFDevice(imageSize, doc));
                SkCanvas canvas(glyphDevice);
                canvas.drawImage(pimg.fImage, 0, 0);
                SkPDFIndirectReference sMask = SkPDFMakeFormXObject(
                        doc, glyphDevice->content(),
                        SkPDFMakeArray(0, 0, pimg.fImage->width(), pimg.fImage->height()),
                        glyphDevice->makeResourceDict(),
                        SkMatrix(), "DeviceGray");

                // Use Form XObject as SMask (luminosity) on the graphics state
                SkPDFIndirectReference smaskGraphicState = SkPDFGraphicState::GetSMaskGraphicState(
                        sMask, false,
                        SkPDFGraphicState::kLuminosity_SMaskMode, doc);
                SkPDFUtils::ApplyGraphicState(smaskGraphicState.fValue, &content);

                // Draw a rectangle the size of the glyph (masked by SMask)
                SkPDFUtils::AppendRectangle(SkRect::Make(pimg.fImage->bounds()), &content);
                SkPDFUtils::PaintPath(SkPaint::kFill_Style, SkPathFillType::kWinding, &content);

                // Add glyph resources to font resource dict
                xobjects->insertRef(SkStringPrintf("Xg%X", gID), sMask);
                // TODO: name must match ApplyGraphicState
                graphicStates->insertRef(SkStringPrintf("G%d", smaskGraphicState.fValue),
                                         smaskGraphicState);
            }
        } else {
            setGlyphWidthAndBoundingBox(pathGlyph->advanceX(), glyphBBox, &content);
        }
        charProcs->insertRef(std::move(characterName),
                             SkPDFStreamOut(nullptr, content.detachAsStream(), doc));
    }

    if (xobjects->size() || graphicStates->size()) {
        auto resources = SkPDFMakeDict();
        if (xobjects->size()) {
            resources->insertObject("XObject", std::move(xobjects));
        }
        if (graphicStates->size()) {
            resources->insertObject("ExtGState", std::move(graphicStates));
        }
        font.insertObject("Resources", std::move(resources));
    }

    encoding->insertObject("Differences", std::move(encDiffs));
    font.insertInt("FirstChar", 0);
    font.insertInt("LastChar", lastGlyphID - firstGlyphID + 1);
    /* FontBBox: "A rectangle expressed in the glyph coordinate
      system, specifying the font bounding box. This is the smallest
      rectangle enclosing the shape that would result if all of the
      glyphs of the font were placed with their origins coincident and
      then filled." */
    font.insertObject("FontBBox", SkPDFMakeArray(bbox.left(),
                                                  bbox.bottom(),
                                                  bbox.right(),
                                                  bbox.top()));

    font.insertName("CIDToGIDMap", "Identity");

    const SkTypeface& pathTypeface = pdfStrike.fPath.fStrikeSpec.typeface();
    const std::vector<SkUnichar>& glyphToUnicode = SkPDFFont::GetUnicodeMap(pathTypeface, doc);
    SkASSERT(glyphToUnicode.size() == SkToSizeT(pathTypeface.countGlyphs()));
    auto toUnicodeCmap = SkPDFMakeToUnicodeCmap(glyphToUnicode.data(),
                                                SkPDFFont::GetUnicodeMapEx(pathTypeface, doc),
                                                &subset,
                                                false,
                                                firstGlyphID,
                                                lastGlyphID);
    font.insertRef("ToUnicode", SkPDFStreamOut(nullptr, std::move(toUnicodeCmap), doc));
    font.insertRef("FontDescriptor", type3_descriptor(doc, pathTypeface, xHeight));
    font.insertObject("Widths", std::move(widthArray));
    font.insertObject("Encoding", std::move(encoding));
    font.insertObject("CharProcs", std::move(charProcs));

    doc->emit(font, pdfFont.indirectReference());
}

void SkPDFFont::emitSubset(SkPDFDocument* doc) const {
    switch (fFontType) {
        case SkAdvancedTypefaceMetrics::kType1CID_Font:
        case SkAdvancedTypefaceMetrics::kTrueType_Font:
            return emit_subset_type0(*this, doc);
#ifndef SK_PDF_DO_NOT_SUPPORT_TYPE_1_FONTS
        case SkAdvancedTypefaceMetrics::kType1_Font:
            return SkPDFEmitType1Font(*this, doc);
#endif
        default:
            return emit_subset_type3(*this, doc);
    }
}

////////////////////////////////////////////////////////////////////////////////

bool SkPDFFont::CanEmbedTypeface(const SkTypeface& typeface, SkPDFDocument* doc) {
    const SkAdvancedTypefaceMetrics* metrics = SkPDFFont::GetMetrics(typeface, doc);
    return metrics && can_embed(*metrics);
}

