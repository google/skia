#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkFlipPixelRef.h"
#include "SkPageFlipper.h"

#include <pthread.h>

#define WIDTH   200
#define HEIGHT  200

static bool gDone;

static void bounce(SkScalar* x, SkScalar* dx, const int max) {
    *x += *dx;
    if (*x < 0) {
        *x = 0;
        if (*dx < 0) {
            *dx = -*dx;
        }
    } else if (*x > SkIntToScalar(max)) {
        *x = SkIntToScalar(max);
        if (*dx > 0) {
            *dx = -*dx;
        }
    }
}

static void* draw_proc(void* context) {
    const int OVALW = 32;
    const int OVALH = 32;

    const SkBitmap* bm = static_cast<const SkBitmap*>(context);
    SkFlipPixelRef* ref = static_cast<SkFlipPixelRef*>(bm->pixelRef());

    const int DSCALE = 1;
    SkScalar    dx = SkIntToScalar(7) / DSCALE;
    SkScalar    dy = SkIntToScalar(5) / DSCALE;
    SkScalar    x = 0;
    SkScalar    y = 0;

    SkPaint paint;
    
    paint.setAntiAlias(true);
    paint.setColor(reinterpret_cast<SkColor>(ref) | (0xFF << 24));
    
    SkRect oval;
    oval.setEmpty();

    while (!gDone) {
        ref->inval(oval, true);
        oval.set(x, y, x + SkIntToScalar(OVALW), y + SkIntToScalar(OVALH));
        ref->inval(oval, true);

        SkAutoFlipUpdate    update(ref);
        
        if (!update.dirty().isEmpty()) {
            // this must be local to the loop, since it needs to forget the pixels
            // its writing to after each iteration, since we do the swap
            SkCanvas    canvas(update.bitmap());

//            SkDebugf("----- dirty [%d %d %d %d]\n", dirty.getBounds().fLeft, dirty.getBounds().fTop, dirty.getBounds().width(), dirty.getBounds().height());
            canvas.clipRegion(update.dirty());
            
            canvas.drawColor(0, SkPorterDuff::kClear_Mode);            
            canvas.drawOval(oval, paint);
        }
        bounce(&x, &dx, WIDTH-OVALW);
        bounce(&y, &dy, HEIGHT-OVALH);
        
#if 1
        for (int i = 0; i < 1000; i++) {
            for (int j = 0; j < 10000; j++) {
                SkFixedMul(j, 10);
            }
        }
#endif
    }
    return NULL;
}

static const SkBitmap::Config gConfigs[] = {
    SkBitmap::kARGB_8888_Config,
#if 1
    SkBitmap::kRGB_565_Config,
    SkBitmap::kARGB_4444_Config,
    SkBitmap::kA8_Config
#endif
};

class PageFlipView : public SkView {
public:
    
    enum { N = SK_ARRAY_COUNT(gConfigs) };
    
    pthread_t   fThreads[N];
    SkBitmap    fBitmaps[N];

	PageFlipView() {
        gDone = false;
        for (int i = 0; i < N; i++) {
            int             status;
            pthread_attr_t  attr;
            
            status = pthread_attr_init(&attr);
            SkASSERT(0 == status);

            fBitmaps[i].setConfig(gConfigs[i], WIDTH, HEIGHT);
            SkFlipPixelRef* pr = new SkFlipPixelRef(gConfigs[i], WIDTH, HEIGHT);
            fBitmaps[i].setPixelRef(pr)->unref();
            fBitmaps[i].eraseColor(0);

            status = pthread_create(&fThreads[i], &attr,  draw_proc, &fBitmaps[i]);
            SkASSERT(0 == status);
        }
    }
    
    virtual ~PageFlipView() {
        gDone = true;
        for (int i = 0; i < N; i++) {
            void* ret;
            int status = pthread_join(fThreads[i], &ret);
            SkASSERT(0 == status);
        }
    }
    
protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PageFlip");
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
        
        SkScalar x = SkIntToScalar(10);
        SkScalar y = SkIntToScalar(10);
        for (int i = 0; i < N; i++) {
            canvas->drawBitmap(fBitmaps[i], x, y);
            x += SkIntToScalar(fBitmaps[i].width() + 20);
        }
        this->inval(NULL);
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

static SkView* MyFactory() { return new PageFlipView; }
static SkViewRegister reg(MyFactory);

