#import <UIKit/UIKit.h>
#import "SkUINavigationController.h"

@interface AppDelegate_iPhone : NSObject <UIApplicationDelegate> {
@private
    UIWindow *window;
    SkUINavigationController* fRoot;
}
@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet SkUINavigationController* fRoot;

@end

