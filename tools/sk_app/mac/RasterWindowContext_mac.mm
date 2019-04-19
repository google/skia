
/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../GLWindowContext.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "ToolUtils.h"
#include "WindowContextFactory_mac.h"
#include "gl/GrGLInterface.h"

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
    void onDestroyContext() override;

private:
    NSView*              fMainView;
    NSOpenGLView*        fRasterView;
    NSOpenGLContext*     fGLContext;
    NSOpenGLPixelFormat* fPixelFormat;
    sk_sp<SkSurface>     fBackbufferSurface;

    typedef GLWindowContext INHERITED;
};

RasterWindowContext_mac::RasterWindowContext_mac(const MacWindowInfo& info,
                                                 const DisplayParams& params)
    : INHERITED(params)
    , fMainView(info.fMainView) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

RasterWindowContext_mac::~RasterWindowContext_mac() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> RasterWindowContext_mac::onInitializeContext() {
    SkASSERT(nil != fMainView);

    // set up pixel format
    constexpr int kMaxAttributes = 18;
    NSOpenGLPixelFormatAttribute attributes[kMaxAttributes];
    int numAttributes = 0;
    attributes[numAttributes++] = NSOpenGLPFAAccelerated;
    attributes[numAttributes++] = NSOpenGLPFAClosestPolicy;
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

    // create view
    NSRect rect = fMainView.bounds;
    fRasterView = [[NSOpenGLView alloc] initWithFrame:rect];
    if (nil == fRasterView) {
        [fGLContext release];
        fGLContext = nil;
        [fPixelFormat release];
        fPixelFormat = nil;
        return nullptr;
    }
    [fRasterView setTranslatesAutoresizingMaskIntoConstraints:NO];

    // attach OpenGL view to main view
    [fMainView addSubview:fRasterView];
    NSDictionary *views = NSDictionaryOfVariableBindings(fRasterView);

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"H:|[fRasterView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    [fMainView addConstraints:
     [NSLayoutConstraint constraintsWithVisualFormat:@"V:|[fRasterView]|"
                                             options:0
                                             metrics:nil
                                               views:views]];

    // make context current
    GLint swapInterval = 1;
    [fGLContext setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
    [fRasterView setOpenGLContext:fGLContext];
    [fRasterView setPixelFormat:fPixelFormat];
    // TODO: support Retina displays
    [fRasterView setWantsBestResolutionOpenGLSurface:NO];
    [fGLContext setView:fRasterView];

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

    const NSRect viewportRect = [fRasterView bounds];
    fWidth = viewportRect.size.width;
    fHeight = viewportRect.size.height;
    glViewport(0, 0, fWidth, fHeight);

    // make the offscreen image
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, fDisplayParams.fColorType,
                                         kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info);
    return GrGLMakeNativeInterface();
}

void RasterWindowContext_mac::onDestroyContext() {
    fBackbufferSurface.reset(nullptr);

    [fRasterView removeFromSuperview];
    [fRasterView release];
    fRasterView = nil;
    [fGLContext release];
    fGLContext = nil;
    [fPixelFormat release];
    fPixelFormat = nil;
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

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewRasterForMac(const MacWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new RasterWindowContext_mac(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
