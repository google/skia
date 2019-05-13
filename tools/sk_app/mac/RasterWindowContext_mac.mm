
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
    void onDestroyContext() override;

private:
    GLFWwindow* fWindow;
    sk_sp<SkSurface>     fBackbufferSurface;

    typedef GLWindowContext INHERITED;
};

RasterWindowContext_mac::RasterWindowContext_mac(const MacWindowInfo& info,
                                                 const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

RasterWindowContext_mac::~RasterWindowContext_mac() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> RasterWindowContext_mac::onInitializeContext() {
    SkASSERT(fWindow);

    fStencilBits = 8;
    fSampleCount = 1;

    glfwMakeContextCurrent(fWindow);

    glfwGetWindowSize(fWindow, &fWidth, &fHeight);
    glViewport(0, 0, fWidth, fHeight);

    // make the offscreen image
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, fDisplayParams.fColorType,
                                         kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info);
    return GrGLMakeNativeInterface();
}

void RasterWindowContext_mac::onDestroyContext() {
    fBackbufferSurface.reset(nullptr);
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

        glfwSwapBuffers(fWindow);
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
