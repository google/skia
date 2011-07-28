//
//  SkAlertPrompt.m
//  iOSSampleApp
//
//  Created by Yang Su on 7/6/11.
//  Copyright 2011 Google Inc.
//  Use of this source code is governed by a BSD-style license that can be
//  found in the LICENSE file.
//

#import "SkAlertPrompt.h"

@implementation SkAlertPrompt
@synthesize textField;

- (id)initWithTitle:(NSString *)title
            message:(NSString *)message
           delegate:(id)delegate
  cancelButtonTitle:(NSString *)cancelButtonTitle
     otherButtonTitles:(NSString *)okayButtonTitle,... {
    if (self = [super initWithTitle:title
                            message:message
                           delegate:delegate
                  cancelButtonTitle:cancelButtonTitle
                  otherButtonTitles:okayButtonTitle, nil]) {
        self.textField = [[UITextField alloc]
                          initWithFrame:CGRectMake(12, 45, 260, 25)];
        [self.textField setBackgroundColor:[UIColor whiteColor]];
        textField.borderStyle = UITextBorderStyleLine;
        [self addSubview:self.textField];
    }
    return self;
}

- (void)show {
    [textField becomeFirstResponder];
    [super show];
}

- (NSString *)enteredText {
    return textField.text;
}

- (void)dealloc {
    [textField release];
    [super dealloc];
}

@end
