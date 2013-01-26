
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>
#import "SkUINavigationController.h"

@interface AppDelegate_iPhone : NSObject <UIApplicationDelegate> {
@private
    UIWindow *window;
    SkUINavigationController* fRoot;
}
@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet SkUINavigationController* fRoot;

@end
