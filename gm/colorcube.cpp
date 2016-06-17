/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkColorCubeFilter.h"
#include "SkData.h"
#include "SkGradientShader.h"
#include "SkTemplates.h"

namespace skiagm {

static sk_sp<SkShader> MakeLinear() {
    static const SkPoint pts[2] = {
            { 0, 0 },
            { SkIntToScalar(80), SkIntToScalar(80) }
        };
    static const SkColor colors[] = { SK_ColorYELLOW, SK_ColorBLUE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kRepeat_TileMode, 0,
                                        &SkMatrix::I());
}

class ColorCubeGM : public GM {
public:
    ColorCubeGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    SkString onShortName() override {
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
        paint.setShader(MakeLinear());
        canvas.drawRect(SkRect::MakeWH(80, 80), paint);
    }

    void make_3Dlut(sk_sp<SkData>* data, int size, bool invR, bool invG, bool invB) {
        *data = SkData::MakeUninitialized(sizeof(SkColor) * size * size * size);
        SkColor* pixels = (SkColor*)((*data)->writable_data());
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
                    pixels[(size * ((size * b) + g)) + r] = sk_tool_utils::color_to_565(
                            SkColorSetARGB(0xFF,
                            invR ? invLut[r] : lut[r],
                            invG ? invLut[g] : lut[g],
                            invB ? invLut[b] : lut[b]));
                }
            }
        }
    }

    SkISize onISize() override {
        return SkISize::Make(500, 100);
    }

    void onDraw(SkCanvas* canvas) override {
        if (!fInitialized) {
            this->make_bitmap();
            this->make_3Dluts();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkPaint paint;
        paint.setColorFilter(SkColorCubeFilter::Make(f3DLut4, 4));
        canvas->drawBitmap(fBitmap, 10, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Make(f3DLut8, 8));
        canvas->drawBitmap(fBitmap, 110, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Make(f3DLut16, 16));
        canvas->drawBitmap(fBitmap, 210, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Make(f3DLut32, 32));
        canvas->drawBitmap(fBitmap, 310, 10, &paint);

        paint.setColorFilter(SkColorCubeFilter::Make(f3DLut64, 64));
        canvas->drawBitmap(fBitmap, 410, 10, &paint);
    }

private:
    typedef GM INHERITED;
    bool fInitialized;
    SkBitmap fBitmap;
    sk_sp<SkData> f3DLut4;
    sk_sp<SkData> f3DLut8;
    sk_sp<SkData> f3DLut16;
    sk_sp<SkData> f3DLut32;
    sk_sp<SkData> f3DLut64;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ColorCubeGM; }
static GMRegistry reg(MyFactory);

}
