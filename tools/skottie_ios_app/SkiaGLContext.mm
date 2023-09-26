// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaContext.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/base/SkTime.h"

#import <GLKit/GLKit.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/ES3/gl.h>

#include <CoreFoundation/CoreFoundation.h>

static void configure_glkview_for_skia(GLKView* view) {
    [view setDrawableColorFormat:GLKViewDrawableColorFormatRGBA8888];
    [view setDrawableDepthFormat:GLKViewDrawableDepthFormat24];
    [view setDrawableStencilFormat:GLKViewDrawableStencilFormat8];
}

static sk_sp<SkSurface> make_gl_surface(GrDirectContext* dContext, int width, int height) {
    static constexpr int kStencilBits = 8;
    static constexpr int kSampleCount = 1;
    static const SkSurfaceProps surfaceProps;
    if (!dContext || width <= 0 || height <= 0) {
        return nullptr;
    }
    GLint fboid = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboid);
    return SkSurfaces::WrapBackendRenderTarget(
            dContext,
            GrBackendRenderTargets::MakeGL(width,
                                           height,
                                           kSampleCount,
                                           kStencilBits,
                                           GrGLFramebufferInfo{(GrGLuint)fboid, GL_RGBA8}),
            kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            nullptr,
            &surfaceProps);
}

// A UIView that uses a GL-backed SkSurface to draw.
@interface SkiaGLView : GLKView
    @property (strong) SkiaViewController* controller;

    // Override of the UIView interface.
    - (void)drawRect:(CGRect)rect;

    // Required initializer.
    - (instancetype)initWithFrame:(CGRect)frame
                    withEAGLContext:(EAGLContext*)eaglContext
                    withDirectContext:(GrDirectContext*)dContext;
@end

@implementation SkiaGLView {
    GrDirectContext* fDContext;
}

- (instancetype)initWithFrame:(CGRect)frame
                withEAGLContext:(EAGLContext*)eaglContext
                withDirectContext:(GrDirectContext*)dContext {
    self = [super initWithFrame:frame context:eaglContext];
    fDContext = dContext;
    configure_glkview_for_skia(self);
    return self;
}

- (void)drawRect:(CGRect)rect {
    SkiaViewController* viewController = [self controller];
    static constexpr double kFrameRate = 1.0 / 30.0;
    double next = [viewController isPaused] ? 0 : kFrameRate + SkTime::GetNSecs() * 1e-9;

    [super drawRect:rect];

    int width  = (int)[self drawableWidth],
        height = (int)[self drawableHeight];
    if (!(fDContext)) {
        NSLog(@"Error: GrDirectContext missing.\n");
        return;
    }
    if (sk_sp<SkSurface> surface = make_gl_surface(fDContext, width, height)) {
        [viewController draw:rect
                        toCanvas:(surface->getCanvas())
                        atSize:CGSize{(CGFloat)width, (CGFloat)height}];
        fDContext->flushAndSubmit(surface.get());
    }
    if (next) {
        [NSTimer scheduledTimerWithTimeInterval:std::max(0.0, next - SkTime::GetNSecs() * 1e-9)
                 target:self
                 selector:@selector(setNeedsDisplay)
                 userInfo:nil
                 repeats:NO];
    }
}
@end

@interface SkiaGLContext : SkiaContext
    @property (strong) EAGLContext* eaglContext;
    - (instancetype) init;
    - (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame;
    - (SkiaViewController*) getViewController:(UIView*)view;
@end

@implementation SkiaGLContext {
    sk_sp<GrDirectContext> fDContext;
}
- (instancetype) init {
    self = [super init];
    [self setEaglContext:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]];
    if (![self eaglContext]) {
        NSLog(@"Falling back to GLES2.\n");
        [self setEaglContext:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]];
    }
    if (![self eaglContext]) {
        NSLog(@"[[EAGLContext alloc] initWithAPI:...] failed");
        return nil;
    }
    EAGLContext* oldContext = [EAGLContext currentContext];
    [EAGLContext setCurrentContext:[self eaglContext]];
    fDContext = GrDirectContexts::MakeGL(nullptr, GrContextOptions());
    [EAGLContext setCurrentContext:oldContext];
    if (!fDContext) {
        NSLog(@"GrDirectContexts::MakeGL failed");
        return nil;
    }
    return self;
}

- (UIView*) makeViewWithController:(SkiaViewController*)vc withFrame:(CGRect)frame {
    SkiaGLView* skiaView = [[SkiaGLView alloc] initWithFrame:frame
                                               withEAGLContext:[self eaglContext]
                                               withDirectContext:fDContext.get()];
    [skiaView setController:vc];
    return skiaView;
}
- (SkiaViewController*) getViewController:(UIView*)view {
    return [view isKindOfClass:[SkiaGLView class]] ? [(SkiaGLView*)view controller] : nil;
}
@end

SkiaContext* MakeSkiaGLContext() { return [[SkiaGLContext alloc] init]; }
