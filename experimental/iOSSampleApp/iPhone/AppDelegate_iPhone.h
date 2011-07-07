#import <UIKit/UIKit.h>
#import "SkUINavigationController.h"

@interface AppDelegate_iPhone : NSObject <UITableViewDelegate, UIApplicationDelegate> {
@private
    UIWindow *window;
    SkUINavigationController* fRoot;
    SkUIDetailViewController* fDetail;
}
@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet SkUINavigationController* fRoot;
@property (nonatomic, retain) IBOutlet SkUIDetailViewController* fDetail;

- (IBAction)displaySampleList:(id)sender;

@end

