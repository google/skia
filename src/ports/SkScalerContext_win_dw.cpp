/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/utils/win/SkDWriteNTDDI_VERSION.h"

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#undef GetGlyphIndices

#include "include/codec/SkCodec.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkPath.h"
#include "include/private/SkMutex.h"
#include "include/private/SkTo.h"
#include "src/core/SkDraw.h"
#include "src/core/SkEndian.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScalerContext.h"
#include "src/core/SkSharedMutex.h"
#include "src/ports/SkScalerContext_win_dw.h"
#include "src/ports/SkTypeface_win_dw.h"
#include "src/sfnt/SkOTTable_EBLC.h"
#include "src/sfnt/SkOTTable_EBSC.h"
#include "src/sfnt/SkOTTable_gasp.h"
#include "src/sfnt/SkOTTable_maxp.h"
#include "src/utils/SkMatrix22.h"
#include "src/utils/win/SkDWrite.h"
#include "src/utils/win/SkDWriteGeometrySink.h"
#include "src/utils/win/SkHRESULT.h"
#include "src/utils/win/SkTScopedComPtr.h"

#include <dwrite.h>
#include <dwrite_1.h>
#include <dwrite_3.h>

/* Note:
 * In versions 8 and 8.1 of Windows, some calls in DWrite are not thread safe.
 * The DWriteFactoryMutex protects the calls that are problematic.
 */
static SkSharedMutex DWriteFactoryMutex;

typedef SkAutoSharedMutexExclusive Exclusive;
typedef SkAutoSharedMutexShared Shared;

