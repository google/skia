#include "SkCanvas.h"
#include "SkCGUtils.h"
#include "SkEvent.h"
#include "SkOSWindow_iOS.h"
#include "SkView.h"
#import "SkUIView_shell.h"

@implementation SkUIView_shell
@synthesize fTitle;

//Overwritten from UIView
- (void)layoutSubviews {
    [super layoutSubviews];
    CGSize s = self.bounds.size;
    fSkWind->resize(s.width, s.height);
}

- (void)drawRect:(CGRect)rect {
    //TODO -- check if our UIView is backed by a CALayer, and possibly use
    //skia's gpu backend
    if (fSkWind != nil) {
        SkCanvas canvas;
        SkIRect dirtyRect = SkIRect::MakeWH(rect.size.width, rect.size.height);
        fSkWind->update(&dirtyRect, &canvas);

        CGImageRef cgimage = SkCreateCGImageRef(fSkWind->getBitmap());
        [[UIImage imageWithCGImage:cgimage] drawAtPoint:CGPointMake(0, 44)];
        CGImageRelease(cgimage);
    }
}

- (void)dealloc {
    [fTitle release];
    [super dealloc];
}

//Instance methods
- (void)setSkWindow:(SkOSWindow*)anSkWindow {
    fSkWind = anSkWindow;
}

//Handlers for SkOSWindow
- (void)setSkTitle:(const char *)title {
    fTitle.title = [NSString stringWithUTF8String:title];
}

- (BOOL)onHandleEvent:(const SkEvent&)event {
    return false;
}

- (void)postInvalWithRect:(const SkIRect*)rect {
    if (rect) {
        [self setNeedsDisplayInRect:CGRectMake(rect->fLeft, rect->fTop,
                                               rect->width(), rect->height())];
    } else {
        [self setNeedsDisplay];
    }
}

//Gesture Handlers
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fSkWind->handleClick(loc.x, loc.y, SkView::Click::kDown_State, touch);
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fSkWind->handleClick(loc.x, loc.y, SkView::Click::kMoved_State, touch);
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fSkWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, touch);
    }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
    for (UITouch *touch in touches) {
        CGPoint loc = [touch locationInView:self];
        fSkWind->handleClick(loc.x, loc.y, SkView::Click::kUp_State, touch);
    }
}
@end
