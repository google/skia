/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkShader.h"

/*
 *  Want to ensure that our bitmap sampler (in bitmap shader) keeps plenty of
 *  precision when scaling very large images (where the dx might get very small.
 */

#define W   2560
#define H   1600

class GiantBitmapGM : public skiagm::GM {
    SkBitmap* fBM;
    SkShader::TileMode fMode;
    
    const SkBitmap& getBitmap() {
        if (NULL == fBM) {
            fBM = new SkBitmap;
            fBM->setConfig(SkBitmap::kARGB_8888_Config, W, H);
            fBM->allocPixels();
            fBM->eraseColor(SK_ColorWHITE);

            const SkColor colors[] = {
                SK_ColorBLUE, SK_ColorRED, SK_ColorBLACK, SK_ColorGREEN
            };

            SkCanvas canvas(*fBM);
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStrokeWidth(SkIntToScalar(30));
            for (int y = -H; y < H; y += 80) {
                SkScalar yy = SkIntToScalar(y);
                paint.setColor(colors[y/80 & 0x3]);
                canvas.drawLine(0, yy, SkIntToScalar(W), yy + SkIntToScalar(W),
                                paint);
            }
        }
        return *fBM;
    }

public:
    GiantBitmapGM(SkShader::TileMode mode) : fBM(NULL), fMode(mode) {}

    virtual ~GiantBitmapGM() {
        SkDELETE(fBM);
    }

protected:
    // work-around for bug http://code.google.com/p/skia/issues/detail?id=520
    //
    virtual uint32_t onGetFlags() const { return kSkipPDF_Flag; }
    
    virtual SkString onShortName() {
        SkString str("giantbitmap_");
        switch (fMode) {
            case SkShader::kClamp_TileMode:
                str.append("clamp");
                break;
            case SkShader::kRepeat_TileMode:
                str.append("repeat");
                break;
            case SkShader::kMirror_TileMode:
                str.append("mirror");
                break;
            default:
                break;
        }
        return str;
    }

    virtual SkISize onISize() { return SkISize::Make(640, 480); }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        SkShader* s = SkShader::CreateBitmapShader(getBitmap(), fMode, fMode);
        SkMatrix m;
        m.setScale(2*SK_Scalar1/3, 2*SK_Scalar1/3);
        s->setLocalMatrix(m);
        paint.setShader(s)->unref();

        canvas->drawPaint(paint);
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* G0(void*) { return new GiantBitmapGM(SkShader::kClamp_TileMode); }
static skiagm::GM* G1(void*) { return new GiantBitmapGM(SkShader::kRepeat_TileMode); }
static skiagm::GM* G2(void*) { return new GiantBitmapGM(SkShader::kMirror_TileMode); }

static skiagm::GMRegistry reg0(G0);
static skiagm::GMRegistry reg1(G1);
static skiagm::GMRegistry reg2(G2);