static bool isLCD(const SkScalerContextRec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

static bool is_hinted(DWriteFontTypeface* typeface) {
    Exclusive l(DWriteFactoryMutex);
    AutoTDWriteTable<SkOTTableMaximumProfile> maxp(typeface->fDWriteFontFace.get());
    if (!maxp.fExists) {
        return false;
    }
    if (maxp.fSize < sizeof(SkOTTableMaximumProfile::Version::TT)) {
        return false;
    }
    if (maxp->version.version != SkOTTableMaximumProfile::Version::TT::VERSION) {
        return false;
    }
    return (0 != maxp->version.tt.maxSizeOfInstructions);
}

/** A GaspRange is inclusive, [min, max]. */
struct GaspRange {
    using Behavior = SkOTTableGridAndScanProcedure::GaspRange::behavior;
    GaspRange(int min, int max, int version, Behavior flags)
        : fMin(min), fMax(max), fVersion(version), fFlags(flags) { }
    int fMin;
    int fMax;
    int fVersion;
    Behavior fFlags;
};

bool get_gasp_range(DWriteFontTypeface* typeface, int size, GaspRange* range) {
    AutoTDWriteTable<SkOTTableGridAndScanProcedure> gasp(typeface->fDWriteFontFace.get());
    if (!gasp.fExists) {
        return false;
    }
    if (gasp.fSize < sizeof(SkOTTableGridAndScanProcedure)) {
        return false;
    }
    if (gasp->version != SkOTTableGridAndScanProcedure::version0 &&
        gasp->version != SkOTTableGridAndScanProcedure::version1)
    {
        return false;
    }

    uint16_t numRanges = SkEndianSwap16(gasp->numRanges);
    if (numRanges > 1024 ||
        gasp.fSize < sizeof(SkOTTableGridAndScanProcedure) +
        sizeof(SkOTTableGridAndScanProcedure::GaspRange) * numRanges)
    {
        return false;
    }

    const SkOTTableGridAndScanProcedure::GaspRange* rangeTable =
            SkTAfter<const SkOTTableGridAndScanProcedure::GaspRange>(gasp.get());
    int minPPEM = -1;
    for (uint16_t i = 0; i < numRanges; ++i, ++rangeTable) {
        int maxPPEM = SkEndianSwap16(rangeTable->maxPPEM);
        if (minPPEM < size && size <= maxPPEM) {
            range->fMin = minPPEM + 1;
            range->fMax = maxPPEM;
            range->fVersion = SkEndian_SwapBE16(gasp->version);
            range->fFlags = rangeTable->flags;
            return true;
        }
        minPPEM = maxPPEM;
    }
    return false;
}
/** If the rendering mode for the specified 'size' is gridfit, then place
 *  the gridfit range into 'range'. Otherwise, leave 'range' alone.
 */
static bool is_gridfit_only(GaspRange::Behavior flags) {
    return flags.raw.value == GaspRange::Behavior::Raw::GridfitMask;
}

static bool has_bitmap_strike(DWriteFontTypeface* typeface, GaspRange range) {
    Exclusive l(DWriteFactoryMutex);
    {
        AutoTDWriteTable<SkOTTableEmbeddedBitmapLocation> eblc(typeface->fDWriteFontFace.get());
        if (!eblc.fExists) {
            return false;
        }
        if (eblc.fSize < sizeof(SkOTTableEmbeddedBitmapLocation)) {
            return false;
        }
        if (eblc->version != SkOTTableEmbeddedBitmapLocation::version_initial) {
            return false;
        }

        uint32_t numSizes = SkEndianSwap32(eblc->numSizes);
        if (numSizes > 1024 ||
            eblc.fSize < sizeof(SkOTTableEmbeddedBitmapLocation) +
                         sizeof(SkOTTableEmbeddedBitmapLocation::BitmapSizeTable) * numSizes)
        {
            return false;
        }

        const SkOTTableEmbeddedBitmapLocation::BitmapSizeTable* sizeTable =
                SkTAfter<const SkOTTableEmbeddedBitmapLocation::BitmapSizeTable>(eblc.get());
        for (uint32_t i = 0; i < numSizes; ++i, ++sizeTable) {
            if (sizeTable->ppemX == sizeTable->ppemY &&
                range.fMin <= sizeTable->ppemX && sizeTable->ppemX <= range.fMax)
            {
                // TODO: determine if we should dig through IndexSubTableArray/IndexSubTable
                // to determine the actual number of glyphs with bitmaps.

                // TODO: Ensure that the bitmaps actually cover a significant portion of the strike.

                // TODO: Ensure that the bitmaps are bi-level?
                if (sizeTable->endGlyphIndex >= sizeTable->startGlyphIndex + 3) {
                    return true;
                }
            }
        }
    }

    {
        AutoTDWriteTable<SkOTTableEmbeddedBitmapScaling> ebsc(typeface->fDWriteFontFace.get());
        if (!ebsc.fExists) {
            return false;
        }
        if (ebsc.fSize < sizeof(SkOTTableEmbeddedBitmapScaling)) {
            return false;
        }
        if (ebsc->version != SkOTTableEmbeddedBitmapScaling::version_initial) {
            return false;
        }

        uint32_t numSizes = SkEndianSwap32(ebsc->numSizes);
        if (numSizes > 1024 ||
            ebsc.fSize < sizeof(SkOTTableEmbeddedBitmapScaling) +
                         sizeof(SkOTTableEmbeddedBitmapScaling::BitmapScaleTable) * numSizes)
        {
            return false;
        }

        const SkOTTableEmbeddedBitmapScaling::BitmapScaleTable* scaleTable =
                SkTAfter<const SkOTTableEmbeddedBitmapScaling::BitmapScaleTable>(ebsc.get());
        for (uint32_t i = 0; i < numSizes; ++i, ++scaleTable) {
            if (scaleTable->ppemX == scaleTable->ppemY &&
                range.fMin <= scaleTable->ppemX && scaleTable->ppemX <= range.fMax) {
                // EBSC tables are normally only found in bitmap only fonts.
                return true;
            }
        }
    }

    return false;
}

static bool both_zero(SkScalar a, SkScalar b) {
    return 0 == a && 0 == b;
}

// returns false if there is any non-90-rotation or skew
static bool is_axis_aligned(const SkScalerContextRec& rec) {
    return 0 == rec.fPreSkewX &&
           (both_zero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
            both_zero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

SkScalerContext_DW::SkScalerContext_DW(sk_sp<DWriteFontTypeface> typefaceRef,
                                       const SkScalerContextEffects& effects,
                                       const SkDescriptor* desc)
        : SkScalerContext(std::move(typefaceRef), effects, desc)
        , fGlyphCount(-1) {

    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    fIsColorFont = typeface->fFactory2 &&
                   typeface->fDWriteFontFace2 &&
                   typeface->fDWriteFontFace2->IsColorFont();

    // In general, all glyphs should use NATURAL_SYMMETRIC
    // except when bi-level rendering is requested or there are embedded
    // bi-level bitmaps (and the embedded bitmap flag is set and no rotation).
    //
    // DirectWrite's IDWriteFontFace::GetRecommendedRenderingMode does not do
    // this. As a result, determine the actual size of the text and then see if
    // there are any embedded bi-level bitmaps of that size. If there are, then
    // force bitmaps by requesting bi-level rendering.
    //
    // FreeType allows for separate ppemX and ppemY, but DirectWrite assumes
    // square pixels and only uses ppemY. Therefore the transform must track any
    // non-uniform x-scale.
    //
    // Also, rotated glyphs should have the same absolute advance widths as
    // horizontal glyphs and the subpixel flag should not affect glyph shapes.

    SkVector scale;
    fRec.computeMatrices(SkScalerContextRec::kVertical_PreMatrixScale, &scale, &fSkXform);

    fXform.m11 = SkScalarToFloat(fSkXform.getScaleX());
    fXform.m12 = SkScalarToFloat(fSkXform.getSkewY());
    fXform.m21 = SkScalarToFloat(fSkXform.getSkewX());
    fXform.m22 = SkScalarToFloat(fSkXform.getScaleY());
    fXform.dx = 0;
    fXform.dy = 0;

    // realTextSize is the actual device size we want (as opposed to the size the user requested).
    // gdiTextSize is the size we request when GDI compatible.
    // If the scale is negative, this means the matrix will do the flip anyway.
    const SkScalar realTextSize = scale.fY;
    // Due to floating point math, the lower bits are suspect. Round carefully.
    SkScalar gdiTextSize = SkScalarRoundToScalar(realTextSize * 64.0f) / 64.0f;
    if (gdiTextSize == 0) {
        gdiTextSize = SK_Scalar1;
    }

    bool bitmapRequested = SkToBool(fRec.fFlags & SkScalerContext::kEmbeddedBitmapText_Flag);
    bool treatLikeBitmap = false;
    bool axisAlignedBitmap = false;
    if (bitmapRequested) {
        // When embedded bitmaps are requested, treat the entire range like
        // a bitmap strike if the range is gridfit only and contains a bitmap.
        int bitmapPPEM = SkScalarTruncToInt(gdiTextSize);
        GaspRange range(bitmapPPEM, bitmapPPEM, 0, GaspRange::Behavior());
        if (get_gasp_range(typeface, bitmapPPEM, &range)) {
            if (!is_gridfit_only(range.fFlags)) {
                range = GaspRange(bitmapPPEM, bitmapPPEM, 0, GaspRange::Behavior());
            }
        }
        treatLikeBitmap = has_bitmap_strike(typeface, range);

        axisAlignedBitmap = is_axis_aligned(fRec);
    }

    GaspRange range(0, 0xFFFF, 0, GaspRange::Behavior());

    // If the user requested aliased, do so with aliased compatible metrics.
    if (SkMask::kBW_Format == fRec.fMaskFormat) {
        fTextSizeRender = gdiTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_ALIASED;
        fTextureType = DWRITE_TEXTURE_ALIASED_1x1;
        fTextSizeMeasure = gdiTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_GDI_CLASSIC;

    // If we can use a bitmap, use gdi classic rendering and measurement.
    // This will not always provide a bitmap, but matches expected behavior.
    } else if (treatLikeBitmap && axisAlignedBitmap) {
        fTextSizeRender = gdiTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_GDI_CLASSIC;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = gdiTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_GDI_CLASSIC;

    // If rotated but the horizontal text could have used a bitmap,
    // render high quality rotated glyphs but measure using bitmap metrics.
    } else if (treatLikeBitmap) {
        fTextSizeRender = gdiTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = gdiTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_GDI_CLASSIC;

    // If the font has a gasp table version 1, use it to determine symmetric rendering.
    } else if (get_gasp_range(typeface, SkScalarRoundToInt(gdiTextSize), &range) &&
               range.fVersion >= 1)
    {
        fTextSizeRender = realTextSize;
        fRenderingMode = range.fFlags.field.SymmetricSmoothing
                       ? DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC
                       : DWRITE_RENDERING_MODE_NATURAL;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;

    // If the requested size is above 20px or there are no bytecode hints, use symmetric rendering.
    } else if (realTextSize > SkIntToScalar(20) || !is_hinted(typeface)) {
        fTextSizeRender = realTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;

    // Fonts with hints, no gasp or gasp version 0, and below 20px get non-symmetric rendering.
    // Often such fonts have hints which were only tested with GDI ClearType classic.
    // Some of these fonts rely on drop out control in the y direction in order to be legible.
    // Tenor Sans
    //    https://fonts.google.com/specimen/Tenor+Sans
    // Gill Sans W04
    //    https://cdn.leagueoflegends.com/lolkit/1.1.9/resources/fonts/gill-sans-w04-book.woff
    //    https://na.leagueoflegends.com/en/news/game-updates/patch/patch-410-notes
    // See https://crbug.com/385897
    } else {
        fTextSizeRender = gdiTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_NATURAL;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;
    }

    // DirectWrite2 allows for grayscale hinting.
    fAntiAliasMode = DWRITE_TEXT_ANTIALIAS_MODE_CLEARTYPE;
    if (typeface->fFactory2 && typeface->fDWriteFontFace2 &&
        SkMask::kA8_Format == fRec.fMaskFormat &&
        !(fRec.fFlags & SkScalerContext::kGenA8FromLCD_Flag))
    {
        // DWRITE_TEXTURE_ALIASED_1x1 is now misnamed, it must also be used with grayscale.
        fTextureType = DWRITE_TEXTURE_ALIASED_1x1;
        fAntiAliasMode = DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE;
    }

    // DirectWrite2 allows hinting to be disabled.
    fGridFitMode = DWRITE_GRID_FIT_MODE_ENABLED;
    if (fRec.getHinting() == SkFontHinting::kNone) {
        fGridFitMode = DWRITE_GRID_FIT_MODE_DISABLED;
        if (fRenderingMode != DWRITE_RENDERING_MODE_ALIASED) {
            fRenderingMode = DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC;
        }
    }

    if (this->isLinearMetrics()) {
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;
    }
}

SkScalerContext_DW::~SkScalerContext_DW() {
}

unsigned SkScalerContext_DW::generateGlyphCount() {
    if (fGlyphCount < 0) {
        fGlyphCount = this->getDWriteTypeface()->fDWriteFontFace->GetGlyphCount();
    }
    return fGlyphCount;
}

bool SkScalerContext_DW::generateAdvance(SkGlyph* glyph) {
    glyph->fAdvanceX = 0;
    glyph->fAdvanceY = 0;

    uint16_t glyphId = glyph->getGlyphID();
    DWRITE_GLYPH_METRICS gm;

    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        Exclusive l(DWriteFactoryMutex);
        HRBM(this->getDWriteTypeface()->fDWriteFontFace->GetGdiCompatibleGlyphMetrics(
                 fTextSizeMeasure,
                 1.0f, // pixelsPerDip
                 // This parameter does not act like the lpmat2 parameter to GetGlyphOutlineW.
                 // If it did then GsA here and G_inv below to mapVectors.
                 nullptr,
                 DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode,
                 &glyphId, 1,
                 &gm),
             "Could not get gdi compatible glyph metrics.");
    } else {
        Exclusive l(DWriteFactoryMutex);
        HRBM(this->getDWriteTypeface()->fDWriteFontFace->GetDesignGlyphMetrics(&glyphId, 1, &gm),
             "Could not get design metrics.");
    }

    DWRITE_FONT_METRICS dwfm;
    {
        Shared l(DWriteFactoryMutex);
        this->getDWriteTypeface()->fDWriteFontFace->GetMetrics(&dwfm);
    }
    SkScalar advanceX = fTextSizeMeasure * gm.advanceWidth / dwfm.designUnitsPerEm;

    SkVector advance = { advanceX, 0 };
    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        // DirectWrite produced 'compatible' metrics, but while close,
        // the end result is not always an integer as it would be with GDI.
        advance.fX = SkScalarRoundToScalar(advance.fX);
    }
    fSkXform.mapVectors(&advance, 1);

    glyph->fAdvanceX = SkScalarToFloat(advance.fX);
    glyph->fAdvanceY = SkScalarToFloat(advance.fY);
    return true;
}

HRESULT SkScalerContext_DW::getBoundingBox(SkGlyph* glyph,
                                           DWRITE_RENDERING_MODE renderingMode,
                                           DWRITE_TEXTURE_TYPE textureType,
                                           RECT* bbox)
{
    //Measure raster size.
    fXform.dx = SkFixedToFloat(glyph->getSubXFixed());
    fXform.dy = SkFixedToFloat(glyph->getSubYFixed());

    FLOAT advance = 0;

    UINT16 glyphId = glyph->getGlyphID();

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0.0f;
    offset.ascenderOffset = 0.0f;

    DWRITE_GLYPH_RUN run;
    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = this->getDWriteTypeface()->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &glyphId;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
    {
        Exclusive l(DWriteFactoryMutex);
        // IDWriteFactory2::CreateGlyphRunAnalysis is very bad at aliased glyphs.
        if (this->getDWriteTypeface()->fFactory2 &&
                (fGridFitMode == DWRITE_GRID_FIT_MODE_DISABLED ||
                 fAntiAliasMode == DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE))
        {
            HRM(this->getDWriteTypeface()->fFactory2->CreateGlyphRunAnalysis(
                    &run,
                    &fXform,
                    renderingMode,
                    fMeasuringMode,
                    fGridFitMode,
                    fAntiAliasMode,
                    0.0f, // baselineOriginX,
                    0.0f, // baselineOriginY,
                    &glyphRunAnalysis),
                "Could not create DW2 glyph run analysis.");
        } else {
            HRM(this->getDWriteTypeface()->fFactory->CreateGlyphRunAnalysis(&run,
                    1.0f, // pixelsPerDip,
                    &fXform,
                    renderingMode,
                    fMeasuringMode,
                    0.0f, // baselineOriginX,
                    0.0f, // baselineOriginY,
                    &glyphRunAnalysis),
                "Could not create glyph run analysis.");
        }
    }
    {
        Shared l(DWriteFactoryMutex);
        HRM(glyphRunAnalysis->GetAlphaTextureBounds(textureType, bbox),
            "Could not get texture bounds.");
    }
    return S_OK;
}

/** GetAlphaTextureBounds succeeds but sometimes returns empty bounds like
 *  { 0x80000000, 0x80000000, 0x80000000, 0x80000000 }
 *  for small, but not quite zero, sized glyphs.
 *  Only set as non-empty if the returned bounds are non-empty.
 */
static bool glyph_check_and_set_bounds(SkGlyph* glyph, const RECT& bbox) {
    if (bbox.left >= bbox.right || bbox.top >= bbox.bottom) {
        return false;
    }

    // We're trying to pack left and top into int16_t,
    // and width and height into uint16_t, after outsetting by 1.
    if (!SkIRect::MakeXYWH(-32767, -32767, 65535, 65535).contains(
                SkIRect::MakeLTRB(bbox.left, bbox.top, bbox.right, bbox.bottom))) {
        return false;
    }

    glyph->fWidth = SkToU16(bbox.right - bbox.left);
    glyph->fHeight = SkToU16(bbox.bottom - bbox.top);
    glyph->fLeft = SkToS16(bbox.left);
    glyph->fTop = SkToS16(bbox.top);
    return true;
}

bool SkScalerContext_DW::isColorGlyph(const SkGlyph& glyph) {
    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayer;
    return getColorGlyphRun(glyph, &colorLayer);
}

bool SkScalerContext_DW::isPngGlyph(const SkGlyph& glyph) {
    if (!this->getDWriteTypeface()->fDWriteFontFace4) {
        return false;
    }

    DWRITE_GLYPH_IMAGE_FORMATS f;
    IDWriteFontFace4* fontFace4 = this->getDWriteTypeface()->fDWriteFontFace4.get();
    HRBM(fontFace4->GetGlyphImageFormats(glyph.getGlyphID(), 0, UINT32_MAX, &f),
         "Cannot get glyph image formats.");
    return f & DWRITE_GLYPH_IMAGE_FORMATS_PNG;
}

bool SkScalerContext_DW::getColorGlyphRun(const SkGlyph& glyph,
                                          IDWriteColorGlyphRunEnumerator** colorGlyph)
{
    FLOAT advance = 0;
    UINT16 glyphId = glyph.getGlyphID();

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0.0f;
    offset.ascenderOffset = 0.0f;

    DWRITE_GLYPH_RUN run;
    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = this->getDWriteTypeface()->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &glyphId;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    HRESULT hr = this->getDWriteTypeface()->fFactory2->TranslateColorGlyphRun(
        0, 0, &run, nullptr, fMeasuringMode, &fXform, 0, colorGlyph);
    if (hr == DWRITE_E_NOCOLOR) {
        return false;
    }
    HRBM(hr, "Failed to translate color glyph run");
    return true;
}

void SkScalerContext_DW::generateColorMetrics(SkGlyph* glyph) {
    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;
    HRVM(getColorGlyphRun(*glyph, &colorLayers), "Could not get color glyph run");
    SkASSERT(colorLayers.get());

    SkRect bounds = SkRect::MakeEmpty();
    BOOL hasNextRun = FALSE;
    while (SUCCEEDED(colorLayers->MoveNext(&hasNextRun)) && hasNextRun) {
        const DWRITE_COLOR_GLYPH_RUN* colorGlyph;
        HRVM(colorLayers->GetCurrentRun(&colorGlyph), "Could not get current color glyph run");

        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRVM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
            "Could not create geometry to path converter.");
        {
            Exclusive l(DWriteFactoryMutex);
            HRVM(colorGlyph->glyphRun.fontFace->GetGlyphRunOutline(
                    colorGlyph->glyphRun.fontEmSize,
                    colorGlyph->glyphRun.glyphIndices,
                    colorGlyph->glyphRun.glyphAdvances,
                    colorGlyph->glyphRun.glyphOffsets,
                    colorGlyph->glyphRun.glyphCount,
                    colorGlyph->glyphRun.isSideways,
                    colorGlyph->glyphRun.bidiLevel % 2, //rtl
                    geometryToPath.get()),
                "Could not create glyph outline.");
        }
        bounds.join(path.getBounds());
    }
    SkMatrix matrix = fSkXform;
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph->getSubXFixed()),
                             SkFixedToScalar(glyph->getSubYFixed()));
    }
    matrix.mapRect(&bounds);
    // Round float bound values into integer.
    SkIRect ibounds = bounds.roundOut();

    glyph->fWidth = ibounds.fRight - ibounds.fLeft;
    glyph->fHeight = ibounds.fBottom - ibounds.fTop;
    glyph->fLeft = ibounds.fLeft;
    glyph->fTop = ibounds.fTop;
}

