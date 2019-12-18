// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "tools/skottie_ios_app/SkiaGLView.h"

#include "include/core/SkSurface.h"
#include "include/core/SkTime.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

#import <OpenGLES/ES3/gl.h>

template<typename T> static inline T* objc_cast(id v) {
    return [v isKindOfClass:[T class]] ? static_cast<T*>(v) : nil;
}

static sk_sp<SkSurface> make_gl_surface(int width, int height,
                                        const GrGLInterface* iface, GrContext* grContext) {
    NSLog(@"make_gl_surface\n");
    const int stencilBits = 8;
    const int sampleCount = 1;
    const SkSurfaceProps surfaceProps = SkSurfaceProps::kLegacyFontHost_InitType;
    GrGLint originalFrameBuffer;
    iface->fFunctions.fGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFrameBuffer);
    if (uint32_t glError = iface->fFunctions.fGetError()) {
        SkDebugf("---- glGetError 0x%x\n");
        return nullptr;
    }
    GrGLFramebufferInfo fbInfo;
    fbInfo.fFBOID = originalFrameBuffer;
    fbInfo.fFormat = GL_RGBA8;
    GrBackendRenderTarget backendRT(width, height, sampleCount, stencilBits, fbInfo);
    return SkSurface::MakeFromBackendRenderTarget(
            grContext, backendRT, kBottomLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
            nullptr, &surfaceProps);
}

static bool configure_eagl_layer(CAEAGLLayer* eaglLayer,
                                 EAGLContext* eaglContext,
                                 CGRect frameRect,
                                 GLuint* renderbuffer) {
    NSLog(@"configure_egl_layer\n");
    eaglLayer.drawableProperties = @{kEAGLDrawablePropertyRetainedBacking : @NO,
                                     kEAGLDrawablePropertyColorFormat     : kEAGLColorFormatRGBA8 };
    eaglLayer.opaque = YES;
    eaglLayer.frame = frameRect;
    eaglLayer.contentsGravity = kCAGravityTopLeft;

    // Set up framebuffer
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glGenRenderbuffers(1, renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *renderbuffer);

    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        NSLog(@"Invalid Framebuffer\n");
        return false;
    }
    glClearStencil(0);
    glClearColor(0, 0, 0, 255);
    glStencilMask(0xFFFFFFFF);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, (int)frameRect.size.width, (int)frameRect.size.height);
    return true;
}

@interface SkiaGLView ()
@property (strong) EAGLContext* eaglContext;  // Private Access
@end

@implementation SkiaGLView {
    CGSize fViewportSize;
    GLuint fRenderbuffer;
}

+ (Class) layerClass { return [CAEAGLLayer class]; }

- (bool) initalize {
    NSLog(@"initialize\n");
    if (!([self grGLInterface] && [self grContext])) {
        return false;
    }
    const CGRect frameRect = [self frame];
    if (frameRect.size.width == fViewportSize.width &&
        frameRect.size.height == fViewportSize.height) {
        return true;
    }
    [self setEaglContext:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]];
    if (![self eaglContext]) {
        NSLog(@"Could Not Create OpenGL ES Context\n");
        return false;
    }
    if (![EAGLContext setCurrentContext:[self eaglContext]]) {
        NSLog(@"Could Not Set OpenGL ES Context As Current\n");
        return false;
    }

    CAEAGLLayer* eaglLayer = objc_cast<CAEAGLLayer>([self layer]);
    if (!eaglLayer) {
        NSLog(@"Could not get UIView layer\n");
        return false;
    }
    if (!configure_eagl_layer(eaglLayer, [self eaglContext], frameRect, &fRenderbuffer)) {
        return false;
    }
    fViewportSize = frameRect.size;
    return true;
}

- (void)drawRect:(CGRect)rect {
    NSLog(@"drawRect\n");
    SkiaViewController* viewController = [self controller];
    double next = [viewController isPaused] ? 0 : (1.0 / 30.0) + SkTime::GetNSecs() * 1e-9;

    [super drawRect:rect];
    if (![self initialize]) {
        return;
    }
    sk_sp<SkSurface> surface = make_gl_surface((int)fViewportSize.width,
                                               (int)fViewportSize.height,
                                               [self grGLInterface], [self grContext]);
    SkASSERT(surface);
    [viewController draw:rect
                    toCanvas:(surface ? surface->getCanvas() : nullptr)
                    atSize:fViewportSize];
    surface = nullptr;
    glBindRenderbuffer(GL_RENDERBUFFER, fRenderbuffer);
    [[self eaglContext] presentRenderbuffer:GL_RENDERBUFFER];

    if (next) {
        [NSTimer scheduledTimerWithTimeInterval:std::max(0.0, next - SkTime::GetNSecs() * 1e-9)
                 target:self
                 selector:@selector(setNeedsDisplay)
                 userInfo:nil
                 repeats:NO];
    }
    return;
}
@end

