/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#undef GetGlyphIndices

#include "SkDraw.h"
#include "SkDWrite.h"
#include "SkDWriteGeometrySink.h"
#include "SkEndian.h"
#include "SkGlyph.h"
#include "SkHRESULT.h"
#include "SkMaskGamma.h"
#include "SkMatrix22.h"
#include "SkMutex.h"
#include "SkOTTable_EBLC.h"
#include "SkOTTable_EBSC.h"
#include "SkOTTable_gasp.h"
#include "SkOTTable_maxp.h"
#include "SkPath.h"
#include "SkRasterClip.h"
#include "SkScalerContext.h"
#include "SkScalerContext_win_dw.h"
#include "SkSharedMutex.h"
#include "SkTScopedComPtr.h"
#include "SkTypeface_win_dw.h"

#include <dwrite.h>
#if SK_HAS_DWRITE_1_H
#  include <dwrite_1.h>
#endif

/* Note:
 * In versions 8 and 8.1 of Windows, some calls in DWrite are not thread safe.
 * The DWriteFactoryMutex protects the calls that are problematic.
 */
static SkSharedMutex DWriteFactoryMutex;

typedef SkAutoSharedMutexShared Shared;

static bool isLCD(const SkScalerContext::Rec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

static bool is_hinted_without_gasp(DWriteFontTypeface* typeface) {
    SkAutoExclusive l(DWriteFactoryMutex);
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

    if (0 == maxp->version.tt.maxSizeOfInstructions) {
        // No hints.
        return false;
    }

    AutoTDWriteTable<SkOTTableGridAndScanProcedure> gasp(typeface->fDWriteFontFace.get());
    return !gasp.fExists;
}

/** A PPEMRange is inclusive, [min, max]. */
struct PPEMRange {
    int min;
    int max;
};

/** If the rendering mode for the specified 'size' is gridfit, then place
 *  the gridfit range into 'range'. Otherwise, leave 'range' alone.
 */
static void expand_range_if_gridfit_only(DWriteFontTypeface* typeface, int size, PPEMRange* range) {
    AutoTDWriteTable<SkOTTableGridAndScanProcedure> gasp(typeface->fDWriteFontFace.get());
    if (!gasp.fExists) {
        return;
    }
    if (gasp.fSize < sizeof(SkOTTableGridAndScanProcedure)) {
        return;
    }
    if (gasp->version != SkOTTableGridAndScanProcedure::version0 &&
        gasp->version != SkOTTableGridAndScanProcedure::version1)
    {
        return;
    }

    uint16_t numRanges = SkEndianSwap16(gasp->numRanges);
    if (numRanges > 1024 ||
        gasp.fSize < sizeof(SkOTTableGridAndScanProcedure) +
                     sizeof(SkOTTableGridAndScanProcedure::GaspRange) * numRanges)
    {
        return;
    }

    const SkOTTableGridAndScanProcedure::GaspRange* rangeTable =
            SkTAfter<const SkOTTableGridAndScanProcedure::GaspRange>(gasp.get());
    int minPPEM = -1;
    for (uint16_t i = 0; i < numRanges; ++i, ++rangeTable) {
        int maxPPEM = SkEndianSwap16(rangeTable->maxPPEM);
        // Test that the size is in range and the range is gridfit only.
        if (minPPEM < size && size <= maxPPEM &&
            rangeTable->flags.raw.value == SkOTTableGridAndScanProcedure::GaspRange::behavior::Raw::GridfitMask)
        {
            range->min = minPPEM + 1;
            range->max = maxPPEM;
            return;
        }
        minPPEM = maxPPEM;
    }
}

static bool has_bitmap_strike(DWriteFontTypeface* typeface, PPEMRange range) {
    SkAutoExclusive l(DWriteFactoryMutex);
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
                range.min <= sizeTable->ppemX && sizeTable->ppemX <= range.max)
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
                range.min <= scaleTable->ppemX && scaleTable->ppemX <= range.max) {
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
static bool is_axis_aligned(const SkScalerContext::Rec& rec) {
    return 0 == rec.fPreSkewX &&
           (both_zero(rec.fPost2x2[0][1], rec.fPost2x2[1][0]) ||
            both_zero(rec.fPost2x2[0][0], rec.fPost2x2[1][1]));
}

SkScalerContext_DW::SkScalerContext_DW(DWriteFontTypeface* typeface,
                                       const SkScalerContextEffects& effects,
                                       const SkDescriptor* desc)
        : SkScalerContext(typeface, effects, desc)
        , fTypeface(SkRef(typeface))
        , fGlyphCount(-1) {

#if SK_HAS_DWRITE_2_H
    fTypeface->fFactory->QueryInterface<IDWriteFactory2>(&fFactory2);

    SkTScopedComPtr<IDWriteFontFace2> fontFace2;
    fTypeface->fDWriteFontFace->QueryInterface<IDWriteFontFace2>(&fontFace2);
    fIsColorFont = fFactory2.get() && fontFace2.get() && fontFace2->IsColorFont();
#endif

    // In general, all glyphs should use CLEARTYPE_NATURAL_SYMMETRIC
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
    SkMatrix GsA;
    fRec.computeMatrices(SkScalerContextRec::kVertical_PreMatrixScale,
                         &scale, &fSkXform, &GsA, &fG_inv);

    fXform.m11 = SkScalarToFloat(fSkXform.getScaleX());
    fXform.m12 = SkScalarToFloat(fSkXform.getSkewY());
    fXform.m21 = SkScalarToFloat(fSkXform.getSkewX());
    fXform.m22 = SkScalarToFloat(fSkXform.getScaleY());
    fXform.dx = 0;
    fXform.dy = 0;

    fGsA.m11 = SkScalarToFloat(GsA.get(SkMatrix::kMScaleX));
    fGsA.m12 = SkScalarToFloat(GsA.get(SkMatrix::kMSkewY)); // This should be ~0.
    fGsA.m21 = SkScalarToFloat(GsA.get(SkMatrix::kMSkewX));
    fGsA.m22 = SkScalarToFloat(GsA.get(SkMatrix::kMScaleY));
    fGsA.dx = 0;
    fGsA.dy = 0;

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
        PPEMRange range = { bitmapPPEM, bitmapPPEM };
        expand_range_if_gridfit_only(typeface, bitmapPPEM, &range);
        treatLikeBitmap = has_bitmap_strike(typeface, range);

        axisAlignedBitmap = is_axis_aligned(fRec);
    }

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
        fRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_GDI_CLASSIC;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = gdiTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_GDI_CLASSIC;

    // If rotated but the horizontal text could have used a bitmap,
    // render high quality rotated glyphs but measure using bitmap metrics.
    } else if (treatLikeBitmap) {
        fTextSizeRender = gdiTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = gdiTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_GDI_CLASSIC;

    // Fonts that have hints but no gasp table get non-symmetric rendering.
    // Usually such fonts have low quality hints which were never tested
    // with anything but GDI ClearType classic. Such fonts often rely on
    // drop out control in the y direction in order to be legible.
    } else if (is_hinted_without_gasp(typeface)) {
        fTextSizeRender = gdiTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;

    // The normal case is to use natural symmetric rendering and linear metrics.
    } else {
        fTextSizeRender = realTextSize;
        fRenderingMode = DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC;
        fTextureType = DWRITE_TEXTURE_CLEARTYPE_3x1;
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;
    }

    if (this->isSubpixel()) {
        fTextSizeMeasure = realTextSize;
        fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;
    }
}

