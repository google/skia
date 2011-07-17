//
//  SkAlertPrompt.h
//  iOSSampleApp
//
//  Created by Yang Su on 7/6/11.
//  Copyright 2011 Google Inc. All rights reserved.
//

#import <UIKit/UIKit.h>


@interface SkAlertPrompt : UIAlertView {
    UITextField *textField;
}
@property (nonatomic, retain) UITextField *textField;

- (NSString*)enteredText;

@end
