
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GLTestContext_DEFINED
#define GLTestContext_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"
#include "tools/gpu/TestContext.h"

namespace sk_gpu_test {
/**
 * An offscreen OpenGL context. Provides a GrGLInterface struct of function pointers for the context
 * This class is intended for Skia's internal testing needs and not for general use.
 * When SK_GL is not defined the GrGLInterface will always be nullptr.
 */
class GLTestContext : public TestContext {
public:
    ~GLTestContext() override;

    GrBackendApi backend() override { return GrBackendApi::kOpenGL; }

    /** Does this represent a successfully created GL context? */
    bool isValid() const;

    const GrGLInterface* gl() const { return fGLInterface.get(); }

    /** Used for testing EGLImage integration. Take a GL_TEXTURE_2D and wraps it in an EGL Image */
    virtual GrEGLImage texture2DToEGLImage(GrGLuint /*texID*/) const { return nullptr; }

    virtual void destroyEGLImage(GrEGLImage) const { }

    /**
     * Used for testing EGLImage integration. Takes a EGLImage and wraps it in a
     * GL_TEXTURE_EXTERNAL_OES.
     */
    virtual GrGLuint eglImageToExternalTexture(GrEGLImage) const { return 0; }

    void testAbandon() override;

    void overrideVersion(const char* version, const char* shadingLanguageVersion);

    /**
     * Creates a new GL context of the same type and makes the returned context current
     * (if not null).
     */
    virtual std::unique_ptr<GLTestContext> makeNew() const { return nullptr; }

    template<typename Ret, typename... Args>
    void getGLProcAddress(Ret(GR_GL_FUNCTION_TYPE** out)(Args...),
                          const char* name, const char* ext = nullptr) const {
        using Proc = Ret(GR_GL_FUNCTION_TYPE*)(Args...);
        if (!SkStrStartsWith(name, "gl")) {
            SK_ABORT("getGLProcAddress: proc name must have 'gl' prefix");
            *out = nullptr;
        } else if (ext) {
            SkString fullname(name);
            fullname.append(ext);
            *out = reinterpret_cast<Proc>(this->onPlatformGetProcAddress(fullname.c_str()));
        } else {
            *out = reinterpret_cast<Proc>(this->onPlatformGetProcAddress(name));
        }
    }

    sk_sp<GrDirectContext> makeContext(const GrContextOptions& options) override;

protected:
    GLTestContext();

    /*
     * Methods that subclasses must call from their constructors and destructors.
     */
    void init(sk_sp<const GrGLInterface>);

    void teardown() override;

    virtual GrGLFuncPtr onPlatformGetProcAddress(const char *) const = 0;

private:
    /** Subclass provides the gl interface object if construction was successful. */
    sk_sp<const GrGLInterface> fOriginalGLInterface;

    /** The same as fOriginalGLInterface unless the version has been overridden. */
    sk_sp<const GrGLInterface> fGLInterface;

    using INHERITED = TestContext;
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
