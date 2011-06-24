#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIDetailViewController.h"
  
@interface SkUISplitViewController : UISplitViewController <UITableViewDelegate, UISplitViewControllerDelegate> {
    SkUIRootViewController* fRoot;
    SkUIDetailViewController* fDetail;
}
@property (nonatomic, retain) IBOutlet SkUIRootViewController* fRoot;
@property (nonatomic, retain) IBOutlet SkUIDetailViewController* fDetail;

- (void)loadData;
@end
