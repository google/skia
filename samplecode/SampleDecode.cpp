#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkImageDecoder.h"
#include "SkStream.h"

static const struct {
    SkBitmap::Config    fPrefConfig;
    bool                fDither;
} gRec[] = {
    { SkBitmap::kIndex8_Config,     false },
    { SkBitmap::kARGB_8888_Config,  false },
    { SkBitmap::kARGB_4444_Config,  false },
    { SkBitmap::kARGB_4444_Config,  true },
    { SkBitmap::kRGB_565_Config,    false },
    { SkBitmap::kRGB_565_Config,    true },
};

class DecodeView : public SkView {
public:
    SkBitmap fBitmap[SK_ARRAY_COUNT(gRec)];

	DecodeView() {
        SkFILEStream stream("/skimages/index.png");
        SkImageDecoder* codec = SkImageDecoder::Factory(&stream);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); i++) {
            stream.rewind();
            codec->setDitherImage(gRec[i].fDither);
            codec->decode(&stream, &fBitmap[i], gRec[i].fPrefConfig,
                          SkImageDecoder::kDecodePixels_Mode);
        }
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ImageDecoder");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas) {
        canvas->drawColor(0xFFDDDDDD);
//        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas) {
        this->drawBG(canvas);
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));
        
        for (size_t i = 0; i < SK_ARRAY_COUNT(fBitmap); i++) {
            canvas->drawBitmap(fBitmap[i], 0, 0);
            canvas->translate(SkIntToScalar(fBitmap[i].width()), 0);
        }
    }
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new DecodeView; }
static SkViewRegister reg(MyFactory);

