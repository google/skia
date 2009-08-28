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

static SkRandom gRand;

static void test_chromium_9005() {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, 800, 600);
    bm.allocPixels();

    SkCanvas canvas(bm);

    SkPoint pt0 = { SkFloatToScalar(799.33374f), SkFloatToScalar(1.2360189f) };
    SkPoint pt1 = { SkFloatToScalar(808.49969f), SkFloatToScalar(-7.4338055f) };
    
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawLine(pt0.fX, pt0.fY, pt1.fX, pt1.fY, paint);
}

static void generate_pts(SkPoint pts[], int count, int w, int h) {
    for (int i = 0; i < count; i++) {
        pts[i].set(gRand.nextUScalar1() * 3 * w - SkIntToScalar(w),
                   gRand.nextUScalar1() * 3 * h - SkIntToScalar(h));
    }
}

static bool check_zeros(const SkPMColor pixels[], int count, int skip) {
    for (int i = 0; i < count; i++) {
        if (*pixels) {
            return false;
        }
        pixels += skip;
    }
    return true;
}

static bool check_bitmap_margin(const SkBitmap& bm, int margin) {
    size_t rb = bm.rowBytes();
    for (int i = 0; i < margin; i++) {
        if (!check_zeros(bm.getAddr32(0, i), bm.width(), 1)) {
            return false;
        }
        int bottom = bm.height() - i - 1;
        if (!check_zeros(bm.getAddr32(0, bottom), bm.width(), 1)) {
            return false;
        }
        // left column
        if (!check_zeros(bm.getAddr32(i, 0), bm.height(), rb >> 2)) {
            return false;
        }
        int right = bm.width() - margin + i;
        if (!check_zeros(bm.getAddr32(right, 0), bm.height(), rb >> 2)) {
            return false;
        }
    }
    return true;
}

#define WIDTH   620
#define HEIGHT  460
#define MARGIN  10

static void line_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 2;
    SkPoint pts[N];
    for (int i = 0; i < 400; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);

        canvas->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, paint);
        if (!check_bitmap_margin(bm, MARGIN)) {
            SkDebugf("---- hairline failure (%g %g) (%g %g)\n",
                     pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY);
            break;
        }
    }
}

static void poly_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 8;
    SkPoint pts[N];
    for (int i = 0; i < 50; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);
        
        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N; j++) {
            path.lineTo(pts[j]);
        }
        canvas->drawPath(path, paint);
    }
}

static SkPoint ave(const SkPoint& a, const SkPoint& b) {
    SkPoint c = a + b;
    c.fX = SkScalarHalf(c.fX);
    c.fY = SkScalarHalf(c.fY);
    return c;
}

static void quad_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 30;
    SkPoint pts[N];
    for (int i = 0; i < 10; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);
        
        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N - 2; j++) {
            path.quadTo(pts[j], ave(pts[j], pts[j+1]));
        }
        path.quadTo(pts[N - 2], pts[N - 1]);
        
        canvas->drawPath(path, paint);
    }
}

static void add_cubic(SkPath* path, const SkPoint& mid, const SkPoint& end) {
    SkPoint start;
    path->getLastPt(&start);
    path->cubicTo(ave(start, mid), ave(mid, end), end);
}

static void cube_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 30;
    SkPoint pts[N];
    for (int i = 0; i < 10; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);
        
        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N - 2; j++) {
            add_cubic(&path, pts[j], ave(pts[j], pts[j+1]));
        }
        add_cubic(&path, pts[N - 2], pts[N - 1]);
        
        canvas->drawPath(path, paint);
    }
}

typedef void (*HairProc)(SkCanvas*, const SkPaint&, const SkBitmap&);

static const struct {
    const char* fName;
    HairProc    fProc;
} gProcs[] = {
    { "line",   line_proc },
    { "poly",   poly_proc },
    { "quad",   quad_proc },
    { "cube",   cube_proc },
};

static int cycle_hairproc_index(int index) {
    return (index + 1) % SK_ARRAY_COUNT(gProcs);
}

class HairlineView : public SkView {
    int fProcIndex;
    bool fDoAA;
public:
	HairlineView() {
        fProcIndex = 0;
        fDoAA = true;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString str;
            str.printf("Hair-%s", gProcs[fProcIndex].fName);
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void show_bitmaps(SkCanvas* canvas, const SkBitmap& b0, const SkBitmap& b1,
                      const SkIRect& inset) {
        canvas->drawBitmap(b0, 0, 0, NULL);
        canvas->drawBitmap(b1, SkIntToScalar(b0.width()), 0, NULL);
    }

    void drawBG(SkCanvas* canvas) {
//        canvas->drawColor(0xFFDDDDDD);
        canvas->drawColor(SK_ColorWHITE);
   //     canvas->drawColor(SK_ColorBLACK);
    }

    int fCounter;

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        if (true) {
            test_chromium_9005();
        }
        
        SkBitmap bm, bm2;
        bm.setConfig(SkBitmap::kARGB_8888_Config,
                     WIDTH + MARGIN*2,
                     HEIGHT + MARGIN*2);
        bm.allocPixels();
        // this will erase our margin, which we want to always stay 0
        bm.eraseColor(0);

        bm2.setConfig(SkBitmap::kARGB_8888_Config, WIDTH, HEIGHT,
                      bm.rowBytes());
        bm2.setPixels(bm.getAddr32(MARGIN, MARGIN));
        
        SkCanvas c2(bm2);
        SkPaint paint;
        paint.setAntiAlias(fDoAA);
        paint.setStyle(SkPaint::kStroke_Style);

        bm2.eraseColor(0);
        gProcs[fProcIndex].fProc(&c2, paint, bm);
        canvas->drawBitmap(bm2, SkIntToScalar(10), SkIntToScalar(10), NULL);
        
        fCounter += 1;
        fDoAA = !fDoAA;
        if (fCounter > 50) {
            fProcIndex = cycle_hairproc_index(fProcIndex);
            // todo: signal that we want to rebuild our TITLE
            fCounter = 0;
        }
        this->inval(NULL);
    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fDoAA = !fDoAA;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new HairlineView; }
static SkViewRegister reg(MyFactory);

