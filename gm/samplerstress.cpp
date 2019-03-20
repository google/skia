/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkMaskFilter.h"
#include "SkPath.h"
#include "SkShader.h"
#include "ToolUtils.h"
#include "gm.h"

namespace skiagm {

/**
 * Stress test the GPU samplers by rendering a textured glyph with a mask and
 * an AA clip
 */
class SamplerStressGM : public GM {
public:
    SamplerStressGM()
    : fTextureCreated(false)
    , fMaskFilter(nullptr) {
    }

protected:

    SkString onShortName() override {
        return SkString("gpusamplerstress");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    /**
     * Create a red & green stripes on black texture
     */
    void createTexture() {
        if (fTextureCreated) {
            return;
        }

        constexpr int xSize = 16;
        constexpr int ySize = 16;

        fTexture.allocN32Pixels(xSize, ySize);
        SkPMColor* addr = fTexture.getAddr32(0, 0);

        for (int y = 0; y < ySize; ++y) {
            for (int x = 0; x < xSize; ++x) {
                addr[y*xSize+x] = SkPreMultiplyColor(SK_ColorBLACK);

                if ((y % 5) == 0) {
                    addr[y*xSize+x] = SkPreMultiplyColor(SK_ColorRED);
                }
                if ((x % 7) == 0) {
                    addr[y*xSize+x] = SkPreMultiplyColor(SK_ColorGREEN);
                }
            }
        }

        fTextureCreated = true;
    }

    void createShader() {
        if (fShader) {
            return;
        }

        createTexture();

        fShader = SkShader::MakeBitmapShader(fTexture, SkShader::kRepeat_TileMode,
                                             SkShader::kRepeat_TileMode);
    }

    void createMaskFilter() {
        if (fMaskFilter) {
            return;
        }

        const SkScalar sigma = 1;
        fMaskFilter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma);
    }

    void onDraw(SkCanvas* canvas) override {
        createShader();
        createMaskFilter();

        canvas->save();

        // draw a letter "M" with a green & red striped texture and a
        // stipple mask with a round rect soft clip
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setShader(fShader);
        paint.setMaskFilter(fMaskFilter);
        SkFont font(ToolUtils::create_portable_typeface(), 72);

        SkRect temp;
        temp.set(SkIntToScalar(115),
                 SkIntToScalar(75),
                 SkIntToScalar(144),
                 SkIntToScalar(110));

        SkPath path;
        path.addRoundRect(temp, SkIntToScalar(5), SkIntToScalar(5));

        canvas->clipPath(path, true); // AA is on

        canvas->drawString("M", 100.0f, 100.0f, font, paint);

        canvas->restore();

        // Now draw stroked versions of the "M" and the round rect so we can
        // see what is going on
        SkPaint paint2;
        paint2.setColor(SK_ColorBLACK);
        paint2.setAntiAlias(true);
        paint2.setStyle(SkPaint::kStroke_Style);
        paint2.setStrokeWidth(1);
        canvas->drawString("M", 100.0f, 100.0f, font, paint2);

        paint2.setColor(SK_ColorGRAY);

        canvas->drawPath(path, paint2);
    }

private:
    SkBitmap        fTexture;
    bool            fTextureCreated;
    sk_sp<SkShader> fShader;
    sk_sp<SkMaskFilter> fMaskFilter;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new SamplerStressGM; )

}
