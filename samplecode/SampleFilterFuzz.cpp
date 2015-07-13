/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkAlphaThresholdFilter.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorCubeFilter.h"
#include "SkColorFilter.h"
#include "SkColorFilterImageFilter.h"
#include "SkComposeImageFilter.h"
#include "SkData.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkFlattenableSerialization.h"
#include "SkLightingImageFilter.h"
#include "SkMagnifierImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkRandom.h"
#include "SkRectShaderImageFilter.h"
#include "SkTestImageFilters.h"
#include "SkTileImageFilter.h"
#include "SkView.h"
#include "SkXfermodeImageFilter.h"
#include <stdio.h>
#include <time.h>

//#define SK_ADD_RANDOM_BIT_FLIPS
//#define SK_FUZZER_IS_VERBOSE

static const uint32_t kSeed = (uint32_t)(time(NULL));
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
    return make_number(positiveOnly);
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

static SkData* make_3Dlut(int* cubeDimension, bool invR, bool invG, bool invB) {
    int size = 4 << R(5);
    SkData* data = SkData::NewUninitialized(sizeof(SkColor) * size * size * size);
    SkColor* pixels = (SkColor*)(data->writable_data());
    SkAutoMalloc lutMemory(size);
    SkAutoMalloc invLutMemory(size);
    uint8_t* lut = (uint8_t*)lutMemory.get();
    uint8_t* invLut = (uint8_t*)invLutMemory.get();
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
    canvas->drawBitmap(make_bitmap(), 0, 0, NULL);
    canvas->restore();

    paint.setAntiAlias(true);

    paint.setColor(SK_ColorRED);
    canvas->drawCircle(SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/3), paint);
    paint.setColor(SK_ColorBLACK);
    paint.setTextSize(SkIntToScalar(kBitmapSize/3));
    canvas->drawText("Picture", 7, SkIntToScalar(kBitmapSize/2), SkIntToScalar(kBitmapSize/4), paint);
}

