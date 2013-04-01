
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBitmap.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkXfermode.h"

namespace skiagm {

/**
 * Renders overlapping circles with random SkXfermode::Modes against a checkerboard.
 */
class MixedXfermodesGM : public GM {
public:
    MixedXfermodesGM() {
        static uint32_t kCheckerPixelData[] = { 0xFFFFFFFF, 0xFFCCCCCC, 0xFFCCCCCC, 0xFFFFFFFF };
        SkBitmap bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, 2, 2, 2 * sizeof(uint32_t));
        bitmap.allocPixels();
        bitmap.lockPixels();
        memcpy(bitmap.getPixels(), kCheckerPixelData, sizeof(kCheckerPixelData));
        bitmap.unlockPixels();
        fBG.reset(SkShader::CreateBitmapShader(bitmap,
                                               SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode));
        SkASSERT(NULL != fBG);
        SkMatrix lm;
        lm.setScale(SkIntToScalar(20), SkIntToScalar(20));
        fBG->setLocalMatrix(lm);
    }

protected:
    virtual SkString onShortName() {
        return SkString("mixed_xfermodes");
    }

    virtual SkISize onISize() {
        return make_isize(790, 640);
    }

    virtual void onDraw(SkCanvas* canvas) {
        // FIXME: Currently necessary for GPU to be able to make dst-copy in SampleApp because
        // main layer is not a texture.
        SkPaint layerPaint;
        layerPaint.setXfermodeMode(SkXfermode::kSrc_Mode);
        canvas->saveLayer(NULL, &layerPaint);
        
        SkPaint bgPaint;
        bgPaint.setShader(fBG.get());
        canvas->drawPaint(bgPaint);
        SkISize size = canvas->getDeviceSize();
        SkScalar areaSqrt = SkScalarSqrt((SkIntToScalar(size.fWidth * size.fHeight)));
        SkScalar minR = areaSqrt / 10;
        SkScalar maxR = areaSqrt / 4;
        SkMWCRandom random;
        for (int i = 0; i < kNumCircles; ++i) {
            SkScalar cx = random.nextRangeScalar(0, SkIntToScalar(size.fWidth));
            SkScalar cy = random.nextRangeScalar(0, SkIntToScalar(size.fHeight));
            SkScalar r = random.nextRangeScalar(minR, maxR);
            SkColor color = random.nextU();
            
            SkXfermode::Mode mode =
                static_cast<SkXfermode::Mode>(random.nextULessThan(SkXfermode::kLastMode + 1));
            // FIXME: Currently testing kDarken on GPU.
            mode = SkXfermode::kDarken_Mode;
            
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(color);
            p.setXfermodeMode(mode);
            canvas->drawCircle(cx, cy, r, p);
        }
        
        // FIXME: Remove text draw once this GM is finished.
        SkPaint txtPaint;
        txtPaint.setTextSize(areaSqrt / 20);
        txtPaint.setAntiAlias(true);
        static const char kTxt[] = "Work in progres... Do not baseline.";
        canvas->drawText(kTxt, strlen(kTxt),
                         areaSqrt/50,
                         SkIntToScalar(size.fHeight / 2),
                         txtPaint);

        canvas->restore();
    }

    // https://code.google.com/p/skia/issues/detail?id=1199&thanks=1199&ts=1364839297
    virtual uint32_t onGetFlags() const SK_OVERRIDE { return kSkipPipe_Flag; }

private:
    enum {
        kMinR = 10,
        kMaxR = 50,
        kNumCircles = 50,
    };
    SkAutoTUnref<SkShader> fBG;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new MixedXfermodesGM; }
static GMRegistry reg(MyFactory);

}
