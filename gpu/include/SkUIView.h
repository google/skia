
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#import <UIKit/UIKit.h>

#include "SkMatrix.h"
#include "FlingState.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

class SkOSWindow;
class SkEvent;

@interface SkUIView : UIView <UIAccelerometerDelegate> {
    BOOL fRedrawRequestPending;
    SkMatrix    fMatrix, fLocalMatrix;
    bool        fNeedGestureEnded;

    SkMatrix    fRotateMatrix;

    float       fFirstPinchX, fFirstPinchY;
    bool        fNeedFirstPinch;

    float       fZoomAroundX, fZoomAroundY;
    bool        fZoomAround;

    FlingState  fFlingState;

    GrAnimateFloat  fWarpState;
    bool            fUseWarp;

    struct {
        EAGLContext*    fContext;
        GLuint          fRenderbuffer;
        GLuint          fStencilbuffer;
        GLuint          fFramebuffer;
        GLint           fWidth;
        GLint           fHeight;
    } fGL;
    
    UINavigationItem* fTitle;
    SkOSWindow* fWind;
}

@property (nonatomic, assign) SkOSWindow *fWind;
@property (nonatomic, retain) UINavigationItem* fTitle;
@property (nonatomic, assign) Backend fBackend;
@property (nonatomic, assign) bool fComplexClip;
@property (nonatomic, assign, setter=setWarpState) bool fUseWarp;

- (void)initGestures;
- (void)flushLocalMatrix;

- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (BOOL)onHandleEvent:(const SkEvent&)event;

@end

