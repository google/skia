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

#define W   257
#define H   161

class GiantBitmapGM : public skiagm::GM {
    SkBitmap* fBM;
    SkShader::TileMode fMode;
    bool fDoFilter;
    bool fDoRotate;

    const SkBitmap& getBitmap() {
        if (NULL == fBM) {
            fBM = new SkBitmap;
            fBM->allocN32Pixels(W, H);
            fBM->eraseColor(SK_ColorWHITE);

            const SkColor colors[] = {
                SK_ColorBLUE, SK_ColorRED, SK_ColorBLACK, SK_ColorGREEN
            };

            SkCanvas canvas(*fBM);
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setStrokeWidth(SkIntToScalar(20));

#if 0
            for (int y = -H*2; y < H; y += 50) {
                SkScalar yy = SkIntToScalar(y);
                paint.setColor(colors[y/50 & 0x3]);
                canvas.drawLine(0, yy, SkIntToScalar(W), yy + SkIntToScalar(W),
                                paint);
            }
#else
            for (int x = -W; x < W; x += 60) {
                paint.setColor(colors[x/60 & 0x3]);

                SkScalar xx = SkIntToScalar(x);
                canvas.drawLine(xx, 0, xx, SkIntToScalar(H),
                                paint);
            }
#endif
        }
        return *fBM;
    }

public:
    GiantBitmapGM(SkShader::TileMode mode, bool doFilter, bool doRotate) : fBM(NULL) {
        fMode = mode;
        fDoFilter = doFilter;
        fDoRotate = doRotate;
    }

    virtual ~GiantBitmapGM() {
        SkDELETE(fBM);
    }

protected:

    SkString onShortName() override {
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
        str.append(fDoFilter ? "_bilerp" : "_point");
        str.append(fDoRotate ? "_rotate" : "_scale");
        return str;
    }

    SkISize onISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        SkMatrix m;
        if (fDoRotate) {
//            m.setRotate(SkIntToScalar(30), 0, 0);
            m.setSkew(SK_Scalar1, 0, 0, 0);
//            m.postScale(2*SK_Scalar1/3, 2*SK_Scalar1/3);
        } else {
            SkScalar scale = 11*SK_Scalar1/12;
            m.setScale(scale, scale);
        }
        SkShader* s = SkShader::CreateBitmapShader(getBitmap(), fMode, fMode, &m);

        paint.setShader(s)->unref();
        paint.setFilterQuality(fDoFilter ? kLow_SkFilterQuality : kNone_SkFilterQuality);

        canvas->translate(SkIntToScalar(50), SkIntToScalar(50));

//        SkRect r = SkRect::MakeXYWH(-50, -50, 32, 16);
//        canvas->drawRect(r, paint); return;
        canvas->drawPaint(paint);
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* G000(void*) { return new GiantBitmapGM(SkShader::kClamp_TileMode, false, false); }
static skiagm::GM* G100(void*) { return new GiantBitmapGM(SkShader::kRepeat_TileMode, false, false); }
static skiagm::GM* G200(void*) { return new GiantBitmapGM(SkShader::kMirror_TileMode, false, false); }
static skiagm::GM* G010(void*) { return new GiantBitmapGM(SkShader::kClamp_TileMode, true, false); }
static skiagm::GM* G110(void*) { return new GiantBitmapGM(SkShader::kRepeat_TileMode, true, false); }
static skiagm::GM* G210(void*) { return new GiantBitmapGM(SkShader::kMirror_TileMode, true, false); }

static skiagm::GM* G001(void*) { return new GiantBitmapGM(SkShader::kClamp_TileMode, false, true); }
static skiagm::GM* G101(void*) { return new GiantBitmapGM(SkShader::kRepeat_TileMode, false, true); }
static skiagm::GM* G201(void*) { return new GiantBitmapGM(SkShader::kMirror_TileMode, false, true); }
static skiagm::GM* G011(void*) { return new GiantBitmapGM(SkShader::kClamp_TileMode, true, true); }
static skiagm::GM* G111(void*) { return new GiantBitmapGM(SkShader::kRepeat_TileMode, true, true); }
static skiagm::GM* G211(void*) { return new GiantBitmapGM(SkShader::kMirror_TileMode, true, true); }

static skiagm::GMRegistry reg000(G000);
static skiagm::GMRegistry reg100(G100);
static skiagm::GMRegistry reg200(G200);
static skiagm::GMRegistry reg010(G010);
static skiagm::GMRegistry reg110(G110);
static skiagm::GMRegistry reg210(G210);

static skiagm::GMRegistry reg001(G001);
static skiagm::GMRegistry reg101(G101);
static skiagm::GMRegistry reg201(G201);
static skiagm::GMRegistry reg011(G011);
static skiagm::GMRegistry reg111(G111);
static skiagm::GMRegistry reg211(G211);
