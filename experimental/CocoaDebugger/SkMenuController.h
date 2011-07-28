
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <Cocoa/Cocoa.h>
#import "SkDebugger.h"
@interface SkMenuController : NSObject {
    IBOutlet SkDebugger *fWindow;
}
-(IBAction) openFile:(id) sender;
@end
