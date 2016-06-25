/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Fuzz.h"
#include "Sk1DPathEffect.h"
#include "Sk2DPathEffect.h"
#include "SkAlphaThresholdFilter.h"
#include "SkArcToPathEffect.h"
#include "SkBlurImageFilter.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorCubeFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkComposeImageFilter.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkData.h"
#include "SkDiscretePathEffect.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkEmbossMaskFilter.h"
#include "SkFlattenableSerialization.h"
#include "SkImageSource.h"
#include "SkLayerRasterizer.h"
#include "SkLightingImageFilter.h"
#include "SkLumaColorFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPaintImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkRandom.h"
#include "SkTableColorFilter.h"
#include "SkTileImageFilter.h"
#include "SkTypeface.h"
#include "SkXfermodeImageFilter.h"
#include <stdio.h>
#include <time.h>

#define SK_ADD_RANDOM_BIT_FLIPS

static Fuzz* fuzz;
static const int kBitmapSize = 24;

static bool return_large = false;
static bool return_undef = false;

static int R(float x) {
    return (int)floor(SkScalarToFloat(fuzz->nextF1()) * x);
}

#if defined _WIN32
#pragma warning ( push )
// we are intentionally causing an overflow here
//      (warning C4756: overflow in constant arithmetic)
#pragma warning ( disable : 4756 )
#endif

static float huge() {
    double d = 1e100;
    float f = (float)d;
    return f;
}

#if defined _WIN32
#pragma warning ( pop )
#endif

static float make_number(bool positiveOnly) {
    float f = positiveOnly ? 1.0f : 0.0f;
    float v = f;
    int sel;

    if (return_large) sel = R(6); else sel = R(4);
    if (!return_undef && sel == 0) sel = 1;

    if (R(2) == 1) v = (float)(R(100)+f); else

    switch (sel) {
        case 0: break;
        case 1: v = f; break;
        case 2: v = 0.000001f; break;
        case 3: v = 10000.0f; break;
        case 4: v = 2000000000.0f; break;
        case 5: v = huge(); break;
    }

    if (!positiveOnly && (R(4) == 1)) v = -v;
    return v;
}

static SkScalar make_scalar(bool positiveOnly = false) {
    return make_number(positiveOnly);
}

static SkString make_string() {
    int length = R(1000);
    SkString str(length);
    for (int i = 0; i < length; ++i) {
        str[i] = static_cast<char>(R(256));
    }
    return str;
}

static SkString make_font_name() {
    int sel = R(8);

    switch(sel) {
        case 0: return SkString("Courier New");
        case 1: return SkString("Helvetica");
        case 2: return SkString("monospace");
        case 3: return SkString("sans-serif");
        case 4: return SkString("serif");
        case 5: return SkString("Times");
        case 6: return SkString("Times New Roman");
        case 7:
        default:
            return make_string();
    }
}

static bool make_bool() {
    return R(2) == 1;
}

static SkRect make_rect() {
    return SkRect::MakeWH(SkIntToScalar(R(static_cast<float>(kBitmapSize))),
                          SkIntToScalar(R(static_cast<float>(kBitmapSize))));
}

static SkRegion make_region() {
    SkIRect iRegion = SkIRect::MakeXYWH(R(static_cast<float>(kBitmapSize)),
                                        R(static_cast<float>(kBitmapSize)),
                                        R(static_cast<float>(kBitmapSize)),
                                        R(static_cast<float>(kBitmapSize)));
    return SkRegion(iRegion);
}

static SkMatrix make_matrix() {
    SkMatrix m;
    for (int i = 0; i < 9; ++i) {
        m[i] = make_scalar();
    }
    return m;
}

static SkXfermode::Mode make_xfermode() {
    return static_cast<SkXfermode::Mode>(R(SkXfermode::kLastMode+1));
}

static SkPaint::Align make_paint_align() {
    return static_cast<SkPaint::Align>(R(SkPaint::kRight_Align+1));
}

static SkPaint::Hinting make_paint_hinting() {
    return static_cast<SkPaint::Hinting>(R(SkPaint::kFull_Hinting+1));
}

static SkPaint::Style make_paint_style() {
    return static_cast<SkPaint::Style>(R(SkPaint::kStrokeAndFill_Style+1));
}

static SkPaint::Cap make_paint_cap() {
    return static_cast<SkPaint::Cap>(R(SkPaint::kDefault_Cap+1));
}

static SkPaint::Join make_paint_join() {
    return static_cast<SkPaint::Join>(R(SkPaint::kDefault_Join+1));
}

static SkPaint::TextEncoding make_paint_text_encoding() {
    return static_cast<SkPaint::TextEncoding>(R(SkPaint::kGlyphID_TextEncoding+1));
}

static SkBlurStyle make_blur_style() {
    return static_cast<SkBlurStyle>(R(kLastEnum_SkBlurStyle+1));
}

static SkBlurMaskFilter::BlurFlags make_blur_mask_filter_flag() {
    return static_cast<SkBlurMaskFilter::BlurFlags>(R(SkBlurMaskFilter::kAll_BlurFlag+1));
}

static SkFilterQuality make_filter_quality() {
    return static_cast<SkFilterQuality>(R(kHigh_SkFilterQuality+1));
}

static SkFontStyle make_typeface_style() {
    return SkFontStyle::FromOldStyle(R(SkTypeface::kBoldItalic+1));
}

static SkPath1DPathEffect::Style make_path_1d_path_effect_style() {
    return static_cast<SkPath1DPathEffect::Style>(R((int)SkPath1DPathEffect::kLastEnum_Style + 1));
}

static SkColor make_color() {
    return (R(2) == 1) ? 0xFFC0F0A0 : 0xFF000090;
}

static SkDropShadowImageFilter::ShadowMode make_shadow_mode() {
    return (R(2) == 1) ? SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode :
                         SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode;
}

static SkPoint3 make_point() {
    return SkPoint3::Make(make_scalar(), make_scalar(), make_scalar(true));
}

static SkDisplacementMapEffect::ChannelSelectorType make_channel_selector_type() {
    return static_cast<SkDisplacementMapEffect::ChannelSelectorType>(R(4)+1);
}

static bool valid_for_raster_canvas(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kAlpha_8_SkColorType:
        case kRGB_565_SkColorType:
            return true;
        case kN32_SkColorType:
            return kPremul_SkAlphaType == info.alphaType() ||
                   kOpaque_SkAlphaType == info.alphaType();
        default:
            break;
    }
    return false;
}

static SkColorType rand_colortype() {
    return (SkColorType)R(kLastEnum_SkColorType + 1);
}

static void rand_bitmap_for_canvas(SkBitmap* bitmap) {
    SkImageInfo info;
    do {
        info = SkImageInfo::Make(kBitmapSize, kBitmapSize, rand_colortype(),
                                 kPremul_SkAlphaType);
    } while (!valid_for_raster_canvas(info) || !bitmap->tryAllocPixels(info));
}

static void make_g_bitmap(SkBitmap& bitmap) {
    rand_bitmap_for_canvas(&bitmap);

    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(0xFF884422);
    paint.setTextSize(SkIntToScalar(kBitmapSize/2));
    const char* str = "g";
    canvas.drawText(str, strlen(str), SkIntToScalar(kBitmapSize/8),
                    SkIntToScalar(kBitmapSize/4), paint);
}

static void make_checkerboard_bitmap(SkBitmap& bitmap) {
    rand_bitmap_for_canvas(&bitmap);

    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    const int i = kBitmapSize / 8;
    const SkScalar f = SkIntToScalar(i);
    for (int y = 0; y < kBitmapSize; y += i) {
        for (int x = 0; x < kBitmapSize; x += i) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, f, f), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(f, 0, f, f), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, f, f, f), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(f, f, f, f), darkPaint);
            canvas.restore();
        }
    }
}

static const SkBitmap& make_bitmap() {
    static SkBitmap bitmap[2];
    static bool initialized = false;
    if (!initialized) {
        make_g_bitmap(bitmap[0]);
        make_checkerboard_bitmap(bitmap[1]);
        initialized = true;
    }
    return bitmap[R(2)];
}

static sk_sp<SkData> make_3Dlut(int* cubeDimension, bool invR, bool invG, bool invB) {
    int size = 4 << R(5);
    auto data = SkData::MakeUninitialized(sizeof(SkColor) * size * size * size);
    SkColor* pixels = (SkColor*)(data->writable_data());
    SkAutoTMalloc<uint8_t> lutMemory(size);
    SkAutoTMalloc<uint8_t> invLutMemory(size);
    uint8_t* lut = lutMemory.get();
    uint8_t* invLut = invLutMemory.get();
    const int maxIndex = size - 1;
    for (int i = 0; i < size; i++) {
        lut[i] = (i * 255) / maxIndex;
        invLut[i] = ((maxIndex - i) * 255) / maxIndex;
    }
    for (int r = 0; r < size; ++r) {
        for (int g = 0; g < size; ++g) {
            for (int b = 0; b < size; ++b) {
                pixels[(size * ((size * b) + g)) + r] = SkColorSetARGB(0xFF,
                        invR ? invLut[r] : lut[r],
                        invG ? invLut[g] : lut[g],
                        invB ? invLut[b] : lut[b]);
            }
        }
    }
    if (cubeDimension) {
        *cubeDimension = size;
    }
    return data;
}

