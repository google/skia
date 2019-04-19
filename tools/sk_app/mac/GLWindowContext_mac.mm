
/*
 * Copyright 2019 Google Inc.
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
    NSView*              fMainView;
    NSOpenGLView*        fGLView;
    NSOpenGLContext*     fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;

    typedef GLWindowContext INHERITED;
};

GLWindowContext_mac::GLWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fMainView(info.fMainView) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

GLWindowContext_mac::~GLWindowContext_mac() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> GLWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    // set up pixel format
    constexpr int kMaxAttributes = 18;
    NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
    int numAttributes = 0;
    attributes[numAttributes++] = NSOpenGLPFAAccelerated;
    attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
    attributes[numAttributes++] = NSOpenGLPFADoubleBuffer;
    attributes[numAttributes++] = NSOpenGLPFAOpenGLProfile;
    attributes[numAttributes++] = NSOpenGLProfileVersion3_2Core;
    attributes[numAttributes++] = NSOpenGLPFAColorSize;
    attributes[numAttributes++] = 24;
    attributes[numAttributes++] = NSOpenGLPFAAlphaSize;
    attributes[numAttributes++] = 8;
    attributes[numAttributes++] = NSOpenGLPFADepthSize;
    attributes[numAttributes++] = 0;
    attributes[numAttributes++] = NSOpenGLPFAStencilSize;
    attributes[numAttributes++] = 8;
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

    // create view
    NSRect rect = fMainView.bounds;
    fGLView = [[NSOpenGLView alloc] initWithFrame:rect];
    if (nil == fGLView) {
        [fGLContext release];
        fGLContext = nil;
        [fPixelFormat release];
        fPixelFormat = nil;
        return nullptr;
    }
    [fGLView setTranslatesAutoresizingMaskIntoConstraints:NO];

    // attach OpenGL view to main view
    [fMainView addSubview:fGLView];
    NSDictionary *views = NSDictionaryOfVariableBindings(fGLView);

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[fGLView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[fGLView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    // make context current
    GLint swapInterval = fDisplayParams.fDisableVsync ? 0 : 1;
    [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
    [fGLView setOpenGLContext:fGLContext];
    [fGLView setPixelFormat:fPixelFormat];
    // TODO: support Retina displays
    [fGLView setWantsBestResolutionOpenGLSurface:NO];
    [fGLContext setView:fGLView];

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
    fSampleCount = SkTMax(fSampleCount, 1);

    const NSRect viewportRect = [fGLView bounds];
    fWidth = viewportRect.size.width;
    fHeight = viewportRect.size.height;
    glViewport(0, 0, fWidth, fHeight);

    return GrGLMakeNativeInterface();
}

void GLWindowContext_mac::onDestroyContext() {
    [fGLView removeFromSuperview];
    [fGLView release];
    fGLView = nil;
    [fGLContext release];
    fGLContext = nil;
    [fPixelFormat release];
    fPixelFormat = nil;
}

void GLWindowContext_mac::onSwapBuffers() {
    [fGLContext flushBuffer];
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
