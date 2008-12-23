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

#include "SkImageRef.h"
#include "SkOSFile.h"
#include "SkStream.h"

#define SPECIFIC_IMAGE  "/skimages/main.gif"

class BitmapRectView : public SkView {
public:
    SkBitmap fBitmap;
    int      fCurrX, fCurrY;

	BitmapRectView() {
        SkImageDecoder::DecodeFile(SPECIFIC_IMAGE, &fBitmap);
        fCurrX = fCurrY = 0;
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SkString str("BitmapRect: ");
            str.append(SPECIFIC_IMAGE);
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(SK_ColorGRAY);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        canvas->drawBitmap(fBitmap, 0, 0, NULL);
        
        SkIRect subset;
        const int SRC_WIDTH = 16;
        const int SRC_HEIGHT = 16;
        
        subset.set(0, 0, SRC_WIDTH, SRC_HEIGHT);
        subset.offset(fCurrX, fCurrY);
        
        SkDebugf("---- src x=%d y=%d\n", subset.fLeft, subset.fTop);
        
        SkRect  dst0, dst1;
        SkScalar y = SkIntToScalar(fBitmap.height() + 16);
        
        dst0.set(SkIntToScalar(50), y,
                 SkIntToScalar(50+SRC_WIDTH),
                 y + SkIntToScalar(SRC_HEIGHT));
        dst1 = dst0;
        dst1.offset(SkIntToScalar(200), 0);
        dst1.fRight = dst1.fLeft + 8 * dst0.width();
        dst1.fBottom = dst1.fTop + 8 * dst0.height();
        
        canvas->drawBitmapRect(fBitmap, &subset, dst0, NULL);
        canvas->drawBitmapRect(fBitmap, &subset, dst1, NULL);
        
        SkPaint paint;
        paint.setColor(0x88FF0000);
        canvas->drawRect(dst0, paint);
        paint.setColor(0x880000FF);
        canvas->drawRect(dst1, paint);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        return new Click(this);
    }
    
    virtual bool onClick(Click* click) 
    {
        fCurrX = click->fICurr.fX;
        fCurrY = click->fICurr.fY;
        this->inval(NULL);
        return true;
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new BitmapRectView; }
static SkViewRegister reg(MyFactory);

