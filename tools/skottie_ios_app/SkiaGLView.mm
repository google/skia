// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaGLView.h"

#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#import <OpenGLES/ES3/gl.h>

static void configure_glkview_for_skia(GLKView* view) {
    [view setDrawableColorFormat:GLKViewDrawableColorFormatRGBA8888];
    [view setDrawableDepthFormat:GLKViewDrawableDepthFormat24];
    [view setDrawableStencilFormat:GLKViewDrawableStencilFormat8];
}

static sk_sp<SkSurface> make_gl_surface(GrContext* grContext, int width, int height) {
    static constexpr int kStencilBits = 8;
    static constexpr int kSampleCount = 1;
    static const SkSurfaceProps surfaceProps = SkSurfaceProps::kLegacyFontHost_InitType;
    if (!grContext || width <= 0 || height <= 0) {
        return nullptr;
    }
    GLint fboid = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fboid);
    return SkSurface::MakeFromBackendRenderTarget(
            grContext,
            GrBackendRenderTarget(width,
                                  height,
                                  kSampleCount,
                                  kStencilBits,
                                  GrGLFramebufferInfo{(GrGLuint)fboid, GL_RGBA8}),
            kBottomLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            nullptr,
            &surfaceProps);
}

@implementation SkiaGLView {}

- (id)initWithFrame:(CGRect)frame context:(EAGLContext*)context {
    self = [super initWithFrame:frame context:context];
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
    if (!([self grContext])) {
        NSLog(@"Error: grContext missing.\n");
        return;
    }
    if (sk_sp<SkSurface> surface = make_gl_surface([self grContext], width, height)) {
        [viewController draw:rect
                        toCanvas:(surface->getCanvas())
                        atSize:CGSize{(CGFloat)width, (CGFloat)height}];
        surface->flush();
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
