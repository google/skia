
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
#include "SkiOSDeviceManager.h"
class SkOSWindow;
class SkEvent;
struct FPSState;
@interface SkUIView : UIView <UIAccelerometerDelegate> {
    BOOL fRedrawRequestPending;
    SkMatrix    fMatrix;

    float       fZoomAroundX, fZoomAroundY;
    bool        fZoomAround;

    struct {
        EAGLContext*    fContext;
        GLuint          fRenderbuffer;
        GLuint          fStencilbuffer;
        GLuint          fFramebuffer;
        GLint           fWidth;
        GLint           fHeight;
    } fGL;
    
    FPSState* fFPSState;
    NSString* fTitle;
    UINavigationItem* fTitleItem;
    SkOSWindow* fWind;
    CALayer* fRasterLayer;
    CAEAGLLayer* fGLLayer;
    
    SkiOSDeviceManager* fDevManager;
}

@property (nonatomic, assign) SkOSWindow *fWind;
@property (nonatomic, retain) UINavigationItem* fTitleItem;
@property (nonatomic, copy) NSString* fTitle;
@property (nonatomic, retain) CALayer* fRasterLayer;
@property (nonatomic, retain) CAEAGLLayer* fGLLayer;

- (void)forceRedraw;

- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;
@end

