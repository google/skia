#include "gm.h"

namespace skiagm {

class NoColorBleedGM : public GM {
public:
    NoColorBleedGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("nocolorbleed");
    }

    virtual SkISize onISize() {
        return make_isize(200, 200);
    }

    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
    }

    virtual void onDraw(SkCanvas* canvas) {
        drawBG(canvas);

        SkBitmap sprite;
        sprite.setConfig(SkBitmap::kARGB_8888_Config, 4, 4, 4*sizeof(SkColor));
        const SkColor spriteData[16] = {
            SK_ColorBLACK,  SK_ColorCYAN,    SK_ColorMAGENTA, SK_ColorYELLOW,
            SK_ColorBLACK,  SK_ColorWHITE,   SK_ColorBLACK,   SK_ColorRED,
            SK_ColorGREEN,  SK_ColorBLACK,   SK_ColorWHITE,   SK_ColorBLUE,
            SK_ColorYELLOW, SK_ColorMAGENTA, SK_ColorCYAN,    SK_ColorBLACK
        };
        sprite.allocPixels();
        sprite.lockPixels();
        SkPMColor* addr = sprite.getAddr32(0, 0);
        for (size_t i = 0; i < SK_ARRAY_COUNT(spriteData); ++i) {
            addr[i] = SkPreMultiplyColor(spriteData[i]);
        }
        sprite.unlockPixels();

        // We draw a magnified subrect of the sprite
        // sample interpolation may cause color bleeding around edges
        // the subrect is a pure white area
        SkIRect srcRect;
        SkRect dstRect;
        SkPaint paint;
        paint.setFilterBitmap(true);
        //First row : full texture with and without filtering
        srcRect.setXYWH(0, 0, 4, 4);
        dstRect.setXYWH(SkIntToScalar(0), SkIntToScalar(0)
                        , SkIntToScalar(100), SkIntToScalar(100));
        canvas->drawBitmapRect(sprite, &srcRect, dstRect, &paint);
        dstRect.setXYWH(SkIntToScalar(100), SkIntToScalar(0)
                        , SkIntToScalar(100), SkIntToScalar(100));
        canvas->drawBitmapRect(sprite, &srcRect, dstRect);
        //Second row : sub rect of texture with and without filtering
        srcRect.setXYWH(1, 1, 2, 2);
        dstRect.setXYWH(SkIntToScalar(25), SkIntToScalar(125)
                        , SkIntToScalar(50), SkIntToScalar(50));
        canvas->drawBitmapRect(sprite, &srcRect, dstRect, &paint);
        dstRect.setXYWH(SkIntToScalar(125), SkIntToScalar(125)
                        , SkIntToScalar(50), SkIntToScalar(50));
        canvas->drawBitmapRect(sprite, &srcRect, dstRect);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new NoColorBleedGM; }
static GMRegistry reg(MyFactory);

}
