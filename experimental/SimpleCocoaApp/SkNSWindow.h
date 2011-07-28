
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
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