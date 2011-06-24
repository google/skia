#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIDetailViewController.h"


@interface SkUINavigationController : UINavigationController {
    SkUIRootViewController* fRoot;
    SkUIDetailViewController* fDetail;
}
@property (nonatomic, retain) IBOutlet SkUIRootViewController* fRoot;
@property (nonatomic, retain) IBOutlet SkUIDetailViewController* fDetail;

- (void)loadData;
@end
