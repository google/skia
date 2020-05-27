/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/bind.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "tools/viewer/SampleSlide.h"
#include <GLES3/gl3.h>
#include <string>

using namespace emscripten;

sk_sp<Slide> MakeSlide(std::string name) {
    if (name == "WavyPathText") {
        extern Sample* MakeWavyPathTextSample();
        return sk_make_sp<SampleSlide>(MakeWavyPathTextSample);
    }
    return nullptr;
}

static void delete_wrapped_framebuffer(SkSurface::ReleaseContext context) {
    GLuint framebuffer = (GLuint)context;
    glDeleteFramebuffers(1, &framebuffer);
}

sk_sp<SkSurface> MakeOffscreenFramebuffer(sk_sp<GrContext> grContext, int width, int height,
                                          int sampleCnt) {
    GLuint colorBuffer;
    glGenRenderbuffers(1, &colorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCnt, GL_RGBA8, width, height);

    GLuint stencilBuffer;
    glGenRenderbuffers(1, &stencilBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, stencilBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, sampleCnt, GL_STENCIL_INDEX8, width, height);

    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                              colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                              stencilBuffer);

    // Unbind "framebuffer" before orphaning its renderbuffers. (Otherwise they are spec'd to be
    // detached from the currently bound framebuffer.)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteRenderbuffers(1, &colorBuffer);
    glDeleteRenderbuffers(1, &stencilBuffer);

    grContext->resetContext(kRenderTarget_GrGLBackendState);

    GrGLFramebufferInfo glInfo;
    glInfo.fFBOID = framebuffer;
    glInfo.fFormat = GL_RGBA8;
    GrBackendRenderTarget backendRenderTarget(width, height, sampleCnt, 8, glInfo);
    return SkSurface::MakeFromBackendRenderTarget(grContext.get(), backendRenderTarget,
                                                  kBottomLeft_GrSurfaceOrigin,
                                                  SkColorType::kRGBA_8888_SkColorType, nullptr,
                                                  nullptr, &delete_wrapped_framebuffer,
                                                  (SkSurface::ReleaseContext)framebuffer);
}

enum class GLFilter {
    kNearest = GL_NEAREST,
    kLinear = GL_LINEAR
};

void BlitOffscreenFramebuffer(sk_sp<SkSurface> surface, int srcX0, int srcY0, int srcX1, int srcY1,
                              int dstX0, int dstY0, int dstX1, int dstY1, GLFilter filter) {
  surface->flush(SkSurface::BackendSurfaceAccess::kPresent, GrFlushInfo());
  GrGLFramebufferInfo glInfo;
  auto backendRT = surface->getBackendRenderTarget(SkSurface::kFlushRead_BackendHandleAccess);
  backendRT.getGLFramebufferInfo(&glInfo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, glInfo.fFBOID);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, GL_COLOR_BUFFER_BIT,
                    (GLenum)filter);
  surface->getContext()->resetContext(kRenderTarget_GrGLBackendState);
}

EMSCRIPTEN_BINDINGS(Viewer) {
    function("MakeSlide", &MakeSlide);
    function("MakeOffscreenFramebuffer", &MakeOffscreenFramebuffer);
    function("BlitOffscreenFramebuffer", &BlitOffscreenFramebuffer);
    class_<Slide>("Slide")
        .smart_ptr<sk_sp<Slide>>("sk_sp<Slide>")
        .function("load", &Slide::load)
        .function("animate", &Slide::animate)
        .function("draw", optional_override([](Slide& self, SkCanvas& canvas) {
            self.draw(&canvas);
        }));
    enum_<GLFilter>("GLFilter")
        .value("Nearest",   GLFilter::kNearest)
        .value("Linear",    GLFilter::kLinear);
}
