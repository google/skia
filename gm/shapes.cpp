#include "gm.h"
#include "SkPicture.h"
#include "SkRectShape.h"
#include "SkGroupShape.h"

namespace skiagm {

static SkRect make_rect(int l, int t, int r, int b) {
    SkRect rect;
    rect.set(SkIntToScalar(l), SkIntToScalar(t),
             SkIntToScalar(r), SkIntToScalar(b));
    return rect;
}

static SkShape* make_shape0(bool red) {
    SkRectShape* s = new SkRectShape;
    s->setRect(make_rect(10, 10, 90, 90));
    if (red) {
        s->paint().setColor(SK_ColorRED);
    }
    return s;
}

static SkShape* make_shape1() {
    SkRectShape* s = new SkRectShape;
    s->setOval(make_rect(10, 10, 90, 90));
    s->paint().setColor(SK_ColorBLUE);
    return s;
}

static SkShape* make_shape2() {
    SkRectShape* s = new SkRectShape;
    s->setRRect(make_rect(10, 10, 90, 90),
                SkIntToScalar(20), SkIntToScalar(20));
    s->paint().setColor(SK_ColorGREEN);
    return s;
}

///////////////////////////////////////////////////////////////////////////////

class ShapesGM : public GM {
    SkGroupShape fGroup;
    SkMatrixRef*    fMatrixRefs[4];
public:
	ShapesGM() {
        SkMatrix m;
        fGroup.appendShape(make_shape0(false))->unref();
        m.setRotate(SkIntToScalar(30), SkIntToScalar(50), SkIntToScalar(50));
        m.postTranslate(0, SkIntToScalar(120));
        fGroup.appendShape(make_shape0(true), m)->unref();

        m.setTranslate(SkIntToScalar(120), 0);
        fGroup.appendShape(make_shape1(), m)->unref();
        m.postTranslate(0, SkIntToScalar(120));
        fGroup.appendShape(make_shape2(), m)->unref();
        
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrixRefs); i++) {
            SkSafeRef(fMatrixRefs[i] = fGroup.getShapeMatrixRef(i));
        }
    }
    
    virtual ~ShapesGM() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrixRefs); i++) {
            SkSafeUnref(fMatrixRefs[i]);
        }
    }
    
protected:
    virtual SkString onShortName() {
        return SkString("shapes");
    }
    
	virtual SkISize onISize() {
        return make_isize(380, 480);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        SkMatrix saveM = *fMatrixRefs[3];
        SkScalar c = SkIntToScalar(50);
        fMatrixRefs[3]->preRotate(SkIntToScalar(30), c, c);
        
        SkMatrix matrix;
     
        SkGroupShape* gs = new SkGroupShape;
        SkAutoUnref aur(gs);
        gs->appendShape(&fGroup);
        matrix.setScale(-SK_Scalar1, SK_Scalar1);
        matrix.postTranslate(SkIntToScalar(220), SkIntToScalar(240));
        gs->appendShape(&fGroup, matrix);
        matrix.setTranslate(SkIntToScalar(240), 0);
        matrix.preScale(SK_Scalar1*2, SK_Scalar1*2);
        gs->appendShape(&fGroup, matrix);
        
#if 0        
        canvas->drawShape(gs);
#else
        SkPicture pict;
        SkCanvas* cv = pict.beginRecording(1000, 1000);
        cv->scale(SK_ScalarHalf, SK_ScalarHalf);
        cv->drawShape(gs);
        cv->translate(SkIntToScalar(680), SkIntToScalar(480));
        cv->scale(-SK_Scalar1, SK_Scalar1);
        cv->drawShape(gs);
        pict.endRecording();
        canvas->drawPicture(pict);
#endif

        *fMatrixRefs[3] = saveM;
}
    
private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new ShapesGM; }
static GMRegistry reg(MyFactory);

}