namespace {
struct Context {
    SkTScopedComPtr<IDWriteFontFace4> fontFace4;
    void* glyphDataContext;
    Context(IDWriteFontFace4* face4, void* context)
        : fontFace4(SkRefComPtr(face4))
        , glyphDataContext(context)
    {}
};

static void ReleaseProc(const void* ptr, void* context) {
    Context* ctx = (Context*)context;
    ctx->fontFace4->ReleaseGlyphImageData(ctx->glyphDataContext);
    delete ctx;
}
}

void SkScalerContext_DW::generatePngMetrics(SkGlyph* glyph) {
    SkASSERT(isPngGlyph(*glyph));
    SkASSERT(glyph->fMaskFormat == SkMask::Format::kARGB32_Format);
    SkASSERT(this->getDWriteTypeface()->fDWriteFontFace4);

    IDWriteFontFace4* fontFace4 = this->getDWriteTypeface()->fDWriteFontFace4.get();
    DWRITE_GLYPH_IMAGE_DATA glyphData;
    void* glyphDataContext;
    HRVM(fontFace4->GetGlyphImageData(glyph->getGlyphID(),
                                      fTextSizeRender,
                                      DWRITE_GLYPH_IMAGE_FORMATS_PNG,
                                      &glyphData,
                                      &glyphDataContext),
         "Glyph image data could not be acquired.");

    Context* context = new Context(fontFace4, glyphDataContext);
    sk_sp<SkData> data = SkData::MakeWithProc(glyphData.imageData,
                                              glyphData.imageDataSize,
                                              &ReleaseProc,
                                              context);

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo();
    SkRect bounds = SkRect::MakeLTRB(SkIntToScalar(info.bounds().fLeft),
                                     SkIntToScalar(info.bounds().fTop),
                                     SkIntToScalar(info.bounds().fRight),
                                     SkIntToScalar(info.bounds().fBottom));

    SkMatrix matrix = fSkXform;
    SkScalar scale = fTextSizeRender / glyphData.pixelsPerEm;
    matrix.preScale(scale, scale);
    matrix.preTranslate(-glyphData.horizontalLeftOrigin.x, -glyphData.horizontalLeftOrigin.y);
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph->getSubXFixed()),
                             SkFixedToScalar(glyph->getSubYFixed()));
    }
    matrix.mapRect(&bounds);
    bounds.roundOut();

    glyph->fWidth = bounds.width();
    glyph->fHeight = bounds.height();
    glyph->fLeft = bounds.left();
    glyph->fTop = bounds.top();
    return;
}

