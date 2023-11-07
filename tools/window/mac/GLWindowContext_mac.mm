
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLInterface.h"
#include "tools/window/GLWindowContext.h"
#include "tools/window/mac/WindowContextFactory_mac.h"

#include <OpenGL/gl.h>
#include <Cocoa/Cocoa.h>

using skwindow::DisplayParams;
using skwindow::MacWindowInfo;
using skwindow::internal::GLWindowContext;

namespace {

class GLWindowContext_mac : public GLWindowContext {
public:
    GLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~GLWindowContext_mac() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

    void resize(int w, int h) override;

private:
    void teardownContext();
    void onSwapBuffers() override;

    NSView*              fMainView;
    NSOpenGLContext*     fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
        : GLWindowContext(params)
        , fMainView(info.fMainView)
        , fGLContext(nil) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
    teardownContext();
}

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
        fPixelFormat = skwindow::GetGLPixelFormat(fDisplayParams.fMSAASampleCount);
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

    GLint swapInterval = fDisplayParams.fDisableVsync ? 0 : 1;
    [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];

    // make context current
    [fGLContext makeCurrentContext];

    glClearStencil(0);
    glClearColor(0, 0, 0, 255);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

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
    glViewport(0, 0, fWidth, fHeight);

    return GrGLMakeNativeInterface();
}

void GLWindowContext_mac::onDestroyContext() {
    // We only need to tear down the GLContext if we've changed the sample count.
    if (fGLContext && fSampleCount != fDisplayParams.fMSAASampleCount) {
        teardownContext();
    }
}

void GLWindowContext_mac::onSwapBuffers() {
    [fGLContext flushBuffer];
}

void GLWindowContext_mac::resize(int w, int h) {
    [fGLContext update];

    // The super class always recreates the context.
    GLWindowContext::resize(0, 0);
}


}  // anonymous namespace

namespace skwindow {

std::unique_ptr<WindowContext> MakeGLForMac(const MacWindowInfo& info,
                                            const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new GLWindowContext_mac(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace skwindow
