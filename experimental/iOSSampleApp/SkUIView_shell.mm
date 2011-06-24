#import "SkCGUtils.h"
#import "SkUIView_shell.h"
#import "SkEvent.h"
#import "SkCanvas.h"

@implementation SkUIView_shell
@synthesize fOffset, fCenter, fScale, fRotation, fTitle;

- (void)dealloc {
    [fTitle release];
    [super dealloc];
}

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
        SkMatrix matrix;
        matrix.setTranslate(fOffset.x + fCenter.x, fOffset.y + fCenter.y);
        matrix.preRotate(fRotation);
        matrix.preScale(fScale, fScale);
        matrix.preTranslate(-fCenter.x, -fCenter.y);
        fSkWind->setMatrix(matrix);
        SkIRect r = SkIRect::MakeWH(rect.size.width, rect.size.height);
        fSkWind->update(&r, &canvas);
        
        CGImageRef cgimage = SkCreateCGImageRef(fSkWind->getBitmap());
        [[UIImage imageWithCGImage:cgimage] drawAtPoint:CGPointMake(0, 44)];
        CGImageRelease(cgimage);
    }
}

- (void)resetTransformations {
    fOffset = CGPointMake(0, 0);
    fCenter = CGPointMake(fSkWind->width() / 2.0, fSkWind->height() / 2.0);
    fRotation = 0;
    fScale = 1.0;
}

- (void)setSkWindow:(SkOSWindow*)anSkWindow {
    fSkWind = anSkWindow;
    [self resetTransformations];
}

///////////////////////////////////////////////////////////////////////////////

- (void)setSkTitle:(const char *)title {
    if (fTitle) {
        fTitle.title = [NSString stringWithUTF8String:title];
    }
}

- (BOOL)onHandleEvent:(const SkEvent&)evt {
    return false;
}

- (void)postInvalWithRect:(const SkIRect*)r {
    if (r) {
        [self setNeedsDisplayInRect:CGRectMake(r->fLeft, r->fTop,
                                               r->width(), r->height())];
    } else {
        [self setNeedsDisplay];
    }
}

@end
