/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "CommandLineFlags.h"
#include "SkBitmap.h"
#include "SkBlendMode.h"
#include "SkCanvas.h"
#include "SkColorData.h"
#include "SkColorPriv.h"
#include "SkFloatingPoint.h"
#include "SkFontMgrPriv.h"
#include "SkFontPriv.h"
#include "SkImage.h"
#include "SkMatrix.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPixelRef.h"
#include "SkPixmap.h"
#include "SkPoint3.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface_win.h"
#include "TestFontMgr.h"

#include <cmath>
#include <cstring>
#include <memory>

namespace ToolUtils {

const char* alphatype_name(SkAlphaType at) {
    switch (at) {
        case kUnknown_SkAlphaType:  return "Unknown";
        case kOpaque_SkAlphaType:   return "Opaque";
        case kPremul_SkAlphaType:   return "Premul";
        case kUnpremul_SkAlphaType: return "Unpremul";
    }
    SkASSERT(false);
    return "unexpected alphatype";
}

const char* colortype_name(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return "Unknown";
        case kAlpha_8_SkColorType:      return "Alpha_8";
        case kRGB_565_SkColorType:      return "RGB_565";
        case kARGB_4444_SkColorType:    return "ARGB_4444";
        case kRGBA_8888_SkColorType:    return "RGBA_8888";
        case kRGB_888x_SkColorType:     return "RGB_888x";
        case kBGRA_8888_SkColorType:    return "BGRA_8888";
        case kRGBA_1010102_SkColorType: return "RGBA_1010102";
        case kRGB_101010x_SkColorType:  return "RGB_101010x";
        case kGray_8_SkColorType:       return "Gray_8";
        case kRGBA_F16Norm_SkColorType: return "RGBA_F16Norm";
        case kRGBA_F16_SkColorType:     return "RGBA_F16";
        case kRGBA_F32_SkColorType:     return "RGBA_F32";
    }
    SkASSERT(false);
    return "unexpected colortype";
}

const char* colortype_depth(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return "Unknown";
        case kAlpha_8_SkColorType:      return "A8";
        case kRGB_565_SkColorType:      return "565";
        case kARGB_4444_SkColorType:    return "4444";
        case kRGBA_8888_SkColorType:    return "8888";
        case kRGB_888x_SkColorType:     return "888";
        case kBGRA_8888_SkColorType:    return "8888";
        case kRGBA_1010102_SkColorType: return "1010102";
        case kRGB_101010x_SkColorType:  return "101010";
        case kGray_8_SkColorType:       return "G8";
        case kRGBA_F16Norm_SkColorType: return "F16Norm";  // TODO: "F16"?
        case kRGBA_F16_SkColorType:     return "F16";
        case kRGBA_F32_SkColorType:     return "F32";
    }
    SkASSERT(false);
    return "unexpected colortype";
}


SkColor color_to_565(SkColor color) {
    // Not a good idea to use this function for greyscale colors...
    // it will add an obvious purple or green tint.
    SkASSERT(SkColorGetR(color) != SkColorGetG(color) || SkColorGetR(color) != SkColorGetB(color) ||
             SkColorGetG(color) != SkColorGetB(color));

    SkPMColor pmColor = SkPreMultiplyColor(color);
    U16CPU    color16 = SkPixel32ToPixel16(pmColor);
    return SkPixel16ToColor(color16);
}

sk_sp<SkShader> create_checkerboard_shader(SkColor c1, SkColor c2, int size) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeS32(2 * size, 2 * size, kPremul_SkAlphaType));
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    return bm.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat);
}

SkBitmap create_checkerboard_bitmap(int w, int h, SkColor c1, SkColor c2, int checkSize) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    SkCanvas canvas(bitmap);

    ToolUtils::draw_checkerboard(&canvas, c1, c2, checkSize);
    return bitmap;
}

void draw_checkerboard(SkCanvas* canvas, SkColor c1, SkColor c2, int size) {
    SkPaint paint;
    paint.setShader(create_checkerboard_shader(c1, c2, size));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
}

SkBitmap
create_string_bitmap(int w, int h, SkColor c, int x, int y, int textSize, const char* str) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(w, h);
    SkCanvas canvas(bitmap);

    SkPaint paint;
    paint.setColor(c);

    SkFont font(ToolUtils::create_portable_typeface(), textSize);

    canvas.clear(0x00000000);
    canvas.drawSimpleText(str,
                          strlen(str),
                          kUTF8_SkTextEncoding,
                          SkIntToScalar(x),
                          SkIntToScalar(y),
                          font,
                          paint);

    // Tag data as sRGB (without doing any color space conversion). Color-space aware configs
    // will process this correctly but legacy configs will render as if this returned N32.
    SkBitmap result;
    result.setInfo(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    result.setPixelRef(sk_ref_sp(bitmap.pixelRef()), 0, 0);
    return result;
}

void add_to_text_blob_w_len(SkTextBlobBuilder* builder,
                            const char*        text,
                            size_t             len,
                            SkTextEncoding     encoding,
                            const SkFont&      font,
                            SkScalar           x,
                            SkScalar           y) {
    int  count = font.countText(text, len, encoding);
    auto run   = builder->allocRun(font, count, x, y);
    font.textToGlyphs(text, len, encoding, run.glyphs, count);
}

