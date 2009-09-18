#include "SampleCode.h"
#include "SkBlurMaskFilter.h"
#include "SkColorPriv.h"
#include "SkGradientShader.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"

static SkBitmap make_bitmap() {
    SkBitmap bm;
    SkColorTable* ctable = new SkColorTable(256);

    SkPMColor* c = ctable->lockColors();
    for (int i = 0; i < 256; i++) {
        c[i] = SkPackARGB32(255 - i, 0, 0, 0);
    }
    ctable->unlockColors(true);
    bm.setConfig(SkBitmap::kIndex8_Config, 256, 256);
    bm.allocPixels(ctable);
    ctable->unref();

    bm.lockPixels();
    const float cx = bm.width() * 0.5f;
    const float cy = bm.height() * 0.5f;
    for (int y = 0; y < bm.height(); y++) {
        float dy = y - cy;
        dy *= dy;
        uint8_t* p = bm.getAddr8(0, y);
        for (int x = 0; x < 256; x++) {
            float dx = x - cx;
            dx *= dx;
            float d = (dx + dy) / (cx/2);
            int id = (int)d;
            if (id > 255) {
                id = 255;
            }
            p[x] = id;
        }
    }
    bm.unlockPixels();
    return bm;
}

class BlurView : public SkView {
    SkBitmap    fBM;
public:
	BlurView() {
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Blur");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);
        
        SkBlurMaskFilter::BlurStyle NONE = SkBlurMaskFilter::BlurStyle(-999);
        static const struct {
            SkBlurMaskFilter::BlurStyle fStyle;
            int                         fCx, fCy;
        } gRecs[] = {
            { NONE,                                 0,  0 },
            { SkBlurMaskFilter::kInner_BlurStyle,  -1,  0 },
            { SkBlurMaskFilter::kNormal_BlurStyle,  0,  1 },
            { SkBlurMaskFilter::kSolid_BlurStyle,   0, -1 },
            { SkBlurMaskFilter::kOuter_BlurStyle,   1,  0 },
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorBLUE);

        for (size_t i = 0; i < SK_ARRAY_COUNT(gRecs); i++) {
            if (gRecs[i].fStyle != NONE) {
                SkMaskFilter* mf = SkBlurMaskFilter::Create(20, gRecs[i].fStyle);
                paint.setMaskFilter(mf)->unref();
            } else {
                paint.setMaskFilter(NULL);
            }
            canvas->drawCircle(200 + gRecs[i].fCx*100,
                               200 + gRecs[i].fCy*100, 50, paint);
        }
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BlurView; }
static SkViewRegister reg(MyFactory);

