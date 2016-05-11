
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_DEFINED
#define GLTestContext_DEFINED

#include "TestContext.h"
#include "gl/GrGLInterface.h"

namespace sk_gpu_test {
/**
 * An offscreen OpenGL context. Provides a GrGLInterface struct of function pointers for the context
 * This class is intended for Skia's internal testing needs and not for general use.
 */
class GLTestContext : public TestContext {
public:
    virtual ~GLTestContext();

    virtual GrBackend backend() override { return kOpenGL_GrBackend; }
    virtual GrBackendContext backendContext() override {
        return reinterpret_cast<GrBackendContext>(fGL.get());
    }

    bool isValid() const override { return SkToBool(this->gl()); }

    const GrGLInterface *gl() const { return fGL.get(); }

    /** Used for testing EGLImage integration. Take a GL_TEXTURE_2D and wraps it in an EGL Image */
    virtual GrEGLImage texture2DToEGLImage(GrGLuint /*texID*/) const { return 0; }

    virtual void destroyEGLImage(GrEGLImage) const { }

    /** Used for testing GL_TEXTURE_RECTANGLE integration. */
    GrGLint createTextureRectangle(int width, int height, GrGLenum internalFormat,
                                   GrGLenum externalFormat, GrGLenum externalType,
                                   GrGLvoid *data);

    /**
     * Used for testing EGLImage integration. Takes a EGLImage and wraps it in a
     * GL_TEXTURE_EXTERNAL_OES.
     */
    virtual GrGLuint eglImageToExternalTexture(GrEGLImage) const { return 0; }

    void testAbandon() override;

    /** Ensures all work is submitted to the GPU for execution. */
    void submit() override;

    /** Wait until all GPU work is finished. */
    void finish() override;

    /**
     * Creates a new GL context of the same type and makes the returned context current
     * (if not null).
     */
    virtual GLTestContext *createNew() const { return nullptr; }

protected:
    GLTestContext();

    /*
     * Methods that sublcasses must call from their constructors and destructors.
     */
    void init(const GrGLInterface *, SkGpuFenceSync * = NULL);

    void teardown() override;

    virtual GrGLFuncPtr onPlatformGetProcAddress(const char *) const = 0;

private:
    class GLFenceSync;  // SkGpuFenceSync implementation that uses the OpenGL functionality.

    /** Subclass provides the gl interface object if construction was
     *  successful. */
    SkAutoTUnref<const GrGLInterface> fGL;

    friend class GLFenceSync;  // For onPlatformGetProcAddress.

    typedef TestContext INHERITED;
};

/**
 * Creates platform-dependent GL context object.  The shareContext parameter is in an optional
 * context with which to share display lists. This should be a pointer to an GLTestContext created
 * with SkCreatePlatformGLTestContext.  NULL indicates that no sharing is to take place. Returns a
 * valid gl context object or NULL if such can not be created.
 */
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext *shareContext = nullptr);

}  // namespace sk_gpu_test
#endif
