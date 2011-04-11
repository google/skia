#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkString.h"

enum Flags {
    kStroke_Flag = 1 << 0,
    kBig_Flag    = 1 << 1
};

#define FLAGS00  Flags(0)
#define FLAGS01  Flags(kStroke_Flag)
#define FLAGS10  Flags(kBig_Flag)
#define FLAGS11  Flags(kStroke_Flag | kBig_Flag)

class PathBench : public SkBenchmark {
    SkPaint     fPaint;
    SkString    fName;
    Flags       fFlags;
    enum { N = 1000 };
public:
    PathBench(void* param, Flags flags) : INHERITED(param), fFlags(flags) {
        fPaint.setStyle(flags & kStroke_Flag ? SkPaint::kStroke_Style :
                        SkPaint::kFill_Style);
        fPaint.setStrokeWidth(SkIntToScalar(5));
        fPaint.setStrokeJoin(SkPaint::kBevel_Join);
    }

    virtual void appendName(SkString*) = 0;
    virtual void makePath(SkPath*) = 0;
    virtual bool iscomplex() { return false; }

protected:
    virtual const char* onGetName() {
        fName.printf("path_%s_%s_",
                     fFlags & kStroke_Flag ? "stroke" : "fill",
                     fFlags & kBig_Flag ? "big" : "small");
        this->appendName(&fName);
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint(fPaint);
        this->setupPaint(&paint);

        SkPath path;
        this->makePath(&path);
        if (fFlags & kBig_Flag) {
            SkMatrix m;
            m.setScale(SkIntToScalar(10), SkIntToScalar(10));
            path.transform(m);
        }

        int count = N;
        if (fFlags & kBig_Flag) {
            count >>= 2;
        }
        if (this->iscomplex()) {
            count >>= 3;
        }

        for (int i = 0; i < count; i++) {
            canvas->drawPath(path, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class TrianglePathBench : public PathBench {
public:
    TrianglePathBench(void* param, Flags flags) : INHERITED(param, flags) {}
    
    virtual void appendName(SkString* name) {
        name->append("triangle");
    }
    virtual void makePath(SkPath* path) {
        static const int gCoord[] = {
            10, 10, 15, 5, 20, 20
        };
        path->moveTo(SkIntToScalar(gCoord[0]), SkIntToScalar(gCoord[1]));
        path->lineTo(SkIntToScalar(gCoord[2]), SkIntToScalar(gCoord[3]));
        path->lineTo(SkIntToScalar(gCoord[4]), SkIntToScalar(gCoord[5]));
        path->close();
    }
private:
    typedef PathBench INHERITED;
};

class RectPathBench : public PathBench {
public:
    RectPathBench(void* param, Flags flags) : INHERITED(param, flags) {}
    
    virtual void appendName(SkString* name) {
        name->append("rect");
    }
    virtual void makePath(SkPath* path) {
        SkRect r = { 10, 10, 20, 20 };
        path->addRect(r);
    }
private:
    typedef PathBench INHERITED;
};

class OvalPathBench : public PathBench {
public:
    OvalPathBench(void* param, Flags flags) : INHERITED(param, flags) {}
    
    virtual void appendName(SkString* name) {
        name->append("oval");
    }
    virtual void makePath(SkPath* path) {
        SkRect r = { 10, 10, 20, 20 };
        path->addOval(r);
    }
private:
    typedef PathBench INHERITED;
};

class SawToothPathBench : public PathBench {
public:
    SawToothPathBench(void* param, Flags flags) : INHERITED(param, flags) {}
    
    virtual void appendName(SkString* name) {
        name->append("sawtooth");
    }
    virtual void makePath(SkPath* path) {
        SkScalar x = SkIntToScalar(20);
        SkScalar y = SkIntToScalar(20);
        const SkScalar x0 = x;
        const SkScalar dx = SK_Scalar1 * 5;
        const SkScalar dy = SK_Scalar1 * 10;

        path->moveTo(x, y);
        for (int i = 0; i < 32; i++) {
            x += dx;
            path->lineTo(x, y - dy);
            x += dx;
            path->lineTo(x, y + dy);
        }
        path->lineTo(x, y + 2 * dy);
        path->lineTo(x0, y + 2 * dy);
        path->close();
    }
    virtual bool iscomplex() { return true; }
private:
    typedef PathBench INHERITED;
};

static SkBenchmark* FactT00(void* p) { return new TrianglePathBench(p, FLAGS00); }
static SkBenchmark* FactT01(void* p) { return new TrianglePathBench(p, FLAGS01); }
static SkBenchmark* FactT10(void* p) { return new TrianglePathBench(p, FLAGS10); }
static SkBenchmark* FactT11(void* p) { return new TrianglePathBench(p, FLAGS11); }

static SkBenchmark* FactR00(void* p) { return new RectPathBench(p, FLAGS00); }
static SkBenchmark* FactR01(void* p) { return new RectPathBench(p, FLAGS01); }
static SkBenchmark* FactR10(void* p) { return new RectPathBench(p, FLAGS10); }
static SkBenchmark* FactR11(void* p) { return new RectPathBench(p, FLAGS11); }

static SkBenchmark* FactO00(void* p) { return new OvalPathBench(p, FLAGS00); }
static SkBenchmark* FactO01(void* p) { return new OvalPathBench(p, FLAGS01); }
static SkBenchmark* FactO10(void* p) { return new OvalPathBench(p, FLAGS10); }
static SkBenchmark* FactO11(void* p) { return new OvalPathBench(p, FLAGS11); }

static SkBenchmark* FactS00(void* p) { return new SawToothPathBench(p, FLAGS00); }
static SkBenchmark* FactS01(void* p) { return new SawToothPathBench(p, FLAGS01); }

static BenchRegistry gRegT00(FactT00);
static BenchRegistry gRegT01(FactT01);
static BenchRegistry gRegT10(FactT10);
static BenchRegistry gRegT11(FactT11);

static BenchRegistry gRegR00(FactR00);
static BenchRegistry gRegR01(FactR01);
static BenchRegistry gRegR10(FactR10);
static BenchRegistry gRegR11(FactR11);

static BenchRegistry gRegO00(FactO00);
static BenchRegistry gRegO01(FactO01);
static BenchRegistry gRegO10(FactO10);
static BenchRegistry gRegO11(FactO11);

static BenchRegistry gRegS00(FactS00);
static BenchRegistry gRegS01(FactS01);

