/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextFactory_DEFINED
#define GrContextFactory_DEFINED

#include "GrContext.h"
#include "GrContextOptions.h"

#include "gl/GLTestContext.h"
#include "vk/VkTestContext.h"
#include "SkTArray.h"

struct GrVkBackendContext;

namespace sk_gpu_test {

class ContextInfo {
public:
    ContextInfo() = default;
    ContextInfo& operator=(const ContextInfo&) = default;

    GrBackend backend() const { return fBackend; }

    GrContext* grContext() const { return fGrContext; }

    TestContext* testContext() const { return fTestContext; }

    GLTestContext* glContext() const {
        SkASSERT(kOpenGL_GrBackend == fBackend);
        return static_cast<GLTestContext*>(fTestContext);
    }

#ifdef SK_VULKAN
    VkTestContext* vkContext() const {
        SkASSERT(kVulkan_GrBackend == fBackend);
        return static_cast<VkTestContext*>(fTestContext);
    }
#endif

private:
    ContextInfo(GrBackend backend, TestContext* testContext, GrContext* grContext)
            : fBackend(backend)
            , fTestContext(testContext)
            , fGrContext(grContext) {}

    GrBackend       fBackend = kOpenGL_GrBackend;
    // Valid until the factory destroys it via abandonContexts() or destroyContexts().
    TestContext*    fTestContext = nullptr;
    GrContext*      fGrContext = nullptr;

    friend class GrContextFactory;
};

/**
 * This is a simple class that is useful in test apps that use different
 * GrContexts backed by different types of GL contexts. It manages creating the
 * GL context and a GrContext that uses it. The GL/Gr contexts persist until the
 * factory is destroyed (though the caller can always grab a ref on the returned
 * Gr and GL contexts to make them outlive the factory).
 */
class GrContextFactory : SkNoncopyable {
public:
    // The availability of context types is subject to platform and build configuration
    // restrictions.
    enum ContextType {
        kGL_ContextType,             //! OpenGL context.
        kGLES_ContextType,           //! OpenGL ES context.
        kANGLE_D3D9_ES2_ContextType, //! ANGLE on Direct3D9 OpenGL ES 2 context.
        kANGLE_D3D11_ES2_ContextType,//! ANGLE on Direct3D11 OpenGL ES 2 context.
        kANGLE_D3D11_ES3_ContextType,//! ANGLE on Direct3D11 OpenGL ES 3 context.
        kANGLE_GL_ES2_ContextType,   //! ANGLE on OpenGL OpenGL ES 2 context.
        kANGLE_GL_ES3_ContextType,   //! ANGLE on OpenGL OpenGL ES 3 context.
        kCommandBuffer_ContextType,  //! Chromium command buffer OpenGL ES context.
        kMESA_ContextType,           //! MESA OpenGL context
        kNullGL_ContextType,         //! Non-rendering OpenGL mock context.
        kDebugGL_ContextType,        //! Non-rendering, state verifying OpenGL context.
        kVulkan_ContextType,         //! Vulkan
        kLastContextType = kVulkan_ContextType
    };

    //! OpenGL or OpenGL ES context depending on the platform. To be removed.
    static const ContextType kNativeGL_ContextType;

    static const int kContextTypeCnt = kLastContextType + 1;

    /**
     * Overrides for the initial GrContextOptions provided at construction time, and required
     * features that will cause context creation to fail if not present.
     */
    enum class ContextOverrides {
        kNone                          = 0x0,
        kDisableNVPR                   = 0x1,
        kUseInstanced                  = 0x2,
        kAllowSRGBWithoutDecodeControl = 0x4,

        kRequireNVPRSupport            = 0x8,
        kRequireSRGBSupport            = 0x10
    };

    static ContextType NativeContextTypeForBackend(GrBackend backend) {
        switch (backend) {
            case kOpenGL_GrBackend:
                return kNativeGL_ContextType;
            case kVulkan_GrBackend:
                return kVulkan_ContextType;
            default:
                SkFAIL("Unknown backend");
                return kNullGL_ContextType;
        }
    }

    static bool IsRenderingContext(ContextType type) {
        switch (type) {
            case kNullGL_ContextType:
            case kDebugGL_ContextType:
                return false;
            default:
                return true;
        }
    }

    static GrBackend ContextTypeBackend(ContextType type) {
        switch (type) {
            case kVulkan_ContextType:
                return kVulkan_GrBackend;
            default:
                return kOpenGL_GrBackend;
        }
    }

    explicit GrContextFactory(const GrContextOptions& opts);
    GrContextFactory();

    ~GrContextFactory();

    void destroyContexts();
    void abandonContexts();
    void releaseResourcesAndAbandonContexts();

    /**
     * Get a context initialized with a type of GL context. It also makes the GL context current.
     */
    ContextInfo getContextInfo(ContextType type,
                               ContextOverrides overrides = ContextOverrides::kNone);
    /**
     * Get a GrContext initialized with a type of GL context. It also makes the GL context current.
     */
    GrContext* get(ContextType type, ContextOverrides overrides = ContextOverrides::kNone) {
        return this->getContextInfo(type, overrides).grContext();
    }
    const GrContextOptions& getGlobalOptions() const { return fGlobalOptions; }

private:
    struct Context {
        ContextType       fType;
        ContextOverrides  fOverrides;
        GrBackend         fBackend;
        TestContext*      fTestContext;
        GrContext*        fGrContext;
        bool              fAbandoned;
    };
    SkTArray<Context, true>         fContexts;
    std::unique_ptr<GLTestContext>  fSentinelGLContext;
    const GrContextOptions          fGlobalOptions;
};
}  // namespace sk_gpu_test

GR_MAKE_BITFIELD_CLASS_OPS(sk_gpu_test::GrContextFactory::ContextOverrides);

#endif
