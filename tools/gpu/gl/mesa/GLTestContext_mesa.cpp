
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <GL/osmesa.h>

#include "gl/mesa/GLTestContext_mesa.h"
#include "gl/GrGLDefines.h"

#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"
#include "osmesa_wrapper.h"

namespace {

static GrGLFuncPtr osmesa_get(void* ctx, const char name[]) {
    SkASSERT(nullptr == ctx);
    SkASSERT(OSMesaGetCurrentContext());
    return OSMesaGetProcAddress(name);
}

static const GrGLInterface* create_mesa_interface() {
    if (nullptr == OSMesaGetCurrentContext()) {
        return nullptr;
    }
    return GrGLAssembleInterface(nullptr, osmesa_get);
}

static const GrGLint gBOGUS_SIZE = 16;

class MesaGLContext : public sk_gpu_test::GLTestContext {
private:
    typedef intptr_t Context;

public:
    MesaGLContext(MesaGLContext* shareContext);
    ~MesaGLContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeCurrent() const override;

    void onPlatformSwapBuffers() const override;

    GrGLFuncPtr onPlatformGetProcAddress(const char *) const override;

    Context fContext;
    GrGLubyte *fImage;
};

MesaGLContext::MesaGLContext(MesaGLContext* shareContext)
        : fContext(static_cast<Context>(0))
        , fImage(nullptr) {
    GR_STATIC_ASSERT(sizeof(Context) == sizeof(OSMesaContext));
    OSMesaContext mesaShareContext = shareContext ? (OSMesaContext)(shareContext->fContext)
                                                  : nullptr;

    /* Create an RGBA-mode context */
#if OSMESA_MAJOR_VERSION * 100 + OSMESA_MINOR_VERSION >= 305
    /* specify Z, stencil, accum sizes */
    fContext = (Context)OSMesaCreateContextExt(OSMESA_BGRA, 0, 0, 0, mesaShareContext);
#else
    fContext = (Context) OSMesaCreateContext(OSMESA_BGRA, mesaShareContext);
#endif
    if (!fContext) {
        SkDebugf("OSMesaCreateContext failed!\n");
        this->destroyGLContext();
        return;
    }
    // Allocate the image buffer
    fImage = (GrGLubyte *) sk_malloc_throw(gBOGUS_SIZE * gBOGUS_SIZE *
                                           4 * sizeof(GrGLubyte));
    if (!fImage) {
        SkDebugf("Alloc image buffer failed!\n");
        this->destroyGLContext();
        return;
    }

    // Bind the buffer to the context and make it current
    if (!OSMesaMakeCurrent((OSMesaContext) fContext,
                           fImage,
                           GR_GL_UNSIGNED_BYTE,
                           gBOGUS_SIZE,
                           gBOGUS_SIZE)) {
        SkDebugf("OSMesaMakeCurrent failed!\n");
        this->destroyGLContext();
        return;
    }

    sk_sp<const GrGLInterface> gl(create_mesa_interface());
    if (nullptr == gl.get()) {
        SkDebugf("Could not create GL interface!\n");
        this->destroyGLContext();
        return;
    }

    if (!gl->validate()) {
        SkDebugf("Could not validate GL interface!\n");
        this->destroyGLContext();
        return;
    }

    this->init(gl.release());
}

MesaGLContext::~MesaGLContext() {
    this->teardown();
    this->destroyGLContext();
}

void MesaGLContext::destroyGLContext() {
    if (fImage) {
        sk_free(fImage);
        fImage = nullptr;
    }

    if (fContext) {
        OSMesaDestroyContext((OSMesaContext) fContext);
        fContext = static_cast<Context>(0);
    }
}


void MesaGLContext::onPlatformMakeCurrent() const {
    if (fContext) {
        if (!OSMesaMakeCurrent((OSMesaContext) fContext, fImage,
                               GR_GL_UNSIGNED_BYTE, gBOGUS_SIZE, gBOGUS_SIZE)) {
            SkDebugf("Could not make MESA context current.");
        }
    }
}

void MesaGLContext::onPlatformSwapBuffers() const { }

GrGLFuncPtr MesaGLContext::onPlatformGetProcAddress(const char *procName) const {
    return OSMesaGetProcAddress(procName);
}
}  // anonymous namespace


namespace sk_gpu_test {
GLTestContext *CreateMesaGLTestContext(GLTestContext* shareContext) {
    MesaGLContext* mesaShareContext = reinterpret_cast<MesaGLContext*>(shareContext);
    MesaGLContext *ctx = new MesaGLContext(mesaShareContext);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}
}  // sk_gpu_test
