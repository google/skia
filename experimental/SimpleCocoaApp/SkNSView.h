#import <Cocoa/Cocoa.h>
class SkNSContainerView;
class SkView;

@interface SkNSView : NSView {
    SkNSContainerView* fView;
    
    @private
    NSPoint fOffset, fCenter;
    CGFloat fScale, fRotation;
}
    @property(readwrite) NSPoint fOffset, fCenter; 
    @property(readwrite) CGFloat fScale, fRotation;

    -(void) addSkView:(SkView*)aView; 
    -(void) resetTransformations;
@end


