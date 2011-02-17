#include "gm.h"
#include "SkRandom.h"

namespace skiagm {

class PointsGM : public GM {
public:
	PointsGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("points");
    }

	virtual SkISize onISize() {
        return make_isize(640, 490);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }

    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand) {
        for (size_t i = 0; i < n; i++)
            pts[i].set(rand->nextUScalar1() * 640, rand->nextUScalar1() * 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);

        canvas->translate(SK_Scalar1, SK_Scalar1);

        SkRandom rand;
        SkPaint  p0, p1, p2, p3;
        const size_t n = 99;

        p0.setColor(SK_ColorRED);
        p1.setColor(SK_ColorGREEN);
        p2.setColor(SK_ColorBLUE);
        p3.setColor(SK_ColorWHITE);

        p0.setStrokeWidth(SkIntToScalar(4));
        p2.setStrokeCap(SkPaint::kRound_Cap);
        p2.setStrokeWidth(SkIntToScalar(6));

        SkPoint* pts = new SkPoint[n];
        fill_pts(pts, n, &rand);

        canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts, p0);
        canvas->drawPoints(SkCanvas::kLines_PointMode, n, pts, p1);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p2);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p3);

        delete[] pts;
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new PointsGM; }
static GMRegistry reg(MyFactory);

}

