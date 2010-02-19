#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkView.h"
#include "SkLayer.h"

///////////////////////////////////////////////////////////////////////////////

class TestLayer : public SkLayer {
public:
    TestLayer(SkColor c) : fColor(c) {}

protected:
    virtual void onDraw(SkCanvas* canvas, SkScalar opacity) {
        SkRect r;
        r.set(0, 0, this->getWidth(), this->getHeight());

        SkPaint paint;
        paint.setColor(fColor);
        paint.setAlpha(SkScalarRound(opacity * 255));

        canvas->drawRect(r, paint);
    }

private:
    SkColor fColor;
};

class SkLayerView : public SkView {
private:
    SkLayer* fRootLayer;

public:
	SkLayerView() {
        static const int W = 640;
        static const int H = 480;
        static const struct {
            int fWidth;
            int fHeight;
            SkColor fColor;
            int fPosX;
            int fPosY;
        } gData[] = {
            { 120, 80, SK_ColorRED, 0, 0 },
            { 120, 80, SK_ColorGREEN, W - 120, 0 },
            { 120, 80, SK_ColorBLUE, 0, H - 80 },
            { 120, 80, SK_ColorMAGENTA, W - 120, H - 80 },
        };

        fRootLayer = new SkLayer;
        fRootLayer->setSize(W, H);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gData); i++) {
            SkLayer* child = new TestLayer(gData[i].fColor);
            child->setSize(gData[i].fWidth, gData[i].fHeight);
            child->setPosition(gData[i].fPosX, gData[i].fPosY);
            fRootLayer->addChild(child)->unref();
        }
    }
    
    virtual ~SkLayerView() {
        SkSafeUnref(fRootLayer);
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "SkLayer");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);

        fRootLayer->draw(canvas);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
}
    
private:
    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SkLayerView; }
static SkViewRegister reg(MyFactory);

