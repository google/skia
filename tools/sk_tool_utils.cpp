/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "sk_tool_utils_flags.h"

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkCommonFlags.h"
#include "SkFontMgr.h"
#include "SkFontStyle.h"
#include "SkPixelRef.h"
#include "SkPoint3.h"
#include "SkShader.h"
#include "SkTestScalerContext.h"
#include "SkTextBlob.h"

DEFINE_bool(portableFonts, false, "Use portable fonts");

namespace sk_tool_utils {

/* these are the default fonts chosen by Chrome for serif, sans-serif, and monospace */
static const char* gStandardFontNames[][3] = {
    { "Times", "Helvetica", "Courier" }, // Mac
    { "Times New Roman", "Helvetica", "Courier" }, // iOS
    { "Times New Roman", "Arial", "Courier New" }, // Win
    { "Times New Roman", "Arial", "Monospace" }, // Ubuntu
    { "serif", "sans-serif", "monospace" }, // Android
    { "Tinos", "Arimo", "Cousine" } // ChromeOS
};

const char* platform_font_name(const char* name) {
    SkString platform = major_platform_os_name();
    int index;
    if (!strcmp(name, "serif")) {
        index = 0;
    } else if (!strcmp(name, "san-serif")) {
        index = 1;
    } else if (!strcmp(name, "monospace")) {
        index = 2;
    } else {
        return name;
    }
    if (platform.equals("Mac")) {
        return gStandardFontNames[0][index];
    }
    if (platform.equals("iOS")) {
        return gStandardFontNames[1][index];
    }
    if (platform.equals("Win")) {
        return gStandardFontNames[2][index];
    }
    if (platform.equals("Ubuntu") || platform.equals("Debian")) {
        return gStandardFontNames[3][index];
    }
    if (platform.equals("Android")) {
        return gStandardFontNames[4][index];
    }
    if (platform.equals("ChromeOS")) {
        return gStandardFontNames[5][index];
    }
    return name;
}

const char* platform_os_emoji() {
    const char* osName = platform_os_name();
    if (!strcmp(osName, "Android") || !strcmp(osName, "Ubuntu") || !strcmp(osName, "Debian")) {
        return "CBDT";
    }
    if (!strncmp(osName, "Mac", 3) || !strncmp(osName, "iOS", 3)) {
        return "SBIX";
    }
    if (!strncmp(osName, "Win", 3)) {
        return "COLR";
    }
    return "";
}

sk_sp<SkTypeface> emoji_typeface() {
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "CBDT")) {
        return MakeResourceAsTypeface("/fonts/Funkster.ttf");
    }
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "SBIX")) {
        return SkTypeface::MakeFromName("Apple Color Emoji", SkFontStyle());
    }
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "COLR")) {
        sk_sp<SkFontMgr> fm(SkFontMgr::RefDefault());
        const char *colorEmojiFontName = "Segoe UI Emoji";
        sk_sp<SkTypeface> typeface(fm->matchFamilyStyle(colorEmojiFontName, SkFontStyle()));
        if (typeface) {
            return typeface;
        }
        sk_sp<SkTypeface> fallback(fm->matchFamilyStyleCharacter(
            colorEmojiFontName, SkFontStyle(), nullptr /* bcp47 */, 0 /* bcp47Count */,
            0x1f4b0 /* character: 💰 */));
        if (fallback) {
            return fallback;
        }
        // If we don't have Segoe UI Emoji and can't find a fallback, try Segoe UI Symbol.
        // Windows 7 does not have Segoe UI Emoji; Segoe UI Symbol has the (non - color) emoji.
        return SkTypeface::MakeFromName("Segoe UI Symbol", SkFontStyle());
    }
    return nullptr;
}

const char* emoji_sample_text() {
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "CBDT")) {
        return "Hamburgefons";
    }
    if (!strcmp(sk_tool_utils::platform_os_emoji(), "SBIX") ||
        !strcmp(sk_tool_utils::platform_os_emoji(), "COLR"))
    {
        return "\xF0\x9F\x92\xB0" "\xF0\x9F\x8F\xA1" "\xF0\x9F\x8E\x85"  // 💰🏡🎅
               "\xF0\x9F\x8D\xAA" "\xF0\x9F\x8D\x95" "\xF0\x9F\x9A\x80"  // 🍪🍕🚀
               "\xF0\x9F\x9A\xBB" "\xF0\x9F\x92\xA9" "\xF0\x9F\x93\xB7" // 🚻💩📷
               "\xF0\x9F\x93\xA6" // 📦
               "\xF0\x9F\x87\xBA" "\xF0\x9F\x87\xB8" "\xF0\x9F\x87\xA6"; // 🇺🇸🇦
    }
    return "";
}

const char* platform_os_name() {
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("os", FLAGS_key[index])) {
            return FLAGS_key[index + 1];
        }
    }
    // when running SampleApp or dm without a --key pair, omit the platform name
    return "";
}

// omit version number in returned value
SkString major_platform_os_name() {
    SkString name;
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("os", FLAGS_key[index])) {
            const char* platform = FLAGS_key[index + 1];
            const char* end = platform;
            while (*end && (*end < '0' || *end > '9')) {
                ++end;
            }
            name.append(platform, end - platform);
            break;
        }
    }
    return name;
}

const char* platform_extra_config(const char* config) {
    for (int index = 0; index < FLAGS_key.count(); index += 2) {
        if (!strcmp("extra_config", FLAGS_key[index]) && !strcmp(config, FLAGS_key[index + 1])) {
            return config;
        }
    }
    return "";
}

const char* colortype_name(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return "Unknown";
        case kAlpha_8_SkColorType:      return "Alpha_8";
        case kARGB_4444_SkColorType:    return "ARGB_4444";
        case kRGB_565_SkColorType:      return "RGB_565";
        case kRGBA_8888_SkColorType:    return "RGBA_8888";
        case kBGRA_8888_SkColorType:    return "BGRA_8888";
        case kRGBA_F16_SkColorType:     return "RGBA_F16";
        default:
            SkASSERT(false);
            return "unexpected colortype";
    }
}

SkColor color_to_565(SkColor color) {
    SkPMColor pmColor = SkPreMultiplyColor(color);
    U16CPU color16 = SkPixel32ToPixel16(pmColor);
    return SkPixel16ToColor(color16);
}

sk_sp<SkTypeface> create_portable_typeface(const char* name, SkFontStyle style) {
    return create_font(name, style);
}

void set_portable_typeface(SkPaint* paint, const char* name, SkFontStyle style) {
    paint->setTypeface(create_font(name, style));
}

void write_pixels(SkCanvas* canvas, const SkBitmap& bitmap, int x, int y,
                  SkColorType colorType, SkAlphaType alphaType) {
    SkBitmap tmp(bitmap);
    const SkImageInfo info = SkImageInfo::Make(tmp.width(), tmp.height(), colorType, alphaType);

    canvas->writePixels(info, tmp.getPixels(), tmp.rowBytes(), x, y);
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

void add_to_text_blob(SkTextBlobBuilder* builder, const char* text, const SkPaint& origPaint,
                      SkScalar x, SkScalar y) {
    SkPaint paint(origPaint);
    SkTDArray<uint16_t> glyphs;

    size_t len = strlen(text);
    glyphs.append(paint.textToGlyphs(text, len, nullptr));
    paint.textToGlyphs(text, len, glyphs.begin());

    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const SkTextBlobBuilder::RunBuffer& run = builder->allocRun(paint, glyphs.count(), x, y,
                                                                nullptr);
    memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
}

static inline void norm_to_rgb(SkBitmap* bm, int x, int y, const SkVector3& norm) {
    SkASSERT(SkScalarNearlyEqual(norm.length(), 1.0f));
    unsigned char r = static_cast<unsigned char>((0.5f * norm.fX + 0.5f) * 255);
    unsigned char g = static_cast<unsigned char>((-0.5f * norm.fY + 0.5f) * 255);
    unsigned char b = static_cast<unsigned char>((0.5f * norm.fZ + 0.5f) * 255);
    *bm->getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
}

void create_hemi_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center = SkPoint::Make(dst.fLeft + (dst.width() / 2.0f),
                                         dst.fTop + (dst.height() / 2.0f));
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
    const SkPoint center = SkPoint::Make(dst.fLeft + (dst.width() / 2.0f),
                                         dst.fTop + (dst.height() / 2.0f));

    SkIRect inner = dst;
    inner.inset(dst.width()/4, dst.height()/4);

    SkPoint3 norm;
    const SkPoint3 left =  SkPoint3::Make(-SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 up =    SkPoint3::Make(0.0f, -SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
    const SkPoint3 right = SkPoint3::Make(SK_ScalarRoot2Over2,  0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 down =  SkPoint3::Make(0.0f,  SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            if (inner.contains(x, y)) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                SkScalar locX = x + 0.5f - center.fX;
                SkScalar locY = y + 0.5f - center.fY;

                if (locX >= 0.0f) {
                    if (locY > 0.0f) {
                        norm = locX >= locY ? right : down;   // LR corner
                    } else {
                        norm = locX > -locY ? right : up;     // UR corner
                    }
                } else {
                    if (locY > 0.0f) {
                        norm = -locX > locY ? left : down;    // LL corner
                    } else {
                        norm = locX > locY ? up : left;       // UL corner
                    }
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_tetra_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center = SkPoint::Make(dst.fLeft + (dst.width() / 2.0f),
                                         dst.fTop + (dst.height() / 2.0f));

    static const SkScalar k1OverRoot3 = 0.5773502692f;

    SkPoint3 norm;
    const SkPoint3 leftUp =  SkPoint3::Make(-k1OverRoot3, -k1OverRoot3, k1OverRoot3);
    const SkPoint3 rightUp = SkPoint3::Make(k1OverRoot3,  -k1OverRoot3, k1OverRoot3);
    const SkPoint3 down =  SkPoint3::Make(0.0f,  SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            SkScalar locX = x + 0.5f - center.fX;
            SkScalar locY = y + 0.5f - center.fY;

            if (locX >= 0.0f) {
                if (locY > 0.0f) {
                    norm = locX >= locY ? rightUp : down;   // LR corner
                } else {
                    norm = rightUp;
                }
            } else {
                if (locY > 0.0f) {
                    norm = -locX > locY ? leftUp : down;    // LL corner
                } else {
                    norm = leftUp;
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

#if defined(_MSC_VER)
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

}  // namespace sk_tool_utils
