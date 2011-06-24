#import <UIKit/UIKit.h>


@interface SkUIView_withSkUIContainerView : UIView {
    SkUIContainerView* fView;
    UINavigationItem* fTitle;
@private
    CGPoint fOffset, fCenter;
    CGFloat fScale, fRotation;
}

@property(assign) CGPoint fOffset, fCenter; 
@property(assign) CGFloat fScale, fRotation;
@property(retain) UINavigationItem* fTitle;
- (void)resetTransformations;
- (void)addSkView:(SkView*)aView; 
- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;
@end
