/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkPixelRef.h"
#include "SkPM4f.h"
#include "SkPoint3.h"
#include "SkShader.h"
#include "SkSurface.h"
#include "SkTestScalerContext.h"
#include "SkTextBlob.h"

namespace sk_tool_utils {

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
        case kRGBA_F16_SkColorType:     return "RGBA_F16";
    }
    SkASSERT(false);
    return "unexpected colortype";
}

SkColor color_to_565(SkColor color) {
    SkPMColor pmColor = SkPreMultiplyColor(color);
    U16CPU color16 = SkPixel32ToPixel16(pmColor);
    return SkPixel16ToColor(color16);
}

void write_pixels(SkCanvas* canvas, const SkBitmap& bitmap, int x, int y,
                  SkColorType colorType, SkAlphaType alphaType) {
    SkBitmap tmp(bitmap);
    const SkImageInfo info = SkImageInfo::Make(tmp.width(), tmp.height(), colorType, alphaType);

    canvas->writePixels(info, tmp.getPixels(), tmp.rowBytes(), x, y);
}

void write_pixels(SkSurface* surface, const SkBitmap& src, int x, int y,
                  SkColorType colorType, SkAlphaType alphaType) {
    const SkImageInfo info = SkImageInfo::Make(src.width(), src.height(), colorType, alphaType);
    surface->writePixels({info, src.getPixels(), src.rowBytes()}, x, y);
}

sk_sp<SkShader> create_checkerboard_shader(SkColor c1, SkColor c2, int size) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeS32(2 * size, 2 * size, kPremul_SkAlphaType));
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    return SkShader::MakeBitmapShader(
            bm, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode);
}

SkBitmap create_checkerboard_bitmap(int w, int h, SkColor c1, SkColor c2, int checkSize) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    SkCanvas canvas(bitmap);

    sk_tool_utils::draw_checkerboard(&canvas, c1, c2, checkSize);
    return bitmap;
}

void draw_checkerboard(SkCanvas* canvas, SkColor c1, SkColor c2, int size) {
    SkPaint paint;
    paint.setShader(create_checkerboard_shader(c1, c2, size));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
}

SkBitmap create_string_bitmap(int w, int h, SkColor c, int x, int y,
                              int textSize, const char* str) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(w, h);
    SkCanvas canvas(bitmap);

    SkPaint paint;
    paint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setColor(c);
    paint.setTextSize(SkIntToScalar(textSize));

    canvas.clear(0x00000000);
    canvas.drawString(str, SkIntToScalar(x), SkIntToScalar(y), paint);

    // Tag data as sRGB (without doing any color space conversion). Color-space aware configs
    // will process this correctly but legacy configs will render as if this returned N32.
    SkBitmap result;
    result.setInfo(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    result.setPixelRef(sk_ref_sp(bitmap.pixelRef()), 0, 0);
    return result;
}

void add_to_text_blob_w_len(SkTextBlobBuilder* builder, const char* text, size_t len,
                            const SkPaint& origPaint, SkScalar x, SkScalar y) {
    SkPaint paint(origPaint);
    SkTDArray<uint16_t> glyphs;

    glyphs.append(paint.textToGlyphs(text, len, nullptr));
    paint.textToGlyphs(text, len, glyphs.begin());

    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const SkTextBlobBuilder::RunBuffer& run = builder->allocRun(paint, glyphs.count(), x, y,
                                                                nullptr);
    memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
}

void add_to_text_blob(SkTextBlobBuilder* builder, const char* text,
                      const SkPaint& origPaint, SkScalar x, SkScalar y) {
    add_to_text_blob_w_len(builder, text, strlen(text), origPaint, x, y);
}

SkPath make_star(const SkRect& bounds, int numPts, int step) {
    SkPath path;
    path.setFillType(SkPath::kEvenOdd_FillType);
    path.moveTo(0,-1);
    for (int i = 1; i < numPts; ++i) {
        int idx = i*step;
        SkScalar theta = idx * 2*SK_ScalarPI/numPts + SK_ScalarPI/2;
        SkScalar x = SkScalarCos(theta);
        SkScalar y = -SkScalarSin(theta);
        path.lineTo(x, y);
    }
    path.transform(SkMatrix::MakeRectToRect(path.getBounds(), bounds, SkMatrix::kFill_ScaleToFit));
    return path;
}

