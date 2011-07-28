/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#import <UIKit/UIKit.h>


@interface SkAlertPrompt : UIAlertView {
    UITextField *textField;
}
@property (nonatomic, retain) UITextField *textField;

- (NSString*)enteredText;

@end
