/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkBicubicImageFilter.h"
#include "SkBitmapDevice.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkComposeImageFilter.h"
#include "SkData.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkFlattenableSerialization.h"
#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkRandom.h"
#include "SkRectShaderImageFilter.h"
#include "SkView.h"
#include "SkXfermodeImageFilter.h"

static const uint32_t kSeed = 1;
static SkRandom gRand(kSeed);
static bool return_large = false;
static bool return_undef = false;

static const int kBitmapSize = 24;

static int R(float x) {
    return (int)floor(SkScalarToFloat(gRand.nextUScalar1()) * x);
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
    return SkFloatToScalar(make_number(positiveOnly));
}

static SkRect make_rect(int offset = 1) {
    return SkRect::MakeWH(SkIntToScalar(R(static_cast<float>(kBitmapSize))+offset),
                          SkIntToScalar(R(static_cast<float>(kBitmapSize))+offset));
}

static SkXfermode::Mode make_xfermode() {
    return static_cast<SkXfermode::Mode>(R(SkXfermode::kLastMode+1));
}

static SkColor make_color() {
    return (R(2) == 1) ? 0xFFC0F0A0 : 0xFF000090;
}

static SkPoint3 make_point() {
    return SkPoint3(make_scalar(), make_scalar(), make_scalar(true));
}

static SkDisplacementMapEffect::ChannelSelectorType make_channel_selector_type() {
    return static_cast<SkDisplacementMapEffect::ChannelSelectorType>(R(4)+1);
}

static void make_g_bitmap(SkBitmap& bitmap) {
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, kBitmapSize, kBitmapSize);
    bitmap.allocPixels();
    SkBitmapDevice device(bitmap);
    SkCanvas canvas(&device);
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
    bitmap.setConfig(SkBitmap::kARGB_8888_Config, kBitmapSize, kBitmapSize);
    bitmap.allocPixels();
    SkBitmapDevice device(bitmap);
    SkCanvas canvas(&device);
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