void SkScalerContext_DW::generateMetrics(SkGlyph* glyph) {
    glyph->fWidth = 0;
    glyph->fHeight = 0;
    glyph->fLeft = 0;
    glyph->fTop = 0;
    glyph->fMaskFormat = fRec.fMaskFormat;

    if (!this->generateAdvance(glyph)) {
        return;
    }

    if (fIsColorFont && isColorGlyph(*glyph)) {
        glyph->fMaskFormat = SkMask::kARGB32_Format;
        generateColorMetrics(glyph);
        return;
    }

    if (fIsColorFont && isPngGlyph(*glyph)) {
        glyph->fMaskFormat = SkMask::kARGB32_Format;
        generatePngMetrics(glyph);
        return;
    }

    RECT bbox;
    HRVM(this->getBoundingBox(glyph, fRenderingMode, fTextureType, &bbox),
         "Requested bounding box could not be determined.");

    if (glyph_check_and_set_bounds(glyph, bbox)) {
        return;
    }

    // GetAlphaTextureBounds succeeds but returns an empty RECT if there are no
    // glyphs of the specified texture type or it is too big for smoothing.
    // When this happens, try with the alternate texture type.
    if (DWRITE_TEXTURE_ALIASED_1x1 != fTextureType ||
        DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE == fAntiAliasMode)
    {
        HRVM(this->getBoundingBox(glyph,
                                  DWRITE_RENDERING_MODE_ALIASED,
                                  DWRITE_TEXTURE_ALIASED_1x1,
                                  &bbox),
             "Fallback bounding box could not be determined.");
        if (glyph_check_and_set_bounds(glyph, bbox)) {
            glyph->fForceBW = 1;
            glyph->fMaskFormat = SkMask::kBW_Format;
        }
    }
    // TODO: handle the case where a request for DWRITE_TEXTURE_ALIASED_1x1
    // fails, and try DWRITE_TEXTURE_CLEARTYPE_3x1.
}

