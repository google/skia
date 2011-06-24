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

    /* The sourceType argument indicates what to draw for the source part. Skia
     * uses the implied shape of the drawing command and these modes
     * demonstrate that.
     *
     * 0x01: A WxH image with a rectangle in the lower right.
     * 0x02: Same as 0x01, but with an alpha of 34.5%.
     * 0x04: Same as 0x02, but scaled down by half.
     * 0x08: The same rectangle as 0x01, but drawn directly instead in an image.
     * 0x10: Two rectangles, first on the right half, second on the bottom half.
     * 0x20: Same as 0x10, but on a layer.
     */
    void draw_mode(SkCanvas* canvas, SkXfermode* mode, int sourceType,
                   SkScalar x, SkScalar y) {
        SkPaint p;
        SkMatrix m;
        bool restoreNeeded = false;
        m.setTranslate(x, y);

        canvas->drawBitmapMatrix(fSrcB, m, &p);
        p.setXfermode(mode);
        switch (sourceType) {
            case 0x20: {
                SkRect bounds = SkRect::MakeXYWH(x, y, W, H);
                canvas->saveLayer(&bounds, &p);
                restoreNeeded = true;
                p.setXfermodeMode(SkXfermode::kSrcOver_Mode);
                // Fall through.
            }
            case 0x10: {
                SkScalar halfW = SkIntToScalar(W) / 2;
                SkScalar halfH = SkIntToScalar(H) / 2;
                p.setColor(0xFF66AAFF);
                SkRect r = SkRect::MakeXYWH(x + halfW, y, halfW, H);
                canvas->drawRect(r, p);
                p.setColor(0xFFAA66FF);
                r = SkRect::MakeXYWH(x, y + halfH, W, halfH);
                canvas->drawRect(r, p);
                break;
            }
            case 0x8: {
                SkScalar w = SkIntToScalar(W);
                SkScalar h = SkIntToScalar(H);
                SkRect r = SkRect::MakeXYWH(x + w / 3, y + h / 3,
                                            w * 37 / 60, h * 37 / 60);
                p.setColor(0xFF66AAFF);
                canvas->drawRect(r, p);
                break;
            }
            case 0x4:
                m.postScale(SK_ScalarHalf, SK_ScalarHalf, x, y);
                // Fall through.
            case 0x2:
                p.setAlpha(0x88);
                // Fall through.
            case 0x1:
                canvas->drawBitmapMatrix(fDstB, m, &p);
                break;
            default:
                break;
        }

        if (restoreNeeded) {
            canvas->restore();
        }
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
        return make_isize(1590, 640);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        this->drawBG(canvas);

        const int kMaxSourceType = 0x20;
        const struct {
            SkXfermode::Mode  fMode;
            const char*       fLabel;
            int               fSourceTypeMask;  // The source types to use this
                                                // mode with. See draw_mode for
                                                // an explanation of each type.
                                                // PDF has to play some tricks
                                                // to support the base modes,
                                                // test those more extensively.
        } gModes[] = {
            { SkXfermode::kClear_Mode,       "Clear",        0x3F  },
            { SkXfermode::kSrc_Mode,         "Src",          0x3F  },
            { SkXfermode::kDst_Mode,         "Dst",          0x3F  },
            { SkXfermode::kSrcOver_Mode,     "SrcOver",      0x3F  },
            { SkXfermode::kDstOver_Mode,     "DstOver",      0x3F  },
            { SkXfermode::kSrcIn_Mode,       "SrcIn",        0x3F  },
            { SkXfermode::kDstIn_Mode,       "DstIn",        0x3F  },
            { SkXfermode::kSrcOut_Mode,      "SrcOut",       0x3F  },
            { SkXfermode::kDstOut_Mode,      "DstOut",       0x3F  },
            { SkXfermode::kSrcATop_Mode,     "SrcATop",      0x3F  },
            { SkXfermode::kDstATop_Mode,     "DstATop",      0x3F  },

            { SkXfermode::kXor_Mode,          "Xor",         0x03  },
            { SkXfermode::kPlus_Mode,         "Plus",        0x03  },
            { SkXfermode::kMultiply_Mode,     "Multiply",    0x3F  },
            { SkXfermode::kScreen_Mode,       "Screen",      0x03  },
            { SkXfermode::kOverlay_Mode,      "Overlay",     0x03  },
            { SkXfermode::kDarken_Mode,       "Darken",      0x03  },
            { SkXfermode::kLighten_Mode,      "Lighten",     0x03  },
            { SkXfermode::kColorDodge_Mode,   "ColorDodge",  0x03  },
            { SkXfermode::kColorBurn_Mode,    "ColorBurn",   0x03  },
            { SkXfermode::kHardLight_Mode,    "HardLight",   0x03  },
            { SkXfermode::kSoftLight_Mode,    "SoftLight",   0x03  },
            { SkXfermode::kDifference_Mode,   "Difference",  0x03  },
            { SkXfermode::kExclusion_Mode,    "Exclusion",   0x03  },
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
        SkScalar y0 = 0;
        for (int sourceType = 1; sourceType <= kMaxSourceType; sourceType *=2) {
            SkScalar x = x0, y = y0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
                if ((gModes[i].fSourceTypeMask & sourceType) == 0) {
                    continue;
                }
                SkXfermode* mode = SkXfermode::Create(gModes[i].fMode);
                SkAutoUnref aur(mode);
                SkRect r;
                r.set(x, y, x+w, y+h);

                SkPaint p;
                p.setStyle(SkPaint::kFill_Style);
                p.setShader(s);
                canvas->drawRect(r, p);

                canvas->saveLayer(&r, NULL, SkCanvas::kARGB_ClipLayer_SaveFlag);
                draw_mode(canvas, mode, sourceType, r.fLeft, r.fTop);
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
            if (y < 320) {
                if (x > x0) {
                    y += h + SkIntToScalar(30);
                }
                y0 = y;
            } else {
                x0 += SkIntToScalar(400);
                y0 = 0;
            }
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
