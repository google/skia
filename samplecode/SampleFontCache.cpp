#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"

#include <pthread.h>

static void call_measure()
{
    SkPaint paint;
    uint16_t text[32];
    SkRandom rand;
    
    paint.setAntiAlias(true);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    for (int j = 0; j < SK_ARRAY_COUNT(text); j++)
        text[j] = (uint16_t)((rand.nextU() & 0xFF) + 32);
    
    for (int i = 9; i < 36; i++)
    {
        SkPaint::FontMetrics m;
        
        paint.setTextSize(SkIntToScalar(i));
        paint.getFontMetrics(&m);
        paint.measureText(text, sizeof(text));
    }
}

static void call_draw(SkCanvas* canvas)
{
    SkPaint paint;
    uint16_t text[32];
    SkRandom rand;
    
    paint.setAntiAlias(true);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    for (int j = 0; j < SK_ARRAY_COUNT(text); j++)
        text[j] = (uint16_t)((rand.nextU() & 0xFF) + 32);
    
    SkScalar x = SkIntToScalar(10);
    SkScalar y = SkIntToScalar(20);
    
    canvas->drawColor(SK_ColorWHITE);
    for (int i = 9; i < 36; i++)
    {
        SkPaint::FontMetrics m;
        
        paint.setTextSize(SkIntToScalar(i));
        paint.getFontMetrics(&m);
        canvas->drawText(text, sizeof(text), x, y, paint);
        y += m.fDescent - m.fAscent;
    }
}

static bool gDone;

static void* measure_proc(void* context)
{
    while (!gDone)
    {
        call_measure();
    }
    return NULL;
}

static void* draw_proc(void* context)
{
    SkBitmap* bm = (SkBitmap*)context;
    SkCanvas    canvas(*bm);

    while (!gDone)
    {
        call_draw(&canvas);
    }
    return NULL;
}

class FontCacheView : public SkView {
public:
    enum { N = 4 };
    
    pthread_t   fMThreads[N];
    pthread_t   fDThreads[N];
    SkBitmap    fBitmaps[N];

	FontCacheView()
    {
        gDone = false;
        for (int i = 0; i < N; i++)
        {
            int             status;
            pthread_attr_t  attr;
            
            status = pthread_attr_init(&attr);
            SkASSERT(0 == status);
            status = pthread_create(&fMThreads[i], &attr,  measure_proc, NULL);
            SkASSERT(0 == status);

            fBitmaps[i].setConfig(SkBitmap::kRGB_565_Config, 320, 240);
            fBitmaps[i].allocPixels();
            status = pthread_create(&fDThreads[i], &attr,  draw_proc, &fBitmaps[i]);
            SkASSERT(0 == status);
        }
    }
    
    virtual ~FontCacheView()
    {
        gDone = true;
        for (int i = 0; i < N; i++)
        {
            void* ret;
            int status = pthread_join(fMThreads[i], &ret);
            SkASSERT(0 == status);
            status = pthread_join(fDThreads[i], &ret);
            SkASSERT(0 == status);
        }
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt)
    {
        if (SampleCode::TitleQ(*evt))
        {
            SampleCode::TitleR(evt, "FontCache");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawBG(SkCanvas* canvas)
    {
        canvas->drawColor(0xFFDDDDDD);
//        canvas->drawColor(SK_ColorWHITE);
    }
    
    virtual void onDraw(SkCanvas* canvas)
    {
        this->drawBG(canvas);
        
        SkScalar x = 0;
        SkScalar y = 0;
        for (int i = 0; i < N; i++)
        {
            canvas->drawBitmap(fBitmaps[i], x, y);
            x += SkIntToScalar(fBitmaps[i].width());
        }
        this->inval(NULL);
    }
    
    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) 
    {
        this->inval(NULL);
        return this->INHERITED::onFindClickHandler(x, y);
    }
    
    virtual bool onClick(Click* click) 
    {
        return this->INHERITED::onClick(click);
    }
    
private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new FontCacheView; }
static SkViewRegister reg(MyFactory);