static void drawSomething(SkCanvas* canvas) {
    SkPaint paint;

    canvas->save();
    canvas->scale(0.5f, 0.5f);
    canvas->drawBitmap(make_bitmap(), 0, 0, nullptr);
    canvas->restore();

    paint.setAntiAlias(true);

    paint.setColor(SK_ColorRED);
    canvas->drawCircle(SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/3), paint);
    paint.setColor(SK_ColorBLACK);
    paint.setTextSize(SkIntToScalar(kBitmapSize/3));
    canvas->drawText("Picture", 7, SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/4), paint);
}

static void rand_color_table(uint8_t* table) {
    for (int i = 0; i < 256; ++i) {
        table[i] = R(256);
    }
}

static sk_sp<SkColorFilter> make_color_filter() {
    switch (R(6)) {
        case 0: {
            SkScalar array[20];
            for (int i = 0; i < 20; ++i) {
                array[i] = make_scalar();
            }
            return SkColorFilter::MakeMatrixFilterRowMajor255(array);
        }
        case 1:
            return SkLumaColorFilter::Make();
        case 2: {
            uint8_t tableA[256];
            uint8_t tableR[256];
            uint8_t tableG[256];
            uint8_t tableB[256];
            rand_color_table(tableA);
            rand_color_table(tableR);
            rand_color_table(tableG);
            rand_color_table(tableB);
            return SkTableColorFilter::MakeARGB(tableA, tableR, tableG, tableB);
        }
        case 3:
            return SkColorFilter::MakeModeFilter(make_color(), make_xfermode());
        case 4:
            return SkColorMatrixFilter::MakeLightingFilter(make_color(), make_color());
        case 5:
        default:
            break;
    }
    return nullptr;
}

static SkPath make_path() {
    SkPath path;
    int numOps = R(30);
    for (int i = 0; i < numOps; ++i) {
        switch (R(6)) {
            case 0:
                path.moveTo(make_scalar(), make_scalar());
                break;
            case 1:
                path.lineTo(make_scalar(), make_scalar());
                break;
            case 2:
                path.quadTo(make_scalar(), make_scalar(), make_scalar(), make_scalar());
                break;
            case 3:
                path.conicTo(make_scalar(), make_scalar(), make_scalar(), make_scalar(), make_scalar());
                break;
            case 4:
                path.cubicTo(make_scalar(), make_scalar(), make_scalar(),
                             make_scalar(), make_scalar(), make_scalar());
                break;
            case 5:
            default:
                path.arcTo(make_scalar(), make_scalar(), make_scalar(), make_scalar(), make_scalar());
                break;

        }
    }
    path.close();
    return path;
}

static sk_sp<SkPathEffect> make_path_effect(bool canBeNull = true) {
    sk_sp<SkPathEffect> pathEffect;
    if (canBeNull && (R(3) == 1)) { return pathEffect; }

    switch (R(9)) {
        case 0:
            pathEffect = SkArcToPathEffect::Make(make_scalar(true));
            break;
        case 1:
            pathEffect = SkComposePathEffect::Make(make_path_effect(false),
                                                   make_path_effect(false));
            break;
        case 2:
            pathEffect = SkCornerPathEffect::Make(make_scalar());
            break;
        case 3: {
            int count = R(10);
            SkScalar intervals[10];
            for (int i = 0; i < count; ++i) {
                intervals[i] = make_scalar();
            }
            pathEffect = SkDashPathEffect::Make(intervals, count, make_scalar());
            break;
        }
        case 4:
            pathEffect = SkDiscretePathEffect::Make(make_scalar(), make_scalar());
            break;
        case 5:
            pathEffect = SkPath1DPathEffect::Make(make_path(), make_scalar(), make_scalar(),
                                                  make_path_1d_path_effect_style());
            break;
        case 6:
            pathEffect = SkLine2DPathEffect::Make(make_scalar(), make_matrix());
            break;
        case 7:
            pathEffect = SkPath2DPathEffect::Make(make_matrix(), make_path());
            break;
        case 8:
        default:
            pathEffect = SkSumPathEffect::Make(make_path_effect(false),
                                               make_path_effect(false));
            break;
    }
    return pathEffect;
}

