
#import "SkCanvas.h"
#import "SkWindow.h"
#include "SkGraphics.h"
#include "SkCGUtils.h"

#include <time.h>
#include <sys/time.h>

bool DrawPixman(SkCanvas* canvas, int step, bool usePixman);

class SkPixmanView : public SkView {
public:
    SkPixmanView() {
        this->setVisibleP(true);
        this->setClipToBounds(false);
        usePixman = true;
        slide = 0;
        step = -1;
    };
protected:
    virtual void onDraw(SkCanvas* canvas) {
        static int step = 0; // 12752; // 17908 ; // 17904; // drawLetters first error
                             // drawStars triggers error at 23275
                             // error is not easy to debug in its current state
        static double seconds;
        if (step == -1) {
            timeval t;
            gettimeofday(&t, NULL);
            seconds = t.tv_sec+t.tv_usec/1000000.0;
            step = 0;
        }
        canvas->drawColor(SK_ColorWHITE);
        if (DrawPixman(canvas, slide, usePixman)) {
            if (step == 100) {
                timeval t;
                gettimeofday(&t, NULL);
                double last = seconds;
                seconds = t.tv_sec+t.tv_usec/1000000.0;
                SkDebugf("usePixman=%d seconds=%g\n", usePixman, seconds - last);
                step = 0;
            }
            inval(NULL);
        }
    }
    
    virtual Click* onFindClickHandler(SkScalar , SkScalar ) {
  //      usePixman ^= true;
            ++slide;
        return NULL;
    }

private:
    bool usePixman;
    int slide;
    int step;
    typedef SkView INHERITED; 
};

void application_init();
void application_term();

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkGraphics::Term();
    SkEvent::Term();
}

class FillLayout : public SkView::Layout {
protected:
    virtual void onLayoutChildren(SkView* parent) {
        SkView* view = SkView::F2BIter(parent).next();
        view->setSize(parent->width(), parent->height());
    }
};

#import "SimpleApp.h"

@implementation SimpleNSView

- (id)initWithDefaults {
    if ((self = [super initWithDefaults])) {
        fWind = new SkOSWindow(self); 
        fWind->setLayout(new FillLayout, false);
        fWind->attachChildToFront(new SkPixmanView)->unref();
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    SkCGDrawBitmap(ctx, fWind->getBitmap(), 0, 0);
}

@end
