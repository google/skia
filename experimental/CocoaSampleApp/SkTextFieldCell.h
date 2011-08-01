#import <Cocoa/Cocoa.h>
//A text field cell that has vertically centered text
@interface SkTextFieldCell : NSTextFieldCell {
    BOOL selectingOrEditing;
}
@end
