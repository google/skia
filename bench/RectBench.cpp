#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkString.h"

class RectBench : public SkBenchmark {
public:
    int fShift;
    enum {
        W = 640,
        H = 480,
        N = 300
    };
    SkRect  fRects[N];
    SkColor fColors[N];

    RectBench(int shift) : fShift(shift) {
        SkRandom rand;
        for (int i = 0; i < N; i++) {
            int x = rand.nextU() % W;
            int y = rand.nextU() % H;
            int w = rand.nextU() % W;
            int h = rand.nextU() % H;
            w >>= shift;
            h >>= shift;
            x -= w/2;
            y -= h/2;
            fRects[i].set(SkIntToScalar(x), SkIntToScalar(y),
                          SkIntToScalar(x+w), SkIntToScalar(y+h));
            fColors[i] = rand.nextU() | 0xFF808080;
        }
    }
    
    SkString fName;
    const char* computeName(const char root[]) {
        fName.set(root);
        fName.appendS32(fShift);
        return fName.c_str();
    }
        
protected:
    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawRect(r, p);
    }

    virtual const char* onGetName() { return computeName("rects"); }
    virtual SkIPoint onGetSize() { return SkMakeIPoint(640, 480); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        for (int i = 0; i < N; i++) {
            paint.setColor(fColors[i]);
            this->setupPaint(&paint);
            this->drawThisRect(canvas, fRects[i], paint);
        }
    }
};

class OvalBench : public RectBench {
public:
    OvalBench(int shift) : RectBench(shift) {}
protected:
    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawOval(r, p);
    }
    virtual const char* onGetName() { return computeName("ovals"); }
};

class RRectBench : public RectBench {
public:
    RRectBench(int shift) : RectBench(shift) {}
protected:
    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawRoundRect(r, r.width() / 4, r.height() / 4, p);
    }
    virtual const char* onGetName() { return computeName("rrects"); }
};

class PointsBench : public RectBench {
public:
    SkCanvas::PointMode fMode;
    const char* fName;

    PointsBench(SkCanvas::PointMode mode, const char* name) : 
        RectBench(2), fMode(mode) {
        fName = name;
    }

protected:
    virtual void onDraw(SkCanvas* canvas) {
        static const SkScalar gSizes[] = {
            SkIntToScalar(7), 0
        };

        SkPaint paint;
        paint.setStrokeCap(SkPaint::kRound_Cap);
        
        for (size_t i = 0; i < SK_ARRAY_COUNT(gSizes); i++) {
            paint.setStrokeWidth(gSizes[i]);
            this->setupPaint(&paint);
            canvas->drawPoints(fMode, N * 2,
                               reinterpret_cast<const SkPoint*>(fRects), paint);
            paint.setColor(fColors[i]);
        }
    }
    virtual const char* onGetName() { return fName; }
};

static SkBenchmark* RectFactory1() { return SkNEW_ARGS(RectBench, (1)); }
static SkBenchmark* RectFactory2() { return SkNEW_ARGS(RectBench, (3)); }
static SkBenchmark* OvalFactory1() { return SkNEW_ARGS(OvalBench, (1)); }
static SkBenchmark* OvalFactory2() { return SkNEW_ARGS(OvalBench, (3)); }
static SkBenchmark* RRectFactory1() { return SkNEW_ARGS(RRectBench, (1)); }
static SkBenchmark* RRectFactory2() { return SkNEW_ARGS(RRectBench, (3)); }
static SkBenchmark* PointsFactory() {
    return SkNEW_ARGS(PointsBench, (SkCanvas::kPoints_PointMode, "points"));
}
static SkBenchmark* LinesFactory() {
    return SkNEW_ARGS(PointsBench, (SkCanvas::kLines_PointMode, "lines"));
}
static SkBenchmark* PolygonFactory() {
    return SkNEW_ARGS(PointsBench, (SkCanvas::kPolygon_PointMode, "polygon"));
}

static SkTRegistry<SkBenchmark> gRectReg1(RectFactory1);
static SkTRegistry<SkBenchmark> gRectReg2(RectFactory2);
static SkTRegistry<SkBenchmark> gOvalReg1(OvalFactory1);
static SkTRegistry<SkBenchmark> gOvalReg2(OvalFactory2);
static SkTRegistry<SkBenchmark> gRRectReg1(RRectFactory1);
static SkTRegistry<SkBenchmark> gRRectReg2(RRectFactory2);
static SkTRegistry<SkBenchmark> gPointsReg(PointsFactory);
static SkTRegistry<SkBenchmark> gLinesReg(LinesFactory);
static SkTRegistry<SkBenchmark> gPolygonReg(PolygonFactory);

