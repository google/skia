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
#include "SkRegion.h"
#include "SkTableColorFilter.h"
#include "SkTileImageFilter.h"
#include "SkTypeface.h"
#include "SkXfermodeImageFilter.h"
#include <cmath>
#include <stdio.h>
#include <time.h>

#define SK_ADD_RANDOM_BIT_FLIPS

static Fuzz* fuzz;
static const int kBitmapSize = 24;


// There should be no more than one make_* used as a function argument.
static bool make_bool() {
    bool b; fuzz->next(&b);
    return b;
}

static float make_number(bool positiveOnly) {
    float f;
    fuzz->next(&f);
    if (positiveOnly) {
        return std::abs(f);
    }
    return f;
}

static SkString make_string() {
    int length;
    fuzz->nextRange(&length, 0, 1000);
    SkString str(length);
    for (int i = 0; i < length; ++i) {
        char c;
        fuzz->nextRange(&c, 0, 255);
        str[i] = c;
    }
    return str;
}

static SkString make_font_name() {
    int sel;
    fuzz->nextRange(&sel, 0, 7);

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

static SkRect make_rect() {
    SkScalar w, h;
    fuzz->nextRange(&w, 0.0f, (float) kBitmapSize-1);
    fuzz->nextRange(&h, 0.0f, (float) kBitmapSize-1);
    return SkRect::MakeWH(w, h);
}

static SkRegion make_region() {
    int32_t x, y, w, h;
    fuzz->nextRange(&x, 0, kBitmapSize-1);
    fuzz->nextRange(&y, 0, kBitmapSize-1);
    fuzz->nextRange(&w, 0, kBitmapSize-1);
    fuzz->nextRange(&h, 0, kBitmapSize-1);
    SkIRect iRegion = SkIRect::MakeXYWH(x,y,w,h);
    return SkRegion(iRegion);
}

static void init_matrix(SkMatrix* m) {
    SkScalar mat[9];
    fuzz->nextN(mat, 9);
    m->set9(mat);
}

static SkBlendMode make_blendmode() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkBlendMode::kLastMode);
    return static_cast<SkBlendMode>(i);
}

static SkPaint::Align make_paint_align() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPaint::kRight_Align);
    return static_cast<SkPaint::Align>(i);
}

static SkPaint::Hinting make_paint_hinting() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPaint::kFull_Hinting);
    return static_cast<SkPaint::Hinting>(i);
}

static SkPaint::Style make_paint_style() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPaint::kStrokeAndFill_Style);
    return static_cast<SkPaint::Style>(i);
}

static SkPaint::Cap make_paint_cap() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPaint::kDefault_Cap);
    return static_cast<SkPaint::Cap>(i);
}

static SkPaint::Join make_paint_join() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPaint::kDefault_Join);
    return static_cast<SkPaint::Join>(i);
}

static SkPaint::TextEncoding make_paint_text_encoding() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPaint::kGlyphID_TextEncoding);
    return static_cast<SkPaint::TextEncoding>(i);
}

static SkBlurStyle make_blur_style() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)kLastEnum_SkBlurStyle);
    return static_cast<SkBlurStyle>(i);
}

static SkBlurMaskFilter::BlurFlags make_blur_mask_filter_flag() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkBlurMaskFilter::kAll_BlurFlag);
    return static_cast<SkBlurMaskFilter::BlurFlags>(i);
}

static SkFilterQuality make_filter_quality() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)kHigh_SkFilterQuality);
    return static_cast<SkFilterQuality>(i);
}

static SkFontStyle make_typeface_style() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkTypeface::kBoldItalic);
    return SkFontStyle::FromOldStyle(i);
}

static SkPath1DPathEffect::Style make_path_1d_path_effect_style() {
    uint8_t i;
    fuzz->nextRange(&i, 0, (uint8_t)SkPath1DPathEffect::kLastEnum_Style);
    return static_cast<SkPath1DPathEffect::Style>(i);
}