#if !defined(__clang__) && defined(_MSC_VER)
    // MSVC takes ~2 minutes to compile this function with optimization.
    // We don't really care to wait that long for this function.
    #pragma optimize("", off)
#endif
void make_big_path(SkPath& path) {
    #include "BigPathBench.inc"
}

static float gaussian2d_value(int x, int y, float sigma) {
    // don't bother with the scale term since we're just going to normalize the
    // kernel anyways
    float temp = expf(-(x*x + y*y)/(2*sigma*sigma));
    return temp;
}

static float* create_2d_kernel(float sigma, int* filterSize) {
    // We will actually take 2*halfFilterSize+1 samples (i.e., our filter kernel
    // sizes are always odd)
    int halfFilterSize = SkScalarCeilToInt(6*sigma)/2;
    int wh = *filterSize = 2*halfFilterSize + 1;

    float* temp = new float[wh*wh];

    float filterTot = 0.0f;
    for (int yOff = 0; yOff < wh; ++yOff) {
        for (int xOff = 0; xOff < wh; ++xOff) {
            temp[yOff*wh+xOff] = gaussian2d_value(xOff-halfFilterSize, yOff-halfFilterSize, sigma);

            filterTot += temp[yOff*wh+xOff];
        }
    }

    // normalize the kernel
    for (int yOff = 0; yOff < wh; ++yOff) {
        for (int xOff = 0; xOff < wh; ++xOff) {
            temp[yOff*wh+xOff] /= filterTot;
        }
    }

    return temp;
}

static SkPMColor blur_pixel(const SkBitmap& bm, int x, int y, float* kernel, int wh) {
    SkASSERT(wh & 0x1);

    int halfFilterSize = (wh-1)/2;

    float r = 0.0f, g = 0.0f, b = 0.0f;
    for (int yOff = 0; yOff < wh; ++yOff) {
        int ySamp = y + yOff - halfFilterSize;

        if (ySamp < 0) {
            ySamp = 0;
        } else if (ySamp > bm.height()-1) {
            ySamp = bm.height()-1;
        }

        for (int xOff = 0; xOff < wh; ++xOff) {
            int xSamp = x + xOff - halfFilterSize;

            if (xSamp < 0) {
                xSamp = 0;
            } else if (xSamp > bm.width()-1) {
                xSamp = bm.width()-1;
            }

            float filter = kernel[yOff*wh + xOff];

            SkPMColor c = *bm.getAddr32(xSamp, ySamp);

            r += SkGetPackedR32(c) * filter;
            g += SkGetPackedG32(c) * filter;
            b += SkGetPackedB32(c) * filter;
        }
    }

    U8CPU r8, g8, b8;

    r8 = (U8CPU) (r+0.5f);
    g8 = (U8CPU) (g+0.5f);
    b8 = (U8CPU) (b+0.5f);

    return SkPackARGB32(255, r8, g8, b8);
}

SkBitmap slow_blur(const SkBitmap& src, float sigma) {
    SkBitmap dst;

    dst.allocN32Pixels(src.width(), src.height(), true);

    int wh;
    std::unique_ptr<float[]> kernel(create_2d_kernel(sigma, &wh));

    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.width(); ++x) {
            *dst.getAddr32(x, y) = blur_pixel(src, x, y, kernel.get(), wh);
        }
    }

    return dst;
}

// compute the intersection point between the diagonal and the ellipse in the
// lower right corner
static SkPoint intersection(SkScalar w, SkScalar h) {
    SkASSERT(w > 0.0f || h > 0.0f);

    return SkPoint::Make(w / SK_ScalarSqrt2, h / SK_ScalarSqrt2);
}