static SkImageFilter* make_image_filter(bool canBeNull = true) {
    SkImageFilter* filter = 0;

    // Add a 1 in 3 chance to get a NULL input
    if (canBeNull && (R(3) == 1)) { return filter; }

    enum { BICUBIC, MERGE, COLOR, BLUR, MAGNIFIER, XFERMODE, OFFSET, COMPOSE,
           DISTANT_LIGHT, POINT_LIGHT, SPOT_LIGHT, NOISE, DROP_SHADOW,
           MORPHOLOGY, BITMAP, DISPLACE, NUM_FILTERS };

    switch (R(NUM_FILTERS)) {
    case BICUBIC:
        // Scale is set to 1 here so that it can fit in the DAG without resizing the output
        filter = SkBicubicImageFilter::CreateMitchell(SkSize::Make(1, 1), make_image_filter());
        break;
    case MERGE:
        filter = new SkMergeImageFilter(make_image_filter(), make_image_filter(), make_xfermode());
        break;
    case COLOR:
    {
        SkAutoTUnref<SkColorFilter> cf((R(2) == 1) ?
                 SkColorFilter::CreateModeFilter(make_color(), make_xfermode()) :
                 SkColorFilter::CreateLightingFilter(make_color(), make_color()));
        filter = cf.get() ? SkColorFilterImageFilter::Create(cf, make_image_filter()) : 0;
    }
        break;
    case BLUR:
        filter = new SkBlurImageFilter(make_scalar(true), make_scalar(true), make_image_filter());
        break;
    case MAGNIFIER:
        filter = new SkMagnifierImageFilter(make_rect(0), make_scalar(true));
        break;
    case XFERMODE:
    {
        SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(make_xfermode()));
        filter = new SkXfermodeImageFilter(mode, make_image_filter(), make_image_filter());
    }
        break;
    case OFFSET:
        filter = new SkOffsetImageFilter(make_scalar(), make_scalar(), make_image_filter());
        break;
    case COMPOSE:
        filter = new SkComposeImageFilter(make_image_filter(), make_image_filter());
        break;
    case DISTANT_LIGHT:
        filter = (R(2) == 1) ?
                 SkLightingImageFilter::CreateDistantLitDiffuse(make_point(),
                 make_color(), make_scalar(), make_scalar(), make_image_filter()) :
                 SkLightingImageFilter::CreateDistantLitSpecular(make_point(),
                 make_color(), make_scalar(), make_scalar(), SkIntToScalar(R(10)),
                 make_image_filter());
        break;
    case POINT_LIGHT:
        filter = (R(2) == 1) ?
                 SkLightingImageFilter::CreatePointLitDiffuse(make_point(),
                 make_color(), make_scalar(), make_scalar(), make_image_filter()) :
                 SkLightingImageFilter::CreatePointLitSpecular(make_point(),
                 make_color(), make_scalar(), make_scalar(), SkIntToScalar(R(10)),
                 make_image_filter());
        break;
    case SPOT_LIGHT:
        filter = (R(2) == 1) ?
                 SkLightingImageFilter::CreateSpotLitDiffuse(SkPoint3(0, 0, 0),
                 make_point(), make_scalar(), make_scalar(), make_color(),
                 make_scalar(), make_scalar(), make_image_filter()) :
                 SkLightingImageFilter::CreateSpotLitSpecular(SkPoint3(0, 0, 0),
                 make_point(), make_scalar(), make_scalar(), make_color(),
                 make_scalar(), make_scalar(), SkIntToScalar(R(10)), make_image_filter());
        break;
    case NOISE:
    {
        SkAutoTUnref<SkShader> shader((R(2) == 1) ?
            SkPerlinNoiseShader::CreateFractalNoise(
                make_scalar(true), make_scalar(true), R(10.0f), make_scalar()) :
            SkPerlinNoiseShader::CreateTubulence(
                make_scalar(true), make_scalar(true), R(10.0f), make_scalar()));
        filter = SkRectShaderImageFilter::Create(shader,
                                                 SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                                                SkIntToScalar(kBitmapSize)));
    }
        break;
    case DROP_SHADOW:
        filter = new SkDropShadowImageFilter(make_scalar(), make_scalar(),
                     make_scalar(true), make_color(), make_image_filter());
        break;
    case MORPHOLOGY:
        if (R(2) == 1)
            filter = new SkDilateImageFilter(R(static_cast<float>(kBitmapSize)),
                R(static_cast<float>(kBitmapSize)), make_image_filter());
        else
            filter = new SkErodeImageFilter(R(static_cast<float>(kBitmapSize)),
                R(static_cast<float>(kBitmapSize)), make_image_filter());
        break;
    case BITMAP:
        filter = new SkBitmapSource(make_bitmap());
        break;
    case DISPLACE:
        filter = new SkDisplacementMapEffect(make_channel_selector_type(),
                     make_channel_selector_type(), make_scalar(),
                     make_image_filter(false), make_image_filter());
        break;
    default:
        break;
    }
    return (filter || canBeNull) ? filter : make_image_filter(canBeNull);
}

static SkImageFilter* make_serialized_image_filter() {
    SkAutoTUnref<SkImageFilter> filter(make_image_filter(false));
    SkAutoTUnref<SkData> data(SkValidatingSerializeFlattenable(filter));
    const unsigned char* ptr = static_cast<const unsigned char*>(data->data());
    size_t len = data->size();
#ifdef SK_ADD_RANDOM_BIT_FLIPS
    unsigned char* p = const_cast<unsigned char*>(ptr);
    for (size_t i = 0; i < len; ++i, ++p) {
        if ((R(1000) == 1)) { // 0.1% of the time, flip a bit
            *p ^= (1 << R(8));
        }
    }
#endif // SK_ADD_RANDOM_BIT_FLIPS
    SkFlattenable* flattenable = SkValidatingDeserializeFlattenable(ptr, len,
                                    SkImageFilter::GetFlattenableType());
    SkASSERT(NULL != flattenable);
    return static_cast<SkImageFilter*>(flattenable);
}

static void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
        SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)));
    canvas->drawBitmap(make_bitmap(), SkIntToScalar(x), SkIntToScalar(y), &paint);
    canvas->restore();
}

static void do_fuzz(SkCanvas* canvas) {
    SkPaint paint;
    paint.setImageFilter(make_serialized_image_filter())->unref();
    drawClippedBitmap(canvas, 0, 0, paint);
}

//////////////////////////////////////////////////////////////////////////////

class ImageFilterFuzzView : public SampleView {
public:
    ImageFilterFuzzView() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ImageFilterFuzzer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        do_fuzz(canvas);
        this->inval(0);
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ImageFilterFuzzView; }
static SkViewRegister reg(MyFactory);