SkScalerContext_DW::~SkScalerContext_DW() {
}

unsigned SkScalerContext_DW::generateGlyphCount() {
    if (fGlyphCount < 0) {
        fGlyphCount = fTypeface->fDWriteFontFace->GetGlyphCount();
    }
    return fGlyphCount;
}

uint16_t SkScalerContext_DW::generateCharToGlyph(SkUnichar uni) {
    uint16_t index = 0;
    fTypeface->fDWriteFontFace->GetGlyphIndices(reinterpret_cast<UINT32*>(&uni), 1, &index);
    return index;
}

void SkScalerContext_DW::generateAdvance(SkGlyph* glyph) {
    //Delta is the difference between the right/left side bearing metric
    //and where the right/left side bearing ends up after hinting.
    //DirectWrite does not provide this information.
    glyph->fRsbDelta = 0;
    glyph->fLsbDelta = 0;

    glyph->fAdvanceX = 0;
    glyph->fAdvanceY = 0;

    uint16_t glyphId = glyph->getGlyphID();
    DWRITE_GLYPH_METRICS gm;

    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        SkAutoExclusive l(DWriteFactoryMutex);
        HRVM(fTypeface->fDWriteFontFace->GetGdiCompatibleGlyphMetrics(
                 fTextSizeMeasure,
                 1.0f, // pixelsPerDip
                 &fGsA,
                 DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode,
                 &glyphId, 1,
                 &gm),
             "Could not get gdi compatible glyph metrics.");
    } else {
        SkAutoExclusive l(DWriteFactoryMutex);
        HRVM(fTypeface->fDWriteFontFace->GetDesignGlyphMetrics(&glyphId, 1, &gm),
             "Could not get design metrics.");
    }

    DWRITE_FONT_METRICS dwfm;
    {
        Shared l(DWriteFactoryMutex);
        fTypeface->fDWriteFontFace->GetMetrics(&dwfm);
    }
    SkScalar advanceX = SkScalarMulDiv(fTextSizeMeasure,
                                       SkIntToScalar(gm.advanceWidth),
                                       SkIntToScalar(dwfm.designUnitsPerEm));

    SkVector vecs[1] = { { advanceX, 0 } };
    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        // DirectWrite produced 'compatible' metrics, but while close,
        // the end result is not always an integer as it would be with GDI.
        vecs[0].fX = SkScalarRoundToScalar(advanceX);
        fG_inv.mapVectors(vecs, SK_ARRAY_COUNT(vecs));
    } else {
        fSkXform.mapVectors(vecs, SK_ARRAY_COUNT(vecs));
    }

    glyph->fAdvanceX = SkScalarToFloat(vecs[0].fX);
    glyph->fAdvanceY = SkScalarToFloat(vecs[0].fY);
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
    run.fontFace = fTypeface->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &glyphId;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
    {
        SkAutoExclusive l(DWriteFactoryMutex);
        HRM(fTypeface->fFactory->CreateGlyphRunAnalysis(
            &run,
            1.0f, // pixelsPerDip,
            &fXform,
            renderingMode,
            fMeasuringMode,
            0.0f, // baselineOriginX,
            0.0f, // baselineOriginY,
            &glyphRunAnalysis),
            "Could not create glyph run analysis.");
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
    glyph->fWidth = SkToU16(bbox.right - bbox.left);
    glyph->fHeight = SkToU16(bbox.bottom - bbox.top);
    glyph->fLeft = SkToS16(bbox.left);
    glyph->fTop = SkToS16(bbox.top);
    return true;
}

bool SkScalerContext_DW::isColorGlyph(const SkGlyph& glyph) {
#if SK_HAS_DWRITE_2_H
    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayer;
    if (getColorGlyphRun(glyph, &colorLayer)) {
        return true;
    }
#endif
    return false;
}

#if SK_HAS_DWRITE_2_H
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
    run.fontFace = fTypeface->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &glyphId;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    HRESULT hr = fFactory2->TranslateColorGlyphRun(
        0, 0, &run, nullptr, fMeasuringMode, &fXform, 0, colorGlyph);
    if (hr == DWRITE_E_NOCOLOR) {
        return false;
    }
    HRBM(hr, "Failed to translate color glyph run");
    return true;
}
#endif

void SkScalerContext_DW::generateMetrics(SkGlyph* glyph) {
    glyph->fWidth = 0;
    glyph->fHeight = 0;
    glyph->fLeft = 0;
    glyph->fTop = 0;

    this->generateAdvance(glyph);

#if SK_HAS_DWRITE_2_H
    if (fIsColorFont && isColorGlyph(*glyph)) {
        glyph->fMaskFormat = SkMask::kARGB32_Format;
    }
#endif

    RECT bbox;
    HRVM(this->getBoundingBox(glyph, fRenderingMode, fTextureType, &bbox),
         "Requested bounding box could not be determined.");

    if (glyph_check_and_set_bounds(glyph, bbox)) {
        return;
    }

    // GetAlphaTextureBounds succeeds but returns an empty RECT if there are no
    // glyphs of the specified texture type. When this happens, try with the
    // alternate texture type.
    if (DWRITE_TEXTURE_CLEARTYPE_3x1 == fTextureType) {
        HRVM(this->getBoundingBox(glyph,
                                  DWRITE_RENDERING_MODE_ALIASED,
                                  DWRITE_TEXTURE_ALIASED_1x1,
                                  &bbox),
             "Fallback bounding box could not be determined.");
        if (glyph_check_and_set_bounds(glyph, bbox)) {
            glyph->fForceBW = 1;
        }
    }
    // TODO: handle the case where a request for DWRITE_TEXTURE_ALIASED_1x1
    // fails, and try DWRITE_TEXTURE_CLEARTYPE_3x1.
}

void SkScalerContext_DW::generateFontMetrics(SkPaint::FontMetrics* metrics) {
    if (nullptr == metrics) {
        return;
    }

    sk_bzero(metrics, sizeof(*metrics));

    DWRITE_FONT_METRICS dwfm;
    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        fTypeface->fDWriteFontFace->GetGdiCompatibleMetrics(
             fTextSizeRender,
             1.0f, // pixelsPerDip
             &fXform,
             &dwfm);
    } else {
        fTypeface->fDWriteFontFace->GetMetrics(&dwfm);
    }

    SkScalar upem = SkIntToScalar(dwfm.designUnitsPerEm);

    metrics->fAscent = -fTextSizeRender * SkIntToScalar(dwfm.ascent) / upem;
    metrics->fDescent = fTextSizeRender * SkIntToScalar(dwfm.descent) / upem;
    metrics->fLeading = fTextSizeRender * SkIntToScalar(dwfm.lineGap) / upem;
    metrics->fXHeight = fTextSizeRender * SkIntToScalar(dwfm.xHeight) / upem;
    metrics->fUnderlineThickness = fTextSizeRender * SkIntToScalar(dwfm.underlineThickness) / upem;
    metrics->fUnderlinePosition = -(fTextSizeRender * SkIntToScalar(dwfm.underlinePosition) / upem);

    metrics->fFlags |= SkPaint::FontMetrics::kUnderlineThinknessIsValid_Flag;
    metrics->fFlags |= SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag;

#if SK_HAS_DWRITE_1_H
    if (fTypeface->fDWriteFontFace1.get()) {
        DWRITE_FONT_METRICS1 dwfm1;
        fTypeface->fDWriteFontFace1->GetMetrics(&dwfm1);
        metrics->fTop = -fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxTop) / upem;
        metrics->fBottom = -fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxBottom) / upem;
        metrics->fXMin = fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxLeft) / upem;
        metrics->fXMax = fTextSizeRender * SkIntToScalar(dwfm1.glyphBoxRight) / upem;

        metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
        return;
    }
#else
#  pragma message("No dwrite_1.h is available, font metrics may be affected.")
#endif

    AutoTDWriteTable<SkOTTableHead> head(fTypeface->fDWriteFontFace.get());
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

#include "SkColorPriv.h"

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
        dst = (uint8_t*)((char*)dst + dstRB);
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
        dst = (uint16_t*)((char*)dst + dstRB);
    }
}

