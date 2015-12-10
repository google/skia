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
        if (nullptr == fBM) {
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
    GiantBitmapGM(SkShader::TileMode mode, bool doFilter, bool doRotate) : fBM(nullptr) {
        fMode = mode;
        fDoFilter = doFilter;
        fDoRotate = doRotate;
    }

    virtual ~GiantBitmapGM() { delete fBM; }

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

DEF_GM( return new GiantBitmapGM(SkShader::kClamp_TileMode, false, false); )
DEF_GM( return new GiantBitmapGM(SkShader::kRepeat_TileMode, false, false); )
DEF_GM( return new GiantBitmapGM(SkShader::kMirror_TileMode, false, false); )
DEF_GM( return new GiantBitmapGM(SkShader::kClamp_TileMode, true, false); )
DEF_GM( return new GiantBitmapGM(SkShader::kRepeat_TileMode, true, false); )
DEF_GM( return new GiantBitmapGM(SkShader::kMirror_TileMode, true, false); )

DEF_GM( return new GiantBitmapGM(SkShader::kClamp_TileMode, false, true); )
DEF_GM( return new GiantBitmapGM(SkShader::kRepeat_TileMode, false, true); )
DEF_GM( return new GiantBitmapGM(SkShader::kMirror_TileMode, false, true); )
DEF_GM( return new GiantBitmapGM(SkShader::kClamp_TileMode, true, true); )
DEF_GM( return new GiantBitmapGM(SkShader::kRepeat_TileMode, true, true); )
DEF_GM( return new GiantBitmapGM(SkShader::kMirror_TileMode, true, true); )
