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
#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkOpenTypeSVGDecoder.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkMutex.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEndian.h"
#include "src/base/SkScopeExit.h"
#include "src/base/SkSharedMutex.h"
#include "src/core/SkDraw.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkMaskGamma.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkScalerContext.h"
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

namespace {
static inline const constexpr bool kSkShowTextBlitCoverage = false;

/* Note:
 * In versions 8 and 8.1 of Windows, some calls in DWrite are not thread safe.
 * The mutex returned from maybe_dw_mutex protects the calls that are
 * problematic.
 */
static SkSharedMutex* maybe_dw_mutex(DWriteFontTypeface& typeface) {
    static SkSharedMutex mutex;
    return typeface.fDWriteFontFace4 ? nullptr : &mutex;
}

class SK_SCOPED_CAPABILITY Exclusive {
public:
    explicit Exclusive(SkSharedMutex* maybe_lock) SK_ACQUIRE(*maybe_lock)
        : fLock(maybe_lock) {
        if (fLock) {
            fLock->acquire();
        }
    }
    ~Exclusive() SK_RELEASE_CAPABILITY() {
        if (fLock) {
            fLock->release();
        }
    }

private:
    SkSharedMutex* fLock;
};
class SK_SCOPED_CAPABILITY Shared {
public:
    explicit Shared(SkSharedMutex* maybe_lock) SK_ACQUIRE_SHARED(*maybe_lock)
        : fLock(maybe_lock)  {
        if (fLock) {
            fLock->acquireShared();
        }
    }

    // You would think this should be SK_RELEASE_SHARED_CAPABILITY, but SK_SCOPED_CAPABILITY
    // doesn't fully understand the difference between shared and exclusive.
    // Please review https://reviews.llvm.org/D52578 for more information.
    ~Shared() SK_RELEASE_CAPABILITY() {
        if (fLock) {
            fLock->releaseShared();
        }
    }

private:
    SkSharedMutex* fLock;
};

static bool isLCD(const SkScalerContextRec& rec) {
    return SkMask::kLCD16_Format == rec.fMaskFormat;
}

static bool is_hinted(DWriteFontTypeface* typeface) {
    Exclusive l(maybe_dw_mutex(*typeface));
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
    Exclusive l(maybe_dw_mutex(*typeface));
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

}  //namespace

SkScalerContext_DW::SkScalerContext_DW(sk_sp<DWriteFontTypeface> typefaceRef,
                                       const SkScalerContextEffects& effects,
                                       const SkDescriptor* desc)
        : SkScalerContext(std::move(typefaceRef), effects, desc)
{
    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    fGlyphCount = typeface->fDWriteFontFace->GetGlyphCount();

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
    fRec.computeMatrices(SkScalerContextRec::PreMatrixScale::kVertical, &scale, &fSkXform);

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

    // The GDI measuring modes don't seem to work well with CBDT fonts (DWrite.dll 10.0.18362.836).
    if (fMeasuringMode != DWRITE_MEASURING_MODE_NATURAL) {
        constexpr UINT32 CBDTTag = DWRITE_MAKE_OPENTYPE_TAG('C','B','D','T');
        AutoDWriteTable CBDT(typeface->fDWriteFontFace.get(), CBDTTag);
        if (CBDT.fExists) {
            fMeasuringMode = DWRITE_MEASURING_MODE_NATURAL;
        }
    }
}

SkScalerContext_DW::~SkScalerContext_DW() {
}

#if DWRITE_CORE || (defined(NTDDI_WIN11_ZN) && NTDDI_VERSION >= NTDDI_WIN11_ZN)

namespace {
SkColor4f sk_color_from(DWRITE_COLOR_F const& color) {
    // DWRITE_COLOR_F and SkColor4f are laid out the same and this should be a no-op.
    return SkColor4f{ color.r, color.g, color.b, color.a };
}
DWRITE_COLOR_F dw_color_from(SkColor4f const& color) {
    // DWRITE_COLOR_F and SkColor4f are laid out the same and this should be a no-op.
    // Avoid brace initialization as DWRITE_COLOR_F can be defined as four floats (dxgitype.h,
    // d3d9types.h) or four unions of two floats (dwrite_2.h, d3dtypes.h). The type changed in
    // Direct3D 10, but the change does not appear to be documented.
    DWRITE_COLOR_F dwColor;
    dwColor.r = color.fR;
    dwColor.g = color.fG;
    dwColor.b = color.fB;
    dwColor.a = color.fA;
    return dwColor;
}

SkRect sk_rect_from(D2D_RECT_F const& rect) {
    // D2D_RECT_F and SkRect are both y-down and laid the same so this should be a no-op.
    return SkRect{ rect.left, rect.top, rect.right, rect.bottom };
}
constexpr bool D2D_RECT_F_is_empty(const D2D_RECT_F& r) {
    return r.right <= r.left || r.bottom <= r.top;
}

SkMatrix sk_matrix_from(DWRITE_MATRIX const& m) {
    // DWRITE_MATRIX and SkMatrix are y-down. However DWRITE_MATRIX is affine only.
    return SkMatrix::MakeAll(
        m.m11, m.m21, m.dx,
        m.m12, m.m22, m.dy,
        0, 0, 1);
}

SkTileMode sk_tile_mode_from(D2D1_EXTEND_MODE extendMode) {
    switch (extendMode) {
    case D2D1_EXTEND_MODE_CLAMP:
        return SkTileMode::kClamp;
    case D2D1_EXTEND_MODE_WRAP:
        return SkTileMode::kRepeat;
    case D2D1_EXTEND_MODE_MIRROR:
        return SkTileMode::kMirror;
    default:
        return SkTileMode::kClamp;
    }
}

SkBlendMode sk_blend_mode_from(DWRITE_COLOR_COMPOSITE_MODE compositeMode) {
    switch (compositeMode) {
    case DWRITE_COLOR_COMPOSITE_CLEAR:
        return SkBlendMode::kClear;
    case DWRITE_COLOR_COMPOSITE_SRC:
        return SkBlendMode::kSrc;
    case DWRITE_COLOR_COMPOSITE_DEST:
        return SkBlendMode::kDst;
    case DWRITE_COLOR_COMPOSITE_SRC_OVER:
        return SkBlendMode::kSrcOver;
    case DWRITE_COLOR_COMPOSITE_DEST_OVER:
        return SkBlendMode::kDstOver;
    case DWRITE_COLOR_COMPOSITE_SRC_IN:
        return SkBlendMode::kSrcIn;
    case DWRITE_COLOR_COMPOSITE_DEST_IN:
        return SkBlendMode::kDstIn;
    case DWRITE_COLOR_COMPOSITE_SRC_OUT:
        return SkBlendMode::kSrcOut;
    case DWRITE_COLOR_COMPOSITE_DEST_OUT:
        return SkBlendMode::kDstOut;
    case DWRITE_COLOR_COMPOSITE_SRC_ATOP:
        return SkBlendMode::kSrcATop;
    case DWRITE_COLOR_COMPOSITE_DEST_ATOP:
        return SkBlendMode::kDstATop;
    case DWRITE_COLOR_COMPOSITE_XOR:
        return SkBlendMode::kXor;
    case DWRITE_COLOR_COMPOSITE_PLUS:
        return SkBlendMode::kPlus;

    case DWRITE_COLOR_COMPOSITE_SCREEN:
        return SkBlendMode::kScreen;
    case DWRITE_COLOR_COMPOSITE_OVERLAY:
        return SkBlendMode::kOverlay;
    case DWRITE_COLOR_COMPOSITE_DARKEN:
        return SkBlendMode::kDarken;
    case DWRITE_COLOR_COMPOSITE_LIGHTEN:
        return SkBlendMode::kLighten;
    case DWRITE_COLOR_COMPOSITE_COLOR_DODGE:
        return SkBlendMode::kColorDodge;
    case DWRITE_COLOR_COMPOSITE_COLOR_BURN:
        return SkBlendMode::kColorBurn;
    case DWRITE_COLOR_COMPOSITE_HARD_LIGHT:
        return SkBlendMode::kHardLight;
    case DWRITE_COLOR_COMPOSITE_SOFT_LIGHT:
        return SkBlendMode::kSoftLight;
    case DWRITE_COLOR_COMPOSITE_DIFFERENCE:
        return SkBlendMode::kDifference;
    case DWRITE_COLOR_COMPOSITE_EXCLUSION:
        return SkBlendMode::kExclusion;
    case DWRITE_COLOR_COMPOSITE_MULTIPLY:
        return SkBlendMode::kMultiply;

    case DWRITE_COLOR_COMPOSITE_HSL_HUE:
        return SkBlendMode::kHue;
    case DWRITE_COLOR_COMPOSITE_HSL_SATURATION:
        return SkBlendMode::kSaturation;
    case DWRITE_COLOR_COMPOSITE_HSL_COLOR:
        return SkBlendMode::kColor;
    case DWRITE_COLOR_COMPOSITE_HSL_LUMINOSITY:
        return SkBlendMode::kLuminosity;
    default:
        return SkBlendMode::kDst;
    }
}

inline SkPoint SkVectorProjection(SkPoint a, SkPoint b) {
    SkScalar length = b.length();
    if (!length) {
        return SkPoint();
    }
    SkPoint bNormalized = b;
    bNormalized.normalize();
    bNormalized.scale(SkPoint::DotProduct(a, b) / length);
    return bNormalized;
}

// This linear interpolation is used for calculating a truncated color line in special edge cases.
// This interpolation needs to be kept in sync with what the gradient shader would normally do when
// truncating and drawing color lines. When drawing into N32 surfaces, this is expected to be true.
// If that changes, or if we support other color spaces in CPAL tables at some point, this needs to
// be looked at.
D2D1_COLOR_F lerpSkColor(D2D1_COLOR_F c0, D2D1_COLOR_F c1, float t) {
    // Due to the floating point calculation in the caller, when interpolating between very narrow
    // stops, we may get values outside the interpolation range, guard against these.
    if (t < 0) {
        return c0;
    }
    if (t > 1) {
        return c1;
    }
    const auto c0_4f = skvx::float4(c0.r, c0.g, c0.b, c0.a),
               c1_4f = skvx::float4(c1.r, c1.g, c1.b, c1.a),
                c_4f = c0_4f + (c1_4f - c0_4f) * t;
    D2D1_COLOR_F r;
    c_4f.store(&r);
    return r;
}

enum TruncateStops {
    TruncateStart,
    TruncateEnd,
};
// Truncate a vector of color stops at a previously computed stop position and insert at that
// position the color interpolated between the surrounding stops.
void truncateToStopInterpolating(SkScalar zeroRadiusStop,
                                 std::vector<D2D1_GRADIENT_STOP>& stops,
                                 TruncateStops truncateStops) {
    if (stops.size() <= 1u ||
        zeroRadiusStop < stops.front().position || stops.back().position < zeroRadiusStop) {
        return;
    }

    auto lcmp = [](D2D1_GRADIENT_STOP const& stop, SkScalar position) {
        return stop.position < position;
    };
    auto ucmp = [](SkScalar position, D2D1_GRADIENT_STOP const& stop) {
        return position < stop.position;
    };
    size_t afterIndex = (truncateStops == TruncateStart)
        ? std::lower_bound(stops.begin(), stops.end(), zeroRadiusStop, lcmp) - stops.begin()
        : std::upper_bound(stops.begin(), stops.end(), zeroRadiusStop, ucmp) - stops.begin();

    const float t = (zeroRadiusStop - stops[afterIndex - 1].position) /
        (stops[afterIndex].position - stops[afterIndex - 1].position);
    D2D1_COLOR_F lerpColor = lerpSkColor(stops[afterIndex - 1].color, stops[afterIndex].color, t);

    if (truncateStops == TruncateStart) {
        stops.erase(stops.begin(), stops.begin() + afterIndex);
        stops.insert(stops.begin(), { 0, lerpColor });
    } else {
        stops.erase(stops.begin() + afterIndex, stops.end());
        stops.insert(stops.end(), { 1, lerpColor });
    }
}
} // namespace

bool SkScalerContext_DW::drawColorV1Paint(SkCanvas& canvas,
                                          IDWritePaintReader& reader,
                                          DWRITE_PAINT_ELEMENT const & element)
{
    // Helper to draw the specified number of children.
    auto drawChildren = [&](uint32_t childCount) -> bool {
        if (childCount != 0) {
            DWRITE_PAINT_ELEMENT childElement;
            HRB(reader.MoveToFirstChild(&childElement));
            this->drawColorV1Paint(canvas, reader, childElement);

            for (uint32_t i = 1; i < childCount; i++) {
                HRB(reader.MoveToNextSibling(&childElement));
                this->drawColorV1Paint(canvas, reader, childElement);
            }

            HRB(reader.MoveToParent());
        }
        return true;
    };

    SkAutoCanvasRestore restoreCanvas(&canvas, true);
    switch (element.paintType) {
    case DWRITE_PAINT_TYPE_NONE:
        return true;

    case DWRITE_PAINT_TYPE_LAYERS: {
        // A layers paint element has a variable number of children.
        return drawChildren(element.paint.layers.childCount);
    }

    case DWRITE_PAINT_TYPE_SOLID_GLYPH: {
        // A solid glyph paint element has no children.
        // glyphIndex, color.value, color.paletteEntryIndex, color.alpha, color.colorAttributes
        auto const& solidGlyph = element.paint.solidGlyph;

        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRBM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
             "Could not create geometry to path converter.");
        UINT16 glyphId = SkTo<UINT16>(solidGlyph.glyphIndex);
        {
            Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
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

        path.transform(SkMatrix::Scale(1.0f / fTextSizeRender, 1.0f / fTextSizeRender));
        SkPaint skPaint;
        skPaint.setColor4f(sk_color_from(solidGlyph.color.value));
        skPaint.setAntiAlias(fRenderingMode != DWRITE_RENDERING_MODE_ALIASED);
        canvas.drawPath(path, skPaint);
        return true;
    }

    case DWRITE_PAINT_TYPE_SOLID: {
        // A solid paint element has no children.
        // value, paletteEntryIndex, alphaMultiplier, colorAttributes
        SkPaint skPaint;
        skPaint.setColor4f(sk_color_from(element.paint.solid.value));
        canvas.drawPaint(skPaint);
        return true;
    }

    case DWRITE_PAINT_TYPE_LINEAR_GRADIENT: {
        auto const& linearGradient = element.paint.linearGradient;
        // A linear gradient paint element has no children.
        // x0, y0, x1, y1, x2, y2, extendMode, gradientStopCount, [colorStops]

        if (linearGradient.gradientStopCount == 0) {
            return true;
        }
        std::vector<D2D1_GRADIENT_STOP> stops;
        stops.resize(linearGradient.gradientStopCount);

        // If success stops will be ordered.
        HRBM(reader.GetGradientStops(0, stops.size(), stops.data()),
             "Could not get linear gradient stops.");
        SkPaint skPaint;
        if (stops.size() == 1) {
            skPaint.setColor4f(sk_color_from(stops[0].color));
            canvas.drawPaint(skPaint);
            return true;
        }
        SkPoint linePositions[2] = { {linearGradient.x0, linearGradient.y0},
                                     {linearGradient.x1, linearGradient.y1} };
        SkPoint p0 = linePositions[0];
        SkPoint p1 = linePositions[1];
        SkPoint p2 = SkPoint::Make(linearGradient.x2, linearGradient.y2);

        // If p0p1 or p0p2 are degenerate probably nothing should be drawn.
        // If p0p1 and p0p2 are parallel then one side is the first color and the other side is
        // the last color, depending on the direction.
        // For now, just use the first color.
        if (p1 == p0 || p2 == p0 || !SkPoint::CrossProduct(p1 - p0, p2 - p0)) {
            skPaint.setColor4f(sk_color_from(stops[0].color));
            canvas.drawPaint(skPaint);
            return true;
        }

        // Follow implementation note in nanoemoji:
        // https://github.com/googlefonts/nanoemoji/blob/0ac6e7bb4d8202db692574d8530a9b643f1b3b3c/src/nanoemoji/svg.py#L188
        // to compute a new gradient end point P3 as the orthogonal
        // projection of the vector from p0 to p1 onto a line perpendicular
        // to line p0p2 and passing through p0.
        SkVector perpendicularToP2P0 = (p2 - p0);
        perpendicularToP2P0 = SkPoint::Make( perpendicularToP2P0.y(),
                                            -perpendicularToP2P0.x());
        SkVector p3 = p0 + SkVectorProjection((p1 - p0), perpendicularToP2P0);
        linePositions[1] = p3;

        // Project/scale points according to stop extrema along p0p3 line,
        // p3 being the result of the projection above, then scale stops to
        // to [0, 1] range so that repeat modes work.  The Skia linear
        // gradient shader performs the repeat modes over the 0 to 1 range,
        // that's why we need to scale the stops to within that range.
        SkTileMode tileMode = sk_tile_mode_from(SkTo<D2D1_EXTEND_MODE>(linearGradient.extendMode));
        SkScalar colorStopRange = stops.back().position - stops.front().position;
        // If the color stops are all at the same offset position, repeat and reflect modes
        // become meaningless.
        if (colorStopRange == 0.f) {
            if (tileMode != SkTileMode::kClamp) {
                //skPaint.setColor(SK_ColorTRANSPARENT);
                return true;
            } else {
                // Insert duplicated fake color stop in pad case at +1.0f to enable the projection
                // of circles for an originally 0-length color stop range. Adding this stop will
                // paint the equivalent gradient, because: All font specified color stops are in the
                // same spot, mode is pad, so everything before this spot is painted with the first
                // color, everything after this spot is painted with the last color. Not adding this
                // stop will skip the projection and result in specifying non-normalized color stops
                // to the shader.
                stops.push_back({ stops.back().position + 1.0f, stops.back().color });
                colorStopRange = 1.0f;
            }
        }
        SkASSERT(colorStopRange != 0.f);

        // If the colorStopRange is 0 at this point, the default behavior of the shader is to
        // clamp to 1 color stops that are above 1, clamp to 0 for color stops that are below 0,
        // and repeat the outer color stops at 0 and 1 if the color stops are inside the
        // range. That will result in the correct rendering.
        if ((colorStopRange != 1 || stops.front().position != 0.f)) {
            SkVector p0p3 = p3 - p0;
            SkVector p0Offset = p0p3;
            p0Offset.scale(stops.front().position);
            SkVector p1Offset = p0p3;
            p1Offset.scale(stops.back().position);

            linePositions[0] = p0 + p0Offset;
            linePositions[1] = p0 + p1Offset;

            SkScalar scaleFactor = 1 / colorStopRange;
            SkScalar startOffset = stops.front().position;
            for (D2D1_GRADIENT_STOP& stop : stops) {
                stop.position = (stop.position - startOffset) * scaleFactor;
            }
        }

        std::unique_ptr<SkColor4f[]> skColors(new SkColor4f[stops.size()]);
        std::unique_ptr<SkScalar[]> skStops(new SkScalar[stops.size()]);
        for (size_t i = 0; i < stops.size(); ++i) {
            skColors[i] = sk_color_from(stops[i].color);
            skStops[i] = stops[i].position;
        }

        sk_sp<SkShader> shader(SkGradientShader::MakeLinear(
            linePositions,
            skColors.get(), SkColorSpace::MakeSRGB(), skStops.get(), stops.size(),
            tileMode,
            SkGradientShader::Interpolation{
                SkGradientShader::Interpolation::InPremul::kNo,
                SkGradientShader::Interpolation::ColorSpace::kSRGB,
                SkGradientShader::Interpolation::HueMethod::kShorter
            },
            nullptr));

        SkASSERT(shader);
        // An opaque color is needed to ensure the gradient is not modulated by alpha.
        skPaint.setColor(SK_ColorBLACK);
        skPaint.setShader(shader);
        canvas.drawPaint(skPaint);
        return true;
    }

    case DWRITE_PAINT_TYPE_RADIAL_GRADIENT: {
        auto const& radialGradient = element.paint.radialGradient;
        // A radial gradient paint element has no children.
        // x0, y0, radius0, x1, y1, radius1, extendMode, gradientStopCount, [colorsStops]

        SkPoint start = SkPoint::Make(radialGradient.x0, radialGradient.y0);
        SkScalar startRadius = radialGradient.radius0;
        SkPoint end = SkPoint::Make(radialGradient.x1, radialGradient.y1);
        SkScalar endRadius = radialGradient.radius1;

        if (radialGradient.gradientStopCount == 0) {
            return true;
        }
        std::vector<D2D1_GRADIENT_STOP> stops;
        stops.resize(radialGradient.gradientStopCount);

        // If success stops will be ordered.
        HRBM(reader.GetGradientStops(0, stops.size(), stops.data()),
             "Could not get radial gradient stops.");
        SkPaint skPaint;
        if (stops.size() == 1) {
            skPaint.setColor4f(sk_color_from(stops[0].color));
            canvas.drawPaint(skPaint);
            return true;
        }

        SkScalar colorStopRange = stops.back().position - stops.front().position;
        SkTileMode tileMode = sk_tile_mode_from(SkTo<D2D1_EXTEND_MODE>(radialGradient.extendMode));

        if (colorStopRange == 0.f) {
            if (tileMode != SkTileMode::kClamp) {
                //skPaint.setColor(SK_ColorTRANSPARENT);
                return true;
            } else {
                // Insert duplicated fake color stop in pad case at +1.0f to enable the projection
                // of circles for an originally 0-length color stop range. Adding this stop will
                // paint the equivalent gradient, because: All font specified color stops are in the
                // same spot, mode is pad, so everything before this spot is painted with the first
                // color, everything after this spot is painted with the last color. Not adding this
                // stop will skip the projection and result in specifying non-normalized color stops
                // to the shader.
                stops.push_back({ stops.back().position + 1.0f, stops.back().color });
                colorStopRange = 1.0f;
            }
        }
        SkASSERT(colorStopRange != 0.f);

        // If the colorStopRange is 0 at this point, the default behavior of the shader is to
        // clamp to 1 color stops that are above 1, clamp to 0 for color stops that are below 0,
        // and repeat the outer color stops at 0 and 1 if the color stops are inside the
        // range. That will result in the correct rendering.
        if (colorStopRange != 1 || stops.front().position != 0.f) {
            // For the Skia two-point caonical shader to understand the
            // COLRv1 color stops we need to scale stops to 0 to 1 range and
            // interpolate new centers and radii. Otherwise the shader
            // clamps stops outside the range to 0 and 1 (larger interval)
            // or repeats the outer stops at 0 and 1 if the (smaller
            // interval).
            SkVector startToEnd = end - start;
            SkScalar radiusDiff = endRadius - startRadius;
            SkScalar scaleFactor = 1 / colorStopRange;
            SkScalar stopsStartOffset = stops.front().position;

            SkVector startOffset = startToEnd;
            startOffset.scale(stops.front().position);
            SkVector endOffset = startToEnd;
            endOffset.scale(stops.back().position);

            // The order of the following computations is important in order to avoid
            // overwriting start or startRadius before the second reassignment.
            end = start + endOffset;
            start = start + startOffset;
            endRadius = startRadius + radiusDiff * stops.back().position;
            startRadius = startRadius + radiusDiff * stops.front().position;

            for (auto& stop : stops) {
                stop.position = (stop.position - stopsStartOffset) * scaleFactor;
            }
        }

        // For negative radii, interpolation is needed to prepare parameters suitable
        // for invoking the shader. Implementation below as resolution discussed in
        // https://github.com/googlefonts/colr-gradients-spec/issues/367.
        // Truncate to manually interpolated color for tile mode clamp, otherwise
        // calculate positive projected circles.
        if (startRadius < 0 || endRadius < 0) {
            if (startRadius == endRadius && startRadius < 0) {
                //skPaint.setColor(SK_ColorTRANSPARENT);
                return true;
            }

            if (tileMode == SkTileMode::kClamp) {
                SkVector startToEnd = end - start;
                SkScalar radiusDiff = endRadius - startRadius;
                SkScalar zeroRadiusStop = 0.f;
                TruncateStops truncateSide = TruncateStart;
                if (startRadius < 0) {
                    truncateSide = TruncateStart;

                    // Compute color stop position where radius is = 0.  After the scaling
                    // of stop positions to the normal 0,1 range that we have done above,
                    // the size of the radius as a function of the color stops is: r(x) = r0
                    // + x*(r1-r0) Solving this function for r(x) = 0, we get: x = -r0 /
                    // (r1-r0)
                    zeroRadiusStop = -startRadius / (endRadius - startRadius);
                    startRadius = 0.f;
                    SkVector startEndDiff = end - start;
                    startEndDiff.scale(zeroRadiusStop);
                    start = start + startEndDiff;
                }

                if (endRadius < 0) {
                    truncateSide = TruncateEnd;
                    zeroRadiusStop = -startRadius / (endRadius - startRadius);
                    endRadius = 0.f;
                    SkVector startEndDiff = end - start;
                    startEndDiff.scale(1 - zeroRadiusStop);
                    end = end - startEndDiff;
                }

                if (!(startRadius == 0 && endRadius == 0)) {
                    truncateToStopInterpolating(zeroRadiusStop, stops, truncateSide);
                } else {
                    // If both radii have become negative and where clamped to 0, we need to
                    // produce a single color cone, otherwise the shader colors the whole
                    // plane in a single color when two radii are specified as 0.
                    if (radiusDiff > 0) {
                        end = start + startToEnd;
                        endRadius = radiusDiff;
                        stops.erase(stops.begin(), stops.end() - 1);
                    } else {
                        start -= startToEnd;
                        startRadius = -radiusDiff;
                        stops.erase(stops.begin() + 1, stops.end());
                    }
                }
            } else {
                if (startRadius < 0 || endRadius < 0) {
                    auto roundIntegerMultiple = [](SkScalar factorZeroCrossing,
                        SkTileMode tileMode) {
                            int roundedMultiple = factorZeroCrossing > 0
                                ? ceilf(factorZeroCrossing)
                                : floorf(factorZeroCrossing) - 1;
                            if (tileMode == SkTileMode::kMirror && roundedMultiple % 2 != 0) {
                                roundedMultiple += roundedMultiple < 0 ? -1 : 1;
                            }
                            return roundedMultiple;
                    };

                    SkVector startToEnd = end - start;
                    SkScalar radiusDiff = endRadius - startRadius;
                    SkScalar factorZeroCrossing = (startRadius / (startRadius - endRadius));
                    bool inRange = 0.f <= factorZeroCrossing && factorZeroCrossing <= 1.0f;
                    SkScalar direction = inRange && radiusDiff < 0 ? -1.0f : 1.0f;
                    SkScalar circleProjectionFactor =
                        roundIntegerMultiple(factorZeroCrossing * direction, tileMode);
                    startToEnd.scale(circleProjectionFactor);
                    startRadius += circleProjectionFactor * radiusDiff;
                    endRadius += circleProjectionFactor * radiusDiff;
                    start += startToEnd;
                    end += startToEnd;
                }
            }
        }

        std::unique_ptr<SkColor4f[]> skColors(new SkColor4f[stops.size()]);
        std::unique_ptr<SkScalar[]> skStops(new SkScalar[stops.size()]);
        for (size_t i = 0; i < stops.size(); ++i) {
            skColors[i] = sk_color_from(stops[i].color);
            skStops[i] = stops[i].position;
        }

        // An opaque color is needed to ensure the gradient is not modulated by alpha.
        skPaint.setColor(SK_ColorBLACK);
        skPaint.setShader(SkGradientShader::MakeTwoPointConical(
            start, startRadius, end, endRadius,
            skColors.get(), SkColorSpace::MakeSRGB(), skStops.get(), stops.size(),
            tileMode,
            SkGradientShader::Interpolation{
                SkGradientShader::Interpolation::InPremul::kNo,
                SkGradientShader::Interpolation::ColorSpace::kSRGB,
                SkGradientShader::Interpolation::HueMethod::kShorter
            },
            nullptr));
        canvas.drawPaint(skPaint);
        return true;
    }

    case DWRITE_PAINT_TYPE_SWEEP_GRADIENT: {
        auto const& sweepGradient = element.paint.sweepGradient;
        // A sweep gradient paint element has no children.
        // centerX, centerY, startAngle, endAngle, extendMode, gradientStopCount, [colorStops]

        if (sweepGradient.gradientStopCount == 0) {
            return true;
        }
        std::vector<D2D1_GRADIENT_STOP> stops;
        stops.resize(sweepGradient.gradientStopCount);

        // If success stops will be ordered.
        HRBM(reader.GetGradientStops(0, stops.size(), stops.data()),
             "Could not get sweep gradient stops");
        SkPaint skPaint;
        if (stops.size() == 1) {
            skPaint.setColor4f(sk_color_from(stops[0].color));
            canvas.drawPaint(skPaint);
            return true;
        }

        SkPoint center = SkPoint::Make(sweepGradient.centerX, sweepGradient.centerY);

        SkScalar startAngle = sweepGradient.startAngle;
        SkScalar endAngle = sweepGradient.endAngle;
        // OpenType 1.9.1 adds a shift to the angle to ease specification of a 0 to 360
        // degree sweep. This appears to already be applied by DW.
        //startAngle += 180.0f;
        //endAngle += 180.0f;

        // An opaque color is needed to ensure the gradient is not modulated by alpha.
        skPaint.setColor(SK_ColorBLACK);

        // New (Var)SweepGradient implementation compliant with OpenType 1.9.1 from here.

        // The shader expects stops from 0 to 1, so we need to account for
        // minimum and maximum stop positions being different from 0 and
        // 1. We do that by scaling minimum and maximum stop positions to
        // the 0 to 1 interval and scaling the angles inverse proportionally.

        // 1) Scale angles to their equivalent positions if stops were from 0 to 1.

        SkScalar sectorAngle = endAngle - startAngle;
        SkTileMode tileMode = sk_tile_mode_from(SkTo<D2D1_EXTEND_MODE>(sweepGradient.extendMode));
        if (sectorAngle == 0 && tileMode != SkTileMode::kClamp) {
            // "If the ColorLine's extend mode is reflect or repeat and start and end angle
            // are equal, nothing is drawn.".
            //skPaint.setColor(SK_ColorTRANSPARENT);
            return true;
        }

        SkScalar startAngleScaled = startAngle + sectorAngle * stops.front().position;
        SkScalar endAngleScaled = startAngle + sectorAngle * stops.back().position;

        // 2) Scale stops accordingly to 0 to 1 range.

        float colorStopRange = stops.back().position - stops.front().position;
        if (colorStopRange == 0.f) {
            if (tileMode != SkTileMode::kClamp) {
                //skPaint.setColor(SK_ColorTRANSPARENT);
                return true;
            } else {
                // Insert duplicated fake color stop in pad case at +1.0f to feed the shader correct
                // values and enable painting a pad sweep gradient with two colors. Adding this stop
                // will paint the equivalent gradient, because: All font specified color stops are
                // in the same spot, mode is pad, so everything before this spot is painted with the
                // first color, everything after this spot is painted with the last color. Not
                // adding this stop will skip the projection and result in specifying non-normalized
                // color stops to the shader.
                stops.push_back({ stops.back().position + 1.0f, stops.back().color });
                colorStopRange = 1.0f;
            }
        }

        SkScalar scaleFactor = 1 / colorStopRange;
        SkScalar startOffset = stops.front().position;

        for (D2D1_GRADIENT_STOP& stop : stops) {
            stop.position = (stop.position - startOffset) * scaleFactor;
        }

        /* https://docs.microsoft.com/en-us/typography/opentype/spec/colr#sweep-gradients
        * "The angles are expressed in counter-clockwise degrees from
        * the direction of the positive x-axis on the design
        * grid. [...]  The color line progresses from the start angle
        * to the end angle in the counter-clockwise direction;" -
        * Convert angles and stops from counter-clockwise to clockwise
        * for the shader if the gradient is not already reversed due to
        * start angle being larger than end angle. */
        startAngleScaled = 360.f - startAngleScaled;
        endAngleScaled = 360.f - endAngleScaled;
        if (startAngleScaled >= endAngleScaled) {
            std::swap(startAngleScaled, endAngleScaled);
            std::reverse(stops.begin(), stops.end());
            for (auto& stop : stops) {
                stop.position = 1.0f - stop.position;
            }
        }

        std::unique_ptr<SkColor4f[]> skColors(new SkColor4f[stops.size()]);
        std::unique_ptr<SkScalar[]> skStops(new SkScalar[stops.size()]);
        for (size_t i = 0; i < stops.size(); ++i) {
            skColors[i] = sk_color_from(stops[i].color);
            skStops[i] = stops[i].position;
        }

        skPaint.setShader(SkGradientShader::MakeSweep(
            center.x(), center.y(),
            skColors.get(), SkColorSpace::MakeSRGB(), skStops.get(), stops.size(),
            tileMode,
            startAngleScaled, endAngleScaled,
            SkGradientShader::Interpolation{
                SkGradientShader::Interpolation::InPremul::kNo,
                SkGradientShader::Interpolation::ColorSpace::kSRGB,
                SkGradientShader::Interpolation::HueMethod::kShorter
            },
            nullptr));
        canvas.drawPaint(skPaint);
        return true;
    }

    case DWRITE_PAINT_TYPE_GLYPH: {
        // A glyph paint element has one child, which is the fill for the glyph shape glyphIndex.
        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRBM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
             "Could not create geometry to path converter.");
        UINT16 glyphId = SkTo<UINT16>(element.paint.glyph.glyphIndex);
        {
            Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
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

        path.transform(SkMatrix::Scale(1.0f / fTextSizeRender, 1.0f / fTextSizeRender));
        canvas.clipPath(path, fRenderingMode != DWRITE_RENDERING_MODE_ALIASED);

        drawChildren(1);
        return true;
    }

    case DWRITE_PAINT_TYPE_COLOR_GLYPH: {
        auto const& colorGlyph = element.paint.colorGlyph;
        // A color glyph paint element has one child, the root of the paint tree for glyphIndex.
        // glyphIndex, clipBox
        if (D2D_RECT_F_is_empty(colorGlyph.clipBox)) {
            // Does not have a clip box
        } else {
            SkRect r = sk_rect_from(colorGlyph.clipBox);
            canvas.clipRect(r, fRenderingMode != DWRITE_RENDERING_MODE_ALIASED);
        }

        drawChildren(1);
        return true;
    }

    case DWRITE_PAINT_TYPE_TRANSFORM: {
        // A transform paint element always has one child, the transformed content.
        canvas.concat(sk_matrix_from(element.paint.transform));
        drawChildren(1);
        return true;
    }

    case DWRITE_PAINT_TYPE_COMPOSITE: {
        // A composite paint element has two children, the source and destination of the operation.

        SkPaint blendModePaint;
        blendModePaint.setBlendMode(sk_blend_mode_from(element.paint.composite.mode));

        SkAutoCanvasRestore acr(&canvas, false);

        // Need to visit the second child first and do savelayers, so manually handle children.
        DWRITE_PAINT_ELEMENT sourceElement;
        DWRITE_PAINT_ELEMENT backdropElement;

        HRBM(reader.MoveToFirstChild(&sourceElement), "Could not move to child.");
        HRBM(reader.MoveToNextSibling(&backdropElement), "Could not move to sibiling.");
        canvas.saveLayer(nullptr, nullptr);
        this->drawColorV1Paint(canvas, reader, backdropElement);

        HRBM(reader.MoveToParent(), "Could not move to parent.");
        HRBM(reader.MoveToFirstChild(&sourceElement), "Could not move to child.");
        canvas.saveLayer(nullptr, &blendModePaint);
        this->drawColorV1Paint(canvas, reader, sourceElement);

        HRBM(reader.MoveToParent(), "Could not move to parent.");

        return true;
    }

    default:
        return false;
    }
}

bool SkScalerContext_DW::drawColorV1Image(const SkGlyph& glyph, SkCanvas& canvas) {
    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    IDWriteFontFace7* fontFace = typeface->fDWriteFontFace7/*.get()*/;
    if (!fontFace) {
        return false;
    }
    UINT32 glyphIndex = glyph.getGlyphID();

    SkTScopedComPtr<IDWritePaintReader> paintReader;
    HRBM(fontFace->CreatePaintReader(DWRITE_GLYPH_IMAGE_FORMATS_COLR_PAINT_TREE,
                                     DWRITE_PAINT_FEATURE_LEVEL_COLR_V1,
                                     &paintReader),
         "Could not create paint reader.");

    DWRITE_PAINT_ELEMENT paintElement;
    D2D_RECT_F clipBox;
    DWRITE_PAINT_ATTRIBUTES attributes;
    HRBM(paintReader->SetCurrentGlyph(glyphIndex, &paintElement, &clipBox, &attributes),
         "Could not set current glyph.");

    if (paintElement.paintType == DWRITE_PAINT_TYPE_NONE) {
        // Does not have paint layers, try another format.
        return false;
    }

    // All coordinates (including top level clip) are reported in "em"s (1 == em).
    // Size up all em units to the current size and transform.
    // Get glyph paths at render size, divide out the render size to get em units.

    SkMatrix matrix = fSkXform;
    SkScalar scale = fTextSizeRender;
    matrix.preScale(scale, scale);
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
    }
    canvas.concat(matrix);