void add_to_text_blob(SkTextBlobBuilder* builder,
                      const char*        text,
                      const SkFont&      font,
                      SkScalar           x,
                      SkScalar           y) {
    add_to_text_blob_w_len(builder, text, strlen(text), kUTF8_SkTextEncoding, font, x, y);
}

void get_text_path(const SkFont&  font,
                   const void*    text,
                   size_t         length,
                   SkTextEncoding encoding,
                   SkPath*        dst,
                   const SkPoint  pos[]) {
    SkAutoToGlyphs        atg(font, text, length, encoding);
    const int             count = atg.count();
    SkAutoTArray<SkPoint> computedPos;
    if (pos == nullptr) {
        computedPos.reset(count);
        font.getPos(atg.glyphs(), count, &computedPos[0]);
        pos = computedPos.get();
    }

    struct Rec {
        SkPath*        fDst;
        const SkPoint* fPos;
    } rec = {dst, pos};
    font.getPaths(atg.glyphs(),
                  atg.count(),
                  [](const SkPath* src, const SkMatrix& mx, void* ctx) {
                      Rec* rec = (Rec*)ctx;
                      if (src) {
                          SkMatrix tmp(mx);
                          tmp.postTranslate(rec->fPos->fX, rec->fPos->fY);
                          rec->fDst->addPath(*src, tmp);
                      }
                      rec->fPos += 1;
                  },
                  &rec);
}

SkPath make_star(const SkRect& bounds, int numPts, int step) {
    SkASSERT(numPts != step);
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0, -1);
    for (int i = 1; i < numPts; ++i) {
        int      idx   = i * step % numPts;
        SkScalar theta = idx * 2 * SK_ScalarPI / numPts + SK_ScalarPI / 2;
        SkScalar x     = SkScalarCos(theta);
        SkScalar y     = -SkScalarSin(theta);
        path.lineTo(x, y);
    }
    path.transform(SkMatrix::MakeRectToRect(path.getBounds(), bounds, SkMatrix::kFill_ScaleToFit));
    return path;
}

static inline void norm_to_rgb(SkBitmap* bm, int x, int y, const SkVector3& norm) {
    SkASSERT(SkScalarNearlyEqual(norm.length(), 1.0f));
    unsigned char r      = static_cast<unsigned char>((0.5f * norm.fX + 0.5f) * 255);
    unsigned char g      = static_cast<unsigned char>((-0.5f * norm.fY + 0.5f) * 255);
    unsigned char b      = static_cast<unsigned char>((0.5f * norm.fZ + 0.5f) * 255);
    *bm->getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
}

void create_hemi_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center =
            SkPoint::Make(dst.fLeft + (dst.width() / 2.0f), dst.fTop + (dst.height() / 2.0f));
    const SkPoint halfSize = SkPoint::Make(dst.width() / 2.0f, dst.height() / 2.0f);

    SkVector3 norm;

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            norm.fX = (x + 0.5f - center.fX) / halfSize.fX;
            norm.fY = (y + 0.5f - center.fY) / halfSize.fY;

            SkScalar tmp = norm.fX * norm.fX + norm.fY * norm.fY;
            if (tmp >= 1.0f) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                norm.fZ = sqrtf(1.0f - tmp);
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_frustum_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center =
            SkPoint::Make(dst.fLeft + (dst.width() / 2.0f), dst.fTop + (dst.height() / 2.0f));

    SkIRect inner = dst;
    inner.inset(dst.width() / 4, dst.height() / 4);

    SkPoint3       norm;
    const SkPoint3 left  = SkPoint3::Make(-SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 up    = SkPoint3::Make(0.0f, -SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
    const SkPoint3 right = SkPoint3::Make(SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 down  = SkPoint3::Make(0.0f, SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            if (inner.contains(x, y)) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                SkScalar locX = x + 0.5f - center.fX;
                SkScalar locY = y + 0.5f - center.fY;

                if (locX >= 0.0f) {
                    if (locY > 0.0f) {
                        norm = locX >= locY ? right : down;  // LR corner
                    } else {
                        norm = locX > -locY ? right : up;  // UR corner
                    }
                } else {
                    if (locY > 0.0f) {
                        norm = -locX > locY ? left : down;  // LL corner
                    } else {
                        norm = locX > locY ? up : left;  // UL corner
                    }
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_tetra_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center =
            SkPoint::Make(dst.fLeft + (dst.width() / 2.0f), dst.fTop + (dst.height() / 2.0f));

    static const SkScalar k1OverRoot3 = 0.5773502692f;

    SkPoint3       norm;
    const SkPoint3 leftUp  = SkPoint3::Make(-k1OverRoot3, -k1OverRoot3, k1OverRoot3);
    const SkPoint3 rightUp = SkPoint3::Make(k1OverRoot3, -k1OverRoot3, k1OverRoot3);
    const SkPoint3 down    = SkPoint3::Make(0.0f, SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            SkScalar locX = x + 0.5f - center.fX;
            SkScalar locY = y + 0.5f - center.fY;

            if (locX >= 0.0f) {
                if (locY > 0.0f) {
                    norm = locX >= locY ? rightUp : down;  // LR corner
                } else {
                    norm = rightUp;
                }
            } else {
                if (locY > 0.0f) {
                    norm = -locX > locY ? leftUp : down;  // LL corner
                } else {
                    norm = leftUp;
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

#if !defined(__clang__) && defined(_MSC_VER)
// MSVC takes ~2 minutes to compile this function with optimization.
// We don't really care to wait that long for this function.
#pragma optimize("", off)
#endif
void make_big_path(SkPath& path) {
#include "BigPathBench.inc"  // IWYU pragma: keep
}

bool copy_to(SkBitmap* dst, SkColorType dstColorType, const SkBitmap& src) {
    SkPixmap srcPM;
    if (!src.peekPixels(&srcPM)) {
        return false;
    }

    SkBitmap    tmpDst;
    SkImageInfo dstInfo = srcPM.info().makeColorType(dstColorType);
    if (!tmpDst.setInfo(dstInfo)) {
        return false;
    }

    if (!tmpDst.tryAllocPixels()) {
        return false;
    }

    SkPixmap dstPM;
    if (!tmpDst.peekPixels(&dstPM)) {
        return false;
    }

    if (!srcPM.readPixels(dstPM)) {
        return false;
    }

    dst->swap(tmpDst);
    return true;
}

void copy_to_g8(SkBitmap* dst, const SkBitmap& src) {
    SkASSERT(kBGRA_8888_SkColorType == src.colorType() ||
             kRGBA_8888_SkColorType == src.colorType());

    SkImageInfo grayInfo = src.info().makeColorType(kGray_8_SkColorType);
    dst->allocPixels(grayInfo);
    uint8_t*        dst8  = (uint8_t*)dst->getPixels();
    const uint32_t* src32 = (const uint32_t*)src.getPixels();

    const int  w      = src.width();
    const int  h      = src.height();
    const bool isBGRA = (kBGRA_8888_SkColorType == src.colorType());
    for (int y = 0; y < h; ++y) {
        if (isBGRA) {
            // BGRA
            for (int x = 0; x < w; ++x) {
                uint32_t s = src32[x];
                dst8[x]    = SkComputeLuminance((s >> 16) & 0xFF, (s >> 8) & 0xFF, s & 0xFF);
            }
        } else {
            // RGBA
            for (int x = 0; x < w; ++x) {
                uint32_t s = src32[x];
                dst8[x]    = SkComputeLuminance(s & 0xFF, (s >> 8) & 0xFF, (s >> 16) & 0xFF);
            }
        }
        src32 = (const uint32_t*)((const char*)src32 + src.rowBytes());
        dst8 += dst->rowBytes();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool equal_pixels(const SkPixmap& a, const SkPixmap& b) {
    if (a.width() != b.width() || a.height() != b.height() || a.colorType() != b.colorType()) {
        return false;
    }

    for (int y = 0; y < a.height(); ++y) {
        const char* aptr = (const char*)a.addr(0, y);
        const char* bptr = (const char*)b.addr(0, y);
        if (memcmp(aptr, bptr, a.width() * a.info().bytesPerPixel())) {
            return false;
        }
        aptr += a.rowBytes();
        bptr += b.rowBytes();
    }
    return true;
}

bool equal_pixels(const SkBitmap& bm0, const SkBitmap& bm1) {
    SkPixmap pm0, pm1;
    return bm0.peekPixels(&pm0) && bm1.peekPixels(&pm1) && equal_pixels(pm0, pm1);
}

bool equal_pixels(const SkImage* a, const SkImage* b) {
    // ensure that peekPixels will succeed
    auto imga = a->makeRasterImage();
    auto imgb = b->makeRasterImage();

    SkPixmap pm0, pm1;
    return imga->peekPixels(&pm0) && imgb->peekPixels(&pm1) && equal_pixels(pm0, pm1);
}

sk_sp<SkSurface> makeSurface(SkCanvas*             canvas,
                             const SkImageInfo&    info,
                             const SkSurfaceProps* props) {
    auto surf = canvas->makeSurface(info, props);
    if (!surf) {
        surf = SkSurface::MakeRaster(info, props);
    }
    return surf;
}

static DEFINE_bool(nativeFonts, true,
                   "If true, use native font manager and rendering. "
                   "If false, fonts will draw as portably as possible.");
#if defined(SK_BUILD_FOR_WIN)
    static DEFINE_bool(gdi, false,
                       "Use GDI instead of DirectWrite for font rendering.");
#endif

void SetDefaultFontMgr() {
    if (!FLAGS_nativeFonts) {
        gSkFontMgr_DefaultFactory = &ToolUtils::MakePortableFontMgr;
    }
#if defined(SK_BUILD_FOR_WIN)
    if (FLAGS_gdi) {
        gSkFontMgr_DefaultFactory = &SkFontMgr_New_GDI;
    }
#endif
}

}  // namespace ToolUtils
