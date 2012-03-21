
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import "SkEventNotifier.h"
#include "SkEvent.h"
#define SkEventClass @"SkEvenClass"
@implementation SkEventNotifier
- (id)init {
    self = [super init];
    if (self) {
        //Register as an observer for SkEventClass events and call
        //receiveSkEvent: upon receiving the event
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(receiveSkEvent:)
                                                     name:SkEventClass
                                                   object:nil];
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

//SkEvent Handers
- (void)receiveSkEvent:(NSNotification *)notification {
    if(SkEvent::ProcessEvent())
        SkEvent::SignalNonEmptyQueue();
}

+ (void)postTimedSkEvent:(NSTimeInterval)timeInterval {
    [NSTimer scheduledTimerWithTimeInterval:timeInterval target:self
                                   selector:@selector(timerFireMethod:)
                                   userInfo:nil repeats:NO];
}

+ (void)timerFireMethod:(NSTimer*)theTimer {
	SkEvent::ServiceQueueTimer();
}

@end
////////////////////////////////////////////////////////////////////////////////
void SkEvent::SignalNonEmptyQueue() {
    //post a SkEventClass event to the default notification queue
    NSNotification* notification = [NSNotification notificationWithName:SkEventClass object:nil];
    [[NSNotificationQueue defaultQueue] enqueueNotification:notification
                                               postingStyle:NSPostWhenIdle
                                               coalesceMask:NSNotificationNoCoalescing
                                                   forModes:nil];
}

void SkEvent::SignalQueueTimer(SkMSec delay) {
	if (delay) {
        //Convert to seconds
        NSTimeInterval ti = delay/(float)SK_MSec1;
        [SkEventNotifier postTimedSkEvent:ti];
	}
}