    if (D2D_RECT_F_is_empty(clipBox)) {
        // Does not have a clip box
    } else {
        canvas.clipRect(sk_rect_from(clipBox));
    }

    // The DirectWrite interface returns resolved colors if these are provided.
    // Indexes and alphas are reported but there is no reason to duplicate the color calculation.
    paintReader->SetTextColor(dw_color_from(SkColor4f::FromColor(fRec.fForegroundColor)));
    paintReader->SetCustomColorPalette(typeface->fDWPalette.get(), typeface->fPaletteEntryCount);

    return this->drawColorV1Paint(canvas, *paintReader, paintElement);
}

bool SkScalerContext_DW::generateColorV1Image(const SkGlyph& glyph, void* imageBuffer) {
    SkASSERT(glyph.maskFormat() == SkMask::Format::kARGB32_Format);

    SkBitmap dstBitmap;
    // TODO: mark this as sRGB when the blits will be sRGB.
    dstBitmap.setInfo(SkImageInfo::Make(glyph.width(), glyph.height(),
                      kN32_SkColorType, kPremul_SkAlphaType),
                      glyph.rowBytes());
    dstBitmap.setPixels(imageBuffer);

    SkCanvas canvas(dstBitmap);
    if constexpr (kSkShowTextBlitCoverage) {
        canvas.clear(0x33FF0000);
    } else {
        canvas.clear(SK_ColorTRANSPARENT);
    }
    canvas.translate(-SkIntToScalar(glyph.left()), -SkIntToScalar(glyph.top()));

    return this->drawColorV1Image(glyph, canvas);
}

bool SkScalerContext_DW::generateColorV1PaintBounds(
    SkMatrix* ctm, SkRect* bounds,
    IDWritePaintReader& reader, DWRITE_PAINT_ELEMENT const & element)
{
    // Helper to iterate over the specified number of children.
    auto boundChildren = [&](UINT32 childCount) -> bool {
        if (childCount == 0) {
            return true;
        }
        DWRITE_PAINT_ELEMENT childElement;
        HRB(reader.MoveToFirstChild(&childElement));
        this->generateColorV1PaintBounds(ctm, bounds, reader, childElement);

        for (uint32_t i = 1; i < childCount; ++i) {
            HRB(reader.MoveToNextSibling(&childElement));
            this->generateColorV1PaintBounds(ctm, bounds, reader, childElement);
        }

        HRB(reader.MoveToParent());
        return true;
    };

    SkMatrix restoreMatrix = *ctm;
    SK_AT_SCOPE_EXIT(*ctm = restoreMatrix);

    switch (element.paintType) {
    case DWRITE_PAINT_TYPE_NONE:
        return false;

    case DWRITE_PAINT_TYPE_LAYERS: {
        // A layers paint element has a variable number of children.
        return boundChildren(element.paint.layers.childCount);
    }

    case DWRITE_PAINT_TYPE_SOLID_GLYPH: {
        // A solid glyph paint element has no children.
        // glyphIndex, color.value, color.paletteEntryIndex, color.alpha, color.colorAttributes

        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRBM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
            "Could not create geometry to path converter.");
        UINT16 glyphId = SkTo<UINT16>(element.paint.solidGlyph.glyphIndex);
        {
            Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
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

        SkMatrix t = *ctm;
        t.preConcat(SkMatrix::Scale(1.0f / fTextSizeRender, 1.0f / fTextSizeRender));
        path.transform(t);
        bounds->join(path.getBounds());
        return true;
    }

    case DWRITE_PAINT_TYPE_SOLID: {
        return true;
    }

    case DWRITE_PAINT_TYPE_LINEAR_GRADIENT: {
        return true;
    }

    case DWRITE_PAINT_TYPE_RADIAL_GRADIENT: {
        return true;
    }

    case DWRITE_PAINT_TYPE_SWEEP_GRADIENT: {
        return true;
    }

    case DWRITE_PAINT_TYPE_GLYPH: {
        // A glyph paint element has one child, which is the fill for the glyph shape glyphIndex.
        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRBM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
             "Could not create geometry to path converter.");
        UINT16 glyphId = SkTo<UINT16>(element.paint.glyph.glyphIndex);
        {
            Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
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

        SkMatrix t = *ctm;
        t.preConcat(SkMatrix::Scale(1.0f / fTextSizeRender, 1.0f / fTextSizeRender));
        path.transform(t);
        bounds->join(path.getBounds());
        return true;
    }

    case DWRITE_PAINT_TYPE_COLOR_GLYPH: {
        // A color glyph paint element has one child, which is the root
        // of the paint tree for the glyph specified by glyphIndex.
        auto const& colorGlyph = element.paint.colorGlyph;
        if (D2D_RECT_F_is_empty(colorGlyph.clipBox)) {
            // Does not have a clip box
            return boundChildren(1);
        }
        SkRect r = sk_rect_from(colorGlyph.clipBox);
        ctm->mapRect(r);
        bounds->join(r);
        return true;
    }

    case DWRITE_PAINT_TYPE_TRANSFORM: {
        // A transform paint element always has one child, which is the transformed content.
        ctm->preConcat(sk_matrix_from(element.paint.transform));
        return boundChildren(1);
    }

    case DWRITE_PAINT_TYPE_COMPOSITE: {
        // A composite paint element has two children, the source and destination of the operation.
        return boundChildren(2);
    }

    default:
        return false;
    }
}

bool SkScalerContext_DW::generateColorV1Metrics(const SkGlyph& glyph, SkRect* bounds) {
    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    IDWriteFontFace7* fontFace = typeface->fDWriteFontFace7/*.get()*/;
    if (!fontFace) {
        return false;
    }
    UINT32 glyphIndex = glyph.getGlyphID();

    SkTScopedComPtr<IDWritePaintReader> paintReader;
    HRESULT hr;
    // No message on failure here, since this will fail if the font has no color glyphs.
    hr = fontFace->CreatePaintReader(DWRITE_GLYPH_IMAGE_FORMATS_COLR_PAINT_TREE,
                                     DWRITE_PAINT_FEATURE_LEVEL_COLR_V1,
                                     &paintReader);
    if (FAILED(hr)) {
        return false;
    }

    DWRITE_PAINT_ELEMENT paintElement;
    D2D_RECT_F clipBox;
    DWRITE_PAINT_ATTRIBUTES attributes;
    // If the glyph is not color this will succeed but return paintType NONE.
    HRBM(paintReader->SetCurrentGlyph(glyphIndex, &paintElement, &clipBox, &attributes),
         "Could not set the current glyph.");

    if (paintElement.paintType == DWRITE_PAINT_TYPE_NONE) {
        // Does not have paint layers, try another format.
        return false;
    }

    // All coordinates (including top level clip) are reported in "em"s (1 == em).
    // Size up all em units to the current size and transform.

    SkMatrix matrix = fSkXform;
    SkScalar scale = fTextSizeRender;
    matrix.preScale(scale, scale);
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
    }

    SkRect r;
    if (D2D_RECT_F_is_empty(clipBox)) {
        // Does not have a clip box.
        r = SkRect::MakeEmpty();
        if (!this->generateColorV1PaintBounds(&matrix, &r, *paintReader, paintElement)) {
            return false;
        }
        *bounds = r;
    } else {
        *bounds = sk_rect_from(clipBox);
        matrix.mapRect(bounds);
    }
    return true;
}

#else  // DWRITE_CORE || (defined(NTDDI_WIN11_ZN) && NTDDI_VERSION >= NTDDI_WIN11_ZN)

bool SkScalerContext_DW::generateColorV1Metrics(const SkGlyph&, SkRect*) { return false; }
bool SkScalerContext_DW::generateColorV1Image(const SkGlyph&, void*) { return false; }
bool SkScalerContext_DW::drawColorV1Image(const SkGlyph&, SkCanvas&) { return false; }

#endif  // DWRITE_CORE || (defined(NTDDI_WIN11_ZN) && NTDDI_VERSION >= NTDDI_WIN11_ZN)

