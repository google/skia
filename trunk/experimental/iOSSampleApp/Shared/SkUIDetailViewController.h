
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>
#import "SkOptionsTableViewController.h"
#import "SkUIRootViewController.h"
#import "SkUIView.h"

class SampleWindow;
class SkData;
@interface SkUIDetailViewController : UIViewController {
    UIPopoverController* fPopOverController;
    SkOptionsTableViewController* fOptionsController;
    UIBarButtonItem* fPrintButton;
    UIBarButtonItem* fOptionsButton;
    SkData* fData;
    SkUIView* fSkUIView;
    SampleWindow* fWind;
}

@property (nonatomic, retain) UIBarButtonItem* fPrintButton;
@property (nonatomic, retain) UIBarButtonItem* fOptionsButton;
@property (nonatomic, retain) SkOptionsTableViewController* fOptionsController;
@property (nonatomic, assign) UIPopoverController* fPopOverController;

//Instance methods
- (void)populateRoot:(SkUIRootViewController*)root;
- (void)goToItem:(NSUInteger)index;
- (void)createButtons;
//UI actions
- (void)printContent;
- (void)presentOptions;

//SplitView popover management
- (void)showRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;
- (void)invalidateRootPopoverButtonItem:(UIBarButtonItem *)barButtonItem;

@end
