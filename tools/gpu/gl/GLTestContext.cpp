
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "GLTestContext.h"
#include "gl/GrGLUtil.h"
#include "SkGpuFenceSync.h"

namespace sk_gpu_test {
class GLTestContext::GLFenceSync : public SkGpuFenceSync {
public:
    static GLFenceSync* CreateIfSupported(const GLTestContext*);

    SkPlatformGpuFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool waitFence(SkPlatformGpuFence fence, bool flush) const override;
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

GLTestContext::GLTestContext()
    : fCurrentFenceIdx(0) {
    memset(fFrameFences, 0, sizeof(fFrameFences));
}

GLTestContext::~GLTestContext() {
    // Subclass should call teardown.
#ifdef SK_DEBUG
    for (size_t i = 0; i < SK_ARRAY_COUNT(fFrameFences); i++) {
        SkASSERT(0 == fFrameFences[i]);
    }
#endif
    SkASSERT(nullptr == fGL.get());
    SkASSERT(nullptr == fFenceSync.get());
}

void GLTestContext::init(const GrGLInterface* gl, SkGpuFenceSync* fenceSync) {
    SkASSERT(!fGL.get());
    fGL.reset(gl);
    fFenceSync.reset(fenceSync ? fenceSync : GLFenceSync::CreateIfSupported(this));
}

void GLTestContext::teardown() {
    if (fFenceSync) {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fFrameFences); i++) {
            if (fFrameFences[i]) {
                fFenceSync->deleteFence(fFrameFences[i]);
                fFrameFences[i] = 0;
            }
        }
        fFenceSync.reset(nullptr);
    }

    fGL.reset(nullptr);
}

void GLTestContext::makeCurrent() const {
    this->onPlatformMakeCurrent();
}

void GLTestContext::swapBuffers() {
    this->onPlatformSwapBuffers();
}

void GLTestContext::waitOnSyncOrSwap() {
    if (!fFenceSync) {
        // Fallback on the platform SwapBuffers method for synchronization. This may have no effect.
        this->swapBuffers();
        return;
    }

    if (fFrameFences[fCurrentFenceIdx]) {
        if (!fFenceSync->waitFence(fFrameFences[fCurrentFenceIdx], true)) {
            SkDebugf("WARNING: Wait failed for fence sync. Timings might not be accurate.\n");
        }
        fFenceSync->deleteFence(fFrameFences[fCurrentFenceIdx]);
    }

    fFrameFences[fCurrentFenceIdx] = fFenceSync->insertFence();
    fCurrentFenceIdx = (fCurrentFenceIdx + 1) % SK_ARRAY_COUNT(fFrameFences);
}

void GLTestContext::testAbandon() {
    if (fGL) {
        fGL->abandon();
    }
    if (fFenceSync) {
        memset(fFrameFences, 0, sizeof(fFrameFences));
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

bool GLTestContext::GLFenceSync::waitFence(SkPlatformGpuFence fence, bool flush) const {
    GLsync glsync = static_cast<GLsync>(fence);
    return GL_WAIT_FAILED != fGLClientWaitSync(glsync, flush ? GL_SYNC_FLUSH_COMMANDS_BIT : 0, -1);
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
