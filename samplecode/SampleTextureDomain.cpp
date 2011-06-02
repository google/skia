#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkDevice.h"

namespace {
SkBitmap make_bitmap() {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config , 5, 5);
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

        // Test that bitmap draws from malloc-backed bitmaps respect
        // the constrained texture domain.
        srcRect.setXYWH(1, 1, 3, 3);
        dstRect.setXYWH(5.0f, 5.0f, 305.0f, 305.0f);
        canvas->drawBitmapRect(fBM, &srcRect, dstRect, &paint);

        // Test that bitmap draws across separate devices also respect
        // the constrainted texture domain.
        // Note:  GPU-backed bitmaps follow a different rendering path
        // when copying from one GPU device to another.
        SkRefPtr<SkDevice> primaryDevice(canvas->getDevice());
        SkRefPtr<SkDevice> secondDevice(canvas->createDevice(
                SkBitmap::kARGB_8888_Config, 5, 5, true, true));
        secondDevice->unref();
        SkCanvas secondCanvas(secondDevice.get());

        srcRect.setXYWH(1, 1, 3, 3);
        dstRect.setXYWH(1.0f, 1.0f, 3.0f, 3.0f);
        secondCanvas.drawBitmapRect(fBM, &srcRect, dstRect, &paint);

        SkBitmap deviceBitmap = secondDevice->accessBitmap(false);

        srcRect.setXYWH(1, 1, 3, 3);
        dstRect.setXYWH(405.0f, 5.0f, 305.0f, 305.0f);
        canvas->drawBitmapRect(deviceBitmap, &srcRect, dstRect, &paint);
    }
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new TextureDomainView; }
static SkViewRegister reg(MyFactory);

