/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#import <Cocoa/Cocoa.h>
//A text field cell that has vertically centered text
@interface SkTextFieldCell : NSTextFieldCell {
    BOOL selectingOrEditing;
}
@end
