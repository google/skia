
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>

@interface SkUIRootViewController : UITableViewController <UITableViewDataSource> {
@private
    UIPopoverController *popoverController;
    UIBarButtonItem *popoverButtonItem;
    NSMutableArray* fSamples;
}
@property (nonatomic, retain) UIPopoverController *popoverController;
@property (nonatomic, retain) UIBarButtonItem *popoverButtonItem;

- (void)addItem:(NSString*)anItem;

@end