const void* SkScalerContext_DW::drawDWMask(const SkGlyph& glyph,
                                           DWRITE_RENDERING_MODE renderingMode,
                                           DWRITE_TEXTURE_TYPE textureType)
{
    int sizeNeeded = glyph.fWidth * glyph.fHeight;
    if (DWRITE_RENDERING_MODE_ALIASED != renderingMode) {
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
    run.fontFace = fTypeface->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &index;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;
    {

        SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
        {
            SkAutoExclusive l(DWriteFactoryMutex);
            HRNM(fTypeface->fFactory->CreateGlyphRunAnalysis(&run,
                1.0f, // pixelsPerDip,
                &fXform,
                renderingMode,
                fMeasuringMode,
                0.0f, // baselineOriginX,
                0.0f, // baselineOriginY,
                &glyphRunAnalysis),
                "Could not create glyph run analysis.");
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

#if SK_HAS_DWRITE_2_H
void SkScalerContext_DW::generateColorGlyphImage(const SkGlyph& glyph) {
    SkASSERT(isColorGlyph(glyph));
    SkASSERT(glyph.fMaskFormat == SkMask::Format::kARGB32_Format);

    memset(glyph.fImage, 0, glyph.computeImageSize());

    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;
    getColorGlyphRun(glyph, &colorLayers);
    SkASSERT(colorLayers.get());

    SkMatrix matrix = fSkXform;
    matrix.postTranslate(-SkIntToScalar(glyph.fLeft), -SkIntToScalar(glyph.fTop));
    SkRasterClip rc(SkIRect::MakeWH(glyph.fWidth, glyph.fHeight));
    SkDraw draw;
    draw.fDst = SkPixmap(SkImageInfo::MakeN32(glyph.fWidth, glyph.fHeight, kPremul_SkAlphaType),
                         glyph.fImage,
                         glyph.ComputeRowBytes(glyph.fWidth, SkMask::Format::kARGB32_Format));
    draw.fMatrix = &matrix;
    draw.fRC = &rc;

    SkPaint paint;
    if (fRenderingMode != DWRITE_RENDERING_MODE_ALIASED) {
        paint.setFlags(SkPaint::Flags::kAntiAlias_Flag);
    }

    BOOL hasNextRun = FALSE;
    while (SUCCEEDED(colorLayers->MoveNext(&hasNextRun)) && hasNextRun) {
        const DWRITE_COLOR_GLYPH_RUN* colorGlyph;
        HRVM(colorLayers->GetCurrentRun(&colorGlyph), "Could not get current color glyph run");

        SkColor color;
        if (colorGlyph->paletteIndex != 0xffff) {
            color = SkColorSetARGB(SkFloatToIntRound(colorGlyph->runColor.a * 255),
                                   SkFloatToIntRound(colorGlyph->runColor.r * 255),
                                   SkFloatToIntRound(colorGlyph->runColor.g * 255),
                                   SkFloatToIntRound(colorGlyph->runColor.b * 255));
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
            SkAutoExclusive l(DWriteFactoryMutex);
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
#endif

void SkScalerContext_DW::generateImage(const SkGlyph& glyph) {
    //Create the mask.
    DWRITE_RENDERING_MODE renderingMode = fRenderingMode;
    DWRITE_TEXTURE_TYPE textureType = fTextureType;
    if (glyph.fForceBW) {
        renderingMode = DWRITE_RENDERING_MODE_ALIASED;
        textureType = DWRITE_TEXTURE_ALIASED_1x1;
    }

#if SK_HAS_DWRITE_2_H
    if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
        generateColorGlyphImage(glyph);
        return;
    }
#endif

    const void* bits = this->drawDWMask(glyph, renderingMode, textureType);
    if (!bits) {
        sk_bzero(glyph.fImage, glyph.computeImageSize());
        return;
    }

    //Copy the mask into the glyph.
    const uint8_t* src = (const uint8_t*)bits;
    if (DWRITE_RENDERING_MODE_ALIASED == renderingMode) {
        bilevel_to_bw(src, glyph);
        const_cast<SkGlyph&>(glyph).fMaskFormat = SkMask::kBW_Format;
    } else if (!isLCD(fRec)) {
        if (fPreBlend.isApplicable()) {
            rgb_to_a8<true>(src, glyph, fPreBlend.fG);
        } else {
            rgb_to_a8<false>(src, glyph, fPreBlend.fG);
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

void SkScalerContext_DW::generatePath(const SkGlyph& glyph, SkPath* path) {
    SkASSERT(path);

    path->reset();

    SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
    HRVM(SkDWriteGeometrySink::Create(path, &geometryToPath),
         "Could not create geometry to path converter.");
    uint16_t glyphId = glyph.getGlyphID();
    {
        SkAutoExclusive l(DWriteFactoryMutex);
        //TODO: convert to<->from DIUs? This would make a difference if hinting.
        //It may not be needed, it appears that DirectWrite only hints at em size.
        HRVM(fTypeface->fDWriteFontFace->GetGlyphRunOutline(SkScalarToFloat(fTextSizeRender),
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
}

#endif//defined(SK_BUILD_FOR_WIN32)
