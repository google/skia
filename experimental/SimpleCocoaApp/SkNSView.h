#import <Cocoa/Cocoa.h>
class SkNSContainerView;
class SkView;

@interface SkNSView : NSView {
    SkNSContainerView* fView;
    
    @private
    NSPoint offset, center;
    CGFloat scale, rotation;
}
    @property(readwrite) NSPoint offset, center; 
    @property(readwrite) CGFloat scale, rotation;

    -(void) addSkView:(SkView*)aView; 
    -(void) resetTransformations;
@end