bool SkScalerContext_DW::setAdvance(const SkGlyph& glyph, SkVector* advance) {
    *advance = {0, 0};
    uint16_t glyphId = glyph.getGlyphID();
    DWriteFontTypeface* typeface = this->getDWriteTypeface();

    // DirectWrite treats all out of bounds glyph ids as having the same data as glyph 0.
    // For consistency with all other backends, treat out of range glyph ids as an error.
    if (fGlyphCount <= glyphId) {
        return false;
    }

    DWRITE_GLYPH_METRICS gm;

    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        Exclusive l(maybe_dw_mutex(*typeface));
        HRBM(typeface->fDWriteFontFace->GetGdiCompatibleGlyphMetrics(
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
        Exclusive l(maybe_dw_mutex(*typeface));
        HRBM(typeface->fDWriteFontFace->GetDesignGlyphMetrics(&glyphId, 1, &gm),
             "Could not get design metrics.");
    }

    DWRITE_FONT_METRICS dwfm;
    {
        Shared l(maybe_dw_mutex(*typeface));
        typeface->fDWriteFontFace->GetMetrics(&dwfm);
    }
    SkScalar advanceX = fTextSizeMeasure * gm.advanceWidth / dwfm.designUnitsPerEm;

    *advance = { advanceX, 0 };
    if (DWRITE_MEASURING_MODE_GDI_CLASSIC == fMeasuringMode ||
        DWRITE_MEASURING_MODE_GDI_NATURAL == fMeasuringMode)
    {
        // DirectWrite produced 'compatible' metrics, but while close,
        // the end result is not always an integer as it would be with GDI.
        advance->fX = SkScalarRoundToScalar(advance->fX);
    }
    fSkXform.mapVectors(advance, 1);
    return true;
}

bool SkScalerContext_DW::generateDWMetrics(const SkGlyph& glyph,
                                           DWRITE_RENDERING_MODE renderingMode,
                                           DWRITE_TEXTURE_TYPE textureType,
                                           SkRect* bounds)
{
    DWriteFontTypeface* typeface = this->getDWriteTypeface();

    //Measure raster size.
    fXform.dx = SkFixedToFloat(glyph.getSubXFixed());
    fXform.dy = SkFixedToFloat(glyph.getSubYFixed());

    FLOAT advance = 0;

    UINT16 glyphId = glyph.getGlyphID();

    DWRITE_GLYPH_OFFSET offset;
    offset.advanceOffset = 0.0f;
    offset.ascenderOffset = 0.0f;

    DWRITE_GLYPH_RUN run;
    run.glyphCount = 1;
    run.glyphAdvances = &advance;
    run.fontFace = typeface->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &glyphId;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;

    SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
    {
        Exclusive l(maybe_dw_mutex(*typeface));
        // IDWriteFactory2::CreateGlyphRunAnalysis is very bad at aliased glyphs.
        if (typeface->fFactory2 &&
                (fGridFitMode == DWRITE_GRID_FIT_MODE_DISABLED ||
                 fAntiAliasMode == DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE))
        {
            HRBM(typeface->fFactory2->CreateGlyphRunAnalysis(
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
            HRBM(typeface->fFactory->CreateGlyphRunAnalysis(&run,
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
    RECT bbox;
    {
        Shared l(maybe_dw_mutex(*typeface));
        HRBM(glyphRunAnalysis->GetAlphaTextureBounds(textureType, &bbox),
             "Could not get texture bounds.");
    }

    // GetAlphaTextureBounds succeeds but sometimes returns empty bounds like
    // { 0x80000000, 0x80000000, 0x80000000, 0x80000000 }
    // for small but not quite zero and large (but not really large) glyphs,
    // Only set as non-empty if the returned bounds are non-empty.
    if (bbox.left >= bbox.right || bbox.top >= bbox.bottom) {
        return false;
    }

    *bounds = SkRect::MakeLTRB(bbox.left, bbox.top, bbox.right, bbox.bottom);
    return true;
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

bool SkScalerContext_DW::generateColorMetrics(const SkGlyph& glyph, SkRect* bounds) {
    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;
    if (!getColorGlyphRun(glyph, &colorLayers)) {
        return false;
    }
    SkASSERT(colorLayers.get());

    *bounds = SkRect::MakeEmpty();
    BOOL hasNextRun = FALSE;
    while (SUCCEEDED(colorLayers->MoveNext(&hasNextRun)) && hasNextRun) {
        const DWRITE_COLOR_GLYPH_RUN* colorGlyph;
        HRBM(colorLayers->GetCurrentRun(&colorGlyph), "Could not get current color glyph run");

        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRBM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
             "Could not create geometry to path converter.");
        {
            Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
            HRBM(colorGlyph->glyphRun.fontFace->GetGlyphRunOutline(
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
        bounds->join(path.getBounds());
    }
    SkMatrix matrix = fSkXform;
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
    }
    matrix.mapRect(bounds);
    return true;
}

bool SkScalerContext_DW::generateSVGMetrics(const SkGlyph& glyph, SkRect* bounds) {
    SkPictureRecorder recorder;
    SkRect infiniteRect = SkRect::MakeLTRB(-SK_ScalarInfinity, -SK_ScalarInfinity,
                                            SK_ScalarInfinity,  SK_ScalarInfinity);
    sk_sp<SkBBoxHierarchy> bboxh = SkRTreeFactory()();
    SkCanvas* recordingCanvas = recorder.beginRecording(infiniteRect, bboxh);
    if (!this->drawSVGImage(glyph, *recordingCanvas)) {
        return false;
    }
    sk_sp<SkPicture> pic = recorder.finishRecordingAsPicture();
    *bounds = pic->cullRect();
    SkASSERT(bounds->isFinite());
    bounds->roundOut(bounds);
    return true;
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

bool SkScalerContext_DW::generatePngMetrics(const SkGlyph& glyph, SkRect* bounds) {
    IDWriteFontFace4* fontFace4 = this->getDWriteTypeface()->fDWriteFontFace4.get();
    if (!fontFace4) {
        return false;
    }

    DWRITE_GLYPH_IMAGE_FORMATS imageFormats;
    HRBM(fontFace4->GetGlyphImageFormats(glyph.getGlyphID(), 0, UINT32_MAX, &imageFormats),
         "Cannot get glyph image formats.");
    if (!(imageFormats & DWRITE_GLYPH_IMAGE_FORMATS_PNG)) {
        return false;
    }

    DWRITE_GLYPH_IMAGE_DATA glyphData;
    void* glyphDataContext;
    HRBM(fontFace4->GetGlyphImageData(glyph.getGlyphID(),
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
        return false;
    }

    SkImageInfo info = codec->getInfo();
    *bounds = SkRect::MakeLTRB(SkIntToScalar(info.bounds().fLeft),
                               SkIntToScalar(info.bounds().fTop),
                               SkIntToScalar(info.bounds().fRight),
                               SkIntToScalar(info.bounds().fBottom));

    SkMatrix matrix = fSkXform;
    SkScalar scale = fTextSizeRender / glyphData.pixelsPerEm;
    matrix.preScale(scale, scale);
    matrix.preTranslate(-glyphData.horizontalLeftOrigin.x, -glyphData.horizontalLeftOrigin.y);
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
    }
    matrix.mapRect(bounds);
    bounds->roundOut(bounds);
    return true;
}

SkScalerContext::GlyphMetrics SkScalerContext_DW::generateMetrics(const SkGlyph& glyph,
                                                                  SkArenaAlloc* alloc) {
    GlyphMetrics mx(glyph.maskFormat());

    mx.extraBits = ScalerContextBits::NONE;

    if (!this->setAdvance(glyph, &mx.advance)) {
        return mx;
    }

    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    if (typeface->fIsColorFont) {
        if (generateColorV1Metrics(glyph, &mx.bounds)) {
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.extraBits |= ScalerContextBits::COLRv1;
            mx.neverRequestPath = true;
            return mx;
        }

        if (generateColorMetrics(glyph, &mx.bounds)) {
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.extraBits |= ScalerContextBits::COLR;
            mx.neverRequestPath = true;
            return mx;
        }

        if (generateSVGMetrics(glyph, &mx.bounds)) {
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.extraBits |= ScalerContextBits::SVG;
            mx.neverRequestPath = true;
            return mx;
        }

        if (generatePngMetrics(glyph, &mx.bounds)) {
            mx.maskFormat = SkMask::kARGB32_Format;
            mx.extraBits |= ScalerContextBits::PNG;
            mx.neverRequestPath = true;
            return mx;
        }
    }

    if (this->generateDWMetrics(glyph, fRenderingMode, fTextureType, &mx.bounds)) {
        mx.extraBits = ScalerContextBits::DW;
        return mx;
    }

    // GetAlphaTextureBounds succeeds but returns an empty RECT if there are no
    // glyphs of the specified texture type or it is too big for smoothing.
    // When this happens, try with the alternate texture type.
    if (DWRITE_TEXTURE_ALIASED_1x1 != fTextureType ||
        DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE == fAntiAliasMode)
    {
        if (this->generateDWMetrics(glyph,
                                    DWRITE_RENDERING_MODE_ALIASED,
                                    DWRITE_TEXTURE_ALIASED_1x1,
                                    &mx.bounds))
        {
            mx.maskFormat = SkMask::kBW_Format;
            mx.extraBits = ScalerContextBits::DW_1;
            return mx;
        }
    }
    // TODO: Try DWRITE_TEXTURE_CLEARTYPE_3x1 if DWRITE_TEXTURE_ALIASED_1x1 fails

    // GetAlphaTextureBounds can fail for various reasons.
    // As a fallback, attempt to generate the metrics and image from the path.
    mx.computeFromPath = true;
    mx.extraBits = ScalerContextBits::PATH;
    return mx;
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

    SkTScopedComPtr<IDWriteFontFace5> fontFace5;
    if (SUCCEEDED(this->getDWriteTypeface()->fDWriteFontFace->QueryInterface(&fontFace5))) {
        if (fontFace5->HasVariations()) {
            // The bounds are only valid for the default variation.
            metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;
        }
    }

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

    // The real bounds weren't actually available.
    metrics->fFlags |= SkFontMetrics::kBoundsInvalid_Flag;
    metrics->fTop = metrics->fAscent;
    metrics->fBottom = metrics->fDescent;
}

///////////////////////////////////////////////////////////////////////////////

#include "include/private/SkColorData.h"

void SkScalerContext_DW::BilevelToBW(const uint8_t* SK_RESTRICT src,
                                     const SkGlyph& glyph, void* imageBuffer) {
    const int width = glyph.width();
    const size_t dstRB = (width + 7) >> 3;
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(imageBuffer);

    int byteCount = width >> 3;
    int bitCount = width & 7;

    for (int y = 0; y < glyph.height(); ++y) {
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

    if constexpr (kSkShowTextBlitCoverage) {
        dst = static_cast<uint8_t*>(imageBuffer);
        for (unsigned y = 0; y < (unsigned)glyph.height(); y += 2) {
            for (unsigned x = (y & 0x2); x < (unsigned)glyph.width(); x+=4) {
                uint8_t& b = dst[(dstRB * y) + (x >> 3)];
                b = b ^ (1 << (0x7 - (x & 0x7)));
            }
        }
    }
}

template<bool APPLY_PREBLEND>
void SkScalerContext_DW::GrayscaleToA8(const uint8_t* SK_RESTRICT src,
                                       const SkGlyph& glyph, void* imageBuffer,
                                       const uint8_t* table8) {
    const size_t dstRB = glyph.rowBytes();
    const int width = glyph.width();
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(imageBuffer);

    for (int y = 0; y < glyph.height(); y++) {
        for (int i = 0; i < width; i++) {
            U8CPU a = *(src++);
            dst[i] = sk_apply_lut_if<APPLY_PREBLEND>(a, table8);
            if constexpr (kSkShowTextBlitCoverage) {
                dst[i] = std::max<U8CPU>(0x30, dst[i]);
            }
        }
        dst = SkTAddOffset<uint8_t>(dst, dstRB);
    }
}

template<bool APPLY_PREBLEND>
void SkScalerContext_DW::RGBToA8(const uint8_t* SK_RESTRICT src,
                                 const SkGlyph& glyph, void* imageBuffer,
                                 const uint8_t* table8) {
    const size_t dstRB = glyph.rowBytes();
    const int width = glyph.width();
    uint8_t* SK_RESTRICT dst = static_cast<uint8_t*>(imageBuffer);

    for (int y = 0; y < glyph.height(); y++) {
        for (int i = 0; i < width; i++) {
            U8CPU r = *(src++);
            U8CPU g = *(src++);
            U8CPU b = *(src++);
            dst[i] = sk_apply_lut_if<APPLY_PREBLEND>((r + g + b) / 3, table8);
            if constexpr (kSkShowTextBlitCoverage) {
                dst[i] = std::max<U8CPU>(0x30, dst[i]);
            }
        }
        dst = SkTAddOffset<uint8_t>(dst, dstRB);
    }
}

template<bool APPLY_PREBLEND, bool RGB>
void SkScalerContext_DW::RGBToLcd16(const uint8_t* SK_RESTRICT src, const SkGlyph& glyph,
                                    void* imageBuffer,
                                    const uint8_t* tableR, const uint8_t* tableG,
                                    const uint8_t* tableB) {
    const size_t dstRB = glyph.rowBytes();
    const int width = glyph.width();
    uint16_t* SK_RESTRICT dst = static_cast<uint16_t*>(imageBuffer);

    for (int y = 0; y < glyph.height(); y++) {
        for (int i = 0; i < width; i++) {
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
            if constexpr (kSkShowTextBlitCoverage) {
                r = std::max<U8CPU>(0x30, r);
                g = std::max<U8CPU>(0x30, g);
                b = std::max<U8CPU>(0x30, b);
            }
            dst[i] = SkPack888ToRGB16(r, g, b);
        }
        dst = SkTAddOffset<uint16_t>(dst, dstRB);
    }
}

const void* SkScalerContext_DW::getDWMaskBits(const SkGlyph& glyph,
                                              DWRITE_RENDERING_MODE renderingMode,
                                              DWRITE_TEXTURE_TYPE textureType)
{
    DWriteFontTypeface* typeface = this->getDWriteTypeface();

    int sizeNeeded = glyph.width() * glyph.height();
    if (DWRITE_TEXTURE_CLEARTYPE_3x1 == textureType) {
        sizeNeeded *= 3;
    }
    if (sizeNeeded > fBits.size()) {
        fBits.resize(sizeNeeded);
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
    run.fontFace = typeface->fDWriteFontFace.get();
    run.fontEmSize = SkScalarToFloat(fTextSizeRender);
    run.bidiLevel = 0;
    run.glyphIndices = &index;
    run.isSideways = FALSE;
    run.glyphOffsets = &offset;
    {
        SkTScopedComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;
        {
            Exclusive l(maybe_dw_mutex(*typeface));
            // IDWriteFactory2::CreateGlyphRunAnalysis is very bad at aliased glyphs.
            if (typeface->fFactory2 &&
                    (fGridFitMode == DWRITE_GRID_FIT_MODE_DISABLED ||
                     fAntiAliasMode == DWRITE_TEXT_ANTIALIAS_MODE_GRAYSCALE))
            {
                HRNM(typeface->fFactory2->CreateGlyphRunAnalysis(&run,
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
                HRNM(typeface->fFactory->CreateGlyphRunAnalysis(&run,
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
        bbox.left = glyph.left();
        bbox.top = glyph.top();
        bbox.right = glyph.left() + glyph.width();
        bbox.bottom = glyph.top() + glyph.height();
        {
            Shared l(maybe_dw_mutex(*typeface));
            HRNM(glyphRunAnalysis->CreateAlphaTexture(textureType,
                    &bbox,
                    fBits.begin(),
                    sizeNeeded),
                 "Could not draw mask.");
        }
    }
    return fBits.begin();
}

bool SkScalerContext_DW::generateDWImage(const SkGlyph& glyph, void* imageBuffer) {
    //Create the mask.
    ScalerContextBits::value_type format = glyph.extraBits();
    DWRITE_RENDERING_MODE renderingMode = fRenderingMode;
    DWRITE_TEXTURE_TYPE textureType = fTextureType;
    if (format == ScalerContextBits::DW_1) {
        renderingMode = DWRITE_RENDERING_MODE_ALIASED;
        textureType = DWRITE_TEXTURE_ALIASED_1x1;
    }
    const void* bits = this->getDWMaskBits(glyph, renderingMode, textureType);
    if (!bits) {
        sk_bzero(imageBuffer, glyph.imageSize());
        return false;
    }

    //Copy the mask into the glyph.
    const uint8_t* src = (const uint8_t*)bits;
    if (DWRITE_RENDERING_MODE_ALIASED == renderingMode) {
        SkASSERT(SkMask::kBW_Format == glyph.maskFormat());
        SkASSERT(DWRITE_TEXTURE_ALIASED_1x1 == textureType);
        BilevelToBW(src, glyph, imageBuffer);
    } else if (!isLCD(fRec)) {
        if (textureType == DWRITE_TEXTURE_ALIASED_1x1) {
            if (fPreBlend.isApplicable()) {
                GrayscaleToA8<true>(src, glyph, imageBuffer, fPreBlend.fG);
            } else {
                GrayscaleToA8<false>(src, glyph, imageBuffer, fPreBlend.fG);
            }
        } else {
            if (fPreBlend.isApplicable()) {
                RGBToA8<true>(src, glyph, imageBuffer, fPreBlend.fG);
            } else {
                RGBToA8<false>(src, glyph, imageBuffer, fPreBlend.fG);
            }
        }
    } else {
        SkASSERT(SkMask::kLCD16_Format == glyph.maskFormat());
        if (fPreBlend.isApplicable()) {
            if (fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag) {
                RGBToLcd16<true, false>(src, glyph, imageBuffer,
                                        fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                RGBToLcd16<true, true>(src, glyph, imageBuffer,
                                       fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        } else {
            if (fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag) {
                RGBToLcd16<false, false>(src, glyph, imageBuffer,
                                         fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            } else {
                RGBToLcd16<false, true>(src, glyph, imageBuffer,
                                        fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
            }
        }
    }
    return true;
}

bool SkScalerContext_DW::drawColorImage(const SkGlyph& glyph, SkCanvas& canvas) {
    SkTScopedComPtr<IDWriteColorGlyphRunEnumerator> colorLayers;
    if (!getColorGlyphRun(glyph, &colorLayers)) {
        SkASSERTF(false, "Could not get color layers");
        return false;
    }

    SkPaint paint;
    paint.setAntiAlias(fRenderingMode != DWRITE_RENDERING_MODE_ALIASED);

    if (this->isSubpixel()) {
        canvas.translate(SkFixedToScalar(glyph.getSubXFixed()),
                         SkFixedToScalar(glyph.getSubYFixed()));
    }
    canvas.concat(fSkXform);

    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    size_t paletteEntryCount = typeface->fPaletteEntryCount;
    SkColor* palette = typeface->fPalette.get();
    BOOL hasNextRun = FALSE;
    while (SUCCEEDED(colorLayers->MoveNext(&hasNextRun)) && hasNextRun) {
        const DWRITE_COLOR_GLYPH_RUN* colorGlyph;
        HRBM(colorLayers->GetCurrentRun(&colorGlyph), "Could not get current color glyph run");

        SkColor color;
        if (colorGlyph->paletteIndex == 0xffff) {
            color = fRec.fForegroundColor;
        } else if (colorGlyph->paletteIndex < paletteEntryCount) {
            color = palette[colorGlyph->paletteIndex];
        } else {
            SK_TRACEHR(DWRITE_E_NOCOLOR, "Invalid palette index.");
            color = SK_ColorBLACK;
        }
        paint.setColor(color);

        SkPath path;
        SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
        HRBM(SkDWriteGeometrySink::Create(&path, &geometryToPath),
             "Could not create geometry to path converter.");
        {
            Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
            HRBM(colorGlyph->glyphRun.fontFace->GetGlyphRunOutline(
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
        canvas.drawPath(path, paint);
    }
    return true;
}

bool SkScalerContext_DW::generateColorImage(const SkGlyph& glyph, void* imageBuffer) {
    SkASSERT(glyph.maskFormat() == SkMask::Format::kARGB32_Format);

    SkBitmap dstBitmap;
    // TODO: mark this as sRGB when the blits will be sRGB.
    dstBitmap.setInfo(SkImageInfo::Make(glyph.width(), glyph.height(),
                                        kN32_SkColorType, kPremul_SkAlphaType),
                                        glyph.rowBytes());
    dstBitmap.setPixels(imageBuffer);

    SkCanvas canvas(dstBitmap);
    if constexpr (kSkShowTextBlitCoverage) {
        canvas.clear(0x33FF0000);
    } else {
        canvas.clear(SK_ColorTRANSPARENT);
    }
    canvas.translate(-SkIntToScalar(glyph.left()), -SkIntToScalar(glyph.top()));

    return this->drawColorImage(glyph, canvas);
}

bool SkScalerContext_DW::drawSVGImage(const SkGlyph& glyph, SkCanvas& canvas) {
    DWriteFontTypeface* typeface = this->getDWriteTypeface();
    IDWriteFontFace4* fontFace4 = typeface->fDWriteFontFace4.get();
    if (!fontFace4) {
        return false;
    }

    DWRITE_GLYPH_IMAGE_FORMATS imageFormats;
    HRBM(fontFace4->GetGlyphImageFormats(glyph.getGlyphID(), 0, UINT32_MAX, &imageFormats),
         "Cannot get glyph image formats.");
    if (!(imageFormats & DWRITE_GLYPH_IMAGE_FORMATS_SVG)) {
        return false;
    }

    SkGraphics::OpenTypeSVGDecoderFactory svgFactory = SkGraphics::GetOpenTypeSVGDecoderFactory();
    if (!svgFactory) {
        return false;
    }

    DWRITE_GLYPH_IMAGE_DATA glyphData;
    void* glyphDataContext;
    HRBM(fontFace4->GetGlyphImageData(glyph.getGlyphID(),
                                      fTextSizeRender,
                                      DWRITE_GLYPH_IMAGE_FORMATS_SVG,
                                      &glyphData,
                                      &glyphDataContext),
         "Glyph SVG data could not be acquired.");
    auto svgDecoder = svgFactory((const uint8_t*)glyphData.imageData, glyphData.imageDataSize);
    fontFace4->ReleaseGlyphImageData(glyphDataContext);
    if (!svgDecoder) {
        return false;
    }

    size_t paletteEntryCount = typeface->fPaletteEntryCount;
    SkColor* palette = typeface->fPalette.get();
    int upem = typeface->getUnitsPerEm();

    SkMatrix matrix = fSkXform;
    SkScalar scale = fTextSizeRender / upem;
    matrix.preScale(scale, scale);
    matrix.preTranslate(-glyphData.horizontalLeftOrigin.x, -glyphData.horizontalLeftOrigin.y);
    if (this->isSubpixel()) {
        matrix.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                             SkFixedToScalar(glyph.getSubYFixed()));
    }
    canvas.concat(matrix);

    return svgDecoder->render(canvas, upem, glyph.getGlyphID(),
                              fRec.fForegroundColor, SkSpan(palette, paletteEntryCount));
}

bool SkScalerContext_DW::generateSVGImage(const SkGlyph& glyph, void* imageBuffer) {
    SkASSERT(glyph.maskFormat() == SkMask::Format::kARGB32_Format);

    SkBitmap dstBitmap;
    // TODO: mark this as sRGB when the blits will be sRGB.
    dstBitmap.setInfo(SkImageInfo::Make(glyph.width(), glyph.height(),
                                        kN32_SkColorType, kPremul_SkAlphaType),
                      glyph.rowBytes());
    dstBitmap.setPixels(imageBuffer);

    SkCanvas canvas(dstBitmap);
    if constexpr (kSkShowTextBlitCoverage) {
        canvas.clear(0x33FF0000);
    } else {
        canvas.clear(SK_ColorTRANSPARENT);
    }
    canvas.translate(-SkIntToScalar(glyph.left()), -SkIntToScalar(glyph.top()));

    return this->drawSVGImage(glyph, canvas);
}

bool SkScalerContext_DW::drawPngImage(const SkGlyph& glyph, SkCanvas& canvas) {
    IDWriteFontFace4* fontFace4 = this->getDWriteTypeface()->fDWriteFontFace4.get();
    if (!fontFace4) {
        return false;
    }

    DWRITE_GLYPH_IMAGE_DATA glyphData;
    void* glyphDataContext;
    HRBM(fontFace4->GetGlyphImageData(glyph.getGlyphID(),
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
    sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(std::move(data));
    if (!image) {
        return false;
    }

    if (this->isSubpixel()) {
        canvas.translate(SkFixedToScalar(glyph.getSubXFixed()),
                         SkFixedToScalar(glyph.getSubYFixed()));
    }
    canvas.concat(fSkXform);
    SkScalar ratio = fTextSizeRender / glyphData.pixelsPerEm;
    canvas.scale(ratio, ratio);
    canvas.translate(-glyphData.horizontalLeftOrigin.x, -glyphData.horizontalLeftOrigin.y);
    canvas.drawImage(image, 0, 0);
    return true;
}

bool SkScalerContext_DW::generatePngImage(const SkGlyph& glyph, void* imageBuffer) {
    SkASSERT(glyph.maskFormat() == SkMask::Format::kARGB32_Format);

    SkBitmap dstBitmap;
    dstBitmap.setInfo(SkImageInfo::Make(glyph.width(), glyph.height(),
                                        kN32_SkColorType, kPremul_SkAlphaType),
                      glyph.rowBytes());
    dstBitmap.setPixels(imageBuffer);

    SkCanvas canvas(dstBitmap);
    canvas.clear(SK_ColorTRANSPARENT);
    canvas.translate(-glyph.left(), -glyph.top());

    return this->drawPngImage(glyph, canvas);
}

void SkScalerContext_DW::generateImage(const SkGlyph& glyph, void* imageBuffer) {
    ScalerContextBits::value_type format = glyph.extraBits();
    if (format == ScalerContextBits::DW ||
        format == ScalerContextBits::DW_1)
    {
        this->generateDWImage(glyph, imageBuffer);
    } else if (format == ScalerContextBits::COLRv1) {
        this->generateColorV1Image(glyph, imageBuffer);
    } else if (format == ScalerContextBits::COLR) {
        this->generateColorImage(glyph, imageBuffer);
    } else if (format == ScalerContextBits::SVG) {
        this->generateSVGImage(glyph, imageBuffer);
    } else if (format == ScalerContextBits::PNG) {
        this->generatePngImage(glyph, imageBuffer);
    } else if (format == ScalerContextBits::PATH) {
        const SkPath* devPath = glyph.path();
        SkASSERT_RELEASE(devPath);
        SkMaskBuilder mask(static_cast<uint8_t*>(imageBuffer),
                           glyph.iRect(), glyph.rowBytes(), glyph.maskFormat());
        SkASSERT(SkMask::kARGB32_Format != mask.fFormat);
        const bool doBGR = SkToBool(fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag);
        const bool doVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);
        const bool a8LCD = SkToBool(fRec.fFlags & SkScalerContext::kGenA8FromLCD_Flag);
        const bool hairline = glyph.pathIsHairline();
        GenerateImageFromPath(mask, *devPath, fPreBlend, doBGR, doVert, a8LCD, hairline);
    } else {
        SK_ABORT("Bad format");
    }
}

bool SkScalerContext_DW::generatePath(const SkGlyph& glyph, SkPath* path) {
    SkASSERT(path);
    path->reset();

    SkGlyphID glyphID = glyph.getGlyphID();

    // DirectWrite treats all out of bounds glyph ids as having the same data as glyph 0.
    // For consistency with all other backends, treat out of range glyph ids as an error.
    if (fGlyphCount <= glyphID) {
        return false;
    }

    SkTScopedComPtr<IDWriteGeometrySink> geometryToPath;
    HRBM(SkDWriteGeometrySink::Create(path, &geometryToPath),
         "Could not create geometry to path converter.");
    UINT16 glyphId = SkTo<UINT16>(glyphID);
    {
        Exclusive l(maybe_dw_mutex(*this->getDWriteTypeface()));
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

sk_sp<SkDrawable> SkScalerContext_DW::generateDrawable(const SkGlyph& glyph) {
    struct GlyphDrawable : public SkDrawable {
        SkScalerContext_DW* fSelf;
        SkGlyph fGlyph;
        GlyphDrawable(SkScalerContext_DW* self, const SkGlyph& glyph) : fSelf(self), fGlyph(glyph){}
        SkRect onGetBounds() override { return fGlyph.rect();  }
        size_t onApproximateBytesUsed() override { return sizeof(GlyphDrawable); }
        void maybeShowTextBlitCoverage(SkCanvas* canvas) {
            if constexpr (kSkShowTextBlitCoverage) {
                SkPaint paint;
                paint.setColor(0x3300FF00);
                paint.setStyle(SkPaint::kFill_Style);
                canvas->drawRect(this->onGetBounds(), paint);
            }
        }
    };
    struct COLRv1GlyphDrawable : public GlyphDrawable {
        using GlyphDrawable::GlyphDrawable;
        void onDraw(SkCanvas* canvas) override {
            this->maybeShowTextBlitCoverage(canvas);
            fSelf->drawColorV1Image(fGlyph, *canvas);
        }
    };
    struct COLRGlyphDrawable : public GlyphDrawable {
        using GlyphDrawable::GlyphDrawable;
        void onDraw(SkCanvas* canvas) override {
            this->maybeShowTextBlitCoverage(canvas);
            fSelf->drawColorImage(fGlyph, *canvas);
        }
    };
    struct SVGGlyphDrawable : public GlyphDrawable {
        using GlyphDrawable::GlyphDrawable;
        void onDraw(SkCanvas* canvas) override {
            this->maybeShowTextBlitCoverage(canvas);
            fSelf->drawSVGImage(fGlyph, *canvas);
        }
    };
    ScalerContextBits::value_type format = glyph.extraBits();
    if (format == ScalerContextBits::COLRv1) {
        return sk_sp<SkDrawable>(new COLRv1GlyphDrawable(this, glyph));
    }
    if (format == ScalerContextBits::COLR) {
        return sk_sp<SkDrawable>(new COLRGlyphDrawable(this, glyph));
    }
    if (format == ScalerContextBits::SVG) {
        return sk_sp<SkDrawable>(new SVGGlyphDrawable(this, glyph));
    }
    return nullptr;
}

#endif//defined(SK_BUILD_FOR_WIN)
