
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>
#import "SkUISplitViewController.h"

@interface AppDelegate_iPad : NSObject <UIApplicationDelegate> {
@private;
    UIWindow* window;
    SkUISplitViewController* splitViewController;
}
@property (nonatomic, retain) IBOutlet UIWindow* window;
@property (nonatomic, retain) IBOutlet SkUISplitViewController* splitViewController;

@end
