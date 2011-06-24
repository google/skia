#import "SkIOSNotifier.h"
#import "SkEvent.h"
#define SkEventClass @"SkEvenClass"
@implementation SkIOSNotifier
- (id)init {
    self = [super init];
    if (self) {
        //Register as an observer for SkEventClass events and call 
        //receiveSkEvent: upon receiving the event
        [[NSNotificationCenter defaultCenter] addObserver:self 
                                                 selector:@selector(receiveSkEvent:) 
                                                     name:SkEventClass object:nil];
        
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

-(BOOL) acceptsFirstResponder {
    return YES;
}

-(void) receiveSkEvent:(NSNotification *)notification {
    if(SkEvent::ProcessEvent())
        SkEvent::SignalNonEmptyQueue();
}

+(void) postTimedEvent:(NSTimeInterval)ti {
    [NSTimer scheduledTimerWithTimeInterval:ti target:self 
                                   selector:@selector(timerFireMethod:)
                                   userInfo:nil repeats:NO];
}

+(void) timerFireMethod:(NSTimer*)theTimer {
	SkEvent::ServiceQueueTimer();
}
@end
////////////////////////////////////////////////////////////////////////////////
void SkEvent::SignalNonEmptyQueue() {
    //post a SkEventClass event to the default notification center
    [[NSNotificationCenter defaultCenter] postNotificationName:SkEventClass 
                                                        object:nil];
}

void SkEvent::SignalQueueTimer(SkMSec delay) {
	if (delay) {
        //Convert to seconds
        NSTimeInterval ti = delay/(float)SK_MSec1;
        [SkIOSNotifier postTimedEvent:ti];
	}  
}