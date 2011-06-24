#import <UIKit/UIKit.h>
#import "SkUISplitViewController.h"
@interface AppDelegate_iPad : NSObject <UIApplicationDelegate> {
    UIWindow* window;
    SkUISplitViewController* splitViewController;
}

@property (nonatomic, retain) IBOutlet UIWindow* window;
@property (nonatomic, retain) IBOutlet SkUISplitViewController* splitViewController;

@end

