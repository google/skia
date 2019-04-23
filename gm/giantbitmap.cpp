/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkShader.h"

/*
 *  Want to ensure that our bitmap sampler (in bitmap shader) keeps plenty of
 *  precision when scaling very large images (where the dx might get very small.
 */

#define W   257
#define H   161

class GiantBitmapGM : public skiagm::GM {
    SkBitmap* fBM;
    SkTileMode fMode;
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
    GiantBitmapGM(SkTileMode mode, bool doFilter, bool doRotate) : fBM(nullptr) {
        fMode = mode;
        fDoFilter = doFilter;
        fDoRotate = doRotate;
    }

    ~GiantBitmapGM() override { delete fBM; }

protected:

    SkString onShortName() override {
        SkString str("giantbitmap_");
        switch (fMode) {
            case SkTileMode::kClamp:
                str.append("clamp");
                break;
            case SkTileMode::kRepeat:
                str.append("repeat");
                break;
            case SkTileMode::kMirror:
                str.append("mirror");
                break;
            case SkTileMode::kDecal:
                str.append("decal");
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
        paint.setShader(getBitmap().makeShader(fMode, fMode, &m));
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

DEF_GM( return new GiantBitmapGM(SkTileMode::kClamp, false, false); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kRepeat, false, false); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kMirror, false, false); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kClamp, true, false); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kRepeat, true, false); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kMirror, true, false); )

DEF_GM( return new GiantBitmapGM(SkTileMode::kClamp, false, true); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kRepeat, false, true); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kMirror, false, true); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kClamp, true, true); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kRepeat, true, true); )
DEF_GM( return new GiantBitmapGM(SkTileMode::kMirror, true, true); )