static SkColor make_color() {
    return make_bool() ? 0xFFC0F0A0 : 0xFF000090;
}

static SkDropShadowImageFilter::ShadowMode make_shadow_mode() {
    return make_bool() ? SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode :
                        SkDropShadowImageFilter::kDrawShadowOnly_ShadowMode;
}

static SkPoint3 make_point() {
    SkScalar a, b, c;
    fuzz->next(&a, &b, &c);
    c = std::abs(c);
    return SkPoint3::Make(a, b, c);
}

static SkDisplacementMapEffect::ChannelSelectorType make_channel_selector_type() {
    uint8_t i;
    fuzz->nextRange(&i, 1, (uint8_t)SkDisplacementMapEffect::kA_ChannelSelectorType);
    return static_cast<SkDisplacementMapEffect::ChannelSelectorType>(i);
}

static SkColorType rand_colortype() {
    uint8_t i;
    fuzz->nextRange(&i, 0, kLastEnum_SkColorType);
    return (SkColorType) i;
}

static void rand_bitmap_for_canvas(SkBitmap* bitmap) {
    SkImageInfo info = SkImageInfo::Make(kBitmapSize, kBitmapSize, rand_colortype(),
                                 kPremul_SkAlphaType);
    if (!bitmap->tryAllocPixels(info)){
        SkDebugf("Bitmap not allocated\n");
    }
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
    canvas.drawString(str, SkIntToScalar(kBitmapSize/8),
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
    uint8_t i;
    fuzz->nextRange(&i, 0, 1);
    return bitmap[i];
}

static sk_sp<SkData> make_3Dlut(int* cubeDimension, bool invR, bool invG, bool invB) {
    uint8_t shift;
    fuzz->nextRange(&shift, 0, 4);
    int size = 4 << shift;
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
    canvas->drawString("Picture", SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/4), paint);
}

static sk_sp<SkColorFilter> make_color_filter() {
    uint8_t s;
    fuzz->nextRange(&s, 0, 5);
    switch (s) {
        case 0: {
            SkScalar array[20];
            fuzz->nextN(array, 20);
            return SkColorFilter::MakeMatrixFilterRowMajor255(array);
        }
        case 1:
            return SkLumaColorFilter::Make();
        case 2: {
            uint8_t tableA[256];
            uint8_t tableR[256];
            uint8_t tableG[256];
            uint8_t tableB[256];
            fuzz->nextN(tableA, 256);
            fuzz->nextN(tableR, 256);
            fuzz->nextN(tableG, 256);
            fuzz->nextN(tableB, 256);
            return SkTableColorFilter::MakeARGB(tableA, tableR, tableG, tableB);
        }
        case 3: {
            SkColor c = make_color();
            SkBlendMode mode = make_blendmode();
            return SkColorFilter::MakeModeFilter(c, mode);
        }
        case 4: {
            SkColor a = make_color();
            SkColor b = make_color();
            return SkColorMatrixFilter::MakeLightingFilter(a, b);
        }
        case 5:
        default:
            break;
    }
    return nullptr;
}

static SkPath make_path() {
    SkPath path;
    uint8_t numOps;
    fuzz->nextRange(&numOps, 0, 30);
    for (uint8_t i = 0; i < numOps; ++i) {
        uint8_t op;
        fuzz->nextRange(&op, 0, 5);
        SkScalar a, b, c, d, e, f;
        switch (op) {
            case 0:
                fuzz->next(&a, &b);
                path.moveTo(a, b);
                break;
            case 1:
                fuzz->next(&a, &b);
                path.lineTo(a, b);
                break;
            case 2:
                fuzz->next(&a, &b, &c, &d);
                path.quadTo(a, b, c, d);
                break;
            case 3:
                fuzz->next(&a, &b, &c, &d, &e);
                path.conicTo(a, b, c, d, e);
                break;
            case 4:
                fuzz->next(&a, &b, &c, &d, &e, &f);
                path.cubicTo(a, b, c, d, e, f);
                break;
            case 5:
            default:
                fuzz->next(&a, &b, &c, &d, &e);
                path.arcTo(a, b, c, d, e);
                break;
        }
    }
    path.close();
    return path;
}

static sk_sp<SkPathEffect> make_path_effect(bool canBeNull = true) {
    sk_sp<SkPathEffect> pathEffect;
    uint8_t s;
    fuzz->nextRange(&s, 0, 2);
    if (canBeNull && s == 0) { return pathEffect; }

    fuzz->nextRange(&s, 0, 8);

    switch (s) {
        case 0: {
            SkScalar a = make_number(true);
            pathEffect = SkArcToPathEffect::Make(a);
            break;
        }
        case 1: {
            sk_sp<SkPathEffect> a = make_path_effect(false);
            sk_sp<SkPathEffect> b = make_path_effect(false);
            pathEffect = SkPathEffect::MakeCompose(a, b);
            break;
        }
        case 2: {
            SkScalar a = make_number(false);
            pathEffect = SkCornerPathEffect::Make(a);
            break;
        }
        case 3: {
            uint8_t count;
            fuzz->nextRange(&count, 0, 9);
            SkScalar intervals[10];
            fuzz->nextN(intervals, 10);
            SkScalar a = make_number(false);
            pathEffect = SkDashPathEffect::Make(intervals, count, a);
            break;
        }
        case 4: {
            SkScalar a, b;
            fuzz->next(&a, &b);
            pathEffect = SkDiscretePathEffect::Make(a, b);
            break;
        }
        case 5: {
            SkPath path = make_path();
            SkScalar a, b;
            fuzz->next(&a, &b);
            SkPath1DPathEffect::Style style = make_path_1d_path_effect_style();
            pathEffect = SkPath1DPathEffect::Make(path, a, b, style);
            break;
        }
        case 6: {
            SkScalar a = make_number(false);
            SkMatrix m;
            init_matrix(&m);
            pathEffect = SkLine2DPathEffect::Make(a, m);
            break;
        }
        case 7: {
            SkPath path = make_path();
            SkMatrix m;
            init_matrix(&m);
            pathEffect = SkPath2DPathEffect::Make(m, path);
            break;
        }
        case 8:
        default: {
            sk_sp<SkPathEffect> a = make_path_effect(false);
            sk_sp<SkPathEffect> b = make_path_effect(false);
            pathEffect = SkPathEffect::MakeCompose(a, b);
            break;
        }
    }
    return pathEffect;
}

