#import <UIKit/UIKit.h>
#import "SkView.h"
#import "SkOSWindow_iOS.h"
class SkUIContainerView;

@interface SkUIView_shell : UIView {
    UINavigationItem* fTitle;
    SkOSWindow* fSkWind;
@private
    CGPoint fOffset, fCenter;
    CGFloat fScale, fRotation;
}

@property(assign) CGPoint fOffset, fCenter; 
@property(assign) CGFloat fScale, fRotation;
@property(retain) UINavigationItem* fTitle;
- (void)resetTransformations;
- (void)setSkWindow:(SkOSWindow*)anSkWindow;
- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;
@end