static sk_sp<SkMaskFilter> make_mask_filter() {
    sk_sp<SkMaskFilter> maskFilter;
    switch (R(3)) {
        case 0:
            maskFilter = SkBlurMaskFilter::Make(make_blur_style(), make_scalar(),
                                                make_blur_mask_filter_flag());
        case 1: {
            SkEmbossMaskFilter::Light light;
            for (int i = 0; i < 3; ++i) {
                light.fDirection[i] = make_scalar();
            }
            light.fPad = R(65536);
            light.fAmbient = R(256);
            light.fSpecular = R(256);
            maskFilter = SkEmbossMaskFilter::Make(make_scalar(), light);
        }
        case 2:
        default:
            break;
    }
    return maskFilter;
}

static sk_sp<SkImageFilter> make_image_filter(bool canBeNull = true);

static SkPaint make_paint() {
    SkPaint paint;
    paint.setHinting(make_paint_hinting());
    paint.setAntiAlias(make_bool());
    paint.setDither(make_bool());
    paint.setLinearText(make_bool());
    paint.setSubpixelText(make_bool());
    paint.setLCDRenderText(make_bool());
    paint.setEmbeddedBitmapText(make_bool());
    paint.setAutohinted(make_bool());
    paint.setVerticalText(make_bool());
    paint.setUnderlineText(make_bool());
    paint.setStrikeThruText(make_bool());
    paint.setFakeBoldText(make_bool());
    paint.setDevKernText(make_bool());
    paint.setFilterQuality(make_filter_quality());
    paint.setStyle(make_paint_style());
    paint.setColor(make_color());
    paint.setStrokeWidth(make_scalar());
    paint.setStrokeMiter(make_scalar());
    paint.setStrokeCap(make_paint_cap());
    paint.setStrokeJoin(make_paint_join());
    paint.setColorFilter(make_color_filter());
    paint.setXfermodeMode(make_xfermode());
    paint.setPathEffect(make_path_effect());
    paint.setMaskFilter(make_mask_filter());

    if (false) {
        // our validating buffer does not support typefaces yet, so skip this for now
        paint.setTypeface(SkTypeface::MakeFromName(make_font_name().c_str(),make_typeface_style()));
    }

    SkLayerRasterizer::Builder rasterizerBuilder;
    SkPaint paintForRasterizer;
    if (R(2) == 1) {
        paintForRasterizer = make_paint();
    }
    rasterizerBuilder.addLayer(paintForRasterizer);
    paint.setRasterizer(rasterizerBuilder.detach());
    paint.setImageFilter(make_image_filter());
    sk_sp<SkData> data(make_3Dlut(nullptr, make_bool(), make_bool(), make_bool()));
    paint.setTextAlign(make_paint_align());
    paint.setTextSize(make_scalar());
    paint.setTextScaleX(make_scalar());
    paint.setTextSkewX(make_scalar());
    paint.setTextEncoding(make_paint_text_encoding());
    return paint;
}

static sk_sp<SkImageFilter> make_image_filter(bool canBeNull) {
    sk_sp<SkImageFilter> filter;

    // Add a 1 in 3 chance to get a nullptr input
    if (canBeNull && (R(3) == 1)) {
        return filter;
    }

    enum { ALPHA_THRESHOLD, MERGE, COLOR, LUT3D, BLUR, MAGNIFIER,
           XFERMODE, OFFSET, MATRIX, MATRIX_CONVOLUTION, COMPOSE,
           DISTANT_LIGHT, POINT_LIGHT, SPOT_LIGHT, NOISE, DROP_SHADOW,
           MORPHOLOGY, BITMAP, DISPLACE, TILE, PICTURE, PAINT, NUM_FILTERS };

    switch (R(NUM_FILTERS)) {
    case ALPHA_THRESHOLD:
        filter = SkAlphaThresholdFilter::Make(make_region(),
                                              make_scalar(),
                                              make_scalar(),
                                              make_image_filter());
        break;
    case MERGE:
        filter = SkMergeImageFilter::Make(make_image_filter(),
                                          make_image_filter(),
                                          make_xfermode());
        break;
    case COLOR: {
        sk_sp<SkColorFilter> cf(make_color_filter());
        filter = cf ? SkColorFilterImageFilter::Make(std::move(cf), make_image_filter())
                    : nullptr;
        break;
    }
    case LUT3D: {
        int cubeDimension;
        sk_sp<SkData> lut3D(make_3Dlut(&cubeDimension, (R(2) == 1), (R(2) == 1), (R(2) == 1)));
        sk_sp<SkColorFilter> cf(SkColorCubeFilter::Make(std::move(lut3D), cubeDimension));
        filter = cf ? SkColorFilterImageFilter::Make(std::move(cf), make_image_filter())
                    : nullptr;
        break;
    }
    case BLUR:
        filter = SkBlurImageFilter::Make(make_scalar(true),
                                         make_scalar(true),
                                         make_image_filter());
        break;
    case MAGNIFIER:
        filter = SkMagnifierImageFilter::Make(make_rect(),
                                              make_scalar(true),
                                              make_image_filter());
        break;
    case XFERMODE:
        filter = SkXfermodeImageFilter::Make(SkXfermode::Make(make_xfermode()),
                                             make_image_filter(),
                                             make_image_filter(),
                                             nullptr);
        break;
    case OFFSET:
        filter = SkOffsetImageFilter::Make(make_scalar(), make_scalar(), make_image_filter());
        break;
    case MATRIX:
        filter = SkImageFilter::MakeMatrixFilter(make_matrix(),
                                                 (SkFilterQuality)R(4),
                                                 make_image_filter());
        break;
    case MATRIX_CONVOLUTION: {
        SkImageFilter::CropRect cropR(SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                                     SkIntToScalar(kBitmapSize)));
        SkISize size = SkISize::Make(R(10)+1, R(10)+1);
        int arraySize = size.width() * size.height();
        SkTArray<SkScalar> kernel(arraySize);
        for (int i = 0; i < arraySize; ++i) {
            kernel.push_back() = make_scalar();
        }
        SkIPoint kernelOffset = SkIPoint::Make(R(SkIntToScalar(size.width())),
                                               R(SkIntToScalar(size.height())));

        filter = SkMatrixConvolutionImageFilter::Make(size,
                                                      kernel.begin(),
                                                      make_scalar(),
                                                      make_scalar(),
                                                      kernelOffset,
                                                      (SkMatrixConvolutionImageFilter::TileMode)R(3),
                                                      R(2) == 1,
                                                      make_image_filter(),
                                                      &cropR);
        break;
    }
    case COMPOSE:
        filter = SkComposeImageFilter::Make(make_image_filter(), make_image_filter());
        break;
    case DISTANT_LIGHT:
        filter = (R(2) == 1)
                 ? SkLightingImageFilter::MakeDistantLitDiffuse(make_point(), make_color(),
                                                                make_scalar(), make_scalar(),
                                                                make_image_filter())
                 : SkLightingImageFilter::MakeDistantLitSpecular(make_point(), make_color(),
                                                                 make_scalar(), make_scalar(),
                                                                 SkIntToScalar(R(10)),
                                                                 make_image_filter());
        break;
    case POINT_LIGHT:
        filter = (R(2) == 1)
                 ? SkLightingImageFilter::MakePointLitDiffuse(make_point(), make_color(),
                                                              make_scalar(), make_scalar(),
                                                              make_image_filter())
                 : SkLightingImageFilter::MakePointLitSpecular(make_point(), make_color(),
                                                               make_scalar(), make_scalar(),
                                                               SkIntToScalar(R(10)),
                                                               make_image_filter());
        break;
    case SPOT_LIGHT:
        filter = (R(2) == 1)
                 ? SkLightingImageFilter::MakeSpotLitDiffuse(SkPoint3::Make(0, 0, 0),
                                                             make_point(), make_scalar(),
                                                             make_scalar(), make_color(),
                                                             make_scalar(), make_scalar(),
                                                             make_image_filter())
                 : SkLightingImageFilter::MakeSpotLitSpecular(SkPoint3::Make(0, 0, 0),
                                                              make_point(), make_scalar(),
                                                              make_scalar(), make_color(),
                                                              make_scalar(), make_scalar(),
                                                              SkIntToScalar(R(10)),
                                                              make_image_filter());
        break;
    case NOISE: {
        sk_sp<SkShader> shader((R(2) == 1)
                ? SkPerlinNoiseShader::MakeFractalNoise(make_scalar(true), make_scalar(true),
                                                        R(10.0f), make_scalar())
                : SkPerlinNoiseShader::MakeTurbulence(make_scalar(true), make_scalar(true),
                                                      R(10.0f), make_scalar()));
        SkPaint paint;
        paint.setShader(shader);
        SkImageFilter::CropRect cropR(SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                                     SkIntToScalar(kBitmapSize)));
        filter = SkPaintImageFilter::Make(paint, &cropR);
        break;
    }
    case DROP_SHADOW:
        filter = SkDropShadowImageFilter::Make(make_scalar(),
                                               make_scalar(),
                                               make_scalar(true),
                                               make_scalar(true),
                                               make_color(),
                                               make_shadow_mode(),
                                               make_image_filter(),
                                               nullptr);
        break;
    case MORPHOLOGY:
        if (R(2) == 1) {
            filter = SkDilateImageFilter::Make(R(static_cast<float>(kBitmapSize)),
                                               R(static_cast<float>(kBitmapSize)),
                                               make_image_filter());
        } else {
            filter = SkErodeImageFilter::Make(R(static_cast<float>(kBitmapSize)),
                                              R(static_cast<float>(kBitmapSize)),
                                              make_image_filter());
        }
        break;
    case BITMAP: {
        sk_sp<SkImage> image(SkImage::MakeFromBitmap(make_bitmap()));
        if (R(2) == 1) {
            filter = SkImageSource::Make(std::move(image),
                                         make_rect(),
                                         make_rect(),
                                         kHigh_SkFilterQuality);
        } else {
            filter = SkImageSource::Make(std::move(image));
        }
        break;
    }
    case DISPLACE:
        filter = SkDisplacementMapEffect::Make(make_channel_selector_type(),
                                               make_channel_selector_type(),
                                               make_scalar(),
                                               make_image_filter(false),
                                               make_image_filter());
        break;
    case TILE:
        filter = SkTileImageFilter::Make(make_rect(), make_rect(), make_image_filter(false));
        break;
    case PICTURE: {
        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(kBitmapSize),
                                                            SkIntToScalar(kBitmapSize),
                                                            &factory, 0);
        drawSomething(recordingCanvas);
        sk_sp<SkPicture> pict(recorder.finishRecordingAsPicture());
        filter = SkPictureImageFilter::Make(pict, make_rect());
        break;
    }
    case PAINT: {
        SkImageFilter::CropRect cropR(make_rect());
        filter = SkPaintImageFilter::Make(make_paint(), &cropR);
        break;
    }
    default:
        break;
    }
    return (filter || canBeNull) ? filter : make_image_filter(canBeNull);
}

