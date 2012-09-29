#import "FileReaderWindow.h"
#import "SkGradientShader.h"

bool gNeverSetToTrueJustNeedToFoolLinker;
static void init_effects() {
  if (gNeverSetToTrueJustNeedToFoolLinker) {
    SkPoint p = SkPoint::Make(0,0);
    SkPoint q = SkPoint::Make(100,100);
    SkPoint pts[] = {p, q};
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
    SkScalar pos[] = { 0, 1.0};
    SkGradientShader::CreateLinear(pts, colors, pos, 2, 
                                   SkShader::kMirror_TileMode);
  }
}

@implementation FileReaderWindow
-(void) installSkViews {
    init_effects();
    fReaderView = new ReaderView;
    fReaderView->setVisibleP(true);
    fReaderView->setSize([self frame].size.width, [self frame].size.height);
    [fView addSkView:fReaderView];
    [fView setNeedsDisplay:YES];
    fReaderView->unref();
    //TODO - Temporary fix. Inval doesn't Seem to be working. 
    [NSTimer scheduledTimerWithTimeInterval:0.01 target:self 
                                   selector:@selector(redraw) userInfo:nil 
                                    repeats:YES];
}

- (void)redraw {
    [fView setNeedsDisplay:YES];
}
@end