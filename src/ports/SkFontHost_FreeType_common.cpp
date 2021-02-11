/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPath.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkColorData.h"
#include "include/private/SkTo.h"
#include "src/core/SkFDot6.h"
#include "src/ports/SkFontHost_FreeType_common.h"

#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#ifdef FT_COLOR_H
#   include FT_COLOR_H
#endif
#include FT_IMAGE_H
#include FT_OUTLINE_H
#include FT_SIZES_H
// In the past, FT_GlyphSlot_Own_Bitmap was defined in this header file.
#include FT_SYNTHESIS_H

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

namespace {

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
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
    r = std::max(r, (U8CPU)0x40);
    g = std::max(g, (U8CPU)0x40);
    b = std::max(b, (U8CPU)0x40);
#endif
    return SkPack888ToRGB16(r, g, b);
}

uint16_t grayToRGB16(U8CPU gray) {
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
    gray = std::max(gray, (U8CPU)0x40);
#endif
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
void copyFT2LCD16(const FT_Bitmap& bitmap, const SkMask& mask, int lcdIsBGR,
                  const uint8_t* tableR, const uint8_t* tableG, const uint8_t* tableB)
{
    SkASSERT(SkMask::kLCD16_Format == mask.fFormat);
    if (FT_PIXEL_MODE_LCD != bitmap.pixel_mode) {
        SkASSERT(mask.fBounds.width() == static_cast<int>(bitmap.width));
    }
    if (FT_PIXEL_MODE_LCD_V != bitmap.pixel_mode) {
        SkASSERT(mask.fBounds.height() == static_cast<int>(bitmap.rows));
    }

    const uint8_t* src = bitmap.buffer;
    uint16_t* dst = reinterpret_cast<uint16_t*>(mask.fImage);
    const size_t dstRB = mask.fRowBytes;

    const int width = mask.fBounds.width();
    const int height = mask.fBounds.height();

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
            SkASSERT(3 * mask.fBounds.width() == static_cast<int>(bitmap.width));
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
            SkASSERT(3 * mask.fBounds.height() == static_cast<int>(bitmap.rows));
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
void copyFTBitmap(const FT_Bitmap& srcFTBitmap, SkMask& dstMask) {
    SkASSERTF(dstMask.fBounds.width() == static_cast<int>(srcFTBitmap.width),
              "dstMask.fBounds.width() = %d\n"
              "static_cast<int>(srcFTBitmap.width) = %d",
              dstMask.fBounds.width(),
              static_cast<int>(srcFTBitmap.width)
    );
    SkASSERTF(dstMask.fBounds.height() == static_cast<int>(srcFTBitmap.rows),
              "dstMask.fBounds.height() = %d\n"
              "static_cast<int>(srcFTBitmap.rows) = %d",
              dstMask.fBounds.height(),
              static_cast<int>(srcFTBitmap.rows)
    );

    const uint8_t* src = reinterpret_cast<const uint8_t*>(srcFTBitmap.buffer);
    const FT_Pixel_Mode srcFormat = static_cast<FT_Pixel_Mode>(srcFTBitmap.pixel_mode);
    // FT_Bitmap::pitch is an int and allowed to be negative.
    const int srcPitch = srcFTBitmap.pitch;
    const size_t srcRowBytes = SkTAbs(srcPitch);

    uint8_t* dst = dstMask.fImage;
    const SkMask::Format dstFormat = static_cast<SkMask::Format>(dstMask.fFormat);
    const size_t dstRowBytes = dstMask.fRowBytes;

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
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
                *(dst_row-1) = SkFourByteInterp256(*(dst_row-1), SK_ColorWHITE, 0x40);
#endif
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

void packA8ToA1(const SkMask& mask, const uint8_t* src, size_t srcRB) {
    const int height = mask.fBounds.height();
    const int width = mask.fBounds.width();
    const int octs = width >> 3;
    const int leftOverBits = width & 7;

    uint8_t* dst = mask.fImage;
    const int dstPad = mask.fRowBytes - SkAlign8(width)/8;
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

bool generateFacePathCOLRv1(FT_Face face, SkGlyphID glyphID, SkPath* path);

inline float SkColrV1AlphaToFloat(uint16_t alpha) { return (alpha / float(1 << 14)); }


inline SkTileMode ToSkTileMode(FT_PaintExtend extend_mode) {
    switch (extend_mode) {
        case FT_COLR_PAINT_EXTEND_REPEAT:
            return SkTileMode::kRepeat;
        case FT_COLR_PAINT_EXTEND_REFLECT:
            return SkTileMode::kMirror;
        default:
            return SkTileMode::kClamp;
    }
}

inline SkBlendMode ToSkBlendMode(FT_Composite_Mode composite) {
    switch (composite) {
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
    // Adjust order to convert from FreeType's FT_Affine23 column major order to SkMatrix row-major
    // order.
    return SkMatrix::MakeAll(
        SkFixedToScalar(affine23.xx),  -SkFixedToScalar(affine23.xy), SkFixedToScalar(affine23.dx),
        -SkFixedToScalar(affine23.yx),  SkFixedToScalar(affine23.yy), -SkFixedToScalar(affine23.dy),
        0,                             0,                             1);
}

inline SkPoint SkVectorProjection(SkPoint a, SkPoint b) {
    SkScalar length = b.length();
    if (!length) return SkPoint();
    SkPoint b_normalized = b;
    b_normalized.normalize();
    b_normalized.scale(SkPoint::DotProduct(a, b) / length);
    return b_normalized;
}

void colrv1_draw_paint(SkCanvas* canvas,
                       const FT_Color* palette,
                       FT_Face face,
                       FT_COLR_Paint colrv1_paint) {
    SkPaint paint;

    switch (colrv1_paint.format) {
        case FT_COLR_PAINTFORMAT_GLYPH: {
            FT_UInt glyphID = colrv1_paint.u.glyph.glyphID;
            SkPath path;
            /* TODO: Currently this call retrieves the path at units_per_em size. If we want to get
             * correct hinting for the scaled size under the transforms at this point in the color
             * glyph graph, we need to extract at least the requested glyph width and height and
             * pass that to the path generation. */
            if (generateFacePathCOLRv1(face, glyphID, &path)) {

#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
              SkPaint highlight_paint;
              highlight_paint.setColor(0x33FF0000);
              canvas->drawRect(path.getBounds(), highlight_paint);
#endif
              canvas->clipPath(path, true /* doAntiAlias */);
            }
            break;
        }
        case FT_COLR_PAINTFORMAT_SOLID: {
            SkColor color =
                    SkColorSetARGB(palette[colrv1_paint.u.solid.color.palette_index].alpha *
                                           SkColrV1AlphaToFloat(colrv1_paint.u.solid.color.alpha),
                                   palette[colrv1_paint.u.solid.color.palette_index].red,
                                   palette[colrv1_paint.u.solid.color.palette_index].green,
                                   palette[colrv1_paint.u.solid.color.palette_index].blue);
            paint.setShader(nullptr);
            paint.setColor(color);
            canvas->drawPaint(paint);
            break;
        }
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT: {
            /* retrieve color stop */

            SkPoint line_positions[2];
            line_positions[0].fX = colrv1_paint.u.linear_gradient.p0.x;
            line_positions[0].fY = -colrv1_paint.u.linear_gradient.p0.y;
            line_positions[1].fX = colrv1_paint.u.linear_gradient.p1.x;
            line_positions[1].fY = -colrv1_paint.u.linear_gradient.p1.y;

            SkPoint& p0 = line_positions[0];
            SkPoint& p1 = line_positions[1];
            SkPoint p2;
            p2.set(colrv1_paint.u.linear_gradient.p2.x,
                   -colrv1_paint.u.linear_gradient.p2.y);

            // Do not draw the gradient of p0p1 is parallel to p0p2.
            if (p1 == p0 || p2 == p0 || !SkPoint::CrossProduct(p1 - p0, p2 - p0)) break;

            // Follow implementation note in nanoemoji:
            // https://github.com/googlefonts/nanoemoji/blob/0ac6e7bb4d8202db692574d8530a9b643f1b3b3c/src/nanoemoji/svg.py#L188
            // to compute a new gradient end point as the orthogonal projection of the vector from p0 to p1 onto a line
            // perpendicular to line p0p2 and passing through p0.
            SkPoint perpendicular_to_p2_p0 = (p2 - p0);
            perpendicular_to_p2_p0 = SkPoint::Make(perpendicular_to_p2_p0.y(), -perpendicular_to_p2_p0.x());
            line_positions[1] = p0 + SkVectorProjection((p1 - p0), perpendicular_to_p2_p0);

            /* populate points */
            const FT_UInt num_color_stops =
                    colrv1_paint.u.linear_gradient.colorline.color_stop_iterator.num_color_stops;
            std::vector<SkScalar> stops;
            std::vector<SkColor> colors;
            stops.resize(num_color_stops);
            colors.resize(num_color_stops);

            FT_ColorStop color_stop;
            while (FT_Get_Colorline_Stops(
                    face, &color_stop,
                    &colrv1_paint.u.linear_gradient.colorline.color_stop_iterator)) {
                FT_UInt index = colrv1_paint.u.linear_gradient.colorline.color_stop_iterator
                                        .current_color_stop -
                                1;
                stops[index] = color_stop.stop_offset / float(1 << 14);
                FT_UInt16& palette_index = color_stop.color.palette_index;
                colors[index] = SkColorSetARGB(
                        palette[palette_index].alpha * SkColrV1AlphaToFloat(color_stop.color.alpha),
                        palette[palette_index].red, palette[palette_index].green,
                        palette[palette_index].blue);
            }

            sk_sp<SkShader> shader(SkGradientShader::MakeLinear(
                    line_positions, colors.data(), stops.data(), num_color_stops,
                    ToSkTileMode(colrv1_paint.u.linear_gradient.colorline.extend)));
            SkASSERT(shader);
            // An opaque color is needed to ensure the gradient's not modulated by alpha.
            paint.setColor(SK_ColorBLACK);
            paint.setShader(shader);


            canvas->drawPaint(paint);
            break;
        }
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT: {
            SkPoint start =
                    SkPoint::Make(colrv1_paint.u.radial_gradient.c0.x,
                                  -colrv1_paint.u.radial_gradient.c0.y);
            SkScalar radius = colrv1_paint.u.radial_gradient.r0;
            SkPoint end = SkPoint::Make(colrv1_paint.u.radial_gradient.c1.x,
                                        -colrv1_paint.u.radial_gradient.c1.y);
            SkScalar end_radius = colrv1_paint.u.radial_gradient.r1;

            const FT_UInt num_color_stops =
                    colrv1_paint.u.radial_gradient.colorline.color_stop_iterator.num_color_stops;
            std::vector<SkScalar> stops;
            std::vector<SkColor> colors;
            stops.resize(num_color_stops);
            colors.resize(num_color_stops);

            FT_ColorStop color_stop;
            while (FT_Get_Colorline_Stops(
                    face, &color_stop,
                    &colrv1_paint.u.radial_gradient.colorline.color_stop_iterator)) {
                FT_UInt index = colrv1_paint.u.radial_gradient.colorline.color_stop_iterator
                                        .current_color_stop -
                                1;
                stops[index] = color_stop.stop_offset / float(1 << 14);
                FT_UInt16& palette_index = color_stop.color.palette_index;
                colors[index] = SkColorSetARGB(
                        palette[palette_index].alpha * SkColrV1AlphaToFloat(color_stop.color.alpha),
                        palette[palette_index].red, palette[palette_index].green,
                        palette[palette_index].blue);
            }

            // An opaque color is needed to ensure the gradient's not modulated by alpha.
            paint.setColor(SK_ColorBLACK);

            paint.setShader(SkGradientShader::MakeTwoPointConical(
                    start, radius, end, end_radius, colors.data(), stops.data(), num_color_stops,
                    ToSkTileMode(colrv1_paint.u.radial_gradient.colorline.extend)));
            canvas->drawPaint(paint);
            break;
        }
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT: {
            SkPoint center = SkPoint::Make(colrv1_paint.u.sweep_gradient.center.x,
                                           -colrv1_paint.u.sweep_gradient.center.y);
            SkScalar startAngle = SkFixedToScalar(colrv1_paint.u.sweep_gradient.start_angle);
            SkScalar endAngle = SkFixedToScalar(colrv1_paint.u.sweep_gradient.end_angle);

            const FT_UInt num_color_stops =
                    colrv1_paint.u.sweep_gradient.colorline.color_stop_iterator.num_color_stops;
            std::vector<SkScalar> stops;
            std::vector<SkColor> colors;
            stops.resize(num_color_stops);
            colors.resize(num_color_stops);

            FT_ColorStop color_stop;
            while (FT_Get_Colorline_Stops(
                    face, &color_stop,
                    &colrv1_paint.u.sweep_gradient.colorline.color_stop_iterator)) {
                FT_UInt index = colrv1_paint.u.sweep_gradient.colorline.color_stop_iterator
                                        .current_color_stop -
                                1;
                stops[index] = color_stop.stop_offset / float(1 << 14);
                FT_UInt16& palette_index = color_stop.color.palette_index;
                colors[index] = SkColorSetARGB(
                        palette[palette_index].alpha * SkColrV1AlphaToFloat(color_stop.color.alpha),
                        palette[palette_index].red, palette[palette_index].green,
                        palette[palette_index].blue);
            }

            // An opaque color is needed to ensure the gradient's not modulated by alpha.
            paint.setColor(SK_ColorBLACK);

            // Prepare angles to be within range for the shader.
            auto clampAngleToRange= [](SkScalar angle) {
              SkScalar clamped_angle = SkScalarMod(angle, 360.f);
              if (clamped_angle < 0)
                return clamped_angle + 360.f;
              return clamped_angle;
            };
            startAngle = clampAngleToRange(startAngle);
            endAngle = clampAngleToRange(endAngle);
            /* TODO: Spec clarifications on which side of the gradient is to be
             * painted, repeat modes, how to handle 0 degrees transition, see
             * https://github.com/googlefonts/colr-gradients-spec/issues/250 */
            if (startAngle >= endAngle)
              endAngle += 360.f;

            // Skia's angles start from the horizontal x-Axis, rotate left 90
            // degrees and then mirror horizontally to correct for Skia angles
            // going clockwise, COLR v1 angles going counterclockwise.
            SkMatrix angle_adjust = SkMatrix::RotateDeg(-90.f, center);
            angle_adjust.postScale(-1, 1, center.x(), center.y());

            paint.setShader(SkGradientShader::MakeSweep(
                    center.x(), center.y(), colors.data(), stops.data(), num_color_stops,
                    SkTileMode::kDecal, startAngle, endAngle, 0, &angle_adjust));
            canvas->drawPaint(paint);
            break;
        }
        case FT_COLR_PAINTFORMAT_TRANSFORMED: {
            SkMatrix transform = ToSkMatrix(colrv1_paint.u.transformed.affine);

            canvas->concat(transform);
            break;
        }
        case FT_COLR_PAINTFORMAT_ROTATE: {
            SkMatrix rotation = SkMatrix::RotateDeg(
                    SkFixedToScalar(colrv1_paint.u.rotate.angle),
                    SkPoint::Make(SkFixedToScalar(colrv1_paint.u.rotate.center_x),
                                  -SkFixedToScalar(colrv1_paint.u.rotate.center_y)));

            canvas->concat(rotation);
            break;
        }
        case FT_COLR_PAINTFORMAT_SKEW: {
            // In the PAINTFORMAT_ROTATE implementation, SkMatrix setRotate
            // snaps to 0 for values very close to 0. Do the same here.

            SkScalar rad_x = SkDegreesToRadians(-SkFixedToFloat(colrv1_paint.u.skew.x_skew_angle));
            float tan_x = SkScalarTan(rad_x);
            tan_x = SkScalarNearlyZero(tan_x) ? 0.0f : tan_x;

            SkScalar rad_y = SkDegreesToRadians(-SkFixedToFloat(colrv1_paint.u.skew.y_skew_angle));
            float tan_y = SkScalarTan(rad_y);
            tan_y = SkScalarNearlyZero(tan_y) ? 0.0f : tan_y;

            SkMatrix translate_to_origin = SkMatrix::Translate(
                    SkFixedToScalar(SkFixedToFloat(colrv1_paint.u.skew.center_x)),
                    SkFixedToScalar(-SkFixedToFloat(colrv1_paint.u.skew.center_y)));

            SkMatrix translate_from_origin;
            SkASSERT(translate_to_origin.invert(&translate_from_origin));

            SkMatrix skew_x = SkMatrix::MakeAll(
                1, tan_x, 0,
                0, 1, 0,
                0, 0, 1);

            SkMatrix skew_y = SkMatrix::MakeAll(
                1, 0, 0,
                tan_y, 1, 0,
                0, 0, 1);

            SkMatrix skew = translate_from_origin.postConcat(skew_x).postConcat(skew_y).postConcat(translate_to_origin);

            canvas->concat(skew);
            break;
        }
        default:
            paint.setShader(nullptr);
            paint.setColor(SK_ColorCYAN);
            break;
    }
}

bool colrv1_start_glyph(SkCanvas* canvas,
                        const FT_Color* palette,
                        FT_Face ft_face,
                        uint16_t glyph_id,
                        FT_Color_Root_Transform root_transform);

bool colrv1_traverse_paint(SkCanvas* canvas,
                           const FT_Color* palette,
                           FT_Face face,
                           FT_OpaquePaint opaque_paint) {
    FT_COLR_Paint paint;
    if (!FT_Get_Paint(face, opaque_paint, &paint)) {
      return false;
    }

    // Keep track of failures to retrieve the FT_COLR_Paint from FreeType in the
    // recursion, cancel recursion when a paint retrieval fails.
    bool traverse_result = true;
    switch (paint.format) {
        case FT_COLR_PAINTFORMAT_COLR_LAYERS: {
            FT_LayerIterator& layer_iterator = paint.u.colr_layers.layer_iterator;
            FT_OpaquePaint opaque_paint_fetch;
            opaque_paint_fetch.p = nullptr;
            while (FT_Get_Paint_Layers(face, &layer_iterator, &opaque_paint_fetch)) {
                colrv1_traverse_paint(canvas, palette, face, opaque_paint_fetch);
            }
            break;
        }
        case FT_COLR_PAINTFORMAT_GLYPH:
            canvas->saveLayer(nullptr, nullptr);
            // Traverse / draw operation will clip layer.
            colrv1_draw_paint(canvas, palette, face, paint);
            traverse_result = colrv1_traverse_paint(canvas, palette, face, paint.u.glyph.paint);
            canvas->restore();
            break;
        case FT_COLR_PAINTFORMAT_COLR_GLYPH:
            traverse_result = colrv1_start_glyph(canvas, palette, face, paint.u.colr_glyph.glyphID,
                                                 FT_COLOR_NO_ROOT_TRANSFORM);
            break;
        case FT_COLR_PAINTFORMAT_TRANSFORMED:
            canvas->saveLayer(nullptr, nullptr);
            // Traverse / draw operation will apply transform.
            colrv1_draw_paint(canvas, palette, face, paint);
            traverse_result =
                    colrv1_traverse_paint(canvas, palette, face, paint.u.transformed.paint);
            canvas->restore();
            break;
      case FT_COLR_PAINTFORMAT_ROTATE:
            canvas->saveLayer(nullptr, nullptr);
            // Traverse / draw operation will apply transform.
            colrv1_draw_paint(canvas, palette, face, paint);
            traverse_result = colrv1_traverse_paint(canvas, palette, face, paint.u.rotate.paint);
            canvas->restore();
            break;
        case FT_COLR_PAINTFORMAT_SKEW:
            canvas->saveLayer(nullptr, nullptr);
            // Traverse / draw operation will apply transform.
            colrv1_draw_paint(canvas, palette, face, paint);
            traverse_result = colrv1_traverse_paint(canvas, palette, face, paint.u.skew.paint);
            canvas->restore();
            break;
        case FT_COLR_PAINTFORMAT_COMPOSITE: {
            traverse_result =
                    colrv1_traverse_paint(canvas, palette, face, paint.u.composite.backdrop_paint);
            SkPaint blend_mode_paint;
            blend_mode_paint.setBlendMode(ToSkBlendMode(paint.u.composite.composite_mode));
            canvas->saveLayer(nullptr, &blend_mode_paint);
            traverse_result =
                    traverse_result &&
                    colrv1_traverse_paint(canvas, palette, face, paint.u.composite.source_paint);
            canvas->restore();
            break;
        }
        case FT_COLR_PAINTFORMAT_SOLID:
        case FT_COLR_PAINTFORMAT_LINEAR_GRADIENT:
        case FT_COLR_PAINTFORMAT_RADIAL_GRADIENT:
        case FT_COLR_PAINTFORMAT_SWEEP_GRADIENT: {
            colrv1_draw_paint(canvas, palette, face, paint);
            break;
        }
        default:
            SkASSERT(false);
            break;
    }
    return traverse_result;
}

bool colrv1_start_glyph(SkCanvas* canvas,
                        const FT_Color* palette,
                        FT_Face ft_face,
                        uint16_t glyph_id,
                        FT_Color_Root_Transform root_transform) {
    FT_OpaquePaint opaque_paint;
    opaque_paint.p = nullptr;
    bool has_colrv1_layers = false;
    if (FT_Get_Color_Glyph_Paint(ft_face, glyph_id, root_transform, &opaque_paint)) {
        has_colrv1_layers = true;
        colrv1_traverse_paint(canvas, palette, ft_face, opaque_paint);
    }
    return has_colrv1_layers;
}
#endif // TT_SUPPORT_COLRV1

}  // namespace

void SkScalerContext_FreeType_Base::generateGlyphImage(
    FT_Face face,
    const SkGlyph& glyph,
    const SkMatrix& bitmapTransform)
{
    const bool doBGR = SkToBool(fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag);
    const bool doVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);

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

            memset(glyph.fImage, 0, glyph.rowBytes() * glyph.fHeight);

#ifdef FT_COLOR_H
            if (SkMask::kARGB32_Format == glyph.fMaskFormat) {
                SkBitmap dstBitmap;
                // TODO: mark this as sRGB when the blits will be sRGB.
                dstBitmap.setInfo(SkImageInfo::Make(glyph.fWidth, glyph.fHeight,
                                                    kN32_SkColorType,
                                                    kPremul_SkAlphaType),
                                                    glyph.rowBytes());
                dstBitmap.setPixels(glyph.fImage);

                // Scale unscaledBitmap into dstBitmap.
                SkCanvas canvas(dstBitmap);
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
                canvas.clear(0x33FF0000);
#else
                canvas.clear(SK_ColorTRANSPARENT);
#endif
                canvas.translate(-glyph.fLeft, -glyph.fTop);

                if (this->isSubpixel()) {
                    canvas.translate(SkFixedToScalar(glyph.getSubXFixed()),
                                     SkFixedToScalar(glyph.getSubYFixed()));
                }

                SkPaint paint;
                paint.setAntiAlias(true);

                FT_Color *palette;
                FT_Error err = FT_Palette_Select(face, 0, &palette);
                if (err) {
                    SK_TRACEFTR(err, "Could not get palette from %s fontFace.", face->family_name);
                    return;
                }

                FT_Bool haveLayers = false;

#ifdef TT_SUPPORT_COLRV1
                // Only attempt to draw COLRv1 glyph is FreeType is new enough
                // to have the COLRv1 additions, as indicated by the
                // TT_SUPPORT_COLRV1 flag defined by the FreeType headers in
                // that case.

                haveLayers = colrv1_start_glyph(&canvas, palette, face, glyph.getGlyphID(),
                                                FT_COLOR_INCLUDE_ROOT_TRANSFORM);
#else
                haveLayers = false;
#endif
                if (!haveLayers) {
                    // If we didn't have colr v1 layers, try v0 layers.
                    FT_LayerIterator layerIterator;
                    layerIterator.p = NULL;
                    FT_UInt layerGlyphIndex = 0;
                    FT_UInt layerColorIndex = 0;
                    while (FT_Get_Color_Glyph_Layer(face, glyph.getGlyphID(), &layerGlyphIndex,
                                                    &layerColorIndex, &layerIterator)) {
                        haveLayers = true;
                        if (layerColorIndex == 0xFFFF) {
                            paint.setColor(SK_ColorBLACK);
                        } else {
                            SkColor color = SkColorSetARGB(palette[layerColorIndex].alpha,
                                                           palette[layerColorIndex].red,
                                                           palette[layerColorIndex].green,
                                                           palette[layerColorIndex].blue);
                            paint.setColor(color);
                        }
                        SkPath path;
                        if (this->generateFacePath(face, layerGlyphIndex, &path)) {
                            canvas.drawPath(path, paint);
                        }
                    }
                }

                if (!haveLayers) {
                    SK_TRACEFTR(err, "Could not get layers (neither v0, nor v1) from %s fontFace.",
                                face->family_name);
                    return;
                }
            } else
#endif
            if (SkMask::kLCD16_Format == glyph.fMaskFormat) {
                FT_Outline_Translate(outline, dx, dy);
                FT_Error err = FT_Render_Glyph(face->glyph, doVert ? FT_RENDER_MODE_LCD_V :
                                                                     FT_RENDER_MODE_LCD);
                if (err) {
                    SK_TRACEFTR(err, "Could not render glyph %x.", face->glyph);
                    return;
                }

                SkMask mask = glyph.mask();
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
                memset(mask.fImage, 0x80, mask.fBounds.height() * mask.fRowBytes);
#endif
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
                    mask.fImage += mask.fRowBytes * (-ftGlyph.bitmap_top - mask.fBounds.fTop);
                    mask.fBounds.fTop = -ftGlyph.bitmap_top;
                }
                if (mask.fBounds.fLeft < ftGlyph.bitmap_left) {
                    mask.fImage += sizeof(uint16_t) * (ftGlyph.bitmap_left - mask.fBounds.fLeft);
                    mask.fBounds.fLeft = ftGlyph.bitmap_left;
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
                    mask.fBounds.fBottom = mask.fBounds.fTop + ftGlyph.bitmap.rows / ftVertScale;
                }
                if (SkToInt(ftGlyph.bitmap.width) < mask.fBounds.width() * ftHoriScale) {
                    mask.fBounds.fRight = mask.fBounds.fLeft + ftGlyph.bitmap.width / ftHoriScale;
                }
                if (fPreBlend.isApplicable()) {
                    copyFT2LCD16<true>(ftGlyph.bitmap, mask, doBGR,
                                       fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
                } else {
                    copyFT2LCD16<false>(ftGlyph.bitmap, mask, doBGR,
                                        fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
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

                target.width = glyph.fWidth;
                target.rows = glyph.fHeight;
                target.pitch = glyph.rowBytes();
                target.buffer = reinterpret_cast<uint8_t*>(glyph.fImage);
                target.pixel_mode = compute_pixel_mode(glyph.fMaskFormat);
                target.num_grays = 256;

                FT_Outline_Get_Bitmap(face->glyph->library, outline, &target);
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
                for (int y = 0; y < glyph.fHeight; ++y) {
                    for (int x = 0; x < glyph.fWidth; ++x) {
                        uint8_t& a = ((uint8_t*)glyph.fImage)[(glyph.rowBytes() * y) + x];
                        a = std::max<uint8_t>(a, 0x20);
                    }
                }
#endif
            }
        } break;

        case FT_GLYPH_FORMAT_BITMAP: {
            FT_Pixel_Mode pixel_mode = static_cast<FT_Pixel_Mode>(face->glyph->bitmap.pixel_mode);
            SkMask::Format maskFormat = static_cast<SkMask::Format>(glyph.fMaskFormat);

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
                SkMask dstMask = glyph.mask();
                copyFTBitmap(face->glyph->bitmap, dstMask);
                break;
            }

            // Otherwise, scale the bitmap.

            // Copy the FT_Bitmap into an SkBitmap (either A8 or ARGB)
            SkBitmap unscaledBitmap;
            // TODO: mark this as sRGB when the blits will be sRGB.
            unscaledBitmap.allocPixels(SkImageInfo::Make(face->glyph->bitmap.width,
                                                         face->glyph->bitmap.rows,
                                                         SkColorType_for_FTPixelMode(pixel_mode),
                                                         kPremul_SkAlphaType));

            SkMask unscaledBitmapAlias;
            unscaledBitmapAlias.fImage = reinterpret_cast<uint8_t*>(unscaledBitmap.getPixels());
            unscaledBitmapAlias.fBounds.setWH(unscaledBitmap.width(), unscaledBitmap.height());
            unscaledBitmapAlias.fRowBytes = unscaledBitmap.rowBytes();
            unscaledBitmapAlias.fFormat = SkMaskFormat_for_SkColorType(unscaledBitmap.colorType());
            copyFTBitmap(face->glyph->bitmap, unscaledBitmapAlias);

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
            dstBitmap.setInfo(SkImageInfo::Make(glyph.fWidth, glyph.fHeight,
                                                SkColorType_for_SkMaskFormat(maskFormat),
                                                kPremul_SkAlphaType),
                              bitmapRowBytes);
            if (SkMask::kBW_Format == maskFormat || SkMask::kLCD16_Format == maskFormat) {
                dstBitmap.allocPixels();
            } else {
                dstBitmap.setPixels(glyph.fImage);
            }

            // Scale unscaledBitmap into dstBitmap.
            SkCanvas canvas(dstBitmap);
#ifdef SK_SHOW_TEXT_BLIT_COVERAGE
            canvas.clear(0x33FF0000);
#else
            canvas.clear(SK_ColorTRANSPARENT);
#endif
            canvas.translate(-glyph.fLeft, -glyph.fTop);
            canvas.concat(bitmapTransform);
            canvas.translate(face->glyph->bitmap_left, -face->glyph->bitmap_top);

            SkSamplingOptions sampling(SkFilterMode::kLinear, SkMipmapMode::kNearest);
            canvas.drawImage(unscaledBitmap.asImage().get(), 0, 0, sampling, nullptr);

            // If the destination is BW or LCD, convert from A8.
            if (SkMask::kBW_Format == maskFormat) {
                // Copy the A8 dstBitmap into the A1 glyph.fImage.
                SkMask dstMask = glyph.mask();
                packA8ToA1(dstMask, dstBitmap.getAddr8(0, 0), dstBitmap.rowBytes());
            } else if (SkMask::kLCD16_Format == maskFormat) {
                // Copy the A8 dstBitmap into the LCD16 glyph.fImage.
                uint8_t* src = dstBitmap.getAddr8(0, 0);
                uint16_t* dst = reinterpret_cast<uint16_t*>(glyph.fImage);
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
            memset(glyph.fImage, 0, glyph.rowBytes() * glyph.fHeight);
            return;
    }

// We used to always do this pre-USE_COLOR_LUMINANCE, but with colorlum,
// it is optional
#if defined(SK_GAMMA_APPLY_TO_A8)
    if (SkMask::kA8_Format == glyph.fMaskFormat && fPreBlend.isApplicable()) {
        uint8_t* SK_RESTRICT dst = (uint8_t*)glyph.fImage;
        unsigned rowBytes = glyph.rowBytes();

        for (int y = glyph.fHeight - 1; y >= 0; --y) {
            for (int x = glyph.fWidth - 1; x >= 0; --x) {
                dst[x] = fPreBlend.fG[dst[x]];
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

    static constexpr const FT_Outline_Funcs Funcs{
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
    FT_Error err = FT_Outline_Decompose(&face->glyph->outline, &SkFTGeometrySink::Funcs, &sink);

    if (err != 0) {
        path->reset();
        return false;
    }

    path->close();
    return true;
}

bool generateFacePathStatic(FT_Face face, SkGlyphID glyphID, SkPath* path) {
    uint32_t flags = 0; //fLoadGlyphFlags;
    flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
    flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)

    FT_Error err = FT_Load_Glyph(face, glyphID, flags);
    if (err != 0) {
        path->reset();
        return false;
    }

    if (!generateGlyphPathStatic(face, path)) {
        path->reset();
        return false;
    }
    return true;
}

#ifdef TT_SUPPORT_COLRV1
bool generateFacePathCOLRv1(FT_Face face, SkGlyphID glyphID, SkPath* path) {
    uint32_t flags = 0;
    flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
    flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)

    flags |= FT_LOAD_IGNORE_TRANSFORM;


    using DoneFTSize = SkFunctionWrapper<decltype(FT_Done_Size), FT_Done_Size>;
    std::unique_ptr<std::remove_pointer_t<FT_Size>, DoneFTSize> unscaledFtSize([face]() -> FT_Size {
        FT_Size size;
        FT_Error err = FT_New_Size(face, &size);
        if (err != 0) {
            SK_TRACEFTR(err, "FT_New_Size(%s) failed in generateFacePathStaticCOLRv1.", face->family_name);
            return nullptr;
        }
        return size;
    }());

    if (!unscaledFtSize) {
      return false;
    }

    FT_Size oldSize = face->size;

    auto try_generate_path = [face, &unscaledFtSize, glyphID, flags, path]() {
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

    bool path_generation_result = try_generate_path();

    FT_Activate_Size(oldSize);

    return path_generation_result;
}
#endif

}  // namespace

bool SkScalerContext_FreeType_Base::generateGlyphPath(FT_Face face, SkPath* path) {
    return generateGlyphPathStatic(face, path);
}

bool SkScalerContext_FreeType_Base::generateFacePath(FT_Face face,
                                                     SkGlyphID glyphID,
                                                     SkPath* path) {
    return generateFacePathStatic(face, glyphID, path);
}
