/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GLTestContext.h"
#include "gl/GrGLUtil.h"

namespace sk_gpu_test {
class GLTestContext::GLFenceSync : public SkGpuFenceSync {
public:
    static GLFenceSync* CreateIfSupported(const GLTestContext*);

    SkPlatformGpuFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool waitFence(SkPlatformGpuFence fence) const override;
    void deleteFence(SkPlatformGpuFence fence) const override;

private:
    GLFenceSync() {}

    static const GrGLenum GL_SYNC_GPU_COMMANDS_COMPLETE  = 0x9117;
    static const GrGLenum GL_WAIT_FAILED                 = 0x911d;
    static const GrGLbitfield GL_SYNC_FLUSH_COMMANDS_BIT = 0x00000001;

    typedef struct __GLsync *GLsync;

    typedef GLsync (GR_GL_FUNCTION_TYPE* GLFenceSyncProc) (GrGLenum, GrGLbitfield);
    typedef GrGLenum (GR_GL_FUNCTION_TYPE* GLClientWaitSyncProc) (GLsync, GrGLbitfield, GrGLuint64);
    typedef GrGLvoid (GR_GL_FUNCTION_TYPE* GLDeleteSyncProc) (GLsync);

    GLFenceSyncProc        fGLFenceSync;
    GLClientWaitSyncProc   fGLClientWaitSync;
    GLDeleteSyncProc       fGLDeleteSync;

    typedef SkGpuFenceSync INHERITED;
};

GLTestContext::GLTestContext() : TestContext() {}

GLTestContext::~GLTestContext() {
    SkASSERT(nullptr == fGL.get());
}

void GLTestContext::init(const GrGLInterface* gl, SkGpuFenceSync* fenceSync) {
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

GLTestContext::GLFenceSync* GLTestContext::GLFenceSync::CreateIfSupported(const GLTestContext* ctx) {
    SkAutoTDelete<GLFenceSync> ret(new GLFenceSync);

    if (kGL_GrGLStandard == ctx->gl()->fStandard) {
        const GrGLubyte* versionStr;
        GR_GL_CALL_RET(ctx->gl(), versionStr, GetString(GR_GL_VERSION));
        GrGLVersion version = GrGLGetVersionFromString(reinterpret_cast<const char*>(versionStr));
        if (version < GR_GL_VER(3,2) && !ctx->gl()->hasExtension("GL_ARB_sync")) {
            return nullptr;
        }
        ret->fGLFenceSync = reinterpret_cast<GLFenceSyncProc>(
            ctx->onPlatformGetProcAddress("glFenceSync"));
        ret->fGLClientWaitSync = reinterpret_cast<GLClientWaitSyncProc>(
            ctx->onPlatformGetProcAddress("glClientWaitSync"));
        ret->fGLDeleteSync = reinterpret_cast<GLDeleteSyncProc>(
            ctx->onPlatformGetProcAddress("glDeleteSync"));
    } else {
        if (!ctx->gl()->hasExtension("GL_APPLE_sync")) {
            return nullptr;
        }
        ret->fGLFenceSync = reinterpret_cast<GLFenceSyncProc>(
            ctx->onPlatformGetProcAddress("glFenceSyncAPPLE"));
        ret->fGLClientWaitSync = reinterpret_cast<GLClientWaitSyncProc>(
            ctx->onPlatformGetProcAddress("glClientWaitSyncAPPLE"));
        ret->fGLDeleteSync = reinterpret_cast<GLDeleteSyncProc>(
            ctx->onPlatformGetProcAddress("glDeleteSyncAPPLE"));
    }

    if (!ret->fGLFenceSync || !ret->fGLClientWaitSync || !ret->fGLDeleteSync) {
        return nullptr;
    }

    return ret.release();
}

SkPlatformGpuFence GLTestContext::GLFenceSync::insertFence() const {
    return fGLFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

bool GLTestContext::GLFenceSync::waitFence(SkPlatformGpuFence fence) const {
    GLsync glsync = static_cast<GLsync>(fence);
    return GL_WAIT_FAILED != fGLClientWaitSync(glsync, GL_SYNC_FLUSH_COMMANDS_BIT, -1);
}

void GLTestContext::GLFenceSync::deleteFence(SkPlatformGpuFence fence) const {
    GLsync glsync = static_cast<GLsync>(fence);
    fGLDeleteSync(glsync);
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
