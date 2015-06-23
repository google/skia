
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gl/SkGLContext.h"
#include "GrGLUtil.h"
#include "SkGpuFenceSync.h"

class SkGLContext::GLFenceSync : public SkGpuFenceSync {
public:
    static GLFenceSync* CreateIfSupported(const SkGLContext*);

    SkPlatformGpuFence SK_WARN_UNUSED_RESULT insertFence() const override;
    bool flushAndWaitFence(SkPlatformGpuFence fence) const override;
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

SkGLContext::SkGLContext()
    : fCurrentFenceIdx(0) {
    memset(fFrameFences, 0, sizeof(fFrameFences));
}

SkGLContext::~SkGLContext() {
    // Subclass should call teardown.
#ifdef SK_DEBUG
    for (size_t i = 0; i < SK_ARRAY_COUNT(fFrameFences); i++) {
        SkASSERT(0 == fFrameFences[i]);
    }
#endif
    SkASSERT(NULL == fGL.get());
    SkASSERT(NULL == fFenceSync.get());
}

void SkGLContext::init(const GrGLInterface* gl, SkGpuFenceSync* fenceSync) {
    SkASSERT(!fGL.get());
    fGL.reset(gl);
    fFenceSync.reset(fenceSync ? fenceSync : GLFenceSync::CreateIfSupported(this));
}

void SkGLContext::teardown() {
    if (fFenceSync) {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fFrameFences); i++) {
            if (fFrameFences[i]) {
                fFenceSync->deleteFence(fFrameFences[i]);
                fFrameFences[i] = 0;
            }
        }
        fFenceSync.reset(NULL);
    }

    fGL.reset(NULL);
}

void SkGLContext::makeCurrent() const {
    this->onPlatformMakeCurrent();
}

void SkGLContext::swapBuffers() {
    if (!fFenceSync) {
        // Fallback on the platform SwapBuffers method for synchronization. This may have no effect.
        this->onPlatformSwapBuffers();
        return;
    }

    if (fFrameFences[fCurrentFenceIdx]) {
        if (!fFenceSync->flushAndWaitFence(fFrameFences[fCurrentFenceIdx])) {
            SkDebugf("WARNING: Wait failed for fence sync. Timings might not be accurate.\n");
        }
        fFenceSync->deleteFence(fFrameFences[fCurrentFenceIdx]);
    }

    fFrameFences[fCurrentFenceIdx] = fFenceSync->insertFence();
    fCurrentFenceIdx = (fCurrentFenceIdx + 1) % SK_ARRAY_COUNT(fFrameFences);
}

void SkGLContext::testAbandon() {
    if (fGL) {
        fGL->abandon();
    }
    if (fFenceSync) {
        memset(fFrameFences, 0, sizeof(fFrameFences));
    }
}

SkGLContext::GLFenceSync* SkGLContext::GLFenceSync::CreateIfSupported(const SkGLContext* ctx) {
    SkAutoTDelete<GLFenceSync> ret(SkNEW(GLFenceSync));

    if (kGL_GrGLStandard == ctx->gl()->fStandard) {
        const GrGLubyte* versionStr;
        SK_GL_RET(*ctx, versionStr, GetString(GR_GL_VERSION));
        GrGLVersion version = GrGLGetVersionFromString(reinterpret_cast<const char*>(versionStr));
        if (version < GR_GL_VER(3,2) && !ctx->gl()->hasExtension("GL_ARB_sync")) {
            return NULL;
        }
        ret->fGLFenceSync = reinterpret_cast<GLFenceSyncProc>(
            ctx->onPlatformGetProcAddress("glFenceSync"));
        ret->fGLClientWaitSync = reinterpret_cast<GLClientWaitSyncProc>(
            ctx->onPlatformGetProcAddress("glClientWaitSync"));
        ret->fGLDeleteSync = reinterpret_cast<GLDeleteSyncProc>(
            ctx->onPlatformGetProcAddress("glDeleteSync"));
    } else {
        if (!ctx->gl()->hasExtension("GL_APPLE_sync")) {
            return NULL;
        }
        ret->fGLFenceSync = reinterpret_cast<GLFenceSyncProc>(
            ctx->onPlatformGetProcAddress("glFenceSyncAPPLE"));
        ret->fGLClientWaitSync = reinterpret_cast<GLClientWaitSyncProc>(
            ctx->onPlatformGetProcAddress("glClientWaitSyncAPPLE"));
        ret->fGLDeleteSync = reinterpret_cast<GLDeleteSyncProc>(
            ctx->onPlatformGetProcAddress("glDeleteSyncAPPLE"));
    }

    if (!ret->fGLFenceSync || !ret->fGLClientWaitSync || !ret->fGLDeleteSync) {
        return NULL;
    }

    return ret.detach();
}

SkPlatformGpuFence SkGLContext::GLFenceSync::insertFence() const {
    return fGLFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

bool SkGLContext::GLFenceSync::flushAndWaitFence(SkPlatformGpuFence fence) const {
    GLsync glsync = static_cast<GLsync>(fence);
    return GL_WAIT_FAILED != fGLClientWaitSync(glsync, GL_SYNC_FLUSH_COMMANDS_BIT, -1);
}

void SkGLContext::GLFenceSync::deleteFence(SkPlatformGpuFence fence) const {
    GLsync glsync = static_cast<GLsync>(fence);
    fGLDeleteSync(glsync);
}