static sk_sp<SkMaskFilter> make_mask_filter() {
    sk_sp<SkMaskFilter> maskFilter;
    uint8_t s;
    fuzz->nextRange(&s, 0, 2);
    switch (s) {
        case 0: {
            SkBlurStyle blur = make_blur_style();
            SkScalar a = make_number(false);
            SkBlurMaskFilter::BlurFlags flags = make_blur_mask_filter_flag();
            maskFilter = SkBlurMaskFilter::Make(blur, a, flags);
            break;
        }
        case 1: {
            SkScalar a = make_number(false);
            SkEmbossMaskFilter::Light light;
            fuzz->nextN(light.fDirection, 3);
            fuzz->nextRange(&light.fPad, 0, 65535);
            fuzz->nextRange(&light.fAmbient, 0, 255);
            fuzz->nextRange(&light.fSpecular, 0, 255);
            maskFilter = SkEmbossMaskFilter::Make(a, light);
            break;
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
    if (fuzz->exhausted()) {
        return paint;
    }
    paint.setHinting(make_paint_hinting());
    paint.setAntiAlias(make_bool());
    paint.setDither(make_bool());
    paint.setLinearText(make_bool());
    paint.setSubpixelText(make_bool());
    paint.setLCDRenderText(make_bool());
    paint.setEmbeddedBitmapText(make_bool());
    paint.setAutohinted(make_bool());
    paint.setVerticalText(make_bool());
    paint.setFakeBoldText(make_bool());
    paint.setDevKernText(make_bool());
    paint.setFilterQuality(make_filter_quality());
    paint.setStyle(make_paint_style());
    paint.setColor(make_color());
    paint.setStrokeWidth(make_number(false));
    paint.setStrokeMiter(make_number(false));
    paint.setStrokeCap(make_paint_cap());
    paint.setStrokeJoin(make_paint_join());
    paint.setColorFilter(make_color_filter());
    paint.setBlendMode(make_blendmode());
    paint.setPathEffect(make_path_effect());
    paint.setMaskFilter(make_mask_filter());

    if (false) {
        // our validating buffer does not support typefaces yet, so skip this for now
        paint.setTypeface(SkTypeface::MakeFromName(make_font_name().c_str(),make_typeface_style()));
    }

    SkLayerRasterizer::Builder rasterizerBuilder;
    SkPaint paintForRasterizer;
    if (make_bool()) {
        paintForRasterizer = make_paint();
    }
    rasterizerBuilder.addLayer(paintForRasterizer);
    paint.setRasterizer(rasterizerBuilder.detach());
    paint.setImageFilter(make_image_filter());
    bool a, b, c;
    fuzz->next(&a, &b, &c);
    sk_sp<SkData> data(make_3Dlut(nullptr, a, b, c));
    paint.setTextAlign(make_paint_align());
    SkScalar d, e, f;
    fuzz->next(&d, &e, &f);
    paint.setTextSize(d);
    paint.setTextScaleX(e);
    paint.setTextSkewX(f);
    paint.setTextEncoding(make_paint_text_encoding());
    return paint;
}

static sk_sp<SkImageFilter> make_image_filter(bool canBeNull) {
    sk_sp<SkImageFilter> filter;

    // Add a 1 in 3 chance to get a nullptr input
    uint8_t i;
    fuzz->nextRange(&i, 0, 2);
    if (fuzz->exhausted() || (canBeNull && i == 1)) {
        return filter;
    }

    enum { ALPHA_THRESHOLD, MERGE, COLOR, BLUR, MAGNIFIER,
           BLENDMODE, OFFSET, MATRIX, MATRIX_CONVOLUTION, COMPOSE,
           DISTANT_LIGHT, POINT_LIGHT, SPOT_LIGHT, NOISE, DROP_SHADOW,
           MORPHOLOGY, BITMAP, DISPLACE, TILE, PICTURE, PAINT, NUM_FILTERS };

    uint8_t s;
    fuzz->nextRange(&s, 0, NUM_FILTERS - 1);
    switch (s) {
    case ALPHA_THRESHOLD: {
        SkRegion reg = make_region();
        SkScalar innerMin, outerMax;
        fuzz->next(&innerMin, &outerMax);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = SkAlphaThresholdFilter::Make(reg, innerMin, outerMax, fil);
        break;
    }
    case MERGE: {
        sk_sp<SkImageFilter> filA = make_image_filter();
        sk_sp<SkImageFilter> filB = make_image_filter();
        filter = SkMergeImageFilter::Make(filA, filB);
        break;
    }
    case COLOR: {
        sk_sp<SkColorFilter> cf(make_color_filter());
        filter = cf ? SkColorFilterImageFilter::Make(std::move(cf), make_image_filter())
                    : nullptr;
        break;
    }
    case BLUR: {
        SkScalar sX = make_number(true);
        SkScalar sY = make_number(true);
        sk_sp<SkImageFilter> fil = make_image_filter();

        filter = SkBlurImageFilter::Make(sX, sY, fil);
        break;
    }
    case MAGNIFIER: {
        SkRect rect = make_rect();
        SkScalar inset = make_number(true);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = SkMagnifierImageFilter::Make(rect, inset, fil);
        break;
    }
    case BLENDMODE: {
        SkBlendMode mode = make_blendmode();
        sk_sp<SkImageFilter> filA = make_image_filter();
        sk_sp<SkImageFilter> filB = make_image_filter();
        filter = SkXfermodeImageFilter::Make(mode, filA, filB, nullptr);
        break;
    }
    case OFFSET: {
        SkScalar dx, dy;
        fuzz->next(&dx, &dy);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = SkOffsetImageFilter::Make(dx, dy, fil);
        break;
    }
    case MATRIX: {
        SkMatrix m;
        init_matrix(&m);
        int qual;
        fuzz->nextRange(&qual, 0, SkFilterQuality::kLast_SkFilterQuality - 1);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = SkImageFilter::MakeMatrixFilter(m, (SkFilterQuality)qual, fil);
        break;
    }
    case MATRIX_CONVOLUTION: {
        SkImageFilter::CropRect cropR(SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                                     SkIntToScalar(kBitmapSize)));
        int w, h;
        fuzz->nextRange(&w, 1, 10);
        fuzz->nextRange(&h, 1, 10);
        SkISize size = SkISize::Make(w, h);
        int arraySize = size.width() * size.height();
        SkTArray<SkScalar> kernel(arraySize);
        for (int i = 0; i < arraySize; ++i) {
            kernel.push_back() = make_number(false);
        }
        fuzz->nextRange(&w, 0, size.width()  - 1);
        fuzz->nextRange(&h, 0, size.height() - 1);
        SkIPoint kernelOffset = SkIPoint::Make(w, h);
        int mode;
        fuzz->nextRange(&mode, 0, SkMatrixConvolutionImageFilter::kMax_TileMode - 1);
        bool convolveAlpha = make_bool();
        SkScalar gain, bias;
        fuzz->next(&gain, &bias);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = SkMatrixConvolutionImageFilter::Make(size,
                                                      kernel.begin(),
                                                      gain,
                                                      bias,
                                                      kernelOffset,
                                                      (SkMatrixConvolutionImageFilter::TileMode)mode,
                                                      convolveAlpha,
                                                      fil,
                                                      &cropR);
        break;
    }
    case COMPOSE: {
        sk_sp<SkImageFilter> filA = make_image_filter();
        sk_sp<SkImageFilter> filB = make_image_filter();
        filter = SkComposeImageFilter::Make(filA, filB);
        break;
    }
    case DISTANT_LIGHT: {
        SkPoint3 p = make_point();
        SkColor c = make_color();
        SkScalar ss, kd;
        fuzz->next(&ss, &kd);
        int shininess;
        fuzz->nextRange(&shininess, 0, 9);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = make_bool()
                 ? SkLightingImageFilter::MakeDistantLitDiffuse(p, c, ss, kd, fil)
                 : SkLightingImageFilter::MakeDistantLitSpecular(p, c, ss, kd, shininess, fil);
        break;
    }
    case POINT_LIGHT: {
        SkPoint3 p = make_point();
        SkColor c = make_color();
        SkScalar ss, kd;
        fuzz->next(&ss, &kd);
        int shininess;
        fuzz->nextRange(&shininess, 0, 9);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = make_bool()
                 ? SkLightingImageFilter::MakePointLitDiffuse(p, c, ss, kd, fil)
                 : SkLightingImageFilter::MakePointLitSpecular(p, c, ss, kd, shininess, fil);
        break;
    }
    case SPOT_LIGHT: {
        SkPoint3 p = make_point();
        SkColor c = make_color();
        SkScalar se, ca, ss, kd;
        fuzz->next(&se, &ca, &ss, &kd);
        int shininess;
        fuzz->nextRange(&shininess, 0, 9);
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = make_bool()
                 ? SkLightingImageFilter::MakeSpotLitDiffuse(SkPoint3::Make(0, 0, 0),
                                                             p, se, ca, c, ss, kd, fil)
                 : SkLightingImageFilter::MakeSpotLitSpecular(SkPoint3::Make(0, 0, 0),
                                                              p, se, ca, c, ss, kd,
                                                              shininess, fil);
        break;
    }
    case NOISE: {
        SkScalar bfx = make_number(true);
        SkScalar bfy = make_number(true);
        SkScalar seed = make_number(false);
        int octaves;
        fuzz->nextRange(&octaves, 0, 9);
        sk_sp<SkShader> shader(make_bool()
                ? SkPerlinNoiseShader::MakeFractalNoise(bfx, bfy, octaves, seed)
                : SkPerlinNoiseShader::MakeTurbulence(bfx, bfy, octaves, seed));
        SkPaint paint;
        paint.setShader(shader);
        SkImageFilter::CropRect cropR(SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                                     SkIntToScalar(kBitmapSize)));
        filter = SkPaintImageFilter::Make(paint, &cropR);
        break;
    }
    case DROP_SHADOW: {
        SkScalar dx, dy, sx, sy;
        fuzz->next(&dx, &dy);
        sx = make_number(true);
        sy = make_number(true);
        SkColor c = make_color();
        SkDropShadowImageFilter::ShadowMode mode = make_shadow_mode();
        sk_sp<SkImageFilter> fil = make_image_filter();
        filter = SkDropShadowImageFilter::Make(dx, dy, sx, sy, c, mode, fil, nullptr);
        break;
    }
    case MORPHOLOGY: {
        int rx, ry;
        fuzz->nextRange(&rx, 0, kBitmapSize);
        fuzz->nextRange(&ry, 0, kBitmapSize);
        sk_sp<SkImageFilter> fil = make_image_filter();
        if (make_bool()) {
            filter = SkDilateImageFilter::Make(rx, ry, fil);
        } else {
            filter = SkErodeImageFilter::Make(rx, ry, fil);
        }
        break;
    }
    case BITMAP: {
        sk_sp<SkImage> image(SkImage::MakeFromBitmap(make_bitmap()));
        if (make_bool()) {
            filter = SkImageSource::Make(std::move(image),
                                         make_rect(),
                                         make_rect(),
                                         kHigh_SkFilterQuality);
        } else {
            filter = SkImageSource::Make(std::move(image));
        }
        break;
    }
    case DISPLACE: {
        SkDisplacementMapEffect::ChannelSelectorType x = make_channel_selector_type();
        SkDisplacementMapEffect::ChannelSelectorType y = make_channel_selector_type();
        SkScalar scale = make_number(false);
        sk_sp<SkImageFilter> filA = make_image_filter(false);
        sk_sp<SkImageFilter> filB = make_image_filter();

        filter = SkDisplacementMapEffect::Make(x, y, scale, filA, filB);
        break;
    }
    case TILE: {
        SkRect src = make_rect();
        SkRect dest = make_rect();
        sk_sp<SkImageFilter> fil = make_image_filter(false);
        filter = SkTileImageFilter::Make(src, dest, fil);
        break;
    }
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
    return filter;
}

static sk_sp<SkImageFilter> make_serialized_image_filter() {
    sk_sp<SkImageFilter> filter(make_image_filter(false));
    sk_sp<SkData> data(SkValidatingSerializeFlattenable(filter.get()));
    const unsigned char* ptr = static_cast<const unsigned char*>(data->data());
    size_t len = data->size();
#ifdef SK_ADD_RANDOM_BIT_FLIPS
    unsigned char* p = const_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < len; ++i, ++p) {
        uint8_t j;
        fuzz->nextRange(&j, 1, 250);
        if (j == 1) { // 0.4% of the time, flip a bit or byte
            uint8_t k;
            fuzz->nextRange(&k, 1, 10);
            if (k == 1) { // Then 10% of the time, change a whole byte
                uint8_t s;
                fuzz->nextRange(&s, 0, 2);
                switch(s) {
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
                uint8_t s;
                fuzz->nextRange(&s, 0, 7);
                *p ^= (1 << 7);
            }
        }
    }
#endif // SK_ADD_RANDOM_BIT_FLIPS
    return SkValidatingDeserializeImageFilter(ptr, len);
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

    SkPaint paint;
    paint.setImageFilter(make_serialized_image_filter());
    SkBitmap bitmap;
    SkCanvas canvas(bitmap);
    drawClippedBitmap(&canvas, 0, 0, paint);
}
