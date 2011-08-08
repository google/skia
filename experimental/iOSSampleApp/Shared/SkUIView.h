
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
#include "SkMatrix.h"
#include "FlingState.h"
#include "SampleApp.h"

class SkiOSDeviceManager;
class SkOSWindow;
class SkEvent;
struct FPSState;
@class SkUIView;

@protocol SkUIViewOptionsDelegate <NSObject>
@optional
// Called when the view needs to handle adding an SkOSMenu
- (void) view:(SkUIView*)view didAddMenu:(const SkOSMenu*)menu;
- (void) view:(SkUIView*)view didUpdateMenu:(const SkOSMenu*)menu;
@end

@interface SkUIView : UIView  {
    BOOL fRedrawRequestPending;

    struct {
        EAGLContext*    fContext;
        GLuint          fRenderbuffer;
        GLuint          fStencilbuffer;
        GLuint          fFramebuffer;
        GLint           fWidth;
        GLint           fHeight;
    } fGL;
    
    NSString* fTitle;
    UINavigationItem* fTitleItem;
    CALayer* fRasterLayer;
    CAEAGLLayer* fGLLayer;
    
    FPSState* fFPSState;
    SkOSWindow* fWind;
    SkiOSDeviceManager* fDevManager;
    
    id<SkUIViewOptionsDelegate> fOptionsDelegate;
}

@property (nonatomic, readonly) SkOSWindow *fWind;
@property (nonatomic, retain) UINavigationItem* fTitleItem;
@property (nonatomic, copy) NSString* fTitle;
@property (nonatomic, retain) CALayer* fRasterLayer;
@property (nonatomic, retain) CAEAGLLayer* fGLLayer;
@property (nonatomic, assign) id<SkUIViewOptionsDelegate> fOptionsDelegate;

- (void)forceRedraw;

- (void)setSkTitle:(const char*)title;
- (void)onAddMenu:(const SkOSMenu*)menu;
- (void)onUpdateMenu:(const SkOSMenu*)menu;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;
@end