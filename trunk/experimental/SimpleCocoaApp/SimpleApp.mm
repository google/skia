#import "SkCanvas.h"
#import "SkPaint.h"
#import "SkWindow.h"
#include "SkGraphics.h"
#include "SkCGUtils.h"
class SkSampleView : public SkView {
public:
    SkSampleView() {
        this->setVisibleP(true);
        this->setClipToBounds(false);
    };
protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(0xFFFFFFFF);
        SkPaint p;
        p.setTextSize(20);
        p.setAntiAlias(true);
        canvas->drawText("Hello World!", 13, 50, 30, p);
        SkRect r = {50, 50, 80, 80};
        p.setColor(0xAA11EEAA);
        canvas->drawRect(r, p);
    }
private:
    typedef SkView INHERITED; 
};

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
    if (self = [super initWithDefaults]) {
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