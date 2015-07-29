/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorCubeFilter.h"
#include "SkBitmapSource.h"
#include "SkData.h"
#include "SkGradientShader.h"

namespace skiagm {

static SkShader* MakeLinear() {
    static const SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(80), SkIntToScalar(80) }
        };
    static const SkColor colors[] = { SK_ColorYELLOW, SK_ColorBLUE };
    return SkGradientShader::CreateLinear(
        pts, colors, NULL, 2, SkShader::kRepeat_TileMode, 0, &SkMatrix::I());
}

class ColorCubeGM : public GM {
public:
    ColorCubeGM()
    : fInitialized(false)
    , f3DLut4(NULL)
    , f3DLut8(NULL)
    , f3DLut16(NULL)
    , f3DLut32(NULL)
    , f3DLut64(NULL)
    {
        this->setBGColor(0xFF000000);
    }

    ~ColorCubeGM() {
        SkSafeUnref(f3DLut4);
        SkSafeUnref(f3DLut8);
        SkSafeUnref(f3DLut16);
        SkSafeUnref(f3DLut32);
        SkSafeUnref(f3DLut64);
    }

protected:
    virtual SkString onShortName() {
        return SkString("colorcube");
    }

    void make_3Dluts() {
        make_3Dlut(&f3DLut4, 4, true, false, false);
        make_3Dlut(&f3DLut8, 8, false, true, false);
        make_3Dlut(&f3DLut16, 16, false, true, true);
        make_3Dlut(&f3DLut32, 32, true, true, false);
        make_3Dlut(&f3DLut64, 64, true, false, true);
    }

    void make_bitmap() {
        fBitmap.allocN32Pixels(80, 80);
        SkCanvas canvas(fBitmap);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        SkShader* shader = MakeLinear();
        paint.setShader(shader);
        SkRect r = { 0, 0, SkIntToScalar(80), SkIntToScalar(80) };
        canvas.drawRect(r, paint);
        shader->unref();
    }

    void make_3Dlut(SkData** data, int size, bool invR, bool invG, bool invB) {
        *data = SkData::NewUninitialized(sizeof(SkColor) * size * size * size);
        SkColor* pixels = (SkColor*)((*data)->writable_data());
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
                    pixels[(size * ((size * b) + g)) + r] = sk_tool_utils::color_to_565(
                            SkColorSetARGB(0xFF,
                            invR ? invLut[r] : lut[r],
                            invG ? invLut[g] : lut[g],
                            invB ? invLut[b] : lut[b]));
                }
            }
        }
    }

    virtual SkISize onISize() {
        return SkISize::Make(500, 100);
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            this->make_bitmap();
            this->make_3Dluts();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkPaint paint;
        paint.setColorFilter(SkColorCubeFilter::Create(f3DLut4, 4))->unref();
        canvas->drawBitmap(fBitmap, 10, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Create(f3DLut8, 8))->unref();
        canvas->drawBitmap(fBitmap, 110, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Create(f3DLut16, 16))->unref();
        canvas->drawBitmap(fBitmap, 210, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Create(f3DLut32, 32))->unref();
        canvas->drawBitmap(fBitmap, 310, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Create(f3DLut64, 64))->unref();
        canvas->drawBitmap(fBitmap, 410, 10, &paint);
    }

private:
    typedef GM INHERITED;
    bool fInitialized;
    SkBitmap fBitmap;
    SkData* f3DLut4;
    SkData* f3DLut8;
    SkData* f3DLut16;
    SkData* f3DLut32;
    SkData* f3DLut64;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ColorCubeGM; }
static GMRegistry reg(MyFactory);

}
