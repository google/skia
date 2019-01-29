
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../MetalWindowContext.h"
#include "WindowContextFactory_mac.h"
// TODO #include "mtl/GrMtlInterface.h"

#import <MetalKit/MetalKit.h>

#include <Cocoa/Cocoa.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::MetalWindowContext;

@interface MetalView : MTKView
@end

@implementation MetalView

- (void)drawRect:(NSRect)dirtyRect {
    // not sure why the parent isn't getting this, but we'll pass it up
    [[self superview] drawRect:dirtyRect];
}

@end

namespace {

class MetalWindowContext_mac : public MetalWindowContext {
public:
    MetalWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~MetalWindowContext_mac() override;

    void onSwapBuffers() override;

    /*sk_sp<const GrGLInterface>*/ bool onInitializeContext() override;
    void onDestroyContext() override;

private:
    NSView*              fMainView;
    MetalView*              fMetalView;
    NSOpenGLContext*     fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;

    typedef MetalWindowContext INHERITED;
};

MetalWindowContext_mac::MetalWindowContext_mac(const MacWindowInfo& info, const DisplayParams& params)
    : INHERITED(params)
    , fMainView(info.fMainView) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

MetalWindowContext_mac::~MetalWindowContext_mac() {
    this->destroyContext();
}

/*sk_sp<const GrGLInterface>*/ bool MetalWindowContext_mac::onInitializeContext() {
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
    fMetalView = [[MetalView alloc] initWithFrame:rect];
    if (nil == fMetalView) {
        [fGLContext release];
        fGLContext = nil;
        [fPixelFormat release];
        fPixelFormat = nil;
        return nullptr;
    }
    [fMetalView setTranslatesAutoresizingMaskIntoConstraints:NO];

    // attach OpenGL view to main view
    [fMainView addSubview:fMetalView];
    NSDictionary *views = NSDictionaryOfVariableBindings(fMetalView);

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[fMetalView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[fMetalView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    // make context current
//    GLint swapInterval = 1;
//    [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
//    [fMetalView setOpenGLContext:fGLContext];
//    [fMetalView setPixelFormat:fPixelFormat];
//    [fMetalView setWantsBestResolutionOpenGLSurface:YES];
//    [fGLContext setView:fMetalView];
//
//    [fGLContext makeCurrentContext];
//
//    glClearStencil(0);
//    glClearColor(0, 0, 0, 255);
//    glStencilMask(0xffffffff);
//    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//
//    GLint stencilBits;
//    [fPixelFormat getValues:&stencilBits forAttribute:NSOpenGLPFAStencilSize forVirtualScreen:0];
//    fStencilBits = stencilBits;
//    GLint sampleCount;
//    [fPixelFormat getValues:&sampleCount forAttribute:NSOpenGLPFASamples forVirtualScreen:0];
//    fSampleCount = sampleCount;
//    fSampleCount = SkTMax(fSampleCount, 1);
//
//    const NSRect backingRect = [fMetalView convertRectToBacking:fMetalView.bounds];
//    fWidth = backingRect.size.width;
//    fHeight = backingRect.size.height;
//    MetalViewport(0, 0, fWidth, fHeight);

    return true;//GrMTLMakeNativeInterface();
}

void MetalWindowContext_mac::onDestroyContext() {
    [fMetalView removeFromSuperview];
    [fMetalView release];
    fMetalView = nil;
    [fGLContext release];
    fGLContext = nil;
    [fPixelFormat release];
    fPixelFormat = nil;
}

void MetalWindowContext_mac::onSwapBuffers() {
    [fGLContext flushBuffer];
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewMetalForMac(const MacWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new MetalWindowContext_mac(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
