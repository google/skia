#import <UIKit/UIKit.h>
#include "SkEvent.h"

class SkOSWindow;
class SkIRect;
@interface SkUIView_shell : UIView {
@private
    UINavigationItem* fTitle;
    SkOSWindow* fSkWind;
}
@property(nonatomic, retain) UINavigationItem* fTitle;

- (void)setSkWindow:(SkOSWindow*)anSkWindow;
- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;

@end
