/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleApp.h"
#import "SkUIView.h"

class SkiOSDeviceManager;
class SkOSWindow;
class SkEvent;
struct FPSState;

@interface SkSampleUIView : SkUIView  {
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
    CALayer* fRasterLayer;
    CAEAGLLayer* fGLLayer;

    FPSState* fFPSState;
    SkiOSDeviceManager* fDevManager;
}

@property (nonatomic, copy) NSString* fTitle;
@property (nonatomic, retain) CALayer* fRasterLayer;
@property (nonatomic, retain) CAEAGLLayer* fGLLayer;

- (id)initWithDefaults;
- (void)drawInRaster;
- (void)forceRedraw;

- (void)setSkTitle:(const char*)title;
- (void)postInvalWithRect:(const SkIRect*)rectOrNil;
- (void)getAttachmentInfo:(SkOSWindow::AttachmentInfo*)info;

@end