void SkScalerContext_DW::generateFontMetrics(SkFontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    sk_bzero(metrics, sizeof(*metrics));

    DWRITE_FONT_METRICS dwfm;
    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        this->getDWriteTypeface()->fDWriteFontFace->GetGdiCompatibleMetrics(
             fTextSizeRender,
             1.0f, // pixelsPerDip
             &fXform,
             &dwfm);
    } else {
        this->getDWriteTypeface()->fDWriteFontFace->GetMetrics(&dwfm);
    }

    SkScalar upem = SkIntToScalar(dwfm.designUnitsPerEm);

    metrics->fAscent = -fTextSizeRender * SkIntToScalar(dwfm.ascent) / upem;
    metrics->fDescent = fTextSizeRender * SkIntToScalar(dwfm.descent) / upem;
    metrics->fLeading = fTextSizeRender * SkIntToScalar(dwfm.lineGap) / upem;
    metrics->fXHeight = fTextSizeRender * SkIntToScalar(dwfm.xHeight) / upem;
    metrics->fCapHeight = fTextSizeRender * SkIntToScalar(dwfm.capHeight) / upem;
    metrics->fUnderlineThickness = fTextSizeRender * SkIntToScalar(dwfm.underlineThickness) / upem;
    metrics->fUnderlinePosition = -(fTextSizeRender * SkIntToScalar(dwfm.underlinePosition) / upem);
    metrics->fStrikeoutThickness = fTextSizeRender * SkIntToScalar(dwfm.strikethroughThickness) / upem;
    metrics->fStrikeoutPosition = -(fTextSizeRender * SkIntToScalar(dwfm.strikethroughPosition) / upem);

    metrics->fFlags |= SkFontMetrics::kUnderlineThicknessIsValid_Flag;
    metrics->fFlags |= SkFontMetrics::kUnderlinePositionIsValid_Flag;
    metrics->fFlags |= SkFontMetrics::kStrikeoutThicknessIsValid_Flag;
    metrics->fFlags |= SkFontMetrics::kStrikeoutPositionIsValid_Flag;

    if (this->getDWriteTypeface()->fDWriteFontFace1.get()) {
        DWRITE_FONT_METRICS1 dwfm1;
        this->getDWriteTypeface()->fDWriteFontFace1->GetMetrics(&dwfm1);
        metrics->fTop = -fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxTop) / upem;
        metrics->fBottom = -fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxBottom) / upem;
        metrics->fXMin = fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxLeft) / upem;
        metrics->fXMax = fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxRight) / upem;

        metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
        return;
    }

    AutoTDWriteTable<SkOTTableHead> head(this->getDWriteTypeface()->fDWriteFontFace.get());
    if (head.fExists &&
        head.fSize >= sizeof(SkOTTableHead) &&
        head->version == SkOTTableHead::version1)
    {
        metrics->fTop = -fTextSizeRender * (int16_t)SkEndian_SwapBE16(head->yMax) / upem;
        metrics->fBottom = -fTextSizeRender * (int16_t)SkEndian_SwapBE16(head->yMin) / upem;
        metrics->fXMin = fTextSizeRender * (int16_t)SkEndian_SwapBE16(head->xMin) / upem;
        metrics->fXMax = fTextSizeRender * (int16_t)SkEndian_SwapBE16(head->xMax) / upem;

        metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
        return;
    }

    metrics->fTop = metrics->fAscent;
    metrics->fBottom = metrics->fDescent;
}

