#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPorterDuff.h"
#include "SkView.h"

#include "SkRectShape.h"
#include "SkGroupShape.h"

static SkRect make_rect(int l, int t, int r, int b) {
    SkRect rect;
    rect.set(SkIntToScalar(l), SkIntToScalar(t),
             SkIntToScalar(r), SkIntToScalar(b));
    return rect;
}

static SkShape* make_shape0(const SkMatrix* matrix = NULL) {
    SkRectShape* s = new SkRectShape;
    s->setRect(make_rect(10, 10, 90, 90));
    if (matrix) {
        s->setMatrix(*matrix);
        s->paint().setColor(SK_ColorRED);
    }
    return s;
}

static SkShape* make_shape1(const SkMatrix* matrix = NULL) {
    SkRectShape* s = new SkRectShape;
    s->setOval(make_rect(10, 10, 90, 90));
    if (matrix) {
        s->setMatrix(*matrix);
    }
    s->paint().setColor(SK_ColorBLUE);
    return s;
}

static SkShape* make_shape2(const SkMatrix* matrix = NULL) {
    SkRectShape* s = new SkRectShape;
    s->setRRect(make_rect(10, 10, 90, 90),
                SkIntToScalar(20), SkIntToScalar(20));
    if (matrix) {
        s->setMatrix(*matrix);
    }
    s->paint().setColor(SK_ColorGREEN);
    return s;
}

///////////////////////////////////////////////////////////////////////////////

class ShapesView : public SkView {
    SkGroupShape fGroup;
public:
	ShapesView() {
        SkMatrix m;
        fGroup.appendShape(make_shape0())->unref();
        m.setRotate(SkIntToScalar(30), SkIntToScalar(50), SkIntToScalar(50));
        m.postTranslate(0, SkIntToScalar(120));
        fGroup.appendShape(make_shape0(&m))->unref();

        m.setTranslate(SkIntToScalar(120), 0);
        fGroup.appendShape(make_shape1(&m))->unref();
        m.postTranslate(0, SkIntToScalar(120));
        fGroup.appendShape(make_shape2(&m))->unref();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Shapes");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkMatrix matrix;
        matrix.setTranslate(SkIntToScalar(240), 0);
        matrix.preScale(SK_Scalar1*2, SK_Scalar1*2);
        
        fGroup.draw(canvas);
        fGroup.drawXY(canvas, 0, SkIntToScalar(240));
        fGroup.drawMatrix(canvas, matrix);
    }
    
private:
    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShapesView; }
static SkViewRegister reg(MyFactory);

