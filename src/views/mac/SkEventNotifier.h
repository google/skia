/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <Foundation/Foundation.h>

@interface SkEventNotifier : NSObject
- (void)receiveSkEvent:(NSNotification*)notification;
+ (void)postTimedSkEvent:(NSTimeInterval)ti;
+ (void)timerFireMethod:(NSTimer*)theTimer;
@end