///////////////////////////////////////////////////////////////////////////////

#include "include/private/SkColorData.h"

static void bilevel_to_bw(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph) {
    const int width = glyph.fWidth;
    const size_t dstRB = (width + 7) >> 3;
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(glyph.fImage);

    int byteCount = width >> 3;
    int bitCount = width & 7;

    for (int y = 0; y < glyph.fHeight; ++y) {
        if (byteCount > 0) {
            for (int i = 0; i < byteCount; ++i) {
                unsigned byte = 0;
                byte |= src[0] & (1 << 7);
                byte |= src[1] & (1 << 6);
                byte |= src[2] & (1 << 5);
                byte |= src[3] & (1 << 4);
                byte |= src[4] & (1 << 3);
                byte |= src[5] & (1 << 2);
                byte |= src[6] & (1 << 1);
                byte |= src[7] & (1 << 0);
                dst[i] = byte;
                src += 8;
            }
        }
        if (bitCount > 0) {
            unsigned byte = 0;
            unsigned mask = 0x80;
            for (int i = 0; i < bitCount; i++) {
                byte |= (src[i]) & mask;
                mask >>= 1;
            }
            dst[byteCount] = byte;
        }
        src += bitCount;
        dst += dstRB;
    }
}

template<bool APPLY_PREBLEND>
static void grayscale_to_a8(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph,
                            const uint8_t* table8) {
    const size_t dstRB = glyph.rowBytes();
    const U16CPU width = glyph.fWidth;
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(glyph.fImage);

    for (U16CPU y = 0; y < glyph.fHeight; y++) {
        for (U16CPU i = 0; i < width; i++) {
            U8CPU a = *(src++);
            dst[i] = sk_apply_lut_if<APPLY_PREBLEND>(a, table8);
        }
        dst = SkTAddOffset<uint8_t>(dst, dstRB);
    }
}

template<bool APPLY_PREBLEND>
static void rgb_to_a8(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph, const uint8_t* table8) {
    const size_t dstRB = glyph.rowBytes();
    const U16CPU width = glyph.fWidth;
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(glyph.fImage);

    for (U16CPU y = 0; y < glyph.fHeight; y++) {
        for (U16CPU i = 0; i < width; i++) {
            U8CPU r = *(src++);
            U8CPU g = *(src++);
            U8CPU b = *(src++);
            dst[i] = sk_apply_lut_if<APPLY_PREBLEND>((r + g + b) / 3, table8);
        }
        dst = SkTAddOffset<uint8_t>(dst, dstRB);
    }
}

template<bool APPLY_PREBLEND, bool RGB>
static void rgb_to_lcd16(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph,
                         const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB) {
    const size_t dstRB = glyph.rowBytes();
    const U16CPU width = glyph.fWidth;
    uint16_t* SK_RESTRICT dst = static_cast<uint16_t*>(glyph.fImage);

    for (U16CPU y = 0; y < glyph.fHeight; y++) {
        for (U16CPU i = 0; i < width; i++) {
            U8CPU r, g, b;
            if (RGB) {
                r = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableR);
                g = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableG);
                b = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableB);
            } else {
                b = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableB);
                g = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableG);
                r = sk_apply_lut_if<APPLY_PREBLEND>(*(src++), tableR);
            }
            dst[i] = SkPack888ToRGB16(r, g, b);
        }
        dst = SkTAddOffset<uint16_t>(dst, dstRB);
    }
}

const void* SkScalerContext_DW::drawDWMask(const SkGlyph& glyph,
                                           DWRITE_RENDERING_MODE renderingMode,
                                           DWRITE_TEXTURE_TYPE textureType)
{
    int sizeNeeded = glyph.fWidth * glyph.fHeight;
    if (DWRITE_TEXTURE_CLEARTYPE_3x1 == textureType) {
        sizeNeeded *= 3;
    }
    if (sizeNeeded > fBits.count()) {
        fBits.setCount(sizeNeeded);
    }

    // erase
    memset(fBits.begin(), 0, sizeNeeded);

    fXform.dx = SkFixedToFloat(glyph.getSubXFixed());
    fXform.dy = SkFixedToFloat(glyph.getSubYFixed());

    FLOAT advance = 0.0f;

    UINT16 index = glyph.getGlyphID();

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0.0f;
    offset.ascenderOffset = 0.0f;

    DWRITE_GLYPH_RUN run;
    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = this->getDWriteTypeface()->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &index;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;
    {
        SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
        {
            Exclusive l(DWriteFactoryMutex);
            // IDWriteFactory2::CreateGlyphRunAnalysis is very bad at aliased glyphs.
            if (this->getDWriteTypeface()->fFactory2 &&
                    (fGridFitMode == DWRITE_GRID_FIT_MODE_DISABLED ||
                     fAntiAliasMode == DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE))
            {
                HRNM(this->getDWriteTypeface()->fFactory2->CreateGlyphRunAnalysis(&run,
                         &fXform,
                         renderingMode,
                         fMeasuringMode,
                         fGridFitMode,
                         fAntiAliasMode,
                         0.0f, // baselineOriginX,
                         0.0f, // baselineOriginY,
                         &glyphRunAnalysis),
                     "Could not create DW2 glyph run analysis.");
            } else {
                HRNM(this->getDWriteTypeface()->fFactory->CreateGlyphRunAnalysis(&run,
                         1.0f, // pixelsPerDip,
                         &fXform,
                         renderingMode,
                         fMeasuringMode,
                         0.0f, // baselineOriginX,
                         0.0f, // baselineOriginY,
                         &glyphRunAnalysis),
                     "Could not create glyph run analysis.");
            }
        }
        //NOTE: this assumes that the glyph has already been measured
        //with an exact same glyph run analysis.
        RECT bbox;
        bbox.left = glyph.fLeft;
        bbox.top = glyph.fTop;
        bbox.right = glyph.fLeft + glyph.fWidth;
        bbox.bottom = glyph.fTop + glyph.fHeight;
        {
            Shared l(DWriteFactoryMutex);
            HRNM(glyphRunAnalysis->CreateAlphaTexture(textureType,
                    &bbox,
                    fBits.begin(),
                    sizeNeeded),
                "Could not draw mask.");
        }
    }
    return fBits.begin();
}

void SkScalerContext_DW::generateColorGlyphImage(const SkGlyph& glyph) {
    SkASSERT(isColorGlyph(glyph));
    SkASSERT(glyph.fMaskFormat == SkMask::Format::kARGB32_Format);

    memset(glyph.fImage, 0, glyph.computeImageSize());

    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;
    getColorGlyphRun(glyph, &colorLayers);
    SkASSERT(colorLayers.get());

    SkMatrix matrix = fSkXform;
    matrix.postTranslate(-SkIntToScalar(glyph.fLeft), -SkIntToScalar(glyph.fTop));
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
    }
    SkRasterClip rc(SkIRect::MakeWH(glyph.fWidth, glyph.fHeight));
    SkDraw draw;
    draw.fDst = SkPixmap(SkImageInfo::MakeN32(glyph.fWidth, glyph.fHeight, kPremul_SkAlphaType),
                         glyph.fImage,
                         glyph.rowBytesUsingFormat(SkMask::Format::kARGB32_Format));
    draw.fMatrix = &matrix;
    draw.fRC = &rc;

    SkPaint paint;
    paint.setAntiAlias(fRenderingMode != DWRITE_RENDERING_MODE_ALIASED);

    BOOL hasNextRun = FALSE;
    while (SUCCEEDED(colorLayers->MoveNext(&hasNextRun)) && hasNextRun) {
        const DWRITE_COLOR_GLYPH_RUN* colorGlyph;
        HRVM(colorLayers->GetCurrentRun(&colorGlyph), "Could not get current color glyph run");

        SkColor color;
        if (colorGlyph->paletteIndex != 0xffff) {
            color = SkColorSetARGB(sk_float_round2int(colorGlyph->runColor.a * 255),
                                   sk_float_round2int(colorGlyph->runColor.r * 255),
                                   sk_float_round2int(colorGlyph->runColor.g * 255),
                                   sk_float_round2int(colorGlyph->runColor.b * 255));
        } else {
            // If all components of runColor are 0 or (equivalently) paletteIndex is 0xFFFF then
            // the 'current brush' is used. fRec.getLuminanceColor() is kinda sorta what is wanted
            // here, but not really, it will often be the wrong value because it wan't designed for
            // this.
            // TODO: implement this fully, bug.skia.org/5788
            color = fRec.getLuminanceColor();
        }
        paint.setColor(color);

        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRVM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
             "Could not create geometry to path converter.");
        {
            Exclusive l(DWriteFactoryMutex);
            HRVM(colorGlyph->glyphRun.fontFace->GetGlyphRunOutline(
                colorGlyph->glyphRun.fontEmSize,
                colorGlyph->glyphRun.glyphIndices,
                colorGlyph->glyphRun.glyphAdvances,
                colorGlyph->glyphRun.glyphOffsets,
                colorGlyph->glyphRun.glyphCount,
                colorGlyph->glyphRun.isSideways,
                colorGlyph->glyphRun.bidiLevel % 2, //rtl
                geometryToPath.get()),
                "Could not create glyph outline.");
        }
        draw.drawPath(path, paint, nullptr, true /* pathIsMutable */);
    }
}

void SkScalerContext_DW::generatePngGlyphImage(const SkGlyph& glyph) {
    SkASSERT(isPngGlyph(glyph));
    SkASSERT(glyph.fMaskFormat == SkMask::Format::kARGB32_Format);
    SkASSERT(this->getDWriteTypeface()->fDWriteFontFace4);

    IDWriteFontFace4* fontFace4 = this->getDWriteTypeface()->fDWriteFontFace4.get();
    DWRITE_GLYPH_IMAGE_DATA glyphData;
    void* glyphDataContext;
    HRVM(fontFace4->GetGlyphImageData(glyph.getGlyphID(),
                                      fTextSizeRender,
                                      DWRITE_GLYPH_IMAGE_FORMATS_PNG,
                                      &glyphData,
                                      &glyphDataContext),
         "Glyph image data could not be acquired.");
    Context* context = new Context(fontFace4, glyphDataContext);
    sk_sp<SkData> data = SkData::MakeWithProc(glyphData.imageData,
                                              glyphData.imageDataSize,
                                              &ReleaseProc,
                                              context);
    sk_sp<SkImage> image = SkImage::MakeFromEncoded(std::move(data));

    SkBitmap dstBitmap;
    dstBitmap.setInfo(SkImageInfo::Make(glyph.fWidth, glyph.fHeight,
                                        kN32_SkColorType,
                                        kPremul_SkAlphaType),
                      glyph.rowBytes());
    dstBitmap.setPixels(glyph.fImage);

    SkCanvas canvas(dstBitmap);
    canvas.clear(SK_ColorTRANSPARENT);
    canvas.translate(-glyph.fLeft, -glyph.fTop);
    if (this->isSubpixel()) {
        canvas.translate(SkFixedToScalar(glyph.getSubXFixed()),
                         SkFixedToScalar(glyph.getSubYFixed()));
    }
    canvas.concat(fSkXform);
    SkScalar ratio = fTextSizeRender / glyphData.pixelsPerEm;
    canvas.scale(ratio, ratio);
    canvas.translate(-glyphData.horizontalLeftOrigin.x, -glyphData.horizontalLeftOrigin.y);
    canvas.drawImage(image, 0, 0, nullptr);
}

void SkScalerContext_DW::generateImage(const SkGlyph& glyph) {
    //Create the mask.
    DWRITE_RENDERING_MODE renderingMode = fRenderingMode;
    DWRITE_TEXTURE_TYPE textureType = fTextureType;
    if (glyph.fForceBW) {
        renderingMode = DWRITE_RENDERING_MODE_ALIASED;
        textureType = DWRITE_TEXTURE_ALIASED_1x1;
    }

    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        if (fIsColorFont) {
            if (isColorGlyph(glyph)) {
                generateColorGlyphImage(glyph);
                return;
            } else if (isPngGlyph(glyph)) {
                generatePngGlyphImage(glyph);
                return;
            }
        }
        SkDEBUGFAIL("Could not generate image from the given color font format.");
        return;
    }

    const void* bits = this->drawDWMask(glyph, renderingMode, textureType);
    if (!bits) {
        sk_bzero(glyph.fImage, glyph.computeImageSize());
        return;
    }

    //Copy the mask into the glyph.
    const uint8_t* src = (const uint8_t*)bits;
    if (DWRITE_RENDERING_MODE_ALIASED == renderingMode) {
        SkASSERT(SkMask::kBW_Format == glyph.fMaskFormat);
        SkASSERT(DWRITE_TEXTURE_ALIASED_1x1 == textureType);
        bilevel_to_bw(src, glyph);
    } else if (!isLCD(fRec)) {
        if (textureType == DWRITE_TEXTURE_ALIASED_1x1) {
            if (fPreBlend.isApplicable()) {
                grayscale_to_a8<true>(src, glyph, fPreBlend.fG);
            } else {
                grayscale_to_a8<false>(src, glyph, fPreBlend.fG);
            }
        } else {
            if (fPreBlend.isApplicable()) {
                rgb_to_a8<true>(src, glyph, fPreBlend.fG);
            } else {
                rgb_to_a8<false>(src, glyph, fPreBlend.fG);
            }
        }
    } else {
        SkASSERT(SkMask::kLCD16_Format == glyph.fMaskFormat);
        if (fPreBlend.isApplicable()) {
            if (fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag) {
                rgb_to_lcd16<true, false>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                rgb_to_lcd16<true, true>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        } else {
            if (fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag) {
                rgb_to_lcd16<false, false>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                rgb_to_lcd16<false, true>(src, glyph, fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        }
    }
}

bool SkScalerContext_DW::generatePath(SkGlyphID glyph, SkPath* path) {
    SkASSERT(path);

    path->reset();

    SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
    HRBM(SkDWriteGeometrySink::Create(path, &geometryToPath),
         "Could not create geometry to path converter.");
    UINT16 glyphId = SkTo<UINT16>(glyph);
    {
        Exclusive l(DWriteFactoryMutex);
        //TODO: convert to<->from DIUs? This would make a difference if hinting.
        //It may not be needed, it appears that DirectWrite only hints at em size.
        HRBM(this->getDWriteTypeface()->fDWriteFontFace->GetGlyphRunOutline(
             SkScalarToFloat(fTextSizeRender),
             &glyphId,
             nullptr, //advances
             nullptr, //offsets
             1, //num glyphs
             FALSE, //sideways
             FALSE, //rtl
             geometryToPath.get()),
             "Could not create glyph outline.");
    }

    path->transform(fSkXform);
    return true;
}

#endif//defined(SK_BUILD_FOR_WIN)
