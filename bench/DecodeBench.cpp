#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "SkString.h"

static const char* gConfigName[] = {
    "ERROR", "a1", "a8", "index8", "565", "4444", "8888"
};

class DecodeBench : public SkBenchmark {
    const char* fFilename;
    SkBitmap::Config fPrefConfig;
    SkString fName;
    enum { N = 10 };
public:
    DecodeBench(void* param, SkBitmap::Config c) : SkBenchmark(param) {
        fFilename = this->findDefine("decode-filename");
        fPrefConfig = c;
        
        const char* fname = NULL;
        if (fFilename) {
            fname = strrchr(fFilename, '/');
            if (fname) {
                fname += 1; // skip the slash
            }
        }
        fName.printf("decode_%s_%s", gConfigName[c], fname);
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (fFilename) {
            for (int i = 0; i < N; i++) {
                SkBitmap bm;
                SkImageDecoder::DecodeFile(fFilename, &bm, fPrefConfig,
                                           SkImageDecoder::kDecodePixels_Mode);
            }
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

static SkBenchmark* Fact0(void* p) { return new DecodeBench(p, SkBitmap::kARGB_8888_Config); }
static SkBenchmark* Fact1(void* p) { return new DecodeBench(p, SkBitmap::kRGB_565_Config); }
static SkBenchmark* Fact2(void* p) { return new DecodeBench(p, SkBitmap::kARGB_4444_Config); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);
