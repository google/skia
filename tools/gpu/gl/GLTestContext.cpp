/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestContext.h"
#include "gl/GrGLUtil.h"

namespace {

class GLFenceSync : public sk_gpu_test::FenceSync {
public:
    static GLFenceSync* CreateIfSupported(const sk_gpu_test::GLTestContext*);

    sk_gpu_test::PlatformFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool waitFence(sk_gpu_test::PlatformFence fence) const override;
    void deleteFence(sk_gpu_test::PlatformFence fence) const override;

private:
    GLFenceSync(const sk_gpu_test::GLTestContext*, const char* ext = "");

    bool validate() { return fGLFenceSync && fGLClientWaitSync && fGLDeleteSync; }

    static constexpr GrGLenum GL_SYNC_GPU_COMMANDS_COMPLETE  = 0x9117;
    static constexpr GrGLenum GL_WAIT_FAILED                 = 0x911d;
    static constexpr GrGLbitfield GL_SYNC_FLUSH_COMMANDS_BIT = 0x00000001;

    typedef struct __GLsync *GLsync;

    typedef GLsync (GR_GL_FUNCTION_TYPE* GLFenceSyncProc) (GrGLenum, GrGLbitfield);
    typedef GrGLenum (GR_GL_FUNCTION_TYPE* GLClientWaitSyncProc) (GLsync, GrGLbitfield, GrGLuint64);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GLDeleteSyncProc) (GLsync);

    GLFenceSyncProc        fGLFenceSync;
    GLClientWaitSyncProc   fGLClientWaitSync;
    GLDeleteSyncProc       fGLDeleteSync;

    typedef FenceSync INHERITED;
};

GLFenceSync* GLFenceSync::CreateIfSupported(const sk_gpu_test::GLTestContext* ctx) {
    SkAutoTDelete<GLFenceSync> ret;
    if (kGL_GrGLStandard == ctx->gl()->fStandard) {
        if (GrGLGetVersion(ctx->gl()) < GR_GL_VER(3,2) && !ctx->gl()->hasExtension("GL_ARB_sync")) {
            return nullptr;
        }
        ret.reset(new GLFenceSync(ctx));
    } else {
        if (!ctx->gl()->hasExtension("GL_APPLE_sync")) {
            return nullptr;
        }
        ret.reset(new GLFenceSync(ctx, "APPLE"));
    }
    return ret->validate() ? ret.release() : nullptr;
}

GLFenceSync::GLFenceSync(const sk_gpu_test::GLTestContext* ctx, const char* ext) {
    ctx->getGLProcAddress(&fGLFenceSync, "glFenceSync");
    ctx->getGLProcAddress(&fGLClientWaitSync, "glClientWaitSync");
    ctx->getGLProcAddress(&fGLDeleteSync, "glDeleteSync");
}

sk_gpu_test::PlatformFence GLFenceSync::insertFence() const {
    __GLsync* glsync = fGLFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    return reinterpret_cast<sk_gpu_test::PlatformFence>(glsync);
}

bool GLFenceSync::waitFence(sk_gpu_test::PlatformFence fence) const {
    GLsync glsync = reinterpret_cast<GLsync>(fence);
    return GL_WAIT_FAILED != fGLClientWaitSync(glsync, GL_SYNC_FLUSH_COMMANDS_BIT, -1);
}

void GLFenceSync::deleteFence(sk_gpu_test::PlatformFence fence) const {
    GLsync glsync = reinterpret_cast<GLsync>(fence);
    fGLDeleteSync(glsync);
}

}  // anonymous namespace

namespace sk_gpu_test {

GLTestContext::GLTestContext() : TestContext() {}

GLTestContext::~GLTestContext() {
    SkASSERT(nullptr == fGL.get());
}

void GLTestContext::init(const GrGLInterface* gl, FenceSync* fenceSync) {
    SkASSERT(!fGL.get());
    fGL.reset(gl);
    fFenceSync = fenceSync ? fenceSync : GLFenceSync::CreateIfSupported(this);
}

void GLTestContext::teardown() {
    fGL.reset(nullptr);
    INHERITED::teardown();
}

void GLTestContext::testAbandon() {
    INHERITED::testAbandon();
    if (fGL) {
        fGL->abandon();
    }
}

void GLTestContext::submit() {
    if (fGL) {
        GR_GL_CALL(fGL.get(), Flush());
    }
}

void GLTestContext::finish() {
    if (fGL) {
        GR_GL_CALL(fGL.get(), Finish());
    }
}

GrGLint GLTestContext::createTextureRectangle(int width, int height, GrGLenum internalFormat,
                                          GrGLenum externalFormat, GrGLenum externalType,
                                          GrGLvoid* data) {
    if (!(kGL_GrGLStandard == fGL->fStandard && GrGLGetVersion(fGL) >= GR_GL_VER(3, 1)) &&
        !fGL->fExtensions.has("GL_ARB_texture_rectangle")) {
        return 0;
    }

    if  (GrGLGetGLSLVersion(fGL) < GR_GLSL_VER(1, 40)) {
        return 0;
    }

    GrGLuint id;
    GR_GL_CALL(fGL, GenTextures(1, &id));
    GR_GL_CALL(fGL, BindTexture(GR_GL_TEXTURE_RECTANGLE, id));
    GR_GL_CALL(fGL, TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_MAG_FILTER,
                                  GR_GL_NEAREST));
    GR_GL_CALL(fGL, TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_MIN_FILTER,
                                  GR_GL_NEAREST));
    GR_GL_CALL(fGL, TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_WRAP_S,
                                  GR_GL_CLAMP_TO_EDGE));    
    GR_GL_CALL(fGL, TexParameteri(GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_WRAP_T,
                                  GR_GL_CLAMP_TO_EDGE));
    GR_GL_CALL(fGL, TexImage2D(GR_GL_TEXTURE_RECTANGLE, 0, internalFormat, width, height, 0,
                               externalFormat, externalType, data));
    return id;
}
}  // namespace sk_gpu_test
