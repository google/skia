
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "tools/ToolUtils.h"
#include "tools/sk_app/GLWindowContext.h"
#include "tools/sk_app/mac/WindowContextFactory_mac.h"

#include <OpenGL/gl.h>

#include <Cocoa/Cocoa.h>

using sk_app::DisplayParams;
using sk_app::window_context_factory::MacWindowInfo;
using sk_app::GLWindowContext;

namespace {

// TODO: This still uses GL to handle the update rather than using a purely raster backend,
// for historical reasons. Writing a pure raster backend would be better in the long run.

class RasterWindowContext_mac : public GLWindowContext {
public:
    RasterWindowContext_mac(const MacWindowInfo&, const DisplayParams&);

    ~RasterWindowContext_mac() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override {}

    void resize(int w, int h) override;

private:
    NSView*              fMainView;
    NSOpenGLContext*     fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;
    sk_sp<SkSurface>     fBackbufferSurface;

    using INHERITED = GLWindowContext;
};

RasterWindowContext_mac::RasterWindowContext_mac(const MacWindowInfo& info,
                                                 const DisplayParams& params)
    : INHERITED(params)
    , fMainView(info.fMainView)
    , fGLContext(nil) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

RasterWindowContext_mac::~RasterWindowContext_mac() {
    [NSOpenGLContext clearCurrentContext];
    [fPixelFormat release];
    fPixelFormat = nil;
    [fGLContext release];
    fGLContext = nil;
}

sk_sp<const GrGLInterface> RasterWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    if (!fGLContext) {
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

        [fMainView setWantsBestResolutionOpenGLSurface:YES];
        [fGLContext setView:fMainView];

        GLint swapInterval = fDisplayParams.fDisableVsync ? 0 : 1;
        [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
    }

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

    CGFloat backingScaleFactor = sk_app::GetBackingScaleFactor(fMainView);
    fWidth = fMainView.bounds.size.width * backingScaleFactor;
    fHeight = fMainView.bounds.size.height * backingScaleFactor;
    glViewport(0, 0, fWidth, fHeight);

    // make the offscreen image
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, fDisplayParams.fColorType,
                                         kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info);
    return GrGLMakeNativeInterface();
}

sk_sp<SkSurface> RasterWindowContext_mac::getBackbufferSurface() { return fBackbufferSurface; }

void RasterWindowContext_mac::onSwapBuffers() {
    if (fBackbufferSurface) {
        // We made/have an off-screen surface. Get the contents as an SkImage:
        sk_sp<SkImage> snapshot = fBackbufferSurface->makeImageSnapshot();

        sk_sp<SkSurface> gpuSurface = INHERITED::getBackbufferSurface();
        SkCanvas* gpuCanvas = gpuSurface->getCanvas();
        gpuCanvas->drawImage(snapshot, 0, 0);
        gpuCanvas->flush();

        [fGLContext flushBuffer];
    }
}

void RasterWindowContext_mac::resize(int w, int h) {
    [fGLContext update];

    // The super class always recreates the context.
    INHERITED::resize(0, 0);
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

std::unique_ptr<WindowContext> MakeRasterForMac(const MacWindowInfo& info,
                                                const DisplayParams& params) {
    std::unique_ptr<WindowContext> ctx(new RasterWindowContext_mac(info, params));
    if (!ctx->isValid()) {
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
