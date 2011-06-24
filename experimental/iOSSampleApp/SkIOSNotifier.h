#import <Foundation/Foundation.h>

@interface SkIOSNotifier : NSObject {
}
-(void) receiveSkEvent:(NSNotification*)notification;
+(void) postTimedEvent:(NSTimeInterval)ti;
+(void) timerFireMethod:(NSTimer*)theTimer;
@end
