#include "EdgeWalker_Test.h"
#include "ShapeOps.h"
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
    // Three circles bounce inside a rectangle. The circles describe three, four
    // or five points which in turn describe a polygon. The polygon points
    // bounce inside the circles. The circles rotate and scale over time. The
    // polygons are combined into a single path, simplified, and stroked.
    static int step = 0;
    const int circles = 3;
    int scales[circles];
    int angles[circles];
    int locs[circles * 2];
    int pts[circles * 2 * 4];
    int c, p;
    for (c = 0; c < circles; ++c) {
        scales[c] = abs(10 - (step + c * 4) % 21);
        angles[c] = (step + c * 6) % 600;
        locs[c * 2] = abs(130 - (step + c * 9) % 261);
        locs[c * 2 + 1] = abs(170 - (step + c * 11) % 341);
        for (p = 0; p < 4; ++p) {
            pts[c * 8 + p * 2] = abs(90 - ((step + c * 121 + p * 13) % 190));
            pts[c * 8 + p * 2 + 1] = abs(110 - ((step + c * 223 + p * 17) % 230));
        }
    }
    SkPath path, out;
    for (c = 0; c < circles; ++c) {
        for (p = 0; p < 4; ++p) {
            SkScalar x = pts[c * 8 + p * 2];
            SkScalar y = pts[c * 8 + p * 2 + 1];
            x *= 3 + scales[c] / 10.0f;
            y *= 3 + scales[c] / 10.0f;
            SkScalar angle = angles[c] * 3.1415f * 2 / 600;
            SkScalar temp = x * cos(angle) - y * sin(angle);
            y = x * sin(angle) + y * cos(angle);
            x = temp;
            x += locs[c * 2] * 200 / 130.0f;
            y += locs[c * 2 + 1] * 200 / 170.0f;
            x += 50;
    //        y += 200;
            if (p == 0) {
                path.moveTo(x, y);
            } else {
                path.lineTo(x, y);
            }
        }
        path.close();
    }
    showPath(path, "original:");
    simplify(path, true, out);
    showPath(out, "simplified:");
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(3);
    paint.setColor(0x3F007fbF);
    canvas->drawPath(path, paint);
    paint.setColor(0xFF60FF00);
    paint.setStrokeWidth(1);
    canvas->drawColor(SK_ColorWHITE);
    canvas->drawPath(out, paint);
    ++step;
    inval(NULL);
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
