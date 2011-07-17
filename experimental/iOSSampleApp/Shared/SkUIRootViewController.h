#import <UIKit/UIKit.h>

@interface SkUIRootViewController : UITableViewController <UITableViewDataSource> {
@private
    UIPopoverController *popoverController;
    UIBarButtonItem *popoverButtonItem;
    NSMutableArray* fSamples;
}
@property (nonatomic, retain) UIPopoverController *popoverController;
@property (nonatomic, retain) UIBarButtonItem *popoverButtonItem;

- (void)initSamples;
- (void)addItem:(NSString*)anItem;

@end