static SkImageFilter* make_serialized_image_filter() {
    sk_sp<SkImageFilter> filter(make_image_filter(false));
    SkAutoTUnref<SkData> data(SkValidatingSerializeFlattenable(filter.get()));
    const unsigned char* ptr = static_cast<const unsigned char*>(data->data());
    size_t len = data->size();
#ifdef SK_ADD_RANDOM_BIT_FLIPS
    unsigned char* p = const_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < len; ++i, ++p) {
        if (R(250) == 1) { // 0.4% of the time, flip a bit or byte
            if (R(10) == 1) { // Then 10% of the time, change a whole byte
                switch(R(3)) {
                case 0:
                    *p ^= 0xFF; // Flip entire byte
                    break;
                case 1:
                    *p = 0xFF; // Set all bits to 1
                    break;
                case 2:
                    *p = 0x00; // Set all bits to 0
                    break;
                }
            } else {
                *p ^= (1 << R(8));
            }
        }
    }
#endif // SK_ADD_RANDOM_BIT_FLIPS
    SkFlattenable* flattenable = SkValidatingDeserializeFlattenable(ptr, len,
                                    SkImageFilter::GetFlattenableType());
    return static_cast<SkImageFilter*>(flattenable);
}

static void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
        SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)));
    canvas->drawBitmap(make_bitmap(), SkIntToScalar(x), SkIntToScalar(y), &paint);
    canvas->restore();
}

DEF_FUZZ(SerializedImageFilter, f) {
    fuzz = f;
    SkImageFilter* filter = make_serialized_image_filter();

    SkPaint paint;
    SkSafeUnref(paint.setImageFilter(filter));
    SkBitmap bitmap;
    SkCanvas canvas(bitmap);
    drawClippedBitmap(&canvas, 0, 0, paint);
}
