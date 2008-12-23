#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"
#include "SkPath.h"
#include "SkPorterDuff.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkImageRef_GlobalPool.h"
#include "SkStream.h"

static const char* gNames[] = {
    "1.bmp", "1.gif", "1.jpg", "1.png",
    "2.bmp", "2.gif", "2.jpg", "2.png"
};

// ownership of the stream is transferred
static bool SetImageRef(SkBitmap* bitmap, SkStream* stream,
                        SkBitmap::Config pref, const char name[] = NULL)
{
    if (SkImageDecoder::DecodeStream(stream, bitmap, pref,
                                     SkImageDecoder::kDecodeBounds_Mode)) {
        SkASSERT(bitmap->config() != SkBitmap::kNo_Config);
    
        SkImageRef* ref = new SkImageRef_GlobalPool(stream, bitmap->config());
        ref->setURI(name);
        bitmap->setPixelRef(ref)->unref();
        return true;
    } else {
        delete stream;
        return false;
    }
}

class ImageView : public SkView {
public:
    SkBitmap*   fBitmaps;
    SkShader*   fShader;

	ImageView() {
        SkImageRef_GlobalPool::SetRAMBudget(32 * 1024);
        
        int i, N = SK_ARRAY_COUNT(gNames);
        fBitmaps = new SkBitmap[N];
        
        for (i = 0; i < N; i++) {
            SkString str("/skimages/");
            str.append(gNames[i]);
            SkFILEStream* stream = new SkFILEStream(str.c_str());
            
            SetImageRef(&fBitmaps[i], stream, SkBitmap::kNo_Config, gNames[i]);
            if (i & 1)
                fBitmaps[i].buildMipMap();
        }
        
        fShader = SkShader::CreateBitmapShader(fBitmaps[5],
                                               SkShader::kRepeat_TileMode,
                                               SkShader::kRepeat_TileMode);
        
        if (true) {
            SkMatrix m;
            
            m.setRotate(SkIntToScalar(30));
            fShader->setLocalMatrix(m);
        }
        
#if 0
        SkImageRef::DumpPool();
        for (i = 0; i < N; i++) {
            SkBitmap& bm = fBitmaps[i];

            SkDebugf("<%s> addr=%p", gNames[i], bm.getPixels());
            bool success = bm.lockPixels();
            SkDebugf(" addr=%d", bm.getPixels());
            if (success)
                bm.unlockPixels();
            SkDebugf(" addr=%p", bm.getPixels());
            success = bm.lockPixels();
            SkDebugf(" addr=%d", bm.getPixels());
            if (success)
                bm.unlockPixels();            
            SkDebugf("\n");
        }
        SkImageRef::DumpPool();
#endif
    }
    
    virtual ~ImageView() {
        delete[] fBitmaps;
        delete fShader;

        SkImageRef_GlobalPool::DumpPool();
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Image");
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
        
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        
        SkScalar x = 0, y = 0;
        
        for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); i++) {
            canvas->drawBitmap(fBitmaps[i], x, y);
            x += SkIntToScalar(fBitmaps[i].width() + 10);
        }
        
        canvas->translate(0, SkIntToScalar(120));

        SkPaint paint;
        paint.setShader(fShader);
        paint.setFilterBitmap(true);
        SkRect r = { 0, 0, SkIntToScalar(300), SkIntToScalar(100) };
        
        canvas->drawRect(r, paint);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) {
        return this->INHERITED::onClick(click);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new ImageView; }
static SkViewRegister reg(MyFactory);

