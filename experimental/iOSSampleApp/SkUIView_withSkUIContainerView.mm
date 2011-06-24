#import "SkView.h"
#import "SkMatrix.h"
#import "SkCanvas.h"

class SkUIContainerView : public SkView {
public:
    SkUIContainerView(UIView* parent){
        fParent = parent;
        fMatrix.reset();
    }
    ~SkUIContainerView() {
        [fParent release];
    }
    void setBeforeChildMatrix(const SkMatrix& m) {fMatrix = m;}
    
protected:
    virtual bool handleInval(const SkRect*) {
        [fParent setNeedsDisplay];
        return true;
    }
    virtual void beforeChild(SkView* child, SkCanvas* canvas) {
        canvas->concat(fMatrix);
    }
    virtual void onSizeChange() {
        this->INHERITED::onSizeChange();
        SkView::F2BIter iter(this);
        SkView* view = iter.next();
        while (view) {
            view->setSize(this->width(), this->height());
            view = iter.next();
        }
    }
    
private:
    UIView*  fParent;
    SkMatrix fMatrix;
    
    typedef SkView INHERITED;
};
////////////////////////////////////////////////////////////////////////////////
#import "SkCGUtils.h"
#import "SkEvent.h"
#import "SkUIView_withSkUIContainerView.h"

@implementation SkUIView_withSkUIContainerView
@synthesize fOffset, fCenter, fScale, fRotation, fTitle;

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code.
        fView = new SkUIContainerView(self);
        fView->setVisibleP(true);
        fView->setSize(frame.size.width, frame.size.height);
        [self resetTransformations];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)decoder {
    self = [super initWithCoder:decoder];
    if (self) {
        // Initialization code.
        fView = new SkUIContainerView(self);
        fView->setVisibleP(true);
        CGSize s = self.bounds.size;
        fView->setSize(s.width, s.height);
        [self resetTransformations];
    }
    return self;
}

- (void)dealloc {
    [fTitle release];
    delete fView;
    [super dealloc];
}

- (void)layoutSubviews {
    CGSize s = self.bounds.size;
    fView->setSize(s.width, s.height);
}

- (void)drawRect:(CGRect)rect {
    //TODO -- check if our UIView is backed by a CALayer, and possibly use 
    //skia's gpu backend
    if (fView != nil) {
        SkBitmap bitmap;
        bitmap.setConfig(SkBitmap::kARGB_8888_Config, fView->width(), 
                         fView->height());
        bitmap.allocPixels();
        SkCanvas canvas(bitmap);
        
        //Apply view transformations so they can be applied to individual 
        //child views without affecting the parent's clip/matrix
        SkMatrix matrix;
        matrix.setTranslate(fOffset.x + fCenter.x, fOffset.y + fCenter.y);
        matrix.preRotate(fRotation);
        matrix.preScale(fScale, fScale);
        matrix.preTranslate(-fCenter.x, -fCenter.y);
        fView->setBeforeChildMatrix(matrix);
        
        fView->draw(&canvas);
        
        //Draw bitmap        
        CGImageRef cgimage = SkCreateCGImageRef(bitmap);
        [[UIImage imageWithCGImage:cgimage] drawAtPoint:CGPointMake(0, 44)];
        CGImageRelease(cgimage);
    }
}

- (void)addSkView:(SkView*)aView {
    SkASSERT(fView);
    fView->attachChildToFront(aView);
}

- (void)resetTransformations {
    fOffset = CGPointMake(0, 0);
    fCenter = CGPointMake(fView->width() / 2.0, fView->height() / 2.0);
    fRotation = 0;
    fScale = 1.0;
}

- (void)setSkTitle:(const char*)title{
    if (fTitle) {
        fTitle.title = [NSString stringWithUTF8String:title];
    }
}

- (void)postInvalWithRect:(const SkIRect*)rectOrNil{
    if (rectOrNil) {
        CGRect r = CGRectMake(rectOrNil->fLeft, rectOrNil->fTop, 
                              rectOrNil->width(), rectOrNil->height());
        [self setNeedsDisplayInRect:r];
    }
    else {
        [self setNeedsDisplay];
    }
}

- (BOOL)onHandleEvent:(const SkEvent&)event{
    [self setNeedsDisplay];
    return YES;
}
@end