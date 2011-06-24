#import <UIKit/UIKit.h>
@interface SkUIRootViewController : UITableViewController <UITableViewDataSource> {                     
    UIPopoverController *popoverController;    
    UIBarButtonItem *popoverButtonItem;
    @private
    NSMutableArray* fSamples;
}
@property (nonatomic, retain) UIPopoverController *popoverController;
@property (nonatomic, retain) UIBarButtonItem *popoverButtonItem;

- (void)initList;
- (void)addItem:(NSString*)anItem;
@end