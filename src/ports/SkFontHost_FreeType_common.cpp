/*
 * Copyright 2006-2012 The Android Open Source Project
 * Copyright 2012 Mozilla Foundation
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkFDot6.h"
#include "SkFontHost_FreeType_common.h"
#include "SkPath.h"

#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_BITMAP_H
// In the past, FT_GlyphSlot_Own_Bitmap was defined in this header file.
#include FT_SYNTHESIS_H

static FT_Pixel_Mode compute_pixel_mode(SkMask::Format format) {
    switch (format) {
        case SkMask::kBW_Format:
            return FT_PIXEL_MODE_MONO;
        case SkMask::kA8_Format:
        default:
            return FT_PIXEL_MODE_GRAY;
    }
}

///////////////////////////////////////////////////////////////////////////////

static uint16_t packTriple(unsigned r, unsigned g, unsigned b) {
    return SkPackRGB16(r >> 3, g >> 2, b >> 3);
}

static uint16_t grayToRGB16(U8CPU gray) {
    SkASSERT(gray <= 255);
    return SkPackRGB16(gray >> 3, gray >> 2, gray >> 3);
}

static int bittst(const uint8_t data[], int bitOffset) {
    SkASSERT(bitOffset >= 0);
    int lowBit = data[bitOffset >> 3] >> (~bitOffset & 7);
    return lowBit & 1;
}

template<bool APPLY_PREBLEND>
static void copyFT2LCD16(const SkGlyph& glyph, const FT_Bitmap& bitmap,
                         int lcdIsBGR, bool lcdIsVert, const uint8_t* tableR,
                         const uint8_t* tableG, const uint8_t* tableB) {
    if (lcdIsVert) {
        SkASSERT(3 * glyph.fHeight == bitmap.rows);
    } else {
        SkASSERT(glyph.fHeight == bitmap.rows);
    }

    uint16_t* dst = reinterpret_cast<uint16_t*>(glyph.fImage);
    const size_t dstRB = glyph.rowBytes();
    const int width = glyph.fWidth;
    const uint8_t* src = bitmap.buffer;

    switch (bitmap.pixel_mode) {
        case FT_PIXEL_MODE_MONO: {
            for (int y = 0; y < glyph.fHeight; ++y) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = -bittst(src, x);
                }
                dst = (uint16_t*)((char*)dst + dstRB);
                src += bitmap.pitch;
            }
        } break;
        case FT_PIXEL_MODE_GRAY: {
            for (int y = 0; y < glyph.fHeight; ++y) {
                for (int x = 0; x < width; ++x) {
                    dst[x] = grayToRGB16(src[x]);
                }
                dst = (uint16_t*)((char*)dst + dstRB);
                src += bitmap.pitch;
            }
        } break;
        default: {
            SkASSERT(lcdIsVert || (glyph.fWidth * 3 == bitmap.width));
            for (int y = 0; y < glyph.fHeight; y++) {
                if (lcdIsVert) {    // vertical stripes
                    const uint8_t* srcR = src;
                    const uint8_t* srcG = srcR + bitmap.pitch;
                    const uint8_t* srcB = srcG + bitmap.pitch;
                    if (lcdIsBGR) {
                        SkTSwap(srcR, srcB);
                    }
                    for (int x = 0; x < width; x++) {
                        dst[x] = packTriple(sk_apply_lut_if<APPLY_PREBLEND>(*srcR++, tableR),
                                            sk_apply_lut_if<APPLY_PREBLEND>(*srcG++, tableG),
                                            sk_apply_lut_if<APPLY_PREBLEND>(*srcB++, tableB));
                    }
                    src += 3 * bitmap.pitch;
                } else {            // horizontal stripes
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
                }
                dst = (uint16_t*)((char*)dst + dstRB);
            }
        } break;
    }
}

void SkScalerContext_FreeType_Base::generateGlyphImage(FT_Face face, const SkGlyph& glyph) {
    const bool doBGR = SkToBool(fRec.fFlags & SkScalerContext::kLCD_BGROrder_Flag);
    const bool doVert = SkToBool(fRec.fFlags & SkScalerContext::kLCD_Vertical_Flag);

    switch ( face->glyph->format ) {
        case FT_GLYPH_FORMAT_OUTLINE: {
            FT_Outline* outline = &face->glyph->outline;
            FT_BBox     bbox;
            FT_Bitmap   target;

            if (fRec.fFlags & SkScalerContext::kEmbolden_Flag) {
                emboldenOutline(face, outline);
            }

            int dx = 0, dy = 0;
            if (fRec.fFlags & SkScalerContext::kSubpixelPositioning_Flag) {
                dx = SkFixedToFDot6(glyph.getSubXFixed());
                dy = SkFixedToFDot6(glyph.getSubYFixed());
                // negate dy since freetype-y-goes-up and skia-y-goes-down
                dy = -dy;
            }
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

            if (SkMask::kLCD16_Format == glyph.fMaskFormat) {
                FT_Render_Glyph(face->glyph, doVert ? FT_RENDER_MODE_LCD_V : FT_RENDER_MODE_LCD);
                if (fPreBlend.isApplicable()) {
                    copyFT2LCD16<true>(glyph, face->glyph->bitmap, doBGR, doVert,
                                       fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
                } else {
                    copyFT2LCD16<false>(glyph, face->glyph->bitmap, doBGR, doVert,
                                        fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
                }
            } else {
                target.width = glyph.fWidth;
                target.rows = glyph.fHeight;
                target.pitch = glyph.rowBytes();
                target.buffer = reinterpret_cast<uint8_t*>(glyph.fImage);
                target.pixel_mode = compute_pixel_mode(
                                                (SkMask::Format)fRec.fMaskFormat);
                target.num_grays = 256;

                memset(glyph.fImage, 0, glyph.rowBytes() * glyph.fHeight);
                FT_Outline_Get_Bitmap(face->glyph->library, outline, &target);
            }
        } break;

        case FT_GLYPH_FORMAT_BITMAP: {
            if (fRec.fFlags & SkScalerContext::kEmbolden_Flag) {
                FT_GlyphSlot_Own_Bitmap(face->glyph);
                FT_Bitmap_Embolden(face->glyph->library, &face->glyph->bitmap, kBitmapEmboldenStrength, 0);
            }
            SkASSERT_CONTINUE(glyph.fWidth == face->glyph->bitmap.width);
            SkASSERT_CONTINUE(glyph.fHeight == face->glyph->bitmap.rows);
            SkASSERT_CONTINUE(glyph.fTop == -face->glyph->bitmap_top);
            SkASSERT_CONTINUE(glyph.fLeft == face->glyph->bitmap_left);

            const uint8_t*  src = (const uint8_t*)face->glyph->bitmap.buffer;
            uint8_t*        dst = (uint8_t*)glyph.fImage;

            if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY ||
                (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO &&
                 glyph.fMaskFormat == SkMask::kBW_Format)) {
                unsigned    srcRowBytes = face->glyph->bitmap.pitch;
                unsigned    dstRowBytes = glyph.rowBytes();
                unsigned    minRowBytes = SkMin32(srcRowBytes, dstRowBytes);
                unsigned    extraRowBytes = dstRowBytes - minRowBytes;

                for (int y = face->glyph->bitmap.rows - 1; y >= 0; --y) {
                    memcpy(dst, src, minRowBytes);
                    memset(dst + minRowBytes, 0, extraRowBytes);
                    src += srcRowBytes;
                    dst += dstRowBytes;
                }
            } else if (face->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO &&
                       glyph.fMaskFormat == SkMask::kA8_Format) {
                for (int y = 0; y < face->glyph->bitmap.rows; ++y) {
                    uint8_t byte = 0;
                    int bits = 0;
                    const uint8_t* src_row = src;
                    uint8_t* dst_row = dst;

                    for (int x = 0; x < face->glyph->bitmap.width; ++x) {
                        if (!bits) {
                            byte = *src_row++;
                            bits = 8;
                        }

                        *dst_row++ = byte & 0x80 ? 0xff : 0;
                        bits--;
                        byte <<= 1;
                    }

                    src += face->glyph->bitmap.pitch;
                    dst += glyph.rowBytes();
                }
            } else if (SkMask::kLCD16_Format == glyph.fMaskFormat) {
                if (fPreBlend.isApplicable()) {
                    copyFT2LCD16<true>(glyph, face->glyph->bitmap, doBGR, doVert,
                                       fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
                } else {
                    copyFT2LCD16<false>(glyph, face->glyph->bitmap, doBGR, doVert,
                                        fPreBlend.fR, fPreBlend.fG, fPreBlend.fB);
                }
            } else {
                SkDEBUGFAIL("unknown glyph bitmap transform needed");
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

static int move_proc(const FT_Vector* pt, void* ctx) {
    SkPath* path = (SkPath*)ctx;
    path->close();  // to close the previous contour (if any)
    path->moveTo(SkFDot6ToScalar(pt->x), -SkFDot6ToScalar(pt->y));
    return 0;
}

static int line_proc(const FT_Vector* pt, void* ctx) {
    SkPath* path = (SkPath*)ctx;
    path->lineTo(SkFDot6ToScalar(pt->x), -SkFDot6ToScalar(pt->y));
    return 0;
}

static int quad_proc(const FT_Vector* pt0, const FT_Vector* pt1,
                     void* ctx) {
    SkPath* path = (SkPath*)ctx;
    path->quadTo(SkFDot6ToScalar(pt0->x), -SkFDot6ToScalar(pt0->y),
                 SkFDot6ToScalar(pt1->x), -SkFDot6ToScalar(pt1->y));
    return 0;
}

static int cubic_proc(const FT_Vector* pt0, const FT_Vector* pt1,
                      const FT_Vector* pt2, void* ctx) {
    SkPath* path = (SkPath*)ctx;
    path->cubicTo(SkFDot6ToScalar(pt0->x), -SkFDot6ToScalar(pt0->y),
                  SkFDot6ToScalar(pt1->x), -SkFDot6ToScalar(pt1->y),
                  SkFDot6ToScalar(pt2->x), -SkFDot6ToScalar(pt2->y));
    return 0;
}

void SkScalerContext_FreeType_Base::generateGlyphPath(FT_Face face,
                                                      SkPath* path)
{
    if (fRec.fFlags & SkScalerContext::kEmbolden_Flag) {
        emboldenOutline(face, &face->glyph->outline);
    }

    FT_Outline_Funcs    funcs;

    funcs.move_to   = move_proc;
    funcs.line_to   = line_proc;
    funcs.conic_to  = quad_proc;
    funcs.cubic_to  = cubic_proc;
    funcs.shift     = 0;
    funcs.delta     = 0;

    FT_Error err = FT_Outline_Decompose(&face->glyph->outline, &funcs, path);

    if (err != 0) {
        path->reset();
        return;
    }

    path->close();
}

void SkScalerContext_FreeType_Base::emboldenOutline(FT_Face face, FT_Outline* outline)
{
    FT_Pos strength;
    strength = FT_MulFix(face->units_per_EM, face->size->metrics.y_scale)
               / 24;
    FT_Outline_Embolden(outline, strength);
}
