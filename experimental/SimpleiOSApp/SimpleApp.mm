#import "SkCanvas.h"
#import "SkPaint.h"
#import "SkWindow.h"
#include "SkGraphics.h"
#include "SkCGUtils.h"

extern void tool_main(int argc, char *argv[]);
void save_args(int argc, char *argv[]);

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
        canvas->drawText("finished", 13, 50, 30, p);
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

int saved_argc;
char** saved_argv;

void save_args(int argc, char *argv[]) {
    saved_argc = argc;
    saved_argv = argv;
}

class FillLayout : public SkView::Layout {
protected:
    virtual void onLayoutChildren(SkView* parent) {
        SkView* view = SkView::F2BIter(parent).next();
        view->setSize(parent->width(), parent->height());
    }
};

#import "SimpleApp.h"
@implementation SimpleApp

- (id)initWithDefaults {
    (void) tool_main(saved_argc, saved_argv);
    if (self = [super initWithDefaults]) {
        fWind = new SkOSWindow(self);
        fWind->setLayout(new FillLayout, false);
        fWind->attachChildToFront(new SkSampleView)->unref();
    }
    return self;
}

@end
