#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "Sk64.h"
#include "SkCornerPathEffect.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkKernel33MaskFilter.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

#include "SkStream.h"
#include "SkXMLParser.h"
#include "SkColorPriv.h"
#include "SkImageDecoder.h"

static int newscale(U8CPU a, U8CPU b, int shift) {
    unsigned prod = a * b + (1 << (shift - 1));
    return (prod + (prod >> shift)) >> shift;
}

static void test_srcover565(SkCanvas* canvas) {
    const int width = 32;
    SkBitmap bm1, bm2, bm3;
    bm1.setConfig(SkBitmap::kRGB_565_Config, width, 256); bm1.allocPixels(NULL);
    bm2.setConfig(SkBitmap::kRGB_565_Config, width, 256); bm2.allocPixels(NULL);
    bm3.setConfig(SkBitmap::kRGB_565_Config, width, 256); bm3.allocPixels(NULL);
    
    int rgb = 0x18;
    int r = rgb >> 3;
    int g = rgb >> 2;
    uint16_t dst = SkPackRGB16(r, g, r);
    for (int alpha = 0; alpha <= 255; alpha++) {
        SkPMColor pm = SkPreMultiplyARGB(alpha, rgb, rgb, rgb);
        uint16_t newdst = SkSrcOver32To16(pm, dst);
        sk_memset16(bm1.getAddr16(0, alpha), newdst, bm1.width());
        
        int ia = 255 - alpha;
        int iscale = SkAlpha255To256(ia);
        int dr = (SkGetPackedR32(pm) + (r * iscale >> 5)) >> 3;
        int dg = (SkGetPackedG32(pm) + (g * iscale >> 6)) >> 2;

        sk_memset16(bm2.getAddr16(0, alpha), SkPackRGB16(dr, dg, dr), bm2.width());

        int dr2 = (SkMulDiv255Round(alpha, rgb) + newscale(r, ia, 5)) >> 3;
        int dg2 = (SkMulDiv255Round(alpha, rgb) + newscale(g, ia, 6)) >> 2;
        
        sk_memset16(bm3.getAddr16(0, alpha), SkPackRGB16(dr2, dg2, dr2), bm3.width());

//        if (mr != dr || mg != dg)
        {
//            SkDebugf("[%d] macro [%d %d] inline [%d %d] new [%d %d]\n", alpha, mr, mg, dr, dg, dr2, dg2);
        }
    }
    
    SkScalar dx = SkIntToScalar(width+4);
    
    canvas->drawBitmap(bm1, 0, 0, NULL); canvas->translate(dx, 0);
    canvas->drawBitmap(bm2, 0, 0, NULL); canvas->translate(dx, 0);
    canvas->drawBitmap(bm3, 0, 0, NULL); canvas->translate(dx, 0);
    
    SkRect rect = { 0, 0, SkIntToScalar(bm1.width()), SkIntToScalar(bm1.height()) };
    SkPaint p;
    p.setARGB(0xFF, rgb, rgb, rgb);
    canvas->drawRect(rect, p);
}

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

class XfermodesView : public SkView {
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
	XfermodesView() {
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
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Xfermodes");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

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
        
        if (false) {
            test_srcover565(canvas);
        }
        
        if (false) {
            SkPaint paint;
            paint.setFlags(0x43);
            paint.setTextSize(SkIntToScalar(128));
            SkMatrix matrix;
            matrix.reset();
            matrix.set(0, 0x0019d049);
            matrix.set(2, 0x712cf400);
            matrix.set(4, 0x0019c96f);
            matrix.set(5, 0xF8d76598);
            canvas->concat(matrix);
            canvas->drawText("HamburgefonsHamburgefonsHamburgefonsHamburgefons",
                             48, 0, 0, paint);
            return;
        }

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

                canvas->drawText(gModes[i].fLabel, strlen(gModes[i].fLabel),
                                 x + w/2, y - labelP.getTextSize()/2, labelP);

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
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new XfermodesView; }
static SkViewRegister reg(MyFactory);

