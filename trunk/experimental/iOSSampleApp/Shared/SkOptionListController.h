#import <UIKit/UIKit.h>

@interface SkOptionListController : UITableViewController {
    NSMutableArray* fOptions;
    NSInteger fSelectedIndex;
    UITableViewCell* fSelectedCell;
    UITableViewCell* fParentCell;
}
@property (nonatomic, retain) NSMutableArray* fOptions;
@property (nonatomic, assign) NSInteger fSelectedIndex;
@property (nonatomic, retain) UITableViewCell* fSelectedCell;
@property (nonatomic, retain) UITableViewCell* fParentCell;

- (void)addOption:(NSString*)option;
- (NSString*)getSelectedOption;
@end
