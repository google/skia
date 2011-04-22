#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkView.h"

#define DO_AA   true

#include "SkRectShape.h"
#include "SkGroupShape.h"

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
    s->paint().setAntiAlias(DO_AA);
    return s;
}

static SkShape* make_shape1() {
    SkRectShape* s = new SkRectShape;
    s->setOval(make_rect(10, 10, 90, 90));
    s->paint().setColor(SK_ColorBLUE);
    s->paint().setAntiAlias(DO_AA);
    return s;
}

static SkShape* make_shape2() {
    SkRectShape* s = new SkRectShape;
    s->setRRect(make_rect(10, 10, 90, 90),
                SkIntToScalar(20), SkIntToScalar(20));
    s->paint().setColor(SK_ColorGREEN);
    s->paint().setAntiAlias(DO_AA);
    return s;
}

///////////////////////////////////////////////////////////////////////////////

class ShapesView : public SampleView {
    SkGroupShape fGroup;
    SkMatrixRef*    fMatrixRefs[4];
public:
	ShapesView() {
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

        this->setBGColor(0xFFDDDDDD);
    }
    
    virtual ~ShapesView() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrixRefs); i++) {
            SkSafeUnref(fMatrixRefs[i]);
        }
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
    
    void drawpicture(SkCanvas* canvas, SkPicture& pict) {
#if 0
        SkDynamicMemoryWStream ostream;
        pict.serialize(&ostream);

        SkMemoryStream istream(ostream.getStream(), ostream.getOffset());
        SkPicture* newPict = new SkPicture(&istream);
        canvas->drawPicture(*newPict);
        newPict->unref();
#else
        canvas->drawPicture(pict);
#endif
    }
    
    virtual void onDrawContent(SkCanvas* canvas) {
        SkScalar angle = SampleCode::GetAnimScalar(SkIntToScalar(180),
                                                   SkIntToScalar(360));

        SkMatrix saveM = *fMatrixRefs[3];
        SkScalar c = SkIntToScalar(50);
        fMatrixRefs[3]->preRotate(angle, c, c);
        
        const SkScalar dx = 350;
        const SkScalar dy = 500;
        const int N = 1;
        for (int v = -N; v <= N; v++) {
            for (int h = -N; h <= N; h++) {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(h * dx, v * dy);
        
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
        SkPicture* pict = new SkPicture;
        SkCanvas* cv = pict->beginRecording(1000, 1000);
        cv->scale(SK_ScalarHalf, SK_ScalarHalf);
        cv->drawShape(gs);
        cv->translate(SkIntToScalar(680), SkIntToScalar(480));
        cv->scale(-SK_Scalar1, SK_Scalar1);
        cv->drawShape(gs);
        pict->endRecording();
        
        drawpicture(canvas, *pict);
        pict->unref();
#endif

        }}

        *fMatrixRefs[3] = saveM;
        this->inval(NULL);
}
    
private:
    typedef SampleView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ShapesView; }
static SkViewRegister reg(MyFactory);