// Use the intersection of the corners' diagonals with their ellipses to shrink
// the bounding rect
SkRect compute_central_occluder(const SkRRect& rr) {
    const SkRect r = rr.getBounds();

    SkScalar newL = r.fLeft, newT = r.fTop, newR = r.fRight, newB = r.fBottom;

    SkVector radii = rr.radii(SkRRect::kUpperLeft_Corner);
    if (!radii.isZero()) {
        SkPoint p = intersection(radii.fX, radii.fY);

        newL = SkTMax(newL, r.fLeft + radii.fX - p.fX);
        newT = SkTMax(newT, r.fTop + radii.fY - p.fY);
    }

    radii = rr.radii(SkRRect::kUpperRight_Corner);
    if (!radii.isZero()) {
        SkPoint p = intersection(radii.fX, radii.fY);

        newR = SkTMin(newR, r.fRight + p.fX - radii.fX);
        newT = SkTMax(newT, r.fTop + radii.fY - p.fY);
    }

    radii = rr.radii(SkRRect::kLowerRight_Corner);
    if (!radii.isZero()) {
        SkPoint p = intersection(radii.fX, radii.fY);

        newR = SkTMin(newR, r.fRight + p.fX - radii.fX);
        newB = SkTMin(newB, r.fBottom - radii.fY + p.fY);
    }

    radii = rr.radii(SkRRect::kLowerLeft_Corner);
    if (!radii.isZero()) {
        SkPoint p = intersection(radii.fX, radii.fY);

        newL = SkTMax(newL, r.fLeft + radii.fX - p.fX);
        newB = SkTMin(newB, r.fBottom - radii.fY + p.fY);
    }

    return SkRect::MakeLTRB(newL, newT, newR, newB);
}

// The widest inset rect
SkRect compute_widest_occluder(const SkRRect& rr) {
    const SkRect& r = rr.getBounds();

    const SkVector& ul = rr.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& ur = rr.radii(SkRRect::kUpperRight_Corner);
    const SkVector& lr = rr.radii(SkRRect::kLowerRight_Corner);
    const SkVector& ll = rr.radii(SkRRect::kLowerLeft_Corner);

    SkScalar maxT = SkTMax(ul.fY, ur.fY);
    SkScalar maxB = SkTMax(ll.fY, lr.fY);

    return SkRect::MakeLTRB(r.fLeft, r.fTop + maxT, r.fRight, r.fBottom - maxB);

}

// The tallest inset rect
SkRect compute_tallest_occluder(const SkRRect& rr) {
    const SkRect& r = rr.getBounds();

    const SkVector& ul = rr.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& ur = rr.radii(SkRRect::kUpperRight_Corner);
    const SkVector& lr = rr.radii(SkRRect::kLowerRight_Corner);
    const SkVector& ll = rr.radii(SkRRect::kLowerLeft_Corner);

    SkScalar maxL = SkTMax(ul.fX, ll.fX);
    SkScalar maxR = SkTMax(ur.fX, lr.fX);

    return SkRect::MakeLTRB(r.fLeft + maxL, r.fTop, r.fRight - maxR, r.fBottom);
}

bool copy_to(SkBitmap* dst, SkColorType dstColorType, const SkBitmap& src) {
    SkPixmap srcPM;
    if (!src.peekPixels(&srcPM)) {
        return false;
    }

    SkBitmap tmpDst;
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
    uint8_t* dst8 = (uint8_t*)dst->getPixels();
    const uint32_t* src32 = (const uint32_t*)src.getPixels();

    const int w = src.width();
    const int h = src.height();
    const bool isBGRA = (kBGRA_8888_SkColorType == src.colorType());
    for (int y = 0; y < h; ++y) {
        if (isBGRA) {
            // BGRA
            for (int x = 0; x < w; ++x) {
                uint32_t s = src32[x];
                dst8[x] = SkComputeLuminance((s >> 16) & 0xFF, (s >> 8) & 0xFF, s & 0xFF);
            }
        } else {
            // RGBA
            for (int x = 0; x < w; ++x) {
                uint32_t s = src32[x];
                dst8[x] = SkComputeLuminance(s & 0xFF, (s >> 8) & 0xFF, (s >> 16) & 0xFF);
            }
        }
        src32 = (const uint32_t*)((const char*)src32 + src.rowBytes());
        dst8 += dst->rowBytes();
    }
}

    //////////////////////////////////////////////////////////////////////////////////////////////

    static int scale255(float x) {
        return sk_float_round2int(x * 255);
    }

    static unsigned diff(const SkColorType ct, const void* a, const void* b) {
        int dr = 0,
            dg = 0,
            db = 0,
            da = 0;
        switch (ct) {
            case kRGBA_8888_SkColorType:
            case kBGRA_8888_SkColorType: {
                SkPMColor c0 = *(const SkPMColor*)a;
                SkPMColor c1 = *(const SkPMColor*)b;
                dr = SkGetPackedR32(c0) - SkGetPackedR32(c1);
                dg = SkGetPackedG32(c0) - SkGetPackedG32(c1);
                db = SkGetPackedB32(c0) - SkGetPackedB32(c1);
                da = SkGetPackedA32(c0) - SkGetPackedA32(c1);
            } break;
            case kRGB_565_SkColorType: {
                uint16_t c0 = *(const uint16_t*)a;
                uint16_t c1 = *(const uint16_t*)b;
                dr = SkGetPackedR16(c0) - SkGetPackedR16(c1);
                dg = SkGetPackedG16(c0) - SkGetPackedG16(c1);
                db = SkGetPackedB16(c0) - SkGetPackedB16(c1);
            } break;
            case kARGB_4444_SkColorType: {
                uint16_t c0 = *(const uint16_t*)a;
                uint16_t c1 = *(const uint16_t*)b;
                dr = SkGetPackedR4444(c0) - SkGetPackedR4444(c1);
                dg = SkGetPackedG4444(c0) - SkGetPackedG4444(c1);
                db = SkGetPackedB4444(c0) - SkGetPackedB4444(c1);
                da = SkGetPackedA4444(c0) - SkGetPackedA4444(c1);
            } break;
            case kAlpha_8_SkColorType:
            case kGray_8_SkColorType:
                da = (const uint8_t*)a - (const uint8_t*)b;
                break;
            case kRGBA_F16_SkColorType: {
                const SkPM4f* c0 = (const SkPM4f*)a;
                const SkPM4f* c1 = (const SkPM4f*)b;
                dr = scale255(c0->r() - c1->r());
                dg = scale255(c0->g() - c1->g());
                db = scale255(c0->b() - c1->b());
                da = scale255(c0->a() - c1->a());
            } break;
            default:
                return 0;
        }
        dr = SkAbs32(dr);
        dg = SkAbs32(dg);
        db = SkAbs32(db);
        da = SkAbs32(da);
        return SkMax32(dr, SkMax32(dg, SkMax32(db, da)));
    }

    bool equal_pixels(const SkPixmap& a, const SkPixmap& b, unsigned maxDiff,
                      bool respectColorSpace) {
        if (a.width() != b.width() ||
            a.height() != b.height() ||
            a.colorType() != b.colorType() ||
            (respectColorSpace && (a.colorSpace() != b.colorSpace())))
        {
            return false;
        }

        for (int y = 0; y < a.height(); ++y) {
            const char* aptr = (const char*)a.addr(0, y);
            const char* bptr = (const char*)b.addr(0, y);
            if (memcmp(aptr, bptr, a.width() * a.info().bytesPerPixel())) {
                for (int x = 0; x < a.width(); ++x) {
                    if (diff(a.colorType(), a.addr(x, y), b.addr(x, y)) > maxDiff) {
                        return false;
                    }
                }
            }
            aptr += a.rowBytes();
            bptr += b.rowBytes();
        }
        return true;
    }

    bool equal_pixels(const SkBitmap& bm0, const SkBitmap& bm1, unsigned maxDiff,
                      bool respectColorSpaces) {
        SkPixmap pm0, pm1;
        return bm0.peekPixels(&pm0) && bm1.peekPixels(&pm1) &&
               equal_pixels(pm0, pm1, maxDiff, respectColorSpaces);
    }

    bool equal_pixels(const SkImage* a, const SkImage* b, unsigned maxDiff,
                      bool respectColorSpaces) {
        // ensure that peekPixels will succeed
        auto imga = a->makeRasterImage();
        auto imgb = b->makeRasterImage();
        a = imga.get();
        b = imgb.get();

        SkPixmap pm0, pm1;
        return a->peekPixels(&pm0) && b->peekPixels(&pm1) &&
               equal_pixels(pm0, pm1, maxDiff, respectColorSpaces);
    }

    sk_sp<SkSurface> makeSurface(SkCanvas* canvas, const SkImageInfo& info,
                                 const SkSurfaceProps* props) {
        auto surf = canvas->makeSurface(info, props);
        if (!surf) {
            surf = SkSurface::MakeRaster(info, props);
        }
        return surf;
    }
}  // namespace sk_tool_utils
