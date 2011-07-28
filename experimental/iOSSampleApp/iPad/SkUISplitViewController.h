
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>
#import "SkUIRootViewController.h"
#import "SkUIDetailViewController.h"

@interface SkUISplitViewController : UISplitViewController <UITableViewDelegate, UISplitViewControllerDelegate> {
@private
    SkUIRootViewController* fRoot;
    SkUIDetailViewController* fDetail;
}
@property (nonatomic, retain) IBOutlet SkUIRootViewController* fRoot;
@property (nonatomic, retain) IBOutlet SkUIDetailViewController* fDetail;

@end
