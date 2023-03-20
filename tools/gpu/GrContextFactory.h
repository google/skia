/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrContextFactory_DEFINED
#define GrContextFactory_DEFINED

#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"

#include "include/private/base/SkTArray.h"

#ifdef SK_GL
#include "tools/gpu/gl/GLTestContext.h"
#endif
#include "tools/gpu/TestContext.h"

struct GrVkBackendContext;

namespace sk_gpu_test {
class ContextInfo;

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
        kGL_ContextType,                 //! OpenGL context.
        kGLES_ContextType,               //! OpenGL ES context.
        kANGLE_D3D9_ES2_ContextType,     //! ANGLE on Direct3D9 OpenGL ES 2 context.
        kANGLE_D3D11_ES2_ContextType,    //! ANGLE on Direct3D11 OpenGL ES 2 context.
        kANGLE_D3D11_ES3_ContextType,    //! ANGLE on Direct3D11 OpenGL ES 3 context.
        kANGLE_GL_ES2_ContextType,       //! ANGLE on OpenGL OpenGL ES 2 context.
        kANGLE_GL_ES3_ContextType,       //! ANGLE on OpenGL OpenGL ES 3 context.
        kANGLE_Metal_ES2_ContextType,    //! ANGLE on Metal ES 2 context.
        kANGLE_Metal_ES3_ContextType,    //! ANGLE on Metal ES 3 context.
        kVulkan_ContextType,             //! Vulkan
        kMetal_ContextType,              //! Metal
        kDirect3D_ContextType,           //! Direct3D 12
        kDawn_ContextType,               //! Dawn
        kMock_ContextType,               //! Mock context that does not draw.
        kLastContextType = kMock_ContextType
    };

    static const int kContextTypeCnt = kLastContextType + 1;

    /**
     * Overrides for the initial GrContextOptions provided at construction time, and required
     * features that will cause context creation to fail if not present.
     */
    enum class ContextOverrides {
        kNone                          = 0x0,
        kAvoidStencilBuffers           = 0x1,
        kFakeGLESVersionAs2            = 0x2,
        kReducedShaders                = 0x4,
    };

    static bool IsRenderingContext(ContextType type) {
        switch (type) {
            case kMock_ContextType:
                return false;
            default:
                return true;
        }
    }

    static GrBackendApi ContextTypeBackend(ContextType type) {
        switch (type) {
            case kVulkan_ContextType:
                return GrBackendApi::kVulkan;
            case kMetal_ContextType:
                return GrBackendApi::kMetal;
            case kDirect3D_ContextType:
                return GrBackendApi::kDirect3D;
            case kDawn_ContextType:
                return GrBackendApi::kDawn;
            case kMock_ContextType:
                return GrBackendApi::kMock;
            default:
                return GrBackendApi::kOpenGL;
        }
    }

    static const char* ContextTypeName(ContextType contextType) {
        switch (contextType) {
            case kGL_ContextType:
                return "OpenGL";
            case kGLES_ContextType:
                return "OpenGLES";
            case kANGLE_D3D9_ES2_ContextType:
                return "ANGLE D3D9 ES2";
            case kANGLE_D3D11_ES2_ContextType:
                return "ANGLE D3D11 ES2";
            case kANGLE_D3D11_ES3_ContextType:
                return "ANGLE D3D11 ES3";
            case kANGLE_GL_ES2_ContextType:
                return "ANGLE GL ES2";
            case kANGLE_GL_ES3_ContextType:
                return "ANGLE GL ES3";
            case kANGLE_Metal_ES2_ContextType:
                return "ANGLE Metal ES2";
            case kANGLE_Metal_ES3_ContextType:
                return "ANGLE Metal ES3";
            case kVulkan_ContextType:
                return "Vulkan";
            case kMetal_ContextType:
                return "Metal";
            case kDirect3D_ContextType:
                return "Direct3D";
            case kDawn_ContextType:
                return "Dawn";
            case kMock_ContextType:
                return "Mock";
        }
        SK_ABORT("Unreachable");
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
    ContextInfo getContextInfo(ContextType type, ContextOverrides = ContextOverrides::kNone);

    /**
     * Get a context in the same share group as the passed in GrContext, with the same type and
     * overrides. To get multiple contexts in a single share group, pass the same shareContext,
     * with different values for shareIndex.
     */
    ContextInfo getSharedContextInfo(GrDirectContext* shareContext, uint32_t shareIndex = 0);

    /**
     * Get a GrContext initialized with a type of GL context. It also makes the GL context current.
     */
    GrDirectContext* get(ContextType type, ContextOverrides overrides = ContextOverrides::kNone);
    const GrContextOptions& getGlobalOptions() const { return fGlobalOptions; }

private:
    ContextInfo getContextInfoInternal(ContextType type, ContextOverrides overrides,
                                       GrDirectContext* shareContext, uint32_t shareIndex);

    struct Context {
        ContextType       fType;
        ContextOverrides  fOverrides;
        GrContextOptions  fOptions;
        GrBackendApi      fBackend;
        TestContext*      fTestContext;
        GrDirectContext*  fGrContext;
        GrDirectContext*  fShareContext;
        uint32_t          fShareIndex;

        bool              fAbandoned;
    };
    skia_private::TArray<Context, true> fContexts;
#ifdef SK_GL
    std::unique_ptr<GLTestContext>      fSentinelGLContext;
#endif

    const GrContextOptions              fGlobalOptions;
};

class ContextInfo {
public:
    ContextInfo() = default;
    ContextInfo(const ContextInfo&) = default;
    ContextInfo& operator=(const ContextInfo&) = default;

    GrContextFactory::ContextType type() const { return fType; }
    GrBackendApi backend() const { return GrContextFactory::ContextTypeBackend(fType); }

    GrDirectContext* directContext() const { return fContext; }
    TestContext* testContext() const { return fTestContext; }

#ifdef SK_GL
    GLTestContext* glContext() const {
        SkASSERT(GrBackendApi::kOpenGL == this->backend());
        return static_cast<GLTestContext*>(fTestContext);
    }
#endif

    const GrContextOptions& options() const { return fOptions; }

private:
    ContextInfo(GrContextFactory::ContextType type,
                TestContext* testContext,
                GrDirectContext* context,
                const GrContextOptions& options)
            : fType(type), fTestContext(testContext), fContext(context), fOptions(options) {}

    GrContextFactory::ContextType fType = GrContextFactory::kGL_ContextType;
    // Valid until the factory destroys it via abandonContexts() or destroyContexts().
    TestContext* fTestContext = nullptr;
    GrDirectContext* fContext = nullptr;
    GrContextOptions fOptions;

    friend class GrContextFactory;
};

}  // namespace sk_gpu_test

GR_MAKE_BITFIELD_CLASS_OPS(sk_gpu_test::GrContextFactory::ContextOverrides)

#endif
