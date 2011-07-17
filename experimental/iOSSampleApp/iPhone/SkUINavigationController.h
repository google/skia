#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIDetailViewController.h"

@interface SkUINavigationController : UINavigationController <UITableViewDelegate, UINavigationBarDelegate> {
@private
    SkUIRootViewController* fRoot;
    SkUIDetailViewController* fDetail;
}
@property (nonatomic, retain) IBOutlet SkUIRootViewController* fRoot;
@property (nonatomic, retain) IBOutlet SkUIDetailViewController* fDetail;

@end
