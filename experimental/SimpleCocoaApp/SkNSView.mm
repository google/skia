#import "SkView.h"
#import "SkMatrix.h"
#import "SkCanvas.h"
#import <AppKit/AppKit.h>
class SkNSContainerView : public SkView {
public:
    SkNSContainerView(NSView* parent){
        fParent = parent;
        fMatrix.reset();
    }
    void setBeforeChildMatrix(const SkMatrix& m) {fMatrix = m;}
    
protected:
    virtual bool handleInval(const SkRect*) {
        [fParent setNeedsDisplay:YES];
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
    NSView*  fParent;
    SkMatrix fMatrix;
    
    typedef SkView INHERITED;
};

////////////////////////////////////////////////////////////////////////////////
#import "SkCGUtils.h"
#import "SkNSView.h"
@implementation SkNSView
@synthesize offset, center, scale, rotation;

-(id) initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        fView = new SkNSContainerView(self);
        fView->setVisibleP(true);
        NSSize viewSize = [self bounds].size;
        fView->setSize(viewSize.width, viewSize.height);
        
        [self resetTransformations];
    }
    return self;
}

-(void) dealloc {
    delete fView;
    [super dealloc];
}

-(void) addSkView:(SkView*)aView {
    fView->attachChildToFront(aView);
}

-(BOOL) isFlipped {
    return YES;
}

-(BOOL) inLiveResize {
    if (fView != nil) {
        NSSize s = [self bounds].size;
        fView->setSize(s.width, s.height);
        [self setNeedsDisplay:YES];
    }
    return [super inLiveResize];
}

-(void) resetTransformations {
    offset = NSMakePoint(0, 0);
    center = NSMakePoint(fView->width() / 2.0, fView->height() / 2.0);
    rotation = 0;
    scale = 1.0;
}

-(void) drawRect:(NSRect)dirtyRect {
    //TODO -- check if our NSView is backed by a CALayer, and possibly use 
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
        matrix.setTranslate(offset.x + center.x, offset.y + center.y);
        matrix.preRotate(rotation);
        matrix.preScale(scale, scale);
        matrix.preTranslate(-center.x, -center.y);
        fView->setBeforeChildMatrix(matrix);

        fView->draw(&canvas);
        
        //Draw bitmap
        NSImage * image = [[NSImage alloc] init];
        CGImageRef cgimage = SkCreateCGImageRef(bitmap);
        NSBitmapImageRep * bitmapRep = 
        [[NSBitmapImageRep alloc] initWithCGImage:cgimage];
        
        [image addRepresentation:bitmapRep];
        [image setSize:NSMakeSize(fView->width(), fView->height())];
        [image setFlipped:TRUE];
        [image drawAtPoint:NSMakePoint(0, 0)
                  fromRect: NSZeroRect
                 operation: NSCompositeSourceOver
                  fraction: 1.0];
        [image release]; 
        CGImageRelease(cgimage);
        [bitmapRep release];
    }
}
@end