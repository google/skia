/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkFontHost_FreeType_common.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkImage.h"
#include "include/core/SkOpenTypeSVGDecoder.h"
#include "include/core/SkPath.h"
#include "include/effects/SkGradientShader.h"
#include "include/pathops/SkPathOps.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkFDot6.h"
#include "src/core/SkSwizzlePriv.h"

#include <algorithm>
#include <utility>

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftbitmap.h>
#ifdef FT_COLOR_H
#   include <freetype/ftcolor.h>
#endif
#include <freetype/ftimage.h>
#include <freetype/ftoutln.h>
#include <freetype/ftsizes.h>
// In the past, FT_GlyphSlot_Own_Bitmap was defined in this header file.
#include <freetype/ftsynth.h>

using namespace skia_private;

namespace {
[[maybe_unused]] static inline const constexpr bool kSkShowTextBlitCoverage = false;
}

#if defined(FT_CONFIG_OPTION_SVG)
#   include <freetype/otsvg.h>
#endif

#ifdef TT_SUPPORT_COLRV1
// FT_ClipBox and FT_Get_Color_Glyph_ClipBox introduced VER-2-11-0-18-g47cf8ebf4
// FT_COLR_COMPOSITE_PLUS and renumbering introduced VER-2-11-0-21-ge40ae7569
// FT_SIZEOF_LONG_LONG introduced VER-2-11-0-31-gffdac8d67
// FT_PaintRadialGradient changed size and layout at VER-2-11-0-147-gd3d3ff76d
// FT_STATIC_CAST introduced VER-2-11-0-172-g9079c5d91
// So undefine TT_SUPPORT_COLRV1 before 2.11.1 but not if FT_STATIC_CAST is defined.
#if (((FREETYPE_MAJOR)  < 2) || \
     ((FREETYPE_MAJOR) == 2 && (FREETYPE_MINOR)  < 11) || \
     ((FREETYPE_MAJOR) == 2 && (FREETYPE_MINOR) == 11 && (FREETYPE_PATCH) < 1)) && \
    !defined(FT_STATIC_CAST)
#    undef TT_SUPPORT_COLRV1
#else
#    include "src/base/SkScopeExit.h"
#endif
#endif

// FT_OUTLINE_OVERLAP was added in FreeType 2.10.3
#ifndef FT_OUTLINE_OVERLAP
#    define FT_OUTLINE_OVERLAP 0x40
#endif

// FT_LOAD_COLOR and the corresponding FT_Pixel_Mode::FT_PIXEL_MODE_BGRA
// were introduced in FreeType 2.5.0.
// The following may be removed once FreeType 2.5.0 is required to build.
#ifndef FT_LOAD_COLOR
#    define FT_LOAD_COLOR ( 1L << 20 )
#    define FT_PIXEL_MODE_BGRA 7
#endif

#ifdef SK_DEBUG
const char* SkTraceFtrGetError(int e) {
    switch ((FT_Error)e) {
        #undef FTERRORS_H_
        #define FT_ERRORDEF( e, v, s ) case v: return s;
        #define FT_ERROR_START_LIST
        #define FT_ERROR_END_LIST
        #include FT_ERRORS_H
        #undef FT_ERRORDEF
        #undef FT_ERROR_START_LIST
        #undef FT_ERROR_END_LIST
        default: return "";
    }
}
#endif  // SK_DEBUG

#ifdef TT_SUPPORT_COLRV1
bool operator==(const FT_OpaquePaint& a, const FT_OpaquePaint& b) {
    return a.p == b.p && a.insert_root_transform == b.insert_root_transform;
}

// The stop_offset field is being upgraded to a larger representation in FreeType, and changed from
// 2.14 to 16.16. Adjust the shift factor depending on size type.
static_assert(sizeof(FT_Fixed) != sizeof(FT_F2Dot14));
constexpr float kColorStopShift =
    sizeof(FT_ColorStop::stop_offset) == sizeof(FT_F2Dot14) ? 1 << 14 : 1 << 16;
#endif

namespace {
using SkUniqueFTSize = std::unique_ptr<FT_SizeRec, SkFunctionObject<FT_Done_Size>>;

FT_Pixel_Mode compute_pixel_mode(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
            return FT_PIXEL_MODE_MONO;
        case SkMask::kA8_Format:
        default:
            return FT_PIXEL_MODE_GRAY;
    }
}

///////////////////////////////////////////////////////////////////////////////

uint16_t packTriple(U8CPU r, U8CPU g, U8CPU b) {
    if constexpr (kSkShowTextBlitCoverage) {
        r = std::max(r, (U8CPU)0x40);
        g = std::max(g, (U8CPU)0x40);
        b = std::max(b, (U8CPU)0x40);
    }
    return SkPack888ToRGB16(r, g, b);
}

uint16_t grayToRGB16(U8CPU gray) {
    if constexpr (kSkShowTextBlitCoverage) {
        gray = std::max(gray, (U8CPU)0x40);
    }
    return SkPack888ToRGB16(gray, gray, gray);
}

int bittst(const uint8_t data[], int bitOffset) {
    SkASSERT(bitOffset >= 0);
    int lowBit = data[bitOffset >> 3] >> (~bitOffset & 7);
    return lowBit & 1;
}

/**
 *  Copies a FT_Bitmap into an SkMask with the same dimensions.
 *
 *  FT_PIXEL_MODE_MONO
 *  FT_PIXEL_MODE_GRAY
 *  FT_PIXEL_MODE_LCD
 *  FT_PIXEL_MODE_LCD_V
 */
template<bool APPLY_PREBLEND>
void copyFT2LCD16(const FT_Bitmap& bitmap, SkMaskBuilder* dstMask, int lcdIsBGR,
                  const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB)
{
    SkASSERT(SkMask::kLCD16_Format == dstMask->fFormat);
    if (FT_PIXEL_MODE_LCD != bitmap.pixel_mode) {
        SkASSERT(dstMask->fBounds.width() == static_cast<int>(bitmap.width));
    }
    if (FT_PIXEL_MODE_LCD_V != bitmap.pixel_mode) {
        SkASSERT(dstMask->fBounds.height() == static_cast<int>(bitmap.rows));
    }

    const uint8_t* src = bitmap.buffer;
    uint16_t* dst = reinterpret_cast<uint16_t*>(dstMask->image());
    const size_t dstRB = dstMask->fRowBytes;

    const int width = dstMask->fBounds.width();
    const int height = dstMask->fBounds.height();

    switch (bitmap.pixel_mode) {
        case FT_PIXEL_MODE_MONO:
            for (int y = height; y --> 0;) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = -bittst(src, x);
                }
                dst = (uint16_t*)((char*)dst + dstRB);
                src += bitmap.pitch;
            }
            break;
        case FT_PIXEL_MODE_GRAY:
            for (int y = height; y --> 0;) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = grayToRGB16(src[x]);
                }
                dst = (uint16_t*)((char*)dst + dstRB);
                src += bitmap.pitch;
            }
            break;
        case FT_PIXEL_MODE_LCD:
            SkASSERT(3 * dstMask->fBounds.width() == static_cast<int>(bitmap.width));
            for (int y = height; y --> 0;) {
                const uint8_t* triple = src;
                if (lcdIsBGR) {
                    for (int x = 0; x < width; x++) {
                        dst[x] = packTriple(sk_apply_lut_if<APPLY_PREBLEND>(triple[2], tableR),
                                            sk_apply_lut_if<APPLY_PREBLEND>(triple[1], tableG),
                                            sk_apply_lut_if<APPLY_PREBLEND>(triple[0], tableB));
                        triple += 3;
                    }
                } else {
                    for (int x = 0; x < width; x++) {
                        dst[x] = packTriple(sk_apply_lut_if<APPLY_PREBLEND>(triple[0], tableR),
                                            sk_apply_lut_if<APPLY_PREBLEND>(triple[1], tableG),
                                            sk_apply_lut_if<APPLY_PREBLEND>(triple[2], tableB));
                        triple += 3;
                    }
                }
                src += bitmap.pitch;
                dst = (uint16_t*)((char*)dst + dstRB);
            }
            break;
        case FT_PIXEL_MODE_LCD_V:
            SkASSERT(3 * dstMask->fBounds.height() == static_cast<int>(bitmap.rows));
            for (int y = height; y --> 0;) {
                const uint8_t* srcR = src;
                const uint8_t* srcG = srcR + bitmap.pitch;
                const uint8_t* srcB = srcG + bitmap.pitch;
                if (lcdIsBGR) {
                    using std::swap;
                    swap(srcR, srcB);
                }
                for (int x = 0; x < width; x++) {
                    dst[x] = packTriple(sk_apply_lut_if<APPLY_PREBLEND>(*srcR++, tableR),
                                        sk_apply_lut_if<APPLY_PREBLEND>(*srcG++, tableG),
                                        sk_apply_lut_if<APPLY_PREBLEND>(*srcB++, tableB));
                }
                src += 3 * bitmap.pitch;
                dst = (uint16_t*)((char*)dst + dstRB);
            }
            break;
        default:
            SkDEBUGF("FT_Pixel_Mode %d", bitmap.pixel_mode);
            SkDEBUGFAIL("unsupported FT_Pixel_Mode for LCD16");
            break;
    }
}

/**
 *  Copies a FT_Bitmap into an SkMask with the same dimensions.
 *
 *  Yes, No, Never Requested, Never Produced
 *
 *                        kBW kA8 k3D kARGB32 kLCD16
 *  FT_PIXEL_MODE_MONO     Y   Y  NR     N       Y
 *  FT_PIXEL_MODE_GRAY     N   Y  NR     N       Y
 *  FT_PIXEL_MODE_GRAY2   NP  NP  NR    NP      NP
 *  FT_PIXEL_MODE_GRAY4   NP  NP  NR    NP      NP
 *  FT_PIXEL_MODE_LCD     NP  NP  NR    NP      NP
 *  FT_PIXEL_MODE_LCD_V   NP  NP  NR    NP      NP
 *  FT_PIXEL_MODE_BGRA     N   N  NR     Y       N
 *
 *  TODO: All of these N need to be Y or otherwise ruled out.
 */
void copyFTBitmap(const FT_Bitmap& srcFTBitmap, SkMaskBuilder* dstMask) {
    SkASSERTF(dstMask->fBounds.width() == static_cast<int>(srcFTBitmap.width),
              "dstMask.fBounds.width() = %d\n"
              "static_cast<int>(srcFTBitmap.width) = %d",
              dstMask->fBounds.width(),
              static_cast<int>(srcFTBitmap.width)
    );
    SkASSERTF(dstMask->fBounds.height() == static_cast<int>(srcFTBitmap.rows),
              "dstMask.fBounds.height() = %d\n"
              "static_cast<int>(srcFTBitmap.rows) = %d",
              dstMask->fBounds.height(),
              static_cast<int>(srcFTBitmap.rows)
    );

    const uint8_t* src = reinterpret_cast<const uint8_t*>(srcFTBitmap.buffer);
    const FT_Pixel_Mode srcFormat = static_cast<FT_Pixel_Mode>(srcFTBitmap.pixel_mode);
    // FT_Bitmap::pitch is an int and allowed to be negative.
    const int srcPitch = srcFTBitmap.pitch;
    const size_t srcRowBytes = SkTAbs(srcPitch);

    uint8_t* dst = dstMask->image();
    const SkMask::Format dstFormat = dstMask->fFormat;
    const size_t dstRowBytes = dstMask->fRowBytes;

    const size_t width = srcFTBitmap.width;
    const size_t height = srcFTBitmap.rows;

    if (SkMask::kLCD16_Format == dstFormat) {
        copyFT2LCD16<false>(srcFTBitmap, dstMask, false, nullptr, nullptr, nullptr);
        return;
    }

    if ((FT_PIXEL_MODE_MONO == srcFormat && SkMask::kBW_Format == dstFormat) ||
        (FT_PIXEL_MODE_GRAY == srcFormat && SkMask::kA8_Format == dstFormat))
    {
        size_t commonRowBytes = std::min(srcRowBytes, dstRowBytes);
        for (size_t y = height; y --> 0;) {
            memcpy(dst, src, commonRowBytes);
            src += srcPitch;
            dst += dstRowBytes;
        }
    } else if (FT_PIXEL_MODE_MONO == srcFormat && SkMask::kA8_Format == dstFormat) {
        for (size_t y = height; y --> 0;) {
            uint8_t byte = 0;
            int bits = 0;
            const uint8_t* src_row = src;
            uint8_t* dst_row = dst;
            for (size_t x = width; x --> 0;) {
                if (0 == bits) {
                    byte = *src_row++;
                    bits = 8;
                }
                *dst_row++ = byte & 0x80 ? 0xff : 0x00;
                bits--;
                byte <<= 1;
            }
            src += srcPitch;
            dst += dstRowBytes;
        }
    } else if (FT_PIXEL_MODE_BGRA == srcFormat && SkMask::kARGB32_Format == dstFormat) {
        // FT_PIXEL_MODE_BGRA is pre-multiplied.
        for (size_t y = height; y --> 0;) {
            const uint8_t* src_row = src;
            SkPMColor* dst_row = reinterpret_cast<SkPMColor*>(dst);
            for (size_t x = 0; x < width; ++x) {
                uint8_t b = *src_row++;
                uint8_t g = *src_row++;
                uint8_t r = *src_row++;
                uint8_t a = *src_row++;
                *dst_row++ = SkPackARGB32(a, r, g, b);
                if constexpr (kSkShowTextBlitCoverage) {
                    *(dst_row-1) = SkFourByteInterp256(*(dst_row-1), SK_ColorWHITE, 0x40);
                }
            }
            src += srcPitch;
            dst += dstRowBytes;
        }
    } else {
        SkDEBUGF("FT_Pixel_Mode %d, SkMask::Format %d\n", srcFormat, dstFormat);
        SkDEBUGFAIL("unsupported combination of FT_Pixel_Mode and SkMask::Format");
    }
}

inline int convert_8_to_1(unsigned byte) {
    SkASSERT(byte <= 0xFF);
    // Arbitrary decision that making the cutoff at 1/4 instead of 1/2 in general looks better.
    return (byte >> 6) != 0;
}

uint8_t pack_8_to_1(const uint8_t alpha[8]) {
    unsigned bits = 0;
    for (int i = 0; i < 8; ++i) {
        bits <<= 1;
        bits |= convert_8_to_1(alpha[i]);
    }
    return SkToU8(bits);
}

void packA8ToA1(SkMaskBuilder* dstMask, const uint8_t* src, size_t srcRB) {
    const int height = dstMask->fBounds.height();
    const int width = dstMask->fBounds.width();
    const int octs = width >> 3;
    const int leftOverBits = width & 7;

    uint8_t* dst = dstMask->image();
    const int dstPad = dstMask->fRowBytes - SkAlign8(width)/8;
    SkASSERT(dstPad >= 0);

    const int srcPad = srcRB - width;
    SkASSERT(srcPad >= 0);

    for (int y = 0; y < height; ++y) {
        for (int i = 0; i < octs; ++i) {
            *dst++ = pack_8_to_1(src);
            src += 8;
        }
        if (leftOverBits > 0) {
            unsigned bits = 0;
            int shift = 7;
            for (int i = 0; i < leftOverBits; ++i, --shift) {
                bits |= convert_8_to_1(*src++) << shift;
            }
            *dst++ = bits;
        }
        src += srcPad;
        dst += dstPad;
    }
}

inline SkMask::Format SkMaskFormat_for_SkColorType(SkColorType colorType) {
    switch (colorType) {
        case kAlpha_8_SkColorType:
            return SkMask::kA8_Format;
        case kN32_SkColorType:
            return SkMask::kARGB32_Format;
        default:
            SkDEBUGFAIL("unsupported SkBitmap::Config");
            return SkMask::kA8_Format;
    }
}

inline SkColorType SkColorType_for_FTPixelMode(FT_Pixel_Mode pixel_mode) {
    switch (pixel_mode) {
        case FT_PIXEL_MODE_MONO:
        case FT_PIXEL_MODE_GRAY:
            return kAlpha_8_SkColorType;
        case FT_PIXEL_MODE_BGRA:
            return kN32_SkColorType;
        default:
            SkDEBUGFAIL("unsupported FT_PIXEL_MODE");
            return kAlpha_8_SkColorType;
    }
}

inline SkColorType SkColorType_for_SkMaskFormat(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
        case SkMask::kA8_Format:
        case SkMask::kLCD16_Format:
            return kAlpha_8_SkColorType;
        case SkMask::kARGB32_Format:
            return kN32_SkColorType;
        default:
            SkDEBUGFAIL("unsupported destination SkBitmap::Config");
            return kAlpha_8_SkColorType;
    }
}

// Only build COLRv1 rendering code if FreeType is new enough to have COLRv1
// additions. FreeType defines a macro in the ftoption header to tell us whether
// it does support these features.
#ifdef TT_SUPPORT_COLRV1

const uint16_t kForegroundColorPaletteIndex = 0xFFFF;

// This linear interpolation is used for calculating a truncated color line in special edge cases.
// This interpolation needs to be kept in sync with what the gradient shader would normally do when
// truncating and drawing color lines. When drawing into N32 surfaces, this is expected to be true.
// If that changes, or if we support other color spaces in CPAL tables at some point, this needs to
// be looked at.
SkColor4f lerpSkColor(SkColor4f c0, SkColor4f c1, float t) {
    // Due to the floating point calculation in the caller, when interpolating between very narrow
    // stops, we may get values outside the interpolation range, guard against these.
    if (t < 0) {
        return c0;
    }
    if (t > 1) {
        return c1;
    }

    const auto c0_4f = skvx::float4::Load(c0.vec());
    const auto c1_4f = skvx::float4::Load(c1.vec());
    const auto c_4f = c0_4f + (c1_4f - c0_4f) * t;

    SkColor4f l;
    c_4f.store(l.vec());
    return l;
}

enum TruncateStops {
    TruncateStart,
    TruncateEnd
};

// Truncate a vector of color stops at a previously computed stop position and insert at that
// position the color interpolated between the surrounding stops.
void truncateToStopInterpolating(SkScalar zeroRadiusStop,
                                 std::vector<SkColor4f>& colors,
                                 std::vector<SkScalar>& stops,
                                 TruncateStops truncateStops) {
    if (stops.size() <= 1u ||
        zeroRadiusStop < stops.front() || stops.back() < zeroRadiusStop)
    {
        return;
    }

    size_t afterIndex = (truncateStops == TruncateStart)
        ? std::lower_bound(stops.begin(), stops.end(), zeroRadiusStop) - stops.begin()
        : std::upper_bound(stops.begin(), stops.end(), zeroRadiusStop) - stops.begin();

    const float t = (zeroRadiusStop - stops[afterIndex - 1]) /
                    (stops[afterIndex] - stops[afterIndex - 1]);
    SkColor4f lerpColor = lerpSkColor(colors[afterIndex - 1], colors[afterIndex], t);

    if (truncateStops == TruncateStart) {
        stops.erase(stops.begin(), stops.begin() + afterIndex);
        colors.erase(colors.begin(), colors.begin() + afterIndex);
        stops.insert(stops.begin(), 0);
        colors.insert(colors.begin(), lerpColor);
    } else {
        stops.erase(stops.begin() + afterIndex, stops.end());
        colors.erase(colors.begin() + afterIndex, colors.end());
        stops.insert(stops.end(), 1);
        colors.insert(colors.end(), lerpColor);
    }
}

struct OpaquePaintHasher {
  size_t operator()(const FT_OpaquePaint& opaquePaint) {
      return SkGoodHash()(opaquePaint.p) ^
             SkGoodHash()(opaquePaint.insert_root_transform);
  }
};

using VisitedSet = THashSet<FT_OpaquePaint, OpaquePaintHasher>;

bool generateFacePathCOLRv1(FT_Face face, SkGlyphID glyphID, SkPath* path);

inline float SkColrV1AlphaToFloat(uint16_t alpha) { return (alpha / float(1 << 14)); }


inline SkTileMode ToSkTileMode(FT_PaintExtend extendMode) {
    switch (extendMode) {
        case FT_COLR_PAINT_EXTEND_REPEAT:
            return SkTileMode::kRepeat;
        case FT_COLR_PAINT_EXTEND_REFLECT:
            return SkTileMode::kMirror;
        default:
            return SkTileMode::kClamp;
    }
}

inline SkBlendMode ToSkBlendMode(FT_Composite_Mode compositeMode) {
    switch (compositeMode) {
        case FT_COLR_COMPOSITE_CLEAR:
            return SkBlendMode::kClear;
        case FT_COLR_COMPOSITE_SRC:
            return SkBlendMode::kSrc;
        case FT_COLR_COMPOSITE_DEST:
            return SkBlendMode::kDst;
        case FT_COLR_COMPOSITE_SRC_OVER:
            return SkBlendMode::kSrcOver;
        case FT_COLR_COMPOSITE_DEST_OVER:
            return SkBlendMode::kDstOver;
        case FT_COLR_COMPOSITE_SRC_IN:
            return SkBlendMode::kSrcIn;
        case FT_COLR_COMPOSITE_DEST_IN:
            return SkBlendMode::kDstIn;
        case FT_COLR_COMPOSITE_SRC_OUT:
            return SkBlendMode::kSrcOut;
        case FT_COLR_COMPOSITE_DEST_OUT:
            return SkBlendMode::kDstOut;
        case FT_COLR_COMPOSITE_SRC_ATOP:
            return SkBlendMode::kSrcATop;
        case FT_COLR_COMPOSITE_DEST_ATOP:
            return SkBlendMode::kDstATop;
        case FT_COLR_COMPOSITE_XOR:
            return SkBlendMode::kXor;
        case FT_COLR_COMPOSITE_PLUS:
            return SkBlendMode::kPlus;
        case FT_COLR_COMPOSITE_SCREEN:
            return SkBlendMode::kScreen;
        case FT_COLR_COMPOSITE_OVERLAY:
            return SkBlendMode::kOverlay;
        case FT_COLR_COMPOSITE_DARKEN:
            return SkBlendMode::kDarken;
        case FT_COLR_COMPOSITE_LIGHTEN:
            return SkBlendMode::kLighten;
        case FT_COLR_COMPOSITE_COLOR_DODGE:
            return SkBlendMode::kColorDodge;
        case FT_COLR_COMPOSITE_COLOR_BURN:
            return SkBlendMode::kColorBurn;
        case FT_COLR_COMPOSITE_HARD_LIGHT:
            return SkBlendMode::kHardLight;
        case FT_COLR_COMPOSITE_SOFT_LIGHT:
            return SkBlendMode::kSoftLight;
        case FT_COLR_COMPOSITE_DIFFERENCE:
            return SkBlendMode::kDifference;
        case FT_COLR_COMPOSITE_EXCLUSION:
            return SkBlendMode::kExclusion;
        case FT_COLR_COMPOSITE_MULTIPLY:
            return SkBlendMode::kMultiply;
        case FT_COLR_COMPOSITE_HSL_HUE:
            return SkBlendMode::kHue;
        case FT_COLR_COMPOSITE_HSL_SATURATION:
            return SkBlendMode::kSaturation;
        case FT_COLR_COMPOSITE_HSL_COLOR:
            return SkBlendMode::kColor;
        case FT_COLR_COMPOSITE_HSL_LUMINOSITY:
            return SkBlendMode::kLuminosity;
        default:
            return SkBlendMode::kDst;
    }
}

