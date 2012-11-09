#include "EdgeDemo.h"
#import "SkCanvas.h"
#import "SkWindow.h"
#include "SkGraphics.h"
#include "SkCGUtils.h"

#include <time.h>
#include <sys/time.h>

class SkSampleView : public SkView {
public:
    SkSampleView() {
        this->setVisibleP(true);
        this->setClipToBounds(false);
        useOld = false;
    };
protected:
    virtual void onDraw(SkCanvas* canvas) {
        static int step = 0; // 17907 drawLetters first error
                             // drawStars triggers error at 33348
                             // drawStars error not easy to debug last time I checked
        static double seconds;
        if (step == -1) {
            timeval t;
            gettimeofday(&t, NULL);
            seconds = t.tv_sec+t.tv_usec/1000000.0;
            step = 0;
        }
        canvas->drawColor(SK_ColorWHITE);
        if (DrawEdgeDemo(canvas, step, useOld)) {
            ++step;
            if (step == -1) {
                timeval t;
                gettimeofday(&t, NULL);
                double last = seconds;
                seconds = t.tv_sec+t.tv_usec/1000000.0;
                SkDebugf("old=%d seconds=%g\n", useOld, seconds - last);
                useOld ^= true;
                step = 0;
            }
            inval(NULL);
        }
    }
    
    virtual Click* onFindClickHandler(SkScalar , SkScalar ) {
        useOld ^= true;
        return NULL;
    }

private:
    bool useOld;
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
        fWind->attachChildToFront(new SkSampleView)->unref();
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    SkCGDrawBitmap(ctx, fWind->getBitmap(), 0, 0);
}

@end
