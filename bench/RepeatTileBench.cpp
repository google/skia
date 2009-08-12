#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkString.h"

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

class RepeatTileBench : public SkBenchmark {
    SkPaint     fPaint;
    SkString    fName;
    enum { N = 20 };
public:
    RepeatTileBench(SkBitmap::Config c) {
        const int w = 50;
        const int h = 50;
        SkBitmap bm;

        if (SkBitmap::kIndex8_Config == c) {
            bm.setConfig(SkBitmap::kARGB_8888_Config, w, h);
        } else {
            bm.setConfig(c, w, h);
        }
        bm.allocPixels();
        bm.eraseColor(0);
        
        drawIntoBitmap(bm);

        if (SkBitmap::kIndex8_Config == c) {
            SkBitmap tmp;
            convertToIndex666(bm, &tmp);
            bm = tmp;
        }

        SkShader* s = SkShader::CreateBitmapShader(bm,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode);
        fPaint.setShader(s)->unref();
        fName.printf("repeatTile_%s", gConfigName[bm.config()]);
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        for (int i = 0; i < N; i++) {
            canvas->drawPaint(paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact0(void*) { return new RepeatTileBench(SkBitmap::kARGB_8888_Config); }
static SkBenchmark* Fact1(void*) { return new RepeatTileBench(SkBitmap::kRGB_565_Config); }
static SkBenchmark* Fact2(void*) { return new RepeatTileBench(SkBitmap::kARGB_4444_Config); }
static SkBenchmark* Fact3(void*) { return new RepeatTileBench(SkBitmap::kIndex8_Config); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
static BenchRegistry gReg3(Fact3);
