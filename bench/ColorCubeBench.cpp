/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkColorCubeFilter.h"
#include "SkGradientShader.h"
#include "SkTemplates.h"

class ColorCubeBench : public Benchmark {
    SkISize fSize;
    int fCubeDimension;
    SkData* fCubeData;
    SkBitmap fBitmap;

public:
    ColorCubeBench()
     : fCubeDimension(0)
     , fCubeData(nullptr) {
        fSize = SkISize::Make(2880, 1800); // 2014 Macbook Pro resolution
    }

    ~ColorCubeBench() {
        SkSafeUnref(fCubeData);
    }

protected:
    const char* onGetName() override {
        return "colorcube";
    }

    void onDelayedSetup() override {
        if (!SkToBool(fCubeData)) {
            this->makeCubeData();
            this->make_bitmap();
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        this->test(loops, canvas);
    }

    SkIPoint onGetSize() override {
        return SkIPoint::Make(fSize.width(), fSize.height());
    }

private:
    static SkShader* MakeLinear(const SkISize& size) {
        const SkPoint pts[2] = {
                { 0, 0 },
                { SkIntToScalar(size.width()), SkIntToScalar(size.height()) }
            };
        static const SkColor colors[] = { SK_ColorYELLOW, SK_ColorBLUE };
        return SkGradientShader::CreateLinear(
            pts, colors, nullptr, 2, SkShader::kRepeat_TileMode, 0, &SkMatrix::I());
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(fSize.width(), fSize.height());
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        SkShader* shader = MakeLinear(fSize);
        paint.setShader(shader);
        SkRect r = { 0, 0, SkIntToScalar(fSize.width()), SkIntToScalar(fSize.height()) };
        canvas.drawRect(r, paint);
        shader->unref();
    }

    void makeCubeData() {
        fCubeDimension = 32;
        fCubeData = SkData::NewUninitialized(sizeof(SkColor) *
            fCubeDimension * fCubeDimension * fCubeDimension);
        SkColor* pixels = (SkColor*)(fCubeData->writable_data());
        SkAutoTMalloc<uint8_t> lutMemory(fCubeDimension);
        uint8_t* lut = lutMemory.get();
        const int maxIndex = fCubeDimension - 1;
        for (int i = 0; i < fCubeDimension; ++i) {
            // Make an invert lut, but the content of
            // the lut shouldn't affect performance.
            lut[i] = ((maxIndex - i) * 255) / maxIndex;
        }
        for (int r = 0; r < fCubeDimension; ++r) {
            for (int g = 0; g < fCubeDimension; ++g) {
                for (int b = 0; b < fCubeDimension; ++b) {
                    pixels[(fCubeDimension * ((fCubeDimension * b) + g)) + r] =
                        SkColorSetARGB(0xFF, lut[r], lut[g], lut[b]);
                }
            }
        }
    }

    void test(int loops, SkCanvas* canvas) {
        SkPaint paint;
        for (int i = 0; i < loops; i++) {
            SkAutoTUnref<SkColorFilter> colorCube(
                SkColorCubeFilter::Create(fCubeData, fCubeDimension));
            paint.setColorFilter(colorCube);
            canvas->drawBitmap(fBitmap, 0, 0, &paint);
        }
    }

    typedef Benchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new ColorCubeBench(); )
