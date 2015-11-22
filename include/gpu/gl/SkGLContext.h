
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkGLContext_DEFINED
#define SkGLContext_DEFINED

#include "GrGLInterface.h"
#include "../private/SkGpuFenceSync.h"

/**
 * Create an offscreen opengl context with an RGBA8 / 8bit stencil FBO.
 * Provides a GrGLInterface struct of function pointers for the context.
 * This class is intended for Skia's testing needs and not for general
 * use.
 */
class SK_API SkGLContext : public SkRefCnt {
public:
    ~SkGLContext() override;

    bool isValid() const { return NULL != gl(); }

    const GrGLInterface* gl() const { return fGL.get(); }

    bool fenceSyncSupport() const { return SkToBool(fFenceSync); }

    bool getMaxGpuFrameLag(int* maxFrameLag) const {
        if (!fFenceSync) {
            return false;
        }
        *maxFrameLag = kMaxFrameLag;
        return true;
    }

    void makeCurrent() const;

    /** Used for testing EGLImage integration. Take a GL_TEXTURE_2D and wraps it in an EGL Image */
    virtual GrEGLImage texture2DToEGLImage(GrGLuint /*texID*/) const { return 0; }
    virtual void destroyEGLImage(GrEGLImage) const {}

    /**
     * Used for testing EGLImage integration. Takes a EGLImage and wraps it in a
     * GL_TEXTURE_EXTERNAL_OES.
     */
    virtual GrGLuint eglImageToExternalTexture(GrEGLImage) const { return 0; }

    /**
     * The only purpose of this function it to provide a means of scheduling
     * work on the GPU (since all of the subclasses create primary buffers for
     * testing that are small and not meant to be rendered to the screen).
     *
     * If the platform supports fence sync (OpenGL 3.2+ or EGL_KHR_fence_sync),
     * this will not swap any buffers, but rather emulate triple buffer
     * synchronization using fences.
     *
     * Otherwise it will call the platform SwapBuffers method. This may or may
     * not perform some sort of synchronization, depending on whether the
     * drawing surface provided by the platform is double buffered.
     */
    void swapBuffers();

    /**
     * This notifies the context that we are deliberately testing abandoning
     * the context. It is useful for debugging contexts that would otherwise
     * test that GPU resources are properly deleted. It also allows a debugging
     * context to test that further GL calls are not made by Skia GPU code.
     */
    void testAbandon();

    /**
     * Creates a new GL context of the same type and makes the returned context current
     * (if not null).
     */
    virtual SkGLContext* createNew() const { return nullptr; }

    class GLFenceSync;  // SkGpuFenceSync implementation that uses the OpenGL functionality.

protected:
    SkGLContext();

    /*
     * Methods that sublcasses must call from their constructors and destructors.
     */
    void init(const GrGLInterface*, SkGpuFenceSync* = NULL);
    void teardown();

    /*
     * Operations that have a platform-dependent implementation.
     */
    virtual void onPlatformMakeCurrent() const = 0;
    virtual void onPlatformSwapBuffers() const = 0;
    virtual GrGLFuncPtr onPlatformGetProcAddress(const char*) const = 0;

private:
    enum { kMaxFrameLag = 3 };

    SkAutoTDelete<SkGpuFenceSync> fFenceSync;
    SkPlatformGpuFence            fFrameFences[kMaxFrameLag - 1];
    int                           fCurrentFenceIdx;

    /** Subclass provides the gl interface object if construction was
     *  successful. */
    SkAutoTUnref<const GrGLInterface> fGL;

    friend class GLFenceSync;  // For onPlatformGetProcAddress.

    typedef SkRefCnt INHERITED;
};

/** Creates platform-dependent GL context object
 * Returns a valid gl context object or NULL if such can not be created.
 * Note: If Skia embedder needs a custom GL context that sets up the GL
 * interface, this function should be implemented by the embedder.
 * Otherwise, the default implementation for the platform should be compiled in
 * the library.
 */
SK_API SkGLContext* SkCreatePlatformGLContext(GrGLStandard forcedGpuAPI);

/**
 * Helper macros for using the GL context through the GrGLInterface. Example:
 * SK_GL(glCtx, GenTextures(1, &texID));
 */
#define SK_GL(ctx, X) (ctx).gl()->fFunctions.f ## X;    \
                      SkASSERT(0 == (ctx).gl()->fFunctions.fGetError())
#define SK_GL_RET(ctx, RET, X) (RET) = (ctx).gl()->fFunctions.f ## X;    \
                  SkASSERT(0 == (ctx).gl()->fFunctions.fGetError())
#define SK_GL_NOERRCHECK(ctx, X) (ctx).gl()->fFunctions.f ## X
#define SK_GL_RET_NOERRCHECK(ctx, RET, X) (RET) = (ctx).gl()->fFunctions.f ## X

#endif
