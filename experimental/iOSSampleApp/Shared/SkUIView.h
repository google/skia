/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#include "SkOSWindow_ios.h"

class SkEvent;
@class SkUIView;

@protocol SkUIViewOptionsDelegate <NSObject>
@optional
// Called when the view needs to handle adding an SkOSMenu
- (void) view:(SkUIView*)view didAddMenu:(const SkOSMenu*)menu;
- (void) view:(SkUIView*)view didUpdateMenu:(SkOSMenu*)menu;
@end

@interface SkUIView : UIView  {
    UINavigationItem* fTitleItem;
    SkOSWindow* fWind;
    id<SkUIViewOptionsDelegate> fOptionsDelegate;
}

@property (nonatomic, readonly) SkOSWindow *fWind;
@property (nonatomic, retain) UINavigationItem* fTitleItem;
@property (nonatomic, assign) id<SkUIViewOptionsDelegate> fOptionsDelegate;

- (id)initWithDefaults;
- (void)setUpWindow;
- (void)forceRedraw;
- (void)drawInRaster;

- (void)setSkTitle:(const char*)title;
- (void)onAddMenu:(const SkOSMenu*)menu;
- (void)onUpdateMenu:(SkOSMenu*)menu;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;
- (void)getAttachmentInfo:(SkOSWindow::AttachmentInfo*)info;

@end
