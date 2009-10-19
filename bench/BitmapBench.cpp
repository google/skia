#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkRandom.h"
#include "SkString.h"

static const char* gTileName[] = {
    "clamp", "repeat", "mirror"
};

static const char* gConfigName[] = {
    "ERROR", "a1", "a8", "index8", "565", "4444", "8888"
};

static void drawIntoBitmap(const SkBitmap& bm) {
    const int w = bm.width();
    const int h = bm.height();

    SkCanvas canvas(bm);
    SkPaint p;
    p.setAntiAlias(true);
    p.setColor(SK_ColorRED);
    canvas.drawCircle(SkIntToScalar(w)/2, SkIntToScalar(h)/2,
                      SkIntToScalar(SkMin32(w, h))*3/8, p);
    
    SkRect r;
    r.set(0, 0, SkIntToScalar(w), SkIntToScalar(h));
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(SkIntToScalar(4));
    p.setColor(SK_ColorBLUE);
    canvas.drawRect(r, p);
}

static int conv6ToByte(int x) {
    return x * 0xFF / 5;
}

static int convByteTo6(int x) {
    return x * 5 / 255;
}

static uint8_t compute666Index(SkPMColor c) {
    int r = SkGetPackedR32(c);
    int g = SkGetPackedG32(c);
    int b = SkGetPackedB32(c);
    
    return convByteTo6(r) * 36 + convByteTo6(g) * 6 + convByteTo6(b);
}

static void convertToIndex666(const SkBitmap& src, SkBitmap* dst) {
    SkColorTable* ctable = new SkColorTable(216);
    SkPMColor* colors = ctable->lockColors();
    // rrr ggg bbb
    for (int r = 0; r < 6; r++) {
        int rr = conv6ToByte(r);
        for (int g = 0; g < 6; g++) {
            int gg = conv6ToByte(g);
            for (int b = 0; b < 6; b++) {
                int bb = conv6ToByte(b);
                *colors++ = SkPreMultiplyARGB(0xFF, rr, gg, bb);
            }
        }
    }
    ctable->unlockColors(true);
    dst->setConfig(SkBitmap::kIndex8_Config, src.width(), src.height());
    dst->allocPixels(ctable);
    ctable->unref();
    
    SkAutoLockPixels alps(src);
    SkAutoLockPixels alpd(*dst);

    for (int y = 0; y < src.height(); y++) {
        const SkPMColor* srcP = src.getAddr32(0, y);
        uint8_t* dstP = dst->getAddr8(0, y);
        for (int x = src.width() - 1; x >= 0; --x) {
            *dstP++ = compute666Index(*srcP++);
        }
    }
}

/*  Variants for bitmaps
 
    - src depth (32 w+w/o alpha), 565, 4444, index, a8
    - paint options: filtering, dither, alpha
    - matrix options: translate, scale, rotate, persp
    - tiling: none, repeat, mirror, clamp
    
 */

class BitmapBench : public SkBenchmark {
    SkBitmap    fBitmap;
    SkPaint     fPaint;
    bool        fIsOpaque;
    int         fTileX, fTileY; // -1 means don't use shader
    SkString    fName;
    enum { N = 300 };
public:
    BitmapBench(void* param, bool isOpaque, SkBitmap::Config c,
                int tx = -1, int ty = -1)
        : INHERITED(param), fIsOpaque(isOpaque), fTileX(tx), fTileY(ty) {
        const int w = 128;
        const int h = 128;
        SkBitmap bm;

        if (SkBitmap::kIndex8_Config == c) {
            bm.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        } else {
            bm.setConfig(c, w, h);
        }
        bm.allocPixels();
        bm.eraseColor(isOpaque ? SK_ColorBLACK : 0);
        
        drawIntoBitmap(bm);

        if (SkBitmap::kIndex8_Config == c) {
            convertToIndex666(bm, &fBitmap);
        } else {
            fBitmap = bm;
        }

        if (fBitmap.getColorTable()) {
            fBitmap.getColorTable()->setIsOpaque(isOpaque);
        }
        fBitmap.setIsOpaque(isOpaque);
    }

protected:
    virtual const char* onGetName() {
        fName.set("bitmap");
        if (fTileX >= 0) {
            fName.appendf("_%s", gTileName[fTileX]);
            if (fTileY != fTileX) {
                fName.appendf("_%s", gTileName[fTileY]);
            }
        }
        fName.appendf("_%s%s", gConfigName[fBitmap.config()],
                      fIsOpaque ? "" : "_A");
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkIPoint dim = this->getSize();
        SkRandom rand;

        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        const SkBitmap& bitmap = fBitmap;
        const SkScalar x0 = SkIntToScalar(-bitmap.width() / 2);
        const SkScalar y0 = SkIntToScalar(-bitmap.height() / 2);
        
        for (int i = 0; i < N; i++) {
            SkScalar x = x0 + rand.nextUScalar1() * dim.fX;
            SkScalar y = y0 + rand.nextUScalar1() * dim.fY;
            canvas->drawBitmap(bitmap, x, y, &paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact0(void* p) { return new BitmapBench(p, false, SkBitmap::kARGB_8888_Config); }
static SkBenchmark* Fact1(void* p) { return new BitmapBench(p, true, SkBitmap::kARGB_8888_Config); }
static SkBenchmark* Fact2(void* p) { return new BitmapBench(p, true, SkBitmap::kRGB_565_Config); }
static SkBenchmark* Fact3(void* p) { return new BitmapBench(p, false, SkBitmap::kARGB_4444_Config); }
static SkBenchmark* Fact4(void* p) { return new BitmapBench(p, true, SkBitmap::kARGB_4444_Config); }
static SkBenchmark* Fact5(void* p) { return new BitmapBench(p, false, SkBitmap::kIndex8_Config); }
static SkBenchmark* Fact6(void* p) { return new BitmapBench(p, true, SkBitmap::kIndex8_Config); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
static BenchRegistry gReg4(Fact4);
static BenchRegistry gReg5(Fact5);
static BenchRegistry gReg6(Fact6);
