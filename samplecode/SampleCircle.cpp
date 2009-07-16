#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkPaint.h"

// ensure that we don't accidentally screw up the bounds when the oval is
// fractional, and the impl computes the center and radii, and uses them to
// reconstruct the edges of the circle.
// see bug# 1504910
static void test_circlebounds(SkCanvas* canvas) {
#ifdef SK_SCALAR_IS_FLOAT
    SkRect r = { 1.39999998, 1, 21.3999996, 21 };
    SkPath p;
    p.addOval(r);
    SkASSERT(r == p.getBounds());
#endif
}

class CircleView : public SkView {
public:
    static const SkScalar ANIM_DX = SK_Scalar1 / 67;
    static const SkScalar ANIM_DY = SK_Scalar1 / 29;
    static const SkScalar ANIM_RAD = SK_Scalar1 / 19;
    SkScalar fDX, fDY, fRAD;

    CircleView() {
        fDX = fDY = fRAD = 0;
        fN = 3;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Circles");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
    }
    
    void circle(SkCanvas* canvas, int width, bool aa) {
        SkPaint paint;
        
        paint.setAntiAlias(aa);
        if (width < 0) {
            paint.setStyle(SkPaint::kFill_Style);
        } else {
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(SkIntToScalar(width));
        }
        canvas->drawCircle(0, 0, SkIntToScalar(9) + fRAD, paint);
    }
    
    void drawSix(SkCanvas* canvas, SkScalar dx, SkScalar dy) {
        for (int width = -1; width <= 1; width++) {
            canvas->save();
            circle(canvas, width, false);
            canvas->translate(0, dy);
            circle(canvas, width, true);
            canvas->restore();
            canvas->translate(dx, 0);
        }
    }
    
    static void blowup(SkCanvas* canvas, const SkIRect& src, const SkRect& dst) {
        SkDevice* device = canvas->getDevice();
        const SkBitmap& bm = device->accessBitmap(false);
        canvas->drawBitmapRect(bm, &src, dst, NULL);
    }
    
    static void make_poly(SkPath* path, int n) {
        if (n <= 0) {
            return;
        }
        path->incReserve(n + 1);
        path->moveTo(SK_Scalar1, 0);
        SkScalar step = SK_ScalarPI * 2 / n;
        SkScalar angle = 0;
        for (int i = 1; i < n; i++) {
            angle += step;
            SkScalar c, s = SkScalarSinCos(angle, &c);
            path->lineTo(c, s);
        }
        path->close();
    }
    
    static void rotate(SkCanvas* canvas, SkScalar angle, SkScalar px, SkScalar py) {
        canvas->translate(-px, -py);
        canvas->rotate(angle);
        canvas->translate(px, py);
    }

    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
//        canvas->drawCircle(250, 250, 220, paint);
        SkMatrix matrix;
        matrix.setScale(SkIntToScalar(100), SkIntToScalar(100));
        matrix.postTranslate(SkIntToScalar(200), SkIntToScalar(200));
        canvas->concat(matrix);
        for (int n = 3; n < 20; n++) {
            SkPath path;
            make_poly(&path, n);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(SkIntToScalar(10) * (n - 3));
            canvas->translate(-SK_Scalar1, 0);
            canvas->drawPath(path, paint);
        }
        
        if (false) {
            test_circlebounds(canvas);
            
            SkScalar dx = SkIntToScalar(32);
            SkScalar dy = SkIntToScalar(32);
            
            canvas->translate(dx + fDX, dy + fDY);
            drawSix(canvas, dx, dy);

            canvas->translate(dx, 0);
            canvas->translate(SK_ScalarHalf, SK_ScalarHalf);
            drawSix(canvas, dx, dy);
        }
        
        fDX += ANIM_DX;
        fDY += ANIM_DY;
        fRAD += ANIM_RAD;
        fN += 1;
        if (fN > 40) {
            fN = 3;
        }
        this->inval(NULL);
    }
    
private:
    int fN;
    typedef SkView INHERITED;
};

const SkScalar CircleView::ANIM_DX;
const SkScalar CircleView::ANIM_DY;
const SkScalar CircleView::ANIM_RAD;

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new CircleView; }
static SkViewRegister reg(MyFactory);

