/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/window/mac/GaneshGLWindowContext_mac.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/mac/GrGLMakeMacInterface.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tools/window/GLWindowContext.h"
#include "tools/window/mac/MacWindowGLUtils.h"
#include "tools/window/mac/MacWindowInfo.h"

#include <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>

using skwindow::DisplayParams;
using skwindow::MacWindowInfo;
using skwindow::internal::GLWindowContext;

// All of NSOpenGL is deprecated.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

namespace {

class GLWindowContext_mac : public GLWindowContext {
public:
    GLWindowContext_mac(const MacWindowInfo&, std::unique_ptr<const DisplayParams>);

    ~GLWindowContext_mac() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    void teardownContext();
    void onSwapBuffers() override;

    NSView* fMainView;
    NSOpenGLContext* fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info,
                                         std::unique_ptr<const DisplayParams> params)
        : GLWindowContext(std::move(params)), fMainView(info.fMainView), fGLContext(nil) {
    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() { teardownContext(); }

void GLWindowContext_mac::teardownContext() {
    [NSOpenGLContext clearCurrentContext];
    [fPixelFormat release];
    fPixelFormat = nil;
    [fGLContext release];
    fGLContext = nil;
}

sk_sp<const GrGLInterface> GLWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    if (!fGLContext) {
        fPixelFormat = skwindow::GetGLPixelFormat(fDisplayParams->msaaSampleCount());
        if (nil == fPixelFormat) {
            return nullptr;
        }

        // create context
        fGLContext = [[NSOpenGLContext alloc] initWithFormat:fPixelFormat shareContext:nil];
        if (nil == fGLContext) {
            [fPixelFormat release];
            fPixelFormat = nil;
            return nullptr;
        }

        [fMainView setWantsBestResolutionOpenGLSurface:YES];
        [fGLContext setView:fMainView];
    }

    GLint swapInterval = fDisplayParams->disableVsync() ? 0 : 1;
    [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

    // make context current
    [fGLContext makeCurrentContext];

    sk_sp<const GrGLInterface> backendContext = GrGLInterfaces::MakeMac();
    const GrGLInterface* gl = backendContext.get();

    GR_GL_CALL(gl, ClearStencil(0));
    GR_GL_CALL(gl, ClearColor(0, 0, 0, 255));
    GR_GL_CALL(gl, StencilMask(0xffffffff));
    GR_GL_CALL(gl, Clear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

    GLint stencilBits;
    [fPixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
    fStencilBits = stencilBits;
    GLint sampleCount;
    [fPixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
    fSampleCount = sampleCount;
    fSampleCount = std::max(fSampleCount, 1);

    CGFloat backingScaleFactor = skwindow::GetBackingScaleFactor(fMainView);
    fWidth = fMainView.bounds.size.width * backingScaleFactor;
    fHeight = fMainView.bounds.size.height * backingScaleFactor;
    GR_GL_CALL(gl, Viewport(0, 0, fWidth, fHeight));

    return backendContext;
}

void GLWindowContext_mac::onDestroyContext() {
    // We only need to tear down the GLContext if we've changed the sample count.
    if (fGLContext && fSampleCount != fDisplayParams->msaaSampleCount()) {
        teardownContext();
    }
}

void GLWindowContext_mac::onSwapBuffers() {
    GrDirectContext* dContext = fSurface->recordingContext()->asDirectContext();
    dContext->flush(fSurface.get(), SkSurfaces::BackendSurfaceAccess::kPresent, {});
    [fGLContext flushBuffer];
}

void GLWindowContext_mac::resize(int w, int h) {
    [fGLContext update];

    // The super class always recreates the context.
    GLWindowContext::resize(0, 0);
}

}  // anonymous namespace

#pragma clang diagnostic pop

namespace skwindow {

std::unique_ptr<WindowContext> MakeGaneshGLForMac(const MacWindowInfo& info,
                                                  std::unique_ptr<const DisplayParams> params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_mac(info, std::move(params)));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
