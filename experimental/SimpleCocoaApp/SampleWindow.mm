#import "SkCanvas.h"
#import "SkPaint.h"
#import "SkView.h"
class SkSampleView : public SkView {
public:
    SkSampleView() {};
protected:
    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(0xFFFFFFFF);
        SkPaint p;
        p.setTextSize(20);
        p.setAntiAlias(true);
        canvas->drawText("Hello World!", 13, 50, 30, p);
        this->INHERITED::onDraw(canvas);
    }
private:
    typedef SkView INHERITED; 
};
////////////////////////////////////////////////////////////////////////////////
#import "SampleWindow.h"
@implementation SampleWindow
-(void) installSkViews {
    fSampleSkView = new SkSampleView;
    fSampleSkView->setVisibleP(true);
    fSampleSkView->setSize([self frame].size.width, [self frame].size.height);
    [fView addSkView:fSampleSkView];
    [fView setNeedsDisplay:YES];
    fSampleSkView->unref();
}
@end