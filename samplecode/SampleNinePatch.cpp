#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkNinePatch.h"
#include "SkPaint.h"
#include "SkUnPreMultiply.h"

class NinePatchView : public SampleView {
public:
    SkBitmap fBM;

	NinePatchView() {
        SkImageDecoder::DecodeFile("/skimages/btn_default_normal_disable.9.png", &fBM);
        
        // trim off the edge guide-lines
        SkBitmap tmp;
        SkIRect r;
        r.set(1, 1, fBM.width() - 1, fBM.height() - 1);
        fBM.extractSubset(&tmp, r);
        fBM.swap(tmp);
        
        fX = SkIntToScalar(fBM.width());
        fY = 0;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "NinePatch");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    virtual void onDrawBackground(SkCanvas* canvas) {
        SkPaint p;
        p.setDither(true);
        p.setColor(0xFF909090);
        canvas->drawPaint(p);
    }

    static void test_rects(SkCanvas* canvas, const SkBitmap& bm, const SkPaint* paint) {
        static const SkIRect src[] = {
            { 0, 0, 18, 34 },
            { 18, 0, 19, 34 },
            { 19, 0, 36, 34 },
            { 0, 34, 18, 35 },
            { 18, 34, 19, 35 },
            { 19, 34, 36, 35 },
            { 0, 35, 18, 72 },
            { 18, 35, 19, 72 },
            { 19, 35, 36, 72 },
        };
        static const SkRect dst[] = {
            { 0, 0, 18, 34 },
            { 18, 0, 283, 34 },
            { 283, 0, 300, 34 },
            { 0, 34, 18, 163 },
            { 18, 34, 283, 163 },
            { 283, 34, 300, 163 },
            { 0, 163, 18, 200 },
            { 18, 163, 283, 200 },
            { 283, 163, 300, 200 },
        };
        for (size_t i = 0; i < SK_ARRAY_COUNT(src); i++) {
            canvas->drawBitmapRect(bm, &src[i], dst[i], paint);
        }
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        canvas->drawBitmap(fBM, 0, 0);
        
        SkIRect margins;
        SkRect  dst;
        int d = 25;
        
        margins.set(d, d, d, d);
        margins.fLeft   = fBM.width()/2 - 1;
        margins.fTop    = fBM.height()/2 - 1;
        margins.fRight  = fBM.width() - margins.fLeft - 1;
        margins.fBottom = fBM.height() - margins.fTop - 1;

   //     canvas->translate(fX/5, fY/5);
        canvas->translate(0, 76);

        dst.set(0, 0, SkIntToScalar(200), SkIntToScalar(200));
        
        SkPaint paint;
        paint.setAntiAlias(false);
        paint.setDither(true);
        paint.setFilterBitmap(false);
    //    SkNinePatch::DrawNine(canvas, dst, fBM, margins, &paint);
        test_rects(canvas, fBM, &paint);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        fX = x / 1.5f;
        fY = y / 1.5f;
        fX = x; fY = y;
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
private:
    SkScalar fX, fY;
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new NinePatchView; }
static SkViewRegister reg(MyFactory);

