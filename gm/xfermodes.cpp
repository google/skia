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
    SkBitmap    fBitmap;
    SkBitmap    fBG;
    SkBitmap    fSrcB, fDstB;

    void draw_mode(SkCanvas* canvas, SkXfermode* mode, int alpha) {
        SkPaint p;
        
        canvas->drawBitmap(fSrcB, 0, 0, &p);        
        p.setAlpha(alpha);
        p.setXfermode(mode);
        canvas->drawBitmap(fDstB, 0, 0, &p);
    }
    
public:
	XfermodesGM() {
        const int W = 64;
        const int H = 64;
        
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, W, H);
        fBitmap.allocPixels();
        
        fBG.setConfig(SkBitmap::kARGB_4444_Config, 2, 2, 4);
        fBG.setPixels(gBG);
        fBG.setIsOpaque(true);
        
        make_bitmaps(W, H, &fSrcB, &fDstB);
    }
    
protected:
    SkString onShortName() {
        return SkString("xfermodes");
    }

	SkISize onISize() { return make_isize(640, 380); }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
        return;
        SkShader* s = SkShader::CreateBitmapShader(fBG,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode);
        SkPaint p;
        SkMatrix m;
        
        p.setShader(s)->unref();
        m.setScale(SkIntToScalar(8), SkIntToScalar(8));
        s->setLocalMatrix(m);
        canvas->drawPaint(p);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        const struct {
            SkPorterDuff::Mode  fMode;
            const char*         fLabel;
        } gModes[] = {
            { SkPorterDuff::kClear_Mode,    "Clear"     },
            { SkPorterDuff::kSrc_Mode,      "Src"       },
            { SkPorterDuff::kDst_Mode,      "Dst"       },
            { SkPorterDuff::kSrcOver_Mode,  "SrcOver"   },
            { SkPorterDuff::kDstOver_Mode,  "DstOver"   },
            { SkPorterDuff::kSrcIn_Mode,    "SrcIn"     },
            { SkPorterDuff::kDstIn_Mode,    "DstIn"     },
            { SkPorterDuff::kSrcOut_Mode,   "SrcOut"    },
            { SkPorterDuff::kDstOut_Mode,   "DstOut"    },
            { SkPorterDuff::kSrcATop_Mode,  "SrcATop"   },
            { SkPorterDuff::kDstATop_Mode,  "DstATop"   },
            { SkPorterDuff::kXor_Mode,      "Xor"       },
            { SkPorterDuff::kDarken_Mode,   "Darken"    },
            { SkPorterDuff::kLighten_Mode,  "Lighten"   },
            { SkPorterDuff::kMultiply_Mode, "Multiply"  },
            { SkPorterDuff::kScreen_Mode,   "Screen"    }
        };
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));
        
        SkCanvas c(fBitmap);
        const SkScalar w = SkIntToScalar(fBitmap.width());
        const SkScalar h = SkIntToScalar(fBitmap.height());
        SkShader* s = SkShader::CreateBitmapShader(fBG,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        s->setLocalMatrix(m);
        
        SkPaint labelP;
        labelP.setAntiAlias(true);
        labelP.setTextAlign(SkPaint::kCenter_Align);

        SkScalar x0 = 0;
        for (int twice = 0; twice < 2; twice++) {
            SkScalar x = x0, y = 0;
            for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
                SkXfermode* mode = SkPorterDuff::CreateXfermode(gModes[i].fMode);

                fBitmap.eraseColor(0);
                draw_mode(&c, mode, twice ? 0x88 : 0xFF);
                mode->safeUnref();
                
                SkPaint p;
                SkRect r;
                r.set(x, y, x+w, y+h);
                r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
                p.setStyle(SkPaint::kStroke_Style);
                canvas->drawRect(r, p);
                p.setStyle(SkPaint::kFill_Style);
                p.setShader(s);
                r.inset(SK_ScalarHalf, SK_ScalarHalf);
                canvas->drawRect(r, p);

                canvas->drawBitmap(fBitmap, x, y, NULL);

#if 1
                canvas->drawText(gModes[i].fLabel, strlen(gModes[i].fLabel),
                                 x + w/2, y - labelP.getTextSize()/2, labelP);
#endif
                x += w + SkIntToScalar(10);
                if ((i & 3) == 3) {
                    x = x0;
                    y += h + SkIntToScalar(30);
                }
            }
            x0 += SkIntToScalar(330);
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

