#include "gm.h"
#include "SkBitmap.h"
#include "SkShader.h"
#include "SkXfermode.h"

namespace skiagm {

static void make_bitmaps(int w, int h, SkBitmap* src, SkBitmap* dst) {
    src->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    src->allocPixels();
    src->eraseColor(0);

    SkCanvas c(*src);
    SkPaint p;
    SkRect r;
    SkScalar ww = SkIntToScalar(w);
    SkScalar hh = SkIntToScalar(h);

    p.setAntiAlias(true);
    p.setColor(0xFFFFCC44);
    r.set(0, 0, ww*3/4, hh*3/4);
    c.drawOval(r, p);

    dst->setConfig(SkBitmap::kARGB_8888_Config, w, h);
    dst->allocPixels();
    dst->eraseColor(0);
    c.setBitmapDevice(*dst);

    p.setColor(0xFF66AAFF);
    r.set(ww/3, hh/3, ww*19/20, hh*19/20);
    c.drawRect(r, p);
}

static uint16_t gBG[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class XfermodesGM : public GM {
    SkBitmap    fBG;
    SkBitmap    fSrcB, fDstB;

    void draw_mode(SkCanvas* canvas, SkXfermode* mode, int alpha,
                   SkScalar x, SkScalar y) {
        SkPaint p;

        canvas->drawBitmap(fSrcB, x, y, &p);
        p.setAlpha(alpha);
        p.setXfermode(mode);
        canvas->drawBitmap(fDstB, x, y, &p);
    }

public:
    const static int W = 64;
    const static int H = 64;
    XfermodesGM() {
        // Do all this work in a temporary so we get a deep copy,
        // especially of gBG.
        SkBitmap scratchBitmap;
        scratchBitmap.setConfig(SkBitmap::kARGB_4444_Config, 2, 2, 4);
        scratchBitmap.setPixels(gBG);
        scratchBitmap.setIsOpaque(true);
        scratchBitmap.copyTo(&fBG, SkBitmap::kARGB_4444_Config);

        make_bitmaps(W, H, &fSrcB, &fDstB);
    }

protected:
    virtual SkString onShortName() {
        return SkString("xfermodes");
    }

    virtual SkISize onISize() {
        return make_isize(790, 640);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        this->drawBG(canvas);

        const struct {
            SkXfermode::Mode  fMode;
            const char*         fLabel;
        } gModes[] = {
            { SkXfermode::kClear_Mode,    "Clear"     },
            { SkXfermode::kSrc_Mode,      "Src"       },
            { SkXfermode::kDst_Mode,      "Dst"       },
            { SkXfermode::kSrcOver_Mode,  "SrcOver"   },
            { SkXfermode::kDstOver_Mode,  "DstOver"   },
            { SkXfermode::kSrcIn_Mode,    "SrcIn"     },
            { SkXfermode::kDstIn_Mode,    "DstIn"     },
            { SkXfermode::kSrcOut_Mode,   "SrcOut"    },
            { SkXfermode::kDstOut_Mode,   "DstOut"    },
            { SkXfermode::kSrcATop_Mode,  "SrcATop"   },
            { SkXfermode::kDstATop_Mode,  "DstATop"   },
            { SkXfermode::kXor_Mode,      "Xor"       },

            { SkXfermode::kPlus_Mode,         "Plus"          },
            { SkXfermode::kMultiply_Mode,     "Multiply"      },
            { SkXfermode::kScreen_Mode,       "Screen"        },
            { SkXfermode::kOverlay_Mode,      "Overlay"       },
            { SkXfermode::kDarken_Mode,       "Darken"        },
            { SkXfermode::kLighten_Mode,      "Lighten"       },
            { SkXfermode::kColorDodge_Mode,   "ColorDodge"    },
            { SkXfermode::kColorBurn_Mode,    "ColorBurn"     },
            { SkXfermode::kHardLight_Mode,    "HardLight"     },
            { SkXfermode::kSoftLight_Mode,    "SoftLight"     },
            { SkXfermode::kDifference_Mode,   "Difference"    },
            { SkXfermode::kExclusion_Mode,    "Exclusion"     },
        };

        const SkScalar w = SkIntToScalar(W);
        const SkScalar h = SkIntToScalar(H);
        SkShader* s = SkShader::CreateBitmapShader(fBG,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        s->setLocalMatrix(m);

        SkPaint labelP;
        labelP.setAntiAlias(true);
        labelP.setTextAlign(SkPaint::kCenter_Align);

        const int W = 5;

        SkScalar x0 = 0;
        for (int twice = 0; twice < 2; twice++) {
            SkScalar x = x0, y = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
                SkXfermode* mode = SkXfermode::Create(gModes[i].fMode);
                SkAutoUnref aur(mode);
                SkRect r;
                r.set(x, y, x+w, y+h);

                SkPaint p;
                p.setStyle(SkPaint::kFill_Style);
                p.setShader(s);
                canvas->drawRect(r, p);

                canvas->saveLayer(&r, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
                draw_mode(canvas, mode, twice ? 0x88 : 0xFF, r.fLeft, r.fTop);
                canvas->restore();

                r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
                p.setStyle(SkPaint::kStroke_Style);
                p.setShader(NULL);
                canvas->drawRect(r, p);

#if 1
                canvas->drawText(gModes[i].fLabel, strlen(gModes[i].fLabel),
                                 x + w/2, y - labelP.getTextSize()/2, labelP);
#endif
                x += w + SkIntToScalar(10);
                if ((i % W) == W - 1) {
                    x = x0;
                    y += h + SkIntToScalar(30);
                }
            }
            x0 += SkIntToScalar(400);
        }
        s->unref();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new XfermodesGM; }
static GMRegistry reg(MyFactory);

}
