#import <Cocoa/Cocoa.h>
#import "SkNSView.h"

@interface SkNSWindow : NSWindow {
}
//Overwrite in subclass to load custom skia views
-(void) installSkViews;

-(void) receiveSkEvent:(NSNotification*)notification;
+(void) postTimedEvent:(NSTimeInterval)ti;
+(void) timerFireMethod:(NSTimer*)theTimer;
@end