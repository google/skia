
/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../GLWindowContext.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "ToolUtils.h"
#include "WindowContextFactory_ios.h"
#include "gl/GrGLInterface.h"

#include <OpenGLES/ES2/gl.h>

#include "SDL.h"

using sk_app::DisplayParams;
using sk_app::window_context_factory::IOSWindowInfo;
using sk_app::GLWindowContext;

namespace {

// We use SDL to support Mac windowing mainly for convenience's sake. However, it
// does not allow us to support a purely raster backend because we have no hooks into
// the NSWindow's drawRect: method. Hence we use GL to handle the update. Should we
// want to avoid this, we will probably need to write our own windowing backend.

class RasterWindowContext_ios : public GLWindowContext {
public:
    RasterWindowContext_ios(const IOSWindowInfo&, const DisplayParams&);

    ~RasterWindowContext_ios() override;

    sk_sp<SkSurface> getBackbufferSurface() override;

    void onSwapBuffers() override;

    sk_sp<const GrGLInterface> onInitializeContext() override;
    void onDestroyContext() override;

private:
    SDL_Window*   fWindow;
    SDL_GLContext fGLContext;
    sk_sp<SkSurface> fBackbufferSurface;

    typedef GLWindowContext INHERITED;
};

RasterWindowContext_ios::RasterWindowContext_ios(const IOSWindowInfo& info,
                                                 const DisplayParams& params)
    : INHERITED(params)
    , fWindow(info.fWindow)
    , fGLContext(nullptr) {

    // any config code here (particularly for msaa)?

    this->initializeContext();
}

RasterWindowContext_ios::~RasterWindowContext_ios() {
    this->destroyContext();
}

sk_sp<const GrGLInterface> RasterWindowContext_ios::onInitializeContext() {
    SkASSERT(fWindow);
    SkASSERT(fGLContext);

    if (0 == SDL_GL_MakeCurrent(fWindow, fGLContext)) {
        glClearStencil(0);
        glClearColor(0, 0, 0, 0);
        glStencilMask(0xffffffff);
        glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &fStencilBits);
        SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &fSampleCount);
        fSampleCount = SkTMax(fSampleCount, 1);

        SDL_GL_GetDrawableSize(fWindow, &fWidth, &fHeight);
        glViewport(0, 0, fWidth, fHeight);
    } else {
        SkDebugf("MakeCurrent failed: %s\n", SDL_GetError());
    }

    // make the offscreen image
    SkImageInfo info = SkImageInfo::Make(fWidth, fHeight, fDisplayParams.fColorType,
                                         kPremul_SkAlphaType, fDisplayParams.fColorSpace);
    fBackbufferSurface = SkSurface::MakeRaster(info);
    return GrGLMakeNativeInterface();
}

void RasterWindowContext_ios::onDestroyContext() {
    fBackbufferSurface.reset(nullptr);
}

sk_sp<SkSurface> RasterWindowContext_ios::getBackbufferSurface() { return fBackbufferSurface; }

void RasterWindowContext_ios::onSwapBuffers() {
    if (fWindow && fGLContext) {
        // We made/have an off-screen surface. Get the contents as an SkImage:
        sk_sp<SkImage> snapshot = fBackbufferSurface->makeImageSnapshot();

        sk_sp<SkSurface> gpuSurface = INHERITED::getBackbufferSurface();
        SkCanvas* gpuCanvas = gpuSurface->getCanvas();
        gpuCanvas->drawImage(snapshot, 0, 0);
        gpuCanvas->flush();

        SDL_GL_SwapWindow(fWindow);
    }
}

}  // anonymous namespace

namespace sk_app {
namespace window_context_factory {

WindowContext* NewRasterForIOS(const IOSWindowInfo& info, const DisplayParams& params) {
    WindowContext* ctx = new RasterWindowContext_ios(info, params);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}

}  // namespace window_context_factory
}  // namespace sk_app
