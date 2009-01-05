#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"

class RectBench : public SkBenchmark {
public:
    enum {
        W = 640,
        H = 480,
        N = 100
    };
    SkRect  fRects[N];
    SkColor fColors[N];

    RectBench() {
        SkRandom rand;
        for (int i = 0; i < N; i++) {
            int x = rand.nextU() % W;
            int y = rand.nextU() % H;
            int w = rand.nextU() % W;
            int h = rand.nextU() % H;
            w >>= 1;
            h >>= 1;
            x -= w/2;
            y -= h/2;
            fRects[i].set(SkIntToScalar(x), SkIntToScalar(y),
                          SkIntToScalar(x+w), SkIntToScalar(y+h));
            fColors[i] = rand.nextU() | 0xFF808080;
        }
    }
        
protected:
    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawRect(r, p);
    }

    virtual const char* onGetName() { return "rectangles"; }
    virtual SkIPoint onGetSize() { return SkMakeIPoint(640, 480); }
    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        for (int i = 0; i < N; i++) {
            paint.setColor(fColors[i]);
            this->drawThisRect(canvas, fRects[i], paint);
        }
    }
};

class OvalBench : public RectBench {
protected:
    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawOval(r, p);
    }
    virtual const char* onGetName() { return "ovals"; }
};

class RRectBench : public RectBench {
protected:
    virtual void drawThisRect(SkCanvas* c, const SkRect& r, const SkPaint& p) {
        c->drawRoundRect(r, r.width() / 4, r.height() / 4, p);
    }
    virtual const char* onGetName() { return "roundrects"; }
};

class PointsBench : public RectBench {
public:
    SkCanvas::PointMode fMode;
    const char* fName;

    PointsBench(SkCanvas::PointMode mode, const char* name) : fMode(mode) {
        fName = name;
    }

protected:
    virtual void onDraw(SkCanvas* canvas) {
        static const SkScalar gSizes[] = {
            SkIntToScalar(7), 0
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeCap(SkPaint::kRound_Cap);
        
        for (size_t i = 0; i < SK_ARRAY_COUNT(gSizes); i++) {
            paint.setStrokeWidth(gSizes[i]);
            canvas->drawPoints(fMode, N * 2,
                               reinterpret_cast<const SkPoint*>(fRects), paint);
            paint.setColor(fColors[i]);
        }
    }
    virtual const char* onGetName() { return fName; }
};

static SkBenchmark* RectFactory() { return SkNEW(RectBench); }
static SkBenchmark* OvalFactory() { return SkNEW(OvalBench); }
static SkBenchmark* RRectFactory() { return SkNEW(RRectBench); }
static SkBenchmark* PointsFactory() {
    return SkNEW_ARGS(PointsBench, (SkCanvas::kPoints_PointMode, "points"));
}
static SkBenchmark* LinesFactory() {
    return SkNEW_ARGS(PointsBench, (SkCanvas::kLines_PointMode, "lines"));
}
static SkBenchmark* PolygonFactory() {
    return SkNEW_ARGS(PointsBench, (SkCanvas::kPolygon_PointMode, "polygon"));
}

static SkTRegistry<SkBenchmark> gRectReg(RectFactory);
static SkTRegistry<SkBenchmark> gOvalReg(OvalFactory);
static SkTRegistry<SkBenchmark> gRRectReg(RRectFactory);
static SkTRegistry<SkBenchmark> gPointsReg(PointsFactory);
static SkTRegistry<SkBenchmark> gLinesReg(LinesFactory);
static SkTRegistry<SkBenchmark> gPolygonReg(PolygonFactory);