static SkImageFilter* make_image_filter(bool canBeNull = true) {
    SkImageFilter* filter = 0;

    // Add a 1 in 3 chance to get a NULL input
    if (canBeNull && (R(3) == 1)) { return filter; }

    enum { ALPHA_THRESHOLD, MERGE, COLOR, LUT3D, BLUR, MAGNIFIER,
           DOWN_SAMPLE, XFERMODE, OFFSET, MATRIX, MATRIX_CONVOLUTION, COMPOSE,
           DISTANT_LIGHT, POINT_LIGHT, SPOT_LIGHT, NOISE, DROP_SHADOW,
           MORPHOLOGY, BITMAP, DISPLACE, TILE, PICTURE, NUM_FILTERS };

    switch (R(NUM_FILTERS)) {
    case ALPHA_THRESHOLD:
        filter = SkAlphaThresholdFilter::Create(make_region(), make_scalar(), make_scalar());
        break;
    case MERGE:
        filter = SkMergeImageFilter::Create(make_image_filter(), make_image_filter(), make_xfermode());
        break;
    case COLOR:
    {
        SkAutoTUnref<SkColorFilter> cf((R(2) == 1) ?
                 SkColorFilter::CreateModeFilter(make_color(), make_xfermode()) :
                 SkColorFilter::CreateLightingFilter(make_color(), make_color()));
        filter = cf.get() ? SkColorFilterImageFilter::Create(cf, make_image_filter()) : 0;
    }
        break;
    case LUT3D:
    {
        int cubeDimension;
        SkAutoDataUnref lut3D(make_3Dlut(&cubeDimension, (R(2) == 1), (R(2) == 1), (R(2) == 1)));
        SkAutoTUnref<SkColorFilter> cf(SkColorCubeFilter::Create(lut3D, cubeDimension));
        filter = cf.get() ? SkColorFilterImageFilter::Create(cf, make_image_filter()) : 0;
    }
        break;
    case BLUR:
        filter = SkBlurImageFilter::Create(make_scalar(true), make_scalar(true), make_image_filter());
        break;
    case MAGNIFIER:
        filter = SkMagnifierImageFilter::Create(make_rect(), make_scalar(true));
        break;
    case DOWN_SAMPLE:
        filter = SkDownSampleImageFilter::Create(make_scalar());
        break;
    case XFERMODE:
    {
        SkAutoTUnref<SkXfermode> mode(SkXfermode::Create(make_xfermode()));
        filter = SkXfermodeImageFilter::Create(mode, make_image_filter(), make_image_filter());
    }
        break;
    case OFFSET:
        filter = SkOffsetImageFilter::Create(make_scalar(), make_scalar(), make_image_filter());
        break;
    case MATRIX:
        filter = SkImageFilter::CreateMatrixFilter(make_matrix(),
                                                   (SkFilterQuality)R(4),
                                                   make_image_filter());
        break;
    case MATRIX_CONVOLUTION:
    {
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
        filter = SkMatrixConvolutionImageFilter::Create(size,
                                                        kernel.begin(),
                                                        make_scalar(),
                                                        make_scalar(),
                                                        kernelOffset,
                                                        (SkMatrixConvolutionImageFilter::TileMode)R(3),
                                                        R(2) == 1,
                                                        make_image_filter(),
                                                        &cropR);
    }
        break;
    case COMPOSE:
        filter = SkComposeImageFilter::Create(make_image_filter(), make_image_filter());
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
                 SkLightingImageFilter::CreateSpotLitDiffuse(SkPoint3::Make(0, 0, 0),
                 make_point(), make_scalar(), make_scalar(), make_color(),
                 make_scalar(), make_scalar(), make_image_filter()) :
                 SkLightingImageFilter::CreateSpotLitSpecular(SkPoint3::Make(0, 0, 0),
                 make_point(), make_scalar(), make_scalar(), make_color(),
                 make_scalar(), make_scalar(), SkIntToScalar(R(10)), make_image_filter());
        break;
    case NOISE:
    {
        SkAutoTUnref<SkShader> shader((R(2) == 1) ?
            SkPerlinNoiseShader::CreateFractalNoise(
                make_scalar(true), make_scalar(true), R(10.0f), make_scalar()) :
            SkPerlinNoiseShader::CreateTurbulence(
                make_scalar(true), make_scalar(true), R(10.0f), make_scalar()));
        SkImageFilter::CropRect cropR(SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                                     SkIntToScalar(kBitmapSize)));
        filter = SkRectShaderImageFilter::Create(shader, &cropR);
    }
        break;
    case DROP_SHADOW:
        filter = SkDropShadowImageFilter::Create(make_scalar(), make_scalar(), make_scalar(true),
                    make_scalar(true), make_color(), make_shadow_mode(), make_image_filter(),
                    NULL);
        break;
    case MORPHOLOGY:
        if (R(2) == 1) {
            filter = SkDilateImageFilter::Create(R(static_cast<float>(kBitmapSize)),
                R(static_cast<float>(kBitmapSize)), make_image_filter());
        } else {
            filter = SkErodeImageFilter::Create(R(static_cast<float>(kBitmapSize)),
                R(static_cast<float>(kBitmapSize)), make_image_filter());
        }
        break;
    case BITMAP:
        if (R(2) == 1) {
            filter = SkBitmapSource::Create(make_bitmap(), make_rect(), make_rect());
        } else {
            filter = SkBitmapSource::Create(make_bitmap());
        }
        break;
    case DISPLACE:
        filter = SkDisplacementMapEffect::Create(make_channel_selector_type(),
                                                 make_channel_selector_type(), make_scalar(),
                                                 make_image_filter(false), make_image_filter());
        break;
    case TILE:
        filter = SkTileImageFilter::Create(make_rect(), make_rect(), make_image_filter(false));
        break;
    case PICTURE:
    {
        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(kBitmapSize), 
                                                            SkIntToScalar(kBitmapSize), 
                                                            &factory, 0);
        drawSomething(recordingCanvas);
        SkAutoTUnref<SkPicture> pict(recorder.endRecording());
        filter = SkPictureImageFilter::Create(pict.get(), make_rect());
    }
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

static void do_fuzz(SkCanvas* canvas) {
    SkImageFilter* filter = make_serialized_image_filter();

#ifdef SK_FUZZER_IS_VERBOSE
    static uint32_t numFilters = 0;
    static uint32_t numValidFilters = 0;
    if (0 == numFilters) {
        printf("Fuzzing with %u\n", kSeed);
    }
    numFilters++;
    if (filter) {
        numValidFilters++;
    }
    printf("Filter no : %u. Valid filters so far : %u\r", numFilters, numValidFilters);
    fflush(stdout);
#endif

    SkPaint paint;
    SkSafeUnref(paint.setImageFilter(filter));
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