inline SkMatrix ToSkMatrix(FT_Affine23 affine23) {
    // Convert from FreeType's FT_Affine23 column major order to SkMatrix row-major order.
    return SkMatrix::MakeAll(
         SkFixedToScalar(affine23.xx), -SkFixedToScalar(affine23.xy),  SkFixedToScalar(affine23.dx),
        -SkFixedToScalar(affine23.yx),  SkFixedToScalar(affine23.yy), -SkFixedToScalar(affine23.dy),
         0,                             0,                             1);
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

bool colrv1_configure_skpaint(FT_Face face,
                              const SkSpan<SkColor>& palette,
                              const SkColor foregroundColor,
                              const FT_COLR_Paint& colrPaint,
                              SkPaint* paint) {
    auto fetchColorStops = [&face, &palette, &foregroundColor](
                                               const FT_ColorStopIterator& colorStopIterator,
                                               std::vector<SkScalar>& stops,
                                               std::vector<SkColor4f>& colors) -> bool {
        const FT_UInt colorStopCount = colorStopIterator.num_color_stops;
        if (colorStopCount == 0) {
            return false;
        }

        // 5.7.11.2.4 ColorIndex, ColorStop and ColorLine
        // "Applications shall apply the colorStops in increasing stopOffset order."
        struct ColorStop {
            SkScalar pos;
            SkColor4f color;
        };
        std::vector<ColorStop> colorStopsSorted;
        colorStopsSorted.resize(colorStopCount);

        FT_ColorStop ftStop;
        FT_ColorStopIterator mutable_color_stop_iterator = colorStopIterator;
        while (FT_Get_Colorline_Stops(face, &ftStop, &mutable_color_stop_iterator)) {
            FT_UInt index = mutable_color_stop_iterator.current_color_stop - 1;
            ColorStop& skStop = colorStopsSorted[index];
            skStop.pos = ftStop.stop_offset / kColorStopShift;
            FT_UInt16& palette_index = ftStop.color.palette_index;
            if (palette_index == kForegroundColorPaletteIndex) {
                skStop.color = SkColor4f::FromColor(foregroundColor);
            } else if (palette_index >= palette.size()) {
                return false;
            } else {
                skStop.color = SkColor4f::FromColor(palette[palette_index]);
            }
            skStop.color.fA *= SkColrV1AlphaToFloat(ftStop.color.alpha);
        }

        std::stable_sort(colorStopsSorted.begin(), colorStopsSorted.end(),
                         [](const ColorStop& a, const ColorStop& b) { return a.pos < b.pos; });

        stops.resize(colorStopCount);
        colors.resize(colorStopCount);
        for (size_t i = 0; i < colorStopCount; ++i) {
            stops[i] = colorStopsSorted[i].pos;
            colors[i] = colorStopsSorted[i].color;
        }
        return true;
    };

    switch (colrPaint.format) {
        case FT_COLR_PAINTFORMAT_SOLID: {
            FT_PaintSolid solid = colrPaint.u.solid;

            // Dont' draw anything with this color if the palette index is out of bounds.
            SkColor4f color = SkColors::kTransparent;
            if (solid.color.palette_index == kForegroundColorPaletteIndex) {
                color = SkColor4f::FromColor(foregroundColor);
            } else if (solid.color.palette_index >= palette.size()) {
                return false;
            } else {
                color = SkColor4f::FromColor(palette[solid.color.palette_index]);
            }
            color.fA *= SkColrV1AlphaToFloat(solid.color.alpha);
            paint->setShader(nullptr);
            paint->setColor(color);
            return true;
        }
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT: {
            const FT_PaintLinearGradient& linearGradient = colrPaint.u.linear_gradient;
            std::vector<SkScalar> stops;
            std::vector<SkColor4f> colors;

            if (!fetchColorStops(linearGradient.colorline.color_stop_iterator, stops, colors)) {
                return false;
            }

            if (stops.size() == 1) {
                paint->setColor(colors[0]);
                return true;
            }

            SkPoint linePositions[2] = {SkPoint::Make( SkFixedToScalar(linearGradient.p0.x),
                                                      -SkFixedToScalar(linearGradient.p0.y)),
                                        SkPoint::Make( SkFixedToScalar(linearGradient.p1.x),
                                                      -SkFixedToScalar(linearGradient.p1.y))};
            SkPoint p0 = linePositions[0];
            SkPoint p1 = linePositions[1];
            SkPoint p2 = SkPoint::Make( SkFixedToScalar(linearGradient.p2.x),
                                       -SkFixedToScalar(linearGradient.p2.y));

            // If p0p1 or p0p2 are degenerate probably nothing should be drawn.
            // If p0p1 and p0p2 are parallel then one side is the first color and the other side is
            // the last color, depending on the direction.
            // For now, just use the first color.
            if (p1 == p0 || p2 == p0 || !SkPoint::CrossProduct(p1 - p0, p2 - p0)) {
                paint->setColor(colors[0]);
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
            SkTileMode tileMode = ToSkTileMode(linearGradient.colorline.extend);
            SkScalar colorStopRange = stops.back() - stops.front();
            // If the color stops are all at the same offset position, repeat and reflect modes
            // become meaningless.
            if (colorStopRange == 0.f) {
              if (tileMode != SkTileMode::kClamp) {
                paint->setColor(SK_ColorTRANSPARENT);
                return true;
              } else {
                // Insert duplicated fake color stop in pad case at +1.0f to enable the projection
                // of circles for an originally 0-length color stop range. Adding this stop will
                // paint the equivalent gradient, because: All font specified color stops are in the
                // same spot, mode is pad, so everything before this spot is painted with the first
                // color, everything after this spot is painted with the last color. Not adding this
                // stop will skip the projection and result in specifying non-normalized color stops
                // to the shader.
                stops.push_back(stops.back() + 1.0f);
                colors.push_back(colors.back());
                colorStopRange = 1.0f;
              }
            }
            SkASSERT(colorStopRange != 0.f);

            // If the colorStopRange is 0 at this point, the default behavior of the shader is to
            // clamp to 1 color stops that are above 1, clamp to 0 for color stops that are below 0,
            // and repeat the outer color stops at 0 and 1 if the color stops are inside the
            // range. That will result in the correct rendering.
            if ((colorStopRange != 1 || stops.front() != 0.f)) {
                SkVector p0p3 = p3 - p0;
                SkVector p0Offset = p0p3;
                p0Offset.scale(stops.front());
                SkVector p1Offset = p0p3;
                p1Offset.scale(stops.back());

                linePositions[0] = p0 + p0Offset;
                linePositions[1] = p0 + p1Offset;

                SkScalar scaleFactor = 1 / colorStopRange;
                SkScalar startOffset = stops.front();
                for (SkScalar& stop : stops) {
                    stop = (stop - startOffset) * scaleFactor;
                }
            }

            sk_sp<SkShader> shader(SkGradientShader::MakeLinear(
                linePositions,
                colors.data(), SkColorSpace::MakeSRGB(), stops.data(), stops.size(),
                tileMode,
                SkGradientShader::Interpolation{
                    SkGradientShader::Interpolation::InPremul::kNo,
                    SkGradientShader::Interpolation::ColorSpace::kSRGB,
                    SkGradientShader::Interpolation::HueMethod::kShorter
                },
                nullptr));

            SkASSERT(shader);
            // An opaque color is needed to ensure the gradient is not modulated by alpha.
            paint->setColor(SK_ColorBLACK);
            paint->setShader(shader);
            return true;
        }
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT: {
            const FT_PaintRadialGradient& radialGradient = colrPaint.u.radial_gradient;
            SkPoint start = SkPoint::Make( SkFixedToScalar(radialGradient.c0.x),
                                          -SkFixedToScalar(radialGradient.c0.y));
            SkScalar startRadius = SkFixedToScalar(radialGradient.r0);
            SkPoint end = SkPoint::Make( SkFixedToScalar(radialGradient.c1.x),
                                        -SkFixedToScalar(radialGradient.c1.y));
            SkScalar endRadius = SkFixedToScalar(radialGradient.r1);


            std::vector<SkScalar> stops;
            std::vector<SkColor4f> colors;
            if (!fetchColorStops(radialGradient.colorline.color_stop_iterator, stops, colors)) {
                return false;
            }

            if (stops.size() == 1) {
                paint->setColor(colors[0]);
                return true;
            }

            SkScalar colorStopRange = stops.back() - stops.front();
            SkTileMode tileMode = ToSkTileMode(radialGradient.colorline.extend);

            if (colorStopRange == 0.f) {
              if (tileMode != SkTileMode::kClamp) {
                paint->setColor(SK_ColorTRANSPARENT);
                return true;
              } else {
                // Insert duplicated fake color stop in pad case at +1.0f to enable the projection
                // of circles for an originally 0-length color stop range. Adding this stop will
                // paint the equivalent gradient, because: All font specified color stops are in the
                // same spot, mode is pad, so everything before this spot is painted with the first
                // color, everything after this spot is painted with the last color. Not adding this
                // stop will skip the projection and result in specifying non-normalized color stops
                // to the shader.
                stops.push_back(stops.back() + 1.0f);
                colors.push_back(colors.back());
                colorStopRange = 1.0f;
              }
            }
            SkASSERT(colorStopRange != 0.f);

            // If the colorStopRange is 0 at this point, the default behavior of the shader is to
            // clamp to 1 color stops that are above 1, clamp to 0 for color stops that are below 0,
            // and repeat the outer color stops at 0 and 1 if the color stops are inside the
            // range. That will result in the correct rendering.
            if (colorStopRange != 1 || stops.front() != 0.f) {
                // For the Skia two-point caonical shader to understand the
                // COLRv1 color stops we need to scale stops to 0 to 1 range and
                // interpolate new centers and radii. Otherwise the shader
                // clamps stops outside the range to 0 and 1 (larger interval)
                // or repeats the outer stops at 0 and 1 if the (smaller
                // interval).
                SkVector startToEnd = end - start;
                SkScalar radiusDiff = endRadius - startRadius;
                SkScalar scaleFactor = 1 / colorStopRange;
                SkScalar stopsStartOffset = stops.front();

                SkVector startOffset = startToEnd;
                startOffset.scale(stops.front());
                SkVector endOffset = startToEnd;
                endOffset.scale(stops.back());

                // The order of the following computations is important in order to avoid
                // overwriting start or startRadius before the second reassignment.
                end = start + endOffset;
                start = start + startOffset;
                endRadius = startRadius + radiusDiff * stops.back();
                startRadius = startRadius + radiusDiff * stops.front();

                for (auto& stop : stops) {
                    stop = (stop - stopsStartOffset) * scaleFactor;
                }
            }

            // For negative radii, interpolation is needed to prepare parameters suitable
            // for invoking the shader. Implementation below as resolution discussed in
            // https://github.com/googlefonts/colr-gradients-spec/issues/367.
            // Truncate to manually interpolated color for tile mode clamp, otherwise
            // calculate positive projected circles.
            if (startRadius < 0 || endRadius < 0) {
                if (startRadius == endRadius && startRadius < 0) {
                    paint->setColor(SK_ColorTRANSPARENT);
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
                        truncateToStopInterpolating(
                                zeroRadiusStop, colors, stops, truncateSide);
                    } else {
                        // If both radii have become negative and where clamped to 0, we need to
                        // produce a single color cone, otherwise the shader colors the whole
                        // plane in a single color when two radii are specified as 0.
                        if (radiusDiff > 0) {
                            end = start + startToEnd;
                            endRadius = radiusDiff;
                            colors.erase(colors.begin(), colors.end() - 1);
                            stops.erase(stops.begin(), stops.end() - 1);
                        } else {
                            start -= startToEnd;
                            startRadius = -radiusDiff;
                            colors.erase(colors.begin() + 1, colors.end());
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

            // An opaque color is needed to ensure the gradient is not modulated by alpha.
            paint->setColor(SK_ColorBLACK);

            paint->setShader(SkGradientShader::MakeTwoPointConical(
                start, startRadius, end, endRadius,
                colors.data(), SkColorSpace::MakeSRGB(), stops.data(), stops.size(),
                tileMode,
                SkGradientShader::Interpolation{
                    SkGradientShader::Interpolation::InPremul::kNo,
                    SkGradientShader::Interpolation::ColorSpace::kSRGB,
                    SkGradientShader::Interpolation::HueMethod::kShorter
                },
                nullptr));

            return true;
        }
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT: {
            const FT_PaintSweepGradient& sweepGradient = colrPaint.u.sweep_gradient;
            SkPoint center = SkPoint::Make( SkFixedToScalar(sweepGradient.center.x),
                                           -SkFixedToScalar(sweepGradient.center.y));


            SkScalar startAngle = SkFixedToScalar(sweepGradient.start_angle * 180.0f);
            SkScalar endAngle = SkFixedToScalar(sweepGradient.end_angle * 180.0f);
            // OpenType 1.9.1 adds a shift to the angle to ease specification of a 0 to 360
            // degree sweep.
            startAngle += 180.0f;
            endAngle += 180.0f;

            std::vector<SkScalar> stops;
            std::vector<SkColor4f> colors;
            if (!fetchColorStops(sweepGradient.colorline.color_stop_iterator, stops, colors)) {
                return false;
            }

            if (stops.size() == 1) {
                paint->setColor(colors[0]);
                return true;
            }

            // An opaque color is needed to ensure the gradient is not modulated by alpha.
            paint->setColor(SK_ColorBLACK);

            // New (Var)SweepGradient implementation compliant with OpenType 1.9.1 from here.

            // The shader expects stops from 0 to 1, so we need to account for
            // minimum and maximum stop positions being different from 0 and
            // 1. We do that by scaling minimum and maximum stop positions to
            // the 0 to 1 interval and scaling the angles inverse proportionally.

            // 1) Scale angles to their equivalent positions if stops were from 0 to 1.

            SkScalar sectorAngle = endAngle - startAngle;
            SkTileMode tileMode = ToSkTileMode(sweepGradient.colorline.extend);
            if (sectorAngle == 0 && tileMode != SkTileMode::kClamp) {
                // "If the ColorLine's extend mode is reflect or repeat and start and end angle
                // are equal, nothing is drawn.".
                paint->setColor(SK_ColorTRANSPARENT);
                return true;
            }


            SkScalar startAngleScaled = startAngle + sectorAngle * stops.front();
            SkScalar endAngleScaled = startAngle + sectorAngle * stops.back();

            // 2) Scale stops accordingly to 0 to 1 range.

            float colorStopRange = stops.back() - stops.front();
            if (colorStopRange == 0.f) {
              if (tileMode != SkTileMode::kClamp) {
                paint->setColor(SK_ColorTRANSPARENT);
                return true;
              } else {
                // Insert duplicated fake color stop in pad case at +1.0f to feed the shader correct
                // values and enable painting a pad sweep gradient with two colors. Adding this stop
                // will paint the equivalent gradient, because: All font specified color stops are
                // in the same spot, mode is pad, so everything before this spot is painted with the
                // first color, everything after this spot is painted with the last color. Not
                // adding this stop will skip the projection and result in specifying non-normalized
                // color stops to the shader.
                stops.push_back(stops.back() + 1.0f);
                colors.push_back(colors.back());
                colorStopRange = 1.0f;
              }
            }

            SkScalar scaleFactor = 1 / colorStopRange;
            SkScalar startOffset = stops.front();

            for (SkScalar& stop : stops) {
                stop = (stop - startOffset) * scaleFactor;
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
                std::reverse(colors.begin(), colors.end());
                for (auto& stop : stops) {
                    stop = 1.0f - stop;
                }
            }

            paint->setShader(SkGradientShader::MakeSweep(
                center.x(), center.y(),
                colors.data(), SkColorSpace::MakeSRGB(), stops.data(), stops.size(),
                tileMode,
                startAngleScaled, endAngleScaled,
                SkGradientShader::Interpolation{
                    SkGradientShader::Interpolation::InPremul::kNo,
                    SkGradientShader::Interpolation::ColorSpace::kSRGB,
                    SkGradientShader::Interpolation::HueMethod::kShorter
                },
                nullptr));

            return true;
        }
        default: {
            SkASSERT(false);
            return false;
        }
    }
    SkUNREACHABLE;
}

bool colrv1_draw_paint(SkCanvas* canvas,
                       const SkSpan<SkColor>& palette,
                       const SkColor foregroundColor,
                       FT_Face face,
                       const FT_COLR_Paint& colrPaint) {
    switch (colrPaint.format) {
        case FT_COLR_PAINTFORMAT_GLYPH: {
            FT_UInt glyphID = colrPaint.u.glyph.glyphID;
            SkPath path;
            /* TODO: Currently this call retrieves the path at units_per_em size. If we want to get
             * correct hinting for the scaled size under the transforms at this point in the color
             * glyph graph, we need to extract at least the requested glyph width and height and
             * pass that to the path generation. */
            if (!generateFacePathCOLRv1(face, glyphID, &path)) {
                return false;
            }
            if constexpr (kSkShowTextBlitCoverage) {
                SkPaint highlight_paint;
                highlight_paint.setColor(0x33FF0000);
                canvas->drawRect(path.getBounds(), highlight_paint);
            }
            canvas->clipPath(path, true /* doAntiAlias */);
            return true;
        }
        case FT_COLR_PAINTFORMAT_SOLID:
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT: {
            SkPaint skPaint;
            if (!colrv1_configure_skpaint(face, palette, foregroundColor, colrPaint, &skPaint)) {
                return false;
            }
            canvas->drawPaint(skPaint);
            return true;
        }
        case FT_COLR_PAINTFORMAT_TRANSFORM:
        case FT_COLR_PAINTFORMAT_TRANSLATE:
        case FT_COLR_PAINTFORMAT_SCALE:
        case FT_COLR_PAINTFORMAT_ROTATE:
        case FT_COLR_PAINTFORMAT_SKEW:
            [[fallthrough]];  // Transforms handled in colrv1_transform.
        default:
            SkASSERT(false);
            return false;
    }
    SkUNREACHABLE;
}

bool colrv1_draw_glyph_with_path(SkCanvas* canvas,
                                 const SkSpan<SkColor>& palette, SkColor foregroundColor,
                                 FT_Face face,
                                 const FT_COLR_Paint& glyphPaint, const FT_COLR_Paint& fillPaint) {
    SkASSERT(glyphPaint.format == FT_COLR_PAINTFORMAT_GLYPH);
    SkASSERT(fillPaint.format == FT_COLR_PAINTFORMAT_SOLID ||
             fillPaint.format == FT_COLR_PAINTFORMAT_LINEAR_GRADIENT ||
             fillPaint.format == FT_COLR_PAINTFORMAT_RADIAL_GRADIENT ||
             fillPaint.format == FT_COLR_PAINTFORMAT_SWEEP_GRADIENT);

    SkPaint skiaFillPaint;
    skiaFillPaint.setAntiAlias(true);
    if (!colrv1_configure_skpaint(face, palette, foregroundColor, fillPaint, &skiaFillPaint)) {
        return false;
    }

    FT_UInt glyphID = glyphPaint.u.glyph.glyphID;
    SkPath path;
    /* TODO: Currently this call retrieves the path at units_per_em size. If we want to get
     * correct hinting for the scaled size under the transforms at this point in the color
     * glyph graph, we need to extract at least the requested glyph width and height and
     * pass that to the path generation. */
    if (!generateFacePathCOLRv1(face, glyphID, &path)) {
        return false;
    }
    if constexpr (kSkShowTextBlitCoverage) {
        SkPaint highlightPaint;
        highlightPaint.setColor(0x33FF0000);
        canvas->drawRect(path.getBounds(), highlightPaint);
    }
    canvas->drawPath(path, skiaFillPaint);
    return true;
}


/* In drawing mode, concatenates the transforms directly on SkCanvas. In
 * bounding box calculation mode, no SkCanvas is specified, but we only want to
 * retrieve the transform from the FreeType paint object. */
void colrv1_transform(FT_Face face,
                      const FT_COLR_Paint& colrPaint,
                      SkCanvas* canvas,
                      SkMatrix* outTransform = nullptr) {
    SkMatrix transform;

    SkASSERT(canvas || outTransform);

    switch (colrPaint.format) {
        case FT_COLR_PAINTFORMAT_TRANSFORM: {
            transform = ToSkMatrix(colrPaint.u.transform.affine);
            break;
        }
        case FT_COLR_PAINTFORMAT_TRANSLATE: {
            transform = SkMatrix::Translate( SkFixedToScalar(colrPaint.u.translate.dx),
                                            -SkFixedToScalar(colrPaint.u.translate.dy));
            break;
        }
        case FT_COLR_PAINTFORMAT_SCALE: {
            transform.setScale( SkFixedToScalar(colrPaint.u.scale.scale_x),
                                SkFixedToScalar(colrPaint.u.scale.scale_y),
                                SkFixedToScalar(colrPaint.u.scale.center_x),
                               -SkFixedToScalar(colrPaint.u.scale.center_y));
            break;
        }
        case FT_COLR_PAINTFORMAT_ROTATE: {
            // COLRv1 angles are counter-clockwise, compare
            // https://docs.microsoft.com/en-us/typography/opentype/spec/colr#formats-24-to-27-paintrotate-paintvarrotate-paintrotatearoundcenter-paintvarrotatearoundcenter
            transform = SkMatrix::RotateDeg(
                    -SkFixedToScalar(colrPaint.u.rotate.angle) * 180.0f,
                    SkPoint::Make( SkFixedToScalar(colrPaint.u.rotate.center_x),
                                  -SkFixedToScalar(colrPaint.u.rotate.center_y)));
            break;
        }
        case FT_COLR_PAINTFORMAT_SKEW: {
            // In the PAINTFORMAT_ROTATE implementation, SkMatrix setRotate
            // snaps to 0 for values very close to 0. Do the same here.

            SkScalar xDeg = SkFixedToScalar(colrPaint.u.skew.x_skew_angle) * 180.0f;
            SkScalar xRad = SkDegreesToRadians(xDeg);
            SkScalar xTan = SkScalarTan(xRad);
            xTan = SkScalarNearlyZero(xTan) ? 0.0f : xTan;

            SkScalar yDeg = SkFixedToScalar(colrPaint.u.skew.y_skew_angle) * 180.0f;
            // Negate y_skew_angle due to Skia's y-down coordinate system to achieve
            // counter-clockwise skew along the y-axis.
            SkScalar yRad = SkDegreesToRadians(-yDeg);
            SkScalar yTan = SkScalarTan(yRad);
            yTan = SkScalarNearlyZero(yTan) ? 0.0f : yTan;

            transform.setSkew(xTan, yTan,
                              SkFixedToScalar(colrPaint.u.skew.center_x),
                             -SkFixedToScalar(colrPaint.u.skew.center_y));
            break;
        }
        default: {
            SkASSERT(false);  // Only transforms are handled in this function.
        }
    }
    if (canvas) {
        canvas->concat(transform);
    }
    if (outTransform) {
        *outTransform = transform;
    }
}

bool colrv1_start_glyph(SkCanvas* canvas,
                        const SkSpan<SkColor>& palette,
                        const SkColor foregroundColor,
                        FT_Face face,
                        uint16_t glyphId,
                        FT_Color_Root_Transform rootTransform,
                        VisitedSet* activePaints);

bool colrv1_traverse_paint(SkCanvas* canvas,
                           const SkSpan<SkColor>& palette,
                           const SkColor foregroundColor,
                           FT_Face face,
                           FT_OpaquePaint opaquePaint,
                           VisitedSet* activePaints) {
    // Cycle detection, see section "5.7.11.1.9 Color glyphs as a directed acyclic graph".
    if (activePaints->contains(opaquePaint)) {
        return true;
    }

    activePaints->add(opaquePaint);
    SK_AT_SCOPE_EXIT(activePaints->remove(opaquePaint));

    FT_COLR_Paint paint;
    if (!FT_Get_Paint(face, opaquePaint, &paint)) {
        return false;
    }

    SkAutoCanvasRestore autoRestore(canvas, true /* doSave */);
    switch (paint.format) {
        case FT_COLR_PAINTFORMAT_COLR_LAYERS: {
            FT_LayerIterator& layerIterator = paint.u.colr_layers.layer_iterator;
            FT_OpaquePaint layerPaint{nullptr, 1};
            while (FT_Get_Paint_Layers(face, &layerIterator, &layerPaint)) {
                if (!colrv1_traverse_paint(canvas, palette, foregroundColor, face,
                                           layerPaint, activePaints)) {
                    return false;
                }
            }
            return true;
        }
        case FT_COLR_PAINTFORMAT_GLYPH:
            // Special case paint graph leaf situations to improve
            // performance. These are situations in the graph where a GlyphPaint
            // is followed by either a solid or a gradient fill. Here we can use
            // drawPath() + SkPaint directly which is faster than setting a
            // clipPath() followed by a drawPaint().
            FT_COLR_Paint fillPaint;
            if (!FT_Get_Paint(face, paint.u.glyph.paint, &fillPaint)) {
                return false;
            }
            if (fillPaint.format == FT_COLR_PAINTFORMAT_SOLID ||
                fillPaint.format == FT_COLR_PAINTFORMAT_LINEAR_GRADIENT ||
                fillPaint.format == FT_COLR_PAINTFORMAT_RADIAL_GRADIENT ||
                fillPaint.format == FT_COLR_PAINTFORMAT_SWEEP_GRADIENT)
            {
                return colrv1_draw_glyph_with_path(canvas, palette, foregroundColor,
                                                   face, paint, fillPaint);
            }
            if (!colrv1_draw_paint(canvas, palette, foregroundColor, face, paint)) {
                return false;
            }
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.glyph.paint, activePaints);
        case FT_COLR_PAINTFORMAT_COLR_GLYPH:
            return colrv1_start_glyph(canvas, palette, foregroundColor,
                                      face, paint.u.colr_glyph.glyphID, FT_COLOR_NO_ROOT_TRANSFORM,
                                      activePaints);
        case FT_COLR_PAINTFORMAT_TRANSFORM:
            colrv1_transform(face, paint, canvas);
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.transform.paint, activePaints);
        case FT_COLR_PAINTFORMAT_TRANSLATE:
            colrv1_transform(face, paint, canvas);
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.translate.paint, activePaints);
        case FT_COLR_PAINTFORMAT_SCALE:
            colrv1_transform(face, paint, canvas);
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.scale.paint, activePaints);
        case FT_COLR_PAINTFORMAT_ROTATE:
            colrv1_transform(face, paint, canvas);
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.rotate.paint, activePaints);
        case FT_COLR_PAINTFORMAT_SKEW:
            colrv1_transform(face, paint, canvas);
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.skew.paint, activePaints);
        case FT_COLR_PAINTFORMAT_COMPOSITE: {
            SkAutoCanvasRestore acr(canvas, false);
            canvas->saveLayer(nullptr, nullptr);
            if (!colrv1_traverse_paint(canvas, palette, foregroundColor,
                                       face, paint.u.composite.backdrop_paint, activePaints)) {
                return false;
            }
            SkPaint blendModePaint;
            blendModePaint.setBlendMode(ToSkBlendMode(paint.u.composite.composite_mode));
            canvas->saveLayer(nullptr, &blendModePaint);
            return colrv1_traverse_paint(canvas, palette, foregroundColor,
                                         face, paint.u.composite.source_paint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_SOLID:
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT: {
            return colrv1_draw_paint(canvas, palette, foregroundColor, face, paint);
        }
        default:
            SkASSERT(false);
            return false;
    }
    SkUNREACHABLE;
}

SkPath GetClipBoxPath(FT_Face face, uint16_t glyphId, bool untransformed) {
    SkPath resultPath;
    SkUniqueFTSize unscaledFtSize = nullptr;
    FT_Size oldSize = face->size;
    FT_Matrix oldTransform;
    FT_Vector oldDelta;
    FT_Error err = 0;

    if (untransformed) {
        unscaledFtSize.reset(
                [face]() -> FT_Size {
                    FT_Size size;
                    FT_Error err = FT_New_Size(face, &size);
                    if (err != 0) {
                        SK_TRACEFTR(err,
                                    "FT_New_Size(%s) failed in generateFacePathStaticCOLRv1.",
                                    face->family_name);
                        return nullptr;
                    }
                    return size;
                }());
        if (!unscaledFtSize) {
            return resultPath;
        }

        err = FT_Activate_Size(unscaledFtSize.get());
        if (err != 0) {
            return resultPath;
        }

        err = FT_Set_Char_Size(face, SkIntToFDot6(face->units_per_EM), 0, 0, 0);
        if (err != 0) {
            return resultPath;
        }

        FT_Get_Transform(face, &oldTransform, &oldDelta);
        FT_Set_Transform(face, nullptr, nullptr);
    }

    FT_ClipBox colrGlyphClipBox;
    if (FT_Get_Color_Glyph_ClipBox(face, glyphId, &colrGlyphClipBox)) {
        resultPath = SkPath::Polygon({{ SkFDot6ToScalar(colrGlyphClipBox.bottom_left.x),
                                       -SkFDot6ToScalar(colrGlyphClipBox.bottom_left.y)},
                                      { SkFDot6ToScalar(colrGlyphClipBox.top_left.x),
                                       -SkFDot6ToScalar(colrGlyphClipBox.top_left.y)},
                                      { SkFDot6ToScalar(colrGlyphClipBox.top_right.x),
                                       -SkFDot6ToScalar(colrGlyphClipBox.top_right.y)},
                                      { SkFDot6ToScalar(colrGlyphClipBox.bottom_right.x),
                                       -SkFDot6ToScalar(colrGlyphClipBox.bottom_right.y)}},
                                     true);
    }

    if (untransformed) {
        err = FT_Activate_Size(oldSize);
        if (err != 0) {
          return resultPath;
        }
        FT_Set_Transform(face, &oldTransform, &oldDelta);
    }

    return resultPath;
}

bool colrv1_start_glyph(SkCanvas* canvas,
                        const SkSpan<SkColor>& palette,
                        const SkColor foregroundColor,
                        FT_Face face,
                        uint16_t glyphId,
                        FT_Color_Root_Transform rootTransform,
                        VisitedSet* activePaints) {
    FT_OpaquePaint opaquePaint{nullptr, 1};
    if (!FT_Get_Color_Glyph_Paint(face, glyphId, rootTransform, &opaquePaint)) {
        return false;
    }

    bool untransformed = rootTransform == FT_COLOR_NO_ROOT_TRANSFORM;
    SkPath clipBoxPath = GetClipBoxPath(face, glyphId, untransformed);
    if (!clipBoxPath.isEmpty()) {
        canvas->clipPath(clipBoxPath, true);
    }

    if (!colrv1_traverse_paint(canvas, palette, foregroundColor,
                               face, opaquePaint, activePaints)) {
        return false;
    }

    return true;
}

bool colrv1_start_glyph_bounds(SkMatrix *ctm,
                               SkRect* bounds,
                               FT_Face face,
                               uint16_t glyphId,
                               FT_Color_Root_Transform rootTransform,
                               VisitedSet* activePaints);

bool colrv1_traverse_paint_bounds(SkMatrix* ctm,
                                  SkRect* bounds,
                                  FT_Face face,
                                  FT_OpaquePaint opaquePaint,
                                  VisitedSet* activePaints) {
    // Cycle detection, see section "5.7.11.1.9 Color glyphs as a directed acyclic graph".
    if (activePaints->contains(opaquePaint)) {
        return false;
    }

    activePaints->add(opaquePaint);
    SK_AT_SCOPE_EXIT(activePaints->remove(opaquePaint));

    FT_COLR_Paint paint;
    if (!FT_Get_Paint(face, opaquePaint, &paint)) {
        return false;
    }

    SkMatrix restoreMatrix = *ctm;
    SK_AT_SCOPE_EXIT(*ctm = restoreMatrix);

    switch (paint.format) {
        case FT_COLR_PAINTFORMAT_COLR_LAYERS: {
            FT_LayerIterator& layerIterator = paint.u.colr_layers.layer_iterator;
            FT_OpaquePaint layerPaint{nullptr, 1};
            while (FT_Get_Paint_Layers(face, &layerIterator, &layerPaint)) {
                if (!colrv1_traverse_paint_bounds(ctm, bounds, face, layerPaint, activePaints)) {
                    return false;
                }
            }
            return true;
        }
        case FT_COLR_PAINTFORMAT_GLYPH: {
            FT_UInt glyphID = paint.u.glyph.glyphID;
            SkPath path;
            if (!generateFacePathCOLRv1(face, glyphID, &path)) {
                return false;
            }
            path.transform(*ctm);
            bounds->join(path.getBounds());
            return true;
        }
        case FT_COLR_PAINTFORMAT_COLR_GLYPH: {
            FT_UInt glyphID = paint.u.colr_glyph.glyphID;
            return colrv1_start_glyph_bounds(ctm, bounds, face, glyphID, FT_COLOR_NO_ROOT_TRANSFORM,
                                             activePaints);
        }
        case FT_COLR_PAINTFORMAT_TRANSFORM: {
            SkMatrix transformMatrix;
            colrv1_transform(face, paint, nullptr, &transformMatrix);
            ctm->preConcat(transformMatrix);
            FT_OpaquePaint& transformPaint = paint.u.transform.paint;
            return colrv1_traverse_paint_bounds(ctm, bounds, face, transformPaint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_TRANSLATE: {
            SkMatrix transformMatrix;
            colrv1_transform(face, paint, nullptr, &transformMatrix);
            ctm->preConcat(transformMatrix);
            FT_OpaquePaint& translatePaint = paint.u.translate.paint;
            return colrv1_traverse_paint_bounds(ctm, bounds, face, translatePaint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_SCALE: {
            SkMatrix transformMatrix;
            colrv1_transform(face, paint, nullptr, &transformMatrix);
            ctm->preConcat(transformMatrix);
            FT_OpaquePaint& scalePaint = paint.u.scale.paint;
            return colrv1_traverse_paint_bounds(ctm, bounds, face, scalePaint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_ROTATE: {
            SkMatrix transformMatrix;
            colrv1_transform(face, paint, nullptr, &transformMatrix);
            ctm->preConcat(transformMatrix);
            FT_OpaquePaint& rotatePaint = paint.u.rotate.paint;
            return colrv1_traverse_paint_bounds(ctm, bounds, face, rotatePaint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_SKEW: {
            SkMatrix transformMatrix;
            colrv1_transform(face, paint, nullptr, &transformMatrix);
            ctm->preConcat(transformMatrix);
            FT_OpaquePaint& skewPaint = paint.u.skew.paint;
            return colrv1_traverse_paint_bounds(ctm, bounds, face, skewPaint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_COMPOSITE: {
            FT_OpaquePaint& backdropPaint = paint.u.composite.backdrop_paint;
            FT_OpaquePaint&   sourcePaint = paint.u.composite.  source_paint;
            return colrv1_traverse_paint_bounds(ctm, bounds, face, backdropPaint, activePaints) &&
                   colrv1_traverse_paint_bounds(ctm, bounds, face,   sourcePaint, activePaints);
        }
        case FT_COLR_PAINTFORMAT_SOLID:
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT: {
            return true;
        }
        default:
            SkASSERT(false);
            return false;
    }
    SkUNREACHABLE;
}


bool colrv1_start_glyph_bounds(SkMatrix *ctm,
                               SkRect* bounds,
                               FT_Face face,
                               uint16_t glyphId,
                               FT_Color_Root_Transform rootTransform,
                               VisitedSet* activePaints) {
    FT_OpaquePaint opaquePaint{nullptr, 1};
    return FT_Get_Color_Glyph_Paint(face, glyphId, rootTransform, &opaquePaint) &&
           colrv1_traverse_paint_bounds(ctm, bounds, face, opaquePaint, activePaints);
}
#endif // TT_SUPPORT_COLRV1

}  // namespace

////////////////

void SkScalerContextFTUtils::init(SkColor fgColor, SkScalerContext::Flags flags) {
    fForegroundColor = fgColor;
    fFlags = flags;
}

#ifdef TT_SUPPORT_COLRV1
bool SkScalerContextFTUtils::drawCOLRv1Glyph(FT_Face face, const SkGlyph& glyph, uint32_t loadGlyphFlags,
                                             SkSpan<SkColor> palette, SkCanvas* canvas) const {
    if (this->isSubpixel()) {
        canvas->translate(SkFixedToScalar(glyph.getSubXFixed()),
                          SkFixedToScalar(glyph.getSubYFixed()));
    }

    VisitedSet activePaints;
    bool haveLayers =  colrv1_start_glyph(canvas, palette,
                                          fForegroundColor,
                                          face, glyph.getGlyphID(),
                                          FT_COLOR_INCLUDE_ROOT_TRANSFORM,
                                          &activePaints);
    SkASSERTF(haveLayers, "Could not get COLRv1 layers from '%s'.", face->family_name);
    return haveLayers;
}
#endif  // TT_SUPPORT_COLRV1

#ifdef FT_COLOR_H
bool SkScalerContextFTUtils::drawCOLRv0Glyph(FT_Face face, const SkGlyph& glyph, LoadGlyphFlags flags,
                                             SkSpan<SkColor> palette, SkCanvas* canvas) const {
    if (this->isSubpixel()) {
        canvas->translate(SkFixedToScalar(glyph.getSubXFixed()),
                          SkFixedToScalar(glyph.getSubYFixed()));
    }

    bool haveLayers = false;
    FT_LayerIterator layerIterator;
    layerIterator.p = nullptr;
    FT_UInt layerGlyphIndex = 0;
    FT_UInt layerColorIndex = 0;
    SkPaint paint;
    paint.setAntiAlias(!(flags & FT_LOAD_TARGET_MONO));
    while (FT_Get_Color_Glyph_Layer(face, glyph.getGlyphID(), &layerGlyphIndex,
                                    &layerColorIndex, &layerIterator)) {
        haveLayers = true;
        if (layerColorIndex == 0xFFFF) {
            paint.setColor(fForegroundColor);
        } else {
            paint.setColor(palette[layerColorIndex]);
        }
        SkPath path;
        if (this->generateFacePath(face, layerGlyphIndex, flags, &path)) {
            canvas->drawPath(path, paint);
        }
    }
    SkASSERTF(haveLayers, "Could not get COLRv0 layers from '%s'.", face->family_name);
    return haveLayers;
}
#endif  // FT_COLOR_H

#if defined(FT_CONFIG_OPTION_SVG)
bool SkScalerContextFTUtils::drawSVGGlyph(FT_Face face, const SkGlyph& glyph, LoadGlyphFlags flags,
                                          SkSpan<SkColor> palette, SkCanvas* canvas) const {
    SkASSERT(face->glyph->format == FT_GLYPH_FORMAT_SVG);

    FT_SVG_Document ftSvg = (FT_SVG_Document)face->glyph->other;
    SkMatrix m;
    FT_Matrix ftMatrix = ftSvg->transform;
    FT_Vector ftOffset = ftSvg->delta;
    m.setAll(
        SkFixedToFloat(ftMatrix.xx), -SkFixedToFloat(ftMatrix.xy),  SkFixedToFloat(ftOffset.x),
       -SkFixedToFloat(ftMatrix.yx),  SkFixedToFloat(ftMatrix.yy), -SkFixedToFloat(ftOffset.y),
        0                          ,  0                          ,  1                        );
    m.postScale(SkFixedToFloat(ftSvg->metrics.x_scale) / 64.0f,
                SkFixedToFloat(ftSvg->metrics.y_scale) / 64.0f);
    if (this->isSubpixel()) {
        m.postTranslate(SkFixedToScalar(glyph.getSubXFixed()),
                        SkFixedToScalar(glyph.getSubYFixed()));
    }
    canvas->concat(m);

    SkGraphics::OpenTypeSVGDecoderFactory svgFactory = SkGraphics::GetOpenTypeSVGDecoderFactory();
    if (!svgFactory) {
        return false;
    }
    auto svgDecoder = svgFactory(ftSvg->svg_document, ftSvg->svg_document_length);
    if (!svgDecoder) {
        return false;
    }
    return svgDecoder->render(*canvas, ftSvg->units_per_EM, glyph.getGlyphID(),
                              fForegroundColor, palette);
}
#endif  // FT_CONFIG_OPTION_SVG

void SkScalerContextFTUtils::generateGlyphImage(FT_Face face, const SkGlyph& glyph, void* imageBuffer,
                                                const SkMatrix& bitmapTransform,
                                                const SkMaskGamma::PreBlend& preBlend) const {
    switch ( face->glyph->format ) {
        case FT_GLYPH_FORMAT_OUTLINE: {
            FT_Outline* outline = &face->glyph->outline;

            int dx = 0, dy = 0;
            if (this->isSubpixel()) {
                dx = SkFixedToFDot6(glyph.getSubXFixed());
                dy = SkFixedToFDot6(glyph.getSubYFixed());
                // negate dy since freetype-y-goes-up and skia-y-goes-down
                dy = -dy;
            }

            memset(imageBuffer, 0, glyph.rowBytes() * glyph.height());

            if (SkMask::kLCD16_Format == glyph.maskFormat()) {
                const bool doBGR = SkToBool(fFlags & SkScalerContext::kLCD_BGROrder_Flag);
                const bool doVert = SkToBool(fFlags & SkScalerContext::kLCD_Vertical_Flag);

                FT_Outline_Translate(outline, dx, dy);
                FT_Error err = FT_Render_Glyph(face->glyph, doVert ? FT_RENDER_MODE_LCD_V :
                                                                     FT_RENDER_MODE_LCD);
                if (err) {
                    SK_TRACEFTR(err, "Could not render glyph %p.", face->glyph);
                    return;
                }

                SkMaskBuilder mask(static_cast<uint8_t*>(imageBuffer),
                                   glyph.iRect(), glyph.rowBytes(), glyph.maskFormat());

                if constexpr (kSkShowTextBlitCoverage) {
                    memset(mask.image(), 0x80, mask.fBounds.height() * mask.fRowBytes);
                }
                FT_GlyphSlotRec& ftGlyph = *face->glyph;

                if (!SkIRect::Intersects(mask.fBounds,
                                         SkIRect::MakeXYWH( ftGlyph.bitmap_left,
                                                           -ftGlyph.bitmap_top,
                                                            ftGlyph.bitmap.width,
                                                            ftGlyph.bitmap.rows)))
                {
                    return;
                }

                // If the FT_Bitmap extent is larger, discard bits of the bitmap outside the mask.
                // If the SkMask extent is larger, shrink mask to fit bitmap (clearing discarded).
                unsigned char* origBuffer = ftGlyph.bitmap.buffer;
                // First align the top left (origin).
                if (-ftGlyph.bitmap_top < mask.fBounds.fTop) {
                    int32_t topDiff = mask.fBounds.fTop - (-ftGlyph.bitmap_top);
                    ftGlyph.bitmap.buffer += ftGlyph.bitmap.pitch * topDiff;
                    ftGlyph.bitmap.rows -= topDiff;
                    ftGlyph.bitmap_top = -mask.fBounds.fTop;
                }
                if (ftGlyph.bitmap_left < mask.fBounds.fLeft) {
                    int32_t leftDiff = mask.fBounds.fLeft - ftGlyph.bitmap_left;
                    ftGlyph.bitmap.buffer += leftDiff;
                    ftGlyph.bitmap.width -= leftDiff;
                    ftGlyph.bitmap_left = mask.fBounds.fLeft;
                }
                if (mask.fBounds.fTop < -ftGlyph.bitmap_top) {
                    mask.image() += mask.fRowBytes * (-ftGlyph.bitmap_top - mask.fBounds.fTop);
                    mask.bounds().fTop = -ftGlyph.bitmap_top;
                }
                if (mask.fBounds.fLeft < ftGlyph.bitmap_left) {
                    mask.image() += sizeof(uint16_t) * (ftGlyph.bitmap_left - mask.fBounds.fLeft);
                    mask.bounds().fLeft = ftGlyph.bitmap_left;
                }
                // Origins aligned, clean up the width and height.
                int ftVertScale = (doVert ? 3 : 1);
                int ftHoriScale = (doVert ? 1 : 3);
                if (mask.fBounds.height() * ftVertScale < SkToInt(ftGlyph.bitmap.rows)) {
                    ftGlyph.bitmap.rows = mask.fBounds.height() * ftVertScale;
                }
                if (mask.fBounds.width() * ftHoriScale < SkToInt(ftGlyph.bitmap.width)) {
                    ftGlyph.bitmap.width = mask.fBounds.width() * ftHoriScale;
                }
                if (SkToInt(ftGlyph.bitmap.rows) < mask.fBounds.height() * ftVertScale) {
                    mask.bounds().fBottom = mask.fBounds.fTop + ftGlyph.bitmap.rows / ftVertScale;
                }
                if (SkToInt(ftGlyph.bitmap.width) < mask.fBounds.width() * ftHoriScale) {
                    mask.bounds().fRight = mask.fBounds.fLeft + ftGlyph.bitmap.width / ftHoriScale;
                }
                if (preBlend.isApplicable()) {
                    copyFT2LCD16<true>(ftGlyph.bitmap, &mask, doBGR,
                                       preBlend.fR, preBlend.fG, preBlend.fB);
                } else {
                    copyFT2LCD16<false>(ftGlyph.bitmap, &mask, doBGR,
                                        preBlend.fR, preBlend.fG, preBlend.fB);
                }
                // Restore the buffer pointer so FreeType can properly free it.
                ftGlyph.bitmap.buffer = origBuffer;
            } else {
                FT_BBox     bbox;
                FT_Bitmap   target;
                FT_Outline_Get_CBox(outline, &bbox);
                /*
                    what we really want to do for subpixel is
                        offset(dx, dy)
                        compute_bounds
                        offset(bbox & !63)
                    but that is two calls to offset, so we do the following, which
                    achieves the same thing with only one offset call.
                */
                FT_Outline_Translate(outline, dx - ((bbox.xMin + dx) & ~63),
                                              dy - ((bbox.yMin + dy) & ~63));

                target.width = glyph.width();
                target.rows = glyph.height();
                target.pitch = glyph.rowBytes();
                target.buffer = static_cast<uint8_t*>(imageBuffer);
                target.pixel_mode = compute_pixel_mode(glyph.maskFormat());
                target.num_grays = 256;

                FT_Outline_Get_Bitmap(face->glyph->library, outline, &target);
                if constexpr (kSkShowTextBlitCoverage) {
                    if (glyph.maskFormat() == SkMask::kBW_Format) {
                        for (unsigned y = 0; y < target.rows; y += 2) {
                            for (unsigned x = (y & 0x2); x < target.width; x+=4) {
                                uint8_t& b = target.buffer[(target.pitch * y) + (x >> 3)];
                                b = b ^ (1 << (0x7 - (x & 0x7)));
                            }
                        }
                    } else {
                        for (unsigned y = 0; y < target.rows; ++y) {
                            for (unsigned x = 0; x < target.width; ++x) {
                                uint8_t& a = target.buffer[(target.pitch * y) + x];
                                a = std::max<uint8_t>(a, 0x20);
                            }
                        }
                    }
                }
            }
        } break;

        case FT_GLYPH_FORMAT_BITMAP: {
            FT_Pixel_Mode pixel_mode = static_cast<FT_Pixel_Mode>(face->glyph->bitmap.pixel_mode);
            SkMask::Format maskFormat = glyph.maskFormat();

            // Assume that the other formats do not exist.
            SkASSERT(FT_PIXEL_MODE_MONO == pixel_mode ||
                     FT_PIXEL_MODE_GRAY == pixel_mode ||
                     FT_PIXEL_MODE_BGRA == pixel_mode);

            // These are the only formats this ScalerContext should request.
            SkASSERT(SkMask::kBW_Format == maskFormat ||
                     SkMask::kA8_Format == maskFormat ||
                     SkMask::kARGB32_Format == maskFormat ||
                     SkMask::kLCD16_Format == maskFormat);

            // If no scaling needed, directly copy glyph bitmap.
            if (bitmapTransform.isIdentity()) {
                SkMaskBuilder dstMask = SkMaskBuilder(static_cast<uint8_t*>(imageBuffer),
                                                      glyph.iRect(), glyph.rowBytes(),
                                                      glyph.maskFormat());
                copyFTBitmap(face->glyph->bitmap, &dstMask);
                break;
            }

            // Otherwise, scale the bitmap.

            // Copy the FT_Bitmap into an SkBitmap (either A8 or ARGB)
            SkBitmap unscaledBitmap;
            // TODO: mark this as sRGB when the blits will be sRGB.
            unscaledBitmap.setInfo(SkImageInfo::Make(face->glyph->bitmap.width,
                                                     face->glyph->bitmap.rows,
                                                     SkColorType_for_FTPixelMode(pixel_mode),
                                                     kPremul_SkAlphaType));
            if (!unscaledBitmap.tryAllocPixels()) {
                // TODO: set the imageBuffer to indicate "missing"
                memset(imageBuffer, 0, glyph.rowBytes() * glyph.height());
                return;
            }

            SkMaskBuilder unscaledBitmapAlias(
                reinterpret_cast<uint8_t*>(unscaledBitmap.getPixels()),
                SkIRect::MakeWH(unscaledBitmap.width(), unscaledBitmap.height()),
                unscaledBitmap.rowBytes(),
                SkMaskFormat_for_SkColorType(unscaledBitmap.colorType()));
            copyFTBitmap(face->glyph->bitmap, &unscaledBitmapAlias);

            // Wrap the glyph's mask in a bitmap, unless the glyph's mask is BW or LCD.
            // BW requires an A8 target for resizing, which can then be down sampled.
            // LCD should use a 4x A8 target, which will then be down sampled.
            // For simplicity, LCD uses A8 and is replicated.
            int bitmapRowBytes = 0;
            if (SkMask::kBW_Format != maskFormat && SkMask::kLCD16_Format != maskFormat) {
                bitmapRowBytes = glyph.rowBytes();
            }
            SkBitmap dstBitmap;
            // TODO: mark this as sRGB when the blits will be sRGB.
            dstBitmap.setInfo(SkImageInfo::Make(glyph.width(), glyph.height(),
                                                SkColorType_for_SkMaskFormat(maskFormat),
                                                kPremul_SkAlphaType),
                              bitmapRowBytes);
            if (SkMask::kBW_Format == maskFormat || SkMask::kLCD16_Format == maskFormat) {
                if (!dstBitmap.tryAllocPixels()) {
                    // TODO: set the fImage to indicate "missing"
                    memset(imageBuffer, 0, glyph.rowBytes() * glyph.height());
                    return;
                }
            } else {
                dstBitmap.setPixels(imageBuffer);
            }

            // Scale unscaledBitmap into dstBitmap.
            SkCanvas canvas(dstBitmap);
            if constexpr (kSkShowTextBlitCoverage) {
                canvas.clear(0x33FF0000);
            } else {
                canvas.clear(SK_ColorTRANSPARENT);
            }
            canvas.translate(-glyph.left(), -glyph.top());
            canvas.concat(bitmapTransform);
            canvas.translate(face->glyph->bitmap_left, -face->glyph->bitmap_top);

            SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNearest);
            canvas.drawImage(unscaledBitmap.asImage().get(), 0, 0, sampling, nullptr);

            // If the destination is BW or LCD, convert from A8.
            if (SkMask::kBW_Format == maskFormat) {
                // Copy the A8 dstBitmap into the A1 imageBuffer.
                SkMaskBuilder dstMask(static_cast<uint8_t*>(imageBuffer),
                                      glyph.iRect(), glyph.rowBytes(), glyph.maskFormat());
                packA8ToA1(&dstMask, dstBitmap.getAddr8(0, 0), dstBitmap.rowBytes());
            } else if (SkMask::kLCD16_Format == maskFormat) {
                // Copy the A8 dstBitmap into the LCD16 imageBuffer.
                uint8_t* src = dstBitmap.getAddr8(0, 0);
                uint16_t* dst = reinterpret_cast<uint16_t*>(imageBuffer);
                for (int y = dstBitmap.height(); y --> 0;) {
                    for (int x = 0; x < dstBitmap.width(); ++x) {
                        dst[x] = grayToRGB16(src[x]);
                    }
                    dst = (uint16_t*)((char*)dst + glyph.rowBytes());
                    src += dstBitmap.rowBytes();
                }
            }
        } break;

        default:
            SkDEBUGFAIL("unknown glyph format");
            memset(imageBuffer, 0, glyph.rowBytes() * glyph.height());
            return;
    }

// We used to always do this pre-USE_COLOR_LUMINANCE, but with colorlum,
// it is optional
#if defined(SK_GAMMA_APPLY_TO_A8)
    if (SkMask::kA8_Format == glyph.maskFormat() && preBlend.isApplicable()) {
        uint8_t* SK_RESTRICT dst = (uint8_t*)imageBuffer;
        unsigned rowBytes = glyph.rowBytes();

        for (int y = glyph.height() - 1; y >= 0; --y) {
            for (int x = glyph.width() - 1; x >= 0; --x) {
                dst[x] = preBlend.fG[dst[x]];
            }
            dst += rowBytes;
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////

namespace {

class SkFTGeometrySink {
    SkPath* fPath;
    bool fStarted;
    FT_Vector fCurrent;

    void goingTo(const FT_Vector* pt) {
        if (!fStarted) {
            fStarted = true;
            fPath->moveTo(SkFDot6ToScalar(fCurrent.x), -SkFDot6ToScalar(fCurrent.y));
        }
        fCurrent = *pt;
    }

    bool currentIsNot(const FT_Vector* pt) {
        return fCurrent.x != pt->x || fCurrent.y != pt->y;
    }

    static int Move(const FT_Vector* pt, void* ctx) {
        SkFTGeometrySink& self = *(SkFTGeometrySink*)ctx;
        if (self.fStarted) {
            self.fPath->close();
            self.fStarted = false;
        }
        self.fCurrent = *pt;
        return 0;
    }

    static int Line(const FT_Vector* pt, void* ctx) {
        SkFTGeometrySink& self = *(SkFTGeometrySink*)ctx;
        if (self.currentIsNot(pt)) {
            self.goingTo(pt);
            self.fPath->lineTo(SkFDot6ToScalar(pt->x), -SkFDot6ToScalar(pt->y));
        }
        return 0;
    }

    static int Quad(const FT_Vector* pt0, const FT_Vector* pt1, void* ctx) {
        SkFTGeometrySink& self = *(SkFTGeometrySink*)ctx;
        if (self.currentIsNot(pt0) || self.currentIsNot(pt1)) {
            self.goingTo(pt1);
            self.fPath->quadTo(SkFDot6ToScalar(pt0->x), -SkFDot6ToScalar(pt0->y),
                               SkFDot6ToScalar(pt1->x), -SkFDot6ToScalar(pt1->y));
        }
        return 0;
    }

    static int Cubic(const FT_Vector* pt0, const FT_Vector* pt1, const FT_Vector* pt2, void* ctx) {
        SkFTGeometrySink& self = *(SkFTGeometrySink*)ctx;
        if (self.currentIsNot(pt0) || self.currentIsNot(pt1) || self.currentIsNot(pt2)) {
            self.goingTo(pt2);
            self.fPath->cubicTo(SkFDot6ToScalar(pt0->x), -SkFDot6ToScalar(pt0->y),
                                SkFDot6ToScalar(pt1->x), -SkFDot6ToScalar(pt1->y),
                                SkFDot6ToScalar(pt2->x), -SkFDot6ToScalar(pt2->y));
        }
        return 0;
    }

public:
    SkFTGeometrySink(SkPath* path) : fPath{path}, fStarted{false}, fCurrent{0,0} {}

    inline static constexpr const FT_Outline_Funcs Funcs{
        /*move_to =*/ SkFTGeometrySink::Move,
        /*line_to =*/ SkFTGeometrySink::Line,
        /*conic_to =*/ SkFTGeometrySink::Quad,
        /*cubic_to =*/ SkFTGeometrySink::Cubic,
        /*shift = */ 0,
        /*delta =*/ 0,
    };
};

bool generateGlyphPathStatic(FT_Face face, SkPath* path) {
    SkFTGeometrySink sink{path};
    if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE ||
        FT_Outline_Decompose(&face->glyph->outline, &SkFTGeometrySink::Funcs, &sink))
    {
        path->reset();
        return false;
    }
    path->close();
    return true;
}

bool generateFacePathStatic(FT_Face face, SkGlyphID glyphID,
                            SkScalerContextFTUtils::LoadGlyphFlags flags, SkPath* path){
    flags |= FT_LOAD_BITMAP_METRICS_ONLY;  // Don't decode any bitmaps.
    flags |= FT_LOAD_NO_BITMAP; // Ignore embedded bitmaps.
    flags &= ~FT_LOAD_RENDER;  // Don't scan convert.
    flags &= ~FT_LOAD_COLOR;  // Ignore SVG.
    if (FT_Load_Glyph(face, glyphID, flags)) {
        path->reset();
        return false;
    }
    return generateGlyphPathStatic(face, path);
}

#ifdef TT_SUPPORT_COLRV1
bool generateFacePathCOLRv1(FT_Face face, SkGlyphID glyphID, SkPath* path) {
    uint32_t flags = 0;
    flags |= FT_LOAD_BITMAP_METRICS_ONLY;  // Don't decode any bitmaps.
    flags |= FT_LOAD_NO_BITMAP; // Ignore embedded bitmaps.
    flags &= ~FT_LOAD_RENDER;  // Don't scan convert.
    flags &= ~FT_LOAD_COLOR;  // Ignore SVG.
    flags |= FT_LOAD_NO_HINTING;
    flags |= FT_LOAD_NO_AUTOHINT;
    flags |= FT_LOAD_IGNORE_TRANSFORM;

    SkUniqueFTSize unscaledFtSize([face]() -> FT_Size {
        FT_Size size;
        FT_Error err = FT_New_Size(face, &size);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_New_Size(%s) failed in generateFacePathStaticCOLRv1.",
                        face->family_name);
            return nullptr;
        }
        return size;
    }());

    if (!unscaledFtSize) {
      return false;
    }

    FT_Size oldSize = face->size;

    auto tryGeneratePath = [face, &unscaledFtSize, glyphID, flags, path]() {
        FT_Error err = 0;

        err = FT_Activate_Size(unscaledFtSize.get());
        if (err != 0) {
          return false;
        }

        err = FT_Set_Char_Size(face, SkIntToFDot6(face->units_per_EM),
                                     SkIntToFDot6(face->units_per_EM), 72, 72);
        if (err != 0) {
            return false;
        }

        err = FT_Load_Glyph(face, glyphID, flags);
        if (err != 0) {
            path->reset();
            return false;
        }

        if (!generateGlyphPathStatic(face, path)) {
            path->reset();
            return false;
        }

        return true;
    };

    bool pathGenerationResult = tryGeneratePath();

    FT_Activate_Size(oldSize);

    return pathGenerationResult;
}
#endif

}  // namespace

bool SkScalerContextFTUtils::generateGlyphPath(FT_Face face, SkPath* path) const {
    if (!generateGlyphPathStatic(face, path)) {
        return false;
    }
    if (face->glyph->outline.flags & FT_OUTLINE_OVERLAP) {
        Simplify(*path, path);
        // Simplify will return an even-odd path.
        // A stroke+fill (for fake bold) may be incorrect for even-odd.
        // https://github.com/flutter/flutter/issues/112546
        AsWinding(*path, path);
    }
    return true;
}

bool SkScalerContextFTUtils::generateFacePath(FT_Face face, SkGlyphID glyphID, LoadGlyphFlags flags,
                                              SkPath* path) const {
    return generateFacePathStatic(face, glyphID, flags, path);
}

#ifdef TT_SUPPORT_COLRV1
bool SkScalerContextFTUtils::computeColrV1GlyphBoundingBox(FT_Face face, SkGlyphID glyphID,
                                                           SkRect* bounds) {
    SkMatrix ctm;
    *bounds = SkRect::MakeEmpty();
    VisitedSet activePaints;
    return colrv1_start_glyph_bounds(&ctm, bounds, face, glyphID,
                                     FT_COLOR_INCLUDE_ROOT_TRANSFORM, &activePaints);
}
#endif
