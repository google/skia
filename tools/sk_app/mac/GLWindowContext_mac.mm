
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../GLWindowContext.h"
#include "WindowContextFactory_mac.h"
#include "gl/GrGLInterface.h"

#include <OpenGL/gl.h>
#include <Cocoa/Cocoa.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::GLWindowContext;

namespace {

class GLWindowContext_mac : public GLWindowContext {
public:
    GLWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~GLWindowContext_mac() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

private:
    NSWindow*            fWindow;
    NSView*              fGLView;
    NSOpenGLContext*     fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;

    typedef GLWindowContext INHERITED;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fGLView(info.fEventView) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fWindow && nil != fGLView);

#if MAC_OS_X_VERSION_MAX_ALLOWED < 1070
    // doesn't support OpenGL 3.0
    return nullptr;
#endif

    // set up pixel format
    constexpr int kMaxAttributes = 16;
    NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
    int numAttributes = 0;
    attributes[numAttributes++] = NSOpenGLPFAAccelerated;
    attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
    attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
    attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
    attributes[numAttributes++] = NSOpenGLPFAColorSize;
    attributes[numAttributes++] = 24;
    attributes[numAttributes++] = NSOpenGLPFADepthSize;
    attributes[numAttributes++] = 0;
    attributes[numAttributes++] = NSOpenGLPFAStencilSize;
    attributes[numAttributes++] = 8;
    attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
    if (fDisplayParams.fMSAASampleCount > 1) {
        attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
        attributes[numAttributes++] = 1;
        attributes[numAttributes++] = NSOpenGLPFASamples;
        attributes[numAttributes++] = fDisplayParams.fMSAASampleCount;
    } else {
        attributes[numAttributes++] = NSOpenGLPFASampleBuffers;
        attributes[numAttributes++] = 0;
    }
    attributes[numAttributes++] = 0;
    SkASSERT(numAttributes <= kMaxAttributes);

    fPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
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

    // make context current
    [fGLContext setView:fGLView];
    [fGLContext makeCurrentContext];

    glClearStencil(0);
    glClearColor(0, 0, 0, 0);
    glStencilMask(0xffffffff);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    GLint stencilBits;
    [fPixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
    fStencilBits = stencilBits;
    GLint sampleCount;
    [fPixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
    fSampleCount = sampleCount;
    fSampleCount = SkTMax(fSampleCount, 1);

    glViewport(0, 0, fGLView.bounds.size.width, fGLView.bounds.size.height);

    return GrGLMakeNativeInterface();
}

void GLWindowContext_mac::onDestroyContext() {
    //*** release view and window?
    [fGLContext release];
    fGLContext = nil;
    [fPixelFormat release];
    fPixelFormat = nil;
}

void GLWindowContext_mac::onSwapBuffers() {
    if (nil != fWindow && nil != fGLContext) {
        [fGLContext flushBuffer];
    }
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewGLForMac(const MacWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new GLWindowContext_mac(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
