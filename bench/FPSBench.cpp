#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"

class FPSBench : public SkBenchmark {
    int32_t fWidth;
    int32_t fHeight;
public:
    FPSBench(void* p) : INHERITED(p) {
        fWidth = 640;
        (void)this->findDefine32("width", &fWidth);
        fHeight = 480;
        (void)this->findDefine32("height", &fHeight);
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }

protected:
    virtual SkIPoint onGetSize() { return SkIPoint::Make(fWidth, fHeight); }

private:
    typedef SkBenchmark INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class Color_FPSBench : public FPSBench {
public:
    Color_FPSBench(void* p, SkColor c, const char name[]) : INHERITED(p) {
        fColor = c;
        fName = name;
    }
    
protected:
    virtual const char* onGetName() { return fName; }
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(fColor);
    }
    
private:
    const char* fName;
    SkColor     fColor;
    
    typedef FPSBench INHERITED;
};

class Bitmap_FPSBench : public FPSBench {
public:
    Bitmap_FPSBench(void* p, SkBitmap::Config config, bool doOpaque, bool doScale) : INHERITED(p) {
        fBitmap.setConfig(config, this->width(), this->height());
        fBitmap.allocPixels();
        fBitmap.eraseColor(0xFFFF0000);
        if (doOpaque) {
            fBitmap.setIsOpaque(true);
        }

        const char* configStr = "565";
        if (config == SkBitmap::kARGB_8888_Config) {
            if (doOpaque) {
                configStr = "X888";
            } else {
                configStr = "8888";
            }
        }
        fName.printf("fps_bitmap_%s_%s", configStr,
                     doScale ? "scale" : "noscale");

        fMatrix.reset();
        if (doScale) {
            fMatrix.setScale(SkIntToScalar(3)/2, SkIntToScalar(3)/2);
        }
    }
    
protected:
    virtual const char* onGetName() { return fName.c_str(); }
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawBitmapMatrix(fBitmap, fMatrix);
    }
    
private:
    SkBitmap    fBitmap;
    SkMatrix    fMatrix;
    SkString    fName;
    
    typedef FPSBench INHERITED;
};

static SkBenchmark* FillFactory(void* p) { return SkNEW_ARGS(Color_FPSBench, (p, 0xFFFF0000, "fps_fill")); }
static SkBenchmark* BlendFactory(void* p) { return SkNEW_ARGS(Color_FPSBench, (p, 0x80FF0000, "fps_blend")); }
static SkBenchmark* BMFactory0(void* p) { return SkNEW_ARGS(Bitmap_FPSBench, (p, SkBitmap::kARGB_8888_Config, false, false)); }
static SkBenchmark* BMFactory1(void* p) { return SkNEW_ARGS(Bitmap_FPSBench, (p, SkBitmap::kARGB_8888_Config, false, true)); }
static SkBenchmark* BMFactory2(void* p) { return SkNEW_ARGS(Bitmap_FPSBench, (p, SkBitmap::kARGB_8888_Config, true, false)); }
static SkBenchmark* BMFactory3(void* p) { return SkNEW_ARGS(Bitmap_FPSBench, (p, SkBitmap::kARGB_8888_Config, true, true)); }
static SkBenchmark* BMFactory4(void* p) { return SkNEW_ARGS(Bitmap_FPSBench, (p, SkBitmap::kRGB_565_Config, false, false)); }
static SkBenchmark* BMFactory5(void* p) { return SkNEW_ARGS(Bitmap_FPSBench, (p, SkBitmap::kRGB_565_Config, false, true)); }

static BenchRegistry gFillReg(FillFactory);
static BenchRegistry gBlendReg(BlendFactory);
static BenchRegistry gBMReg0(BMFactory0);
static BenchRegistry gBMReg1(BMFactory1);
static BenchRegistry gBMReg2(BMFactory2);
static BenchRegistry gBMReg3(BMFactory3);
static BenchRegistry gBMReg4(BMFactory4);
static BenchRegistry gBMReg5(BMFactory5);

