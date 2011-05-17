#include "SampleCode.h"
#include "SkCanvas.h"

namespace {
SkBitmap make_bitmap() {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config , 10, 10);
    bm.allocPixels();

    for (int y = 0; y < bm.height(); y++) {
        uint32_t* p = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); x++) {
            p[x] = ((x + y) & 1) ? SK_ColorWHITE : SK_ColorBLACK;
        }
    }
    bm.unlockPixels();
    return bm;
}
} // unnamed namespace

class TextureDomainView : public SampleView {
    SkBitmap    fBM;

public:
    TextureDomainView(){
        fBM = make_bitmap();
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Texture Domian");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkIRect srcRect;
        SkRect dstRect;
        SkPaint paint;
        paint.setFilterBitmap(true);
        srcRect.setXYWH(1, 1, 8, 8);
        dstRect.setXYWH(10.0f, 10.0f, 810.0f, 810.0f);
        canvas->drawBitmapRect(fBM, &srcRect, dstRect, &paint);
    }
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextureDomainView; }
static SkViewRegister reg(MyFactory);

