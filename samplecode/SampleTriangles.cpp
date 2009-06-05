#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkConcaveToTriangles.h"

#define SIZE    SkIntToScalar(150)

typedef void (*PathProc)(SkPath*);

static void make_path0(SkPath* path) {
    SkRect r;
    r.set(0, 0, SIZE, SIZE);
    path->addRect(r);
}

static void make_path1(SkPath* path) {
    SkRect r;
    r.set(0, 0, SIZE, SIZE);
    path->addRoundRect(r, SIZE/4, SIZE/4);
}

static void make_path2(SkPath* path) {
    SkRect r;
    r.set(0, 0, SIZE, SIZE);
    path->addOval(r);
}

static const PathProc gProcs[] = {
    make_path0,
    make_path1,
    make_path2,
};

#define COUNT_PROCS SK_ARRAY_COUNT(gProcs)

class TriangleView : public SkView {
public:
    SkPath fPaths[COUNT_PROCS];

	TriangleView() {
        for (size_t i = 0; i < COUNT_PROCS; i++) {
            gProcs[i](&fPaths[i]);
        }
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Triangles");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorGRAY);
    }
    
    static void draw_path(SkCanvas* canvas, const SkPaint& pathPaint,
                          const SkPath& path, const SkPaint& triPaint) {
        canvas->drawPath(path, pathPaint);
        
        int n = path.getPoints(NULL, 0);
        SkPoint* pts = new SkPoint[n];
        path.getPoints(pts, n);
        
        SkTDArray<SkPoint> triangles;
        if (SkConcaveToTriangles(n, pts, &triangles)) {
            canvas->drawVertices(SkCanvas::kTriangles_VertexMode,
                                 triangles.count(), triangles.begin(), NULL,
                                 NULL, NULL, NULL, 0, triPaint);
        }
        
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        paint.setStrokeWidth(SkIntToScalar(4));
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, paint);
        delete[] pts;
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        canvas->translate(SIZE/2, SIZE/2);

        SkPaint pathPaint, triPaint;
        
        pathPaint.setColor(SK_ColorBLUE);
        pathPaint.setStrokeWidth(SIZE / 12);

        triPaint.setColor(SK_ColorRED);
        triPaint.setStyle(SkPaint::kStroke_Style);

        for (size_t i = 0; i < COUNT_PROCS; i++) {
            pathPaint.setStyle(SkPaint::kFill_Style);
            draw_path(canvas, pathPaint, fPaths[i], triPaint);

            canvas->save();
            canvas->translate(0, SIZE * 6 / 5);

            pathPaint.setStyle(SkPaint::kStroke_Style);
            draw_path(canvas, pathPaint, fPaths[i], triPaint);
            
            canvas->restore();
            canvas->translate(SIZE * 6 / 5, 0);
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TriangleView; }
static SkViewRegister reg(MyFactory);

