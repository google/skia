/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file implements many functions defined in Test.h that are required to be implemented by
 * test runners (such as DM) to support GPU backends.
 */

#include "tests/Test.h"

#include "include/gpu/GrDirectContext.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "tools/graphite/ContextFactory.h"
#endif

using sk_gpu_test::GrContextFactory;
using sk_gpu_test::ContextInfo;

#ifdef SK_GL
using sk_gpu_test::GLTestContext;
#endif

namespace skiatest {

bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kOpenGL == GrContextFactory::ContextTypeBackend(type);
}
bool IsVulkanContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kVulkan == GrContextFactory::ContextTypeBackend(type);
}
bool IsMetalContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kMetal == GrContextFactory::ContextTypeBackend(type);
}
bool IsDirect3DContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kDirect3D == GrContextFactory::ContextTypeBackend(type);
}
bool IsDawnContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return GrBackendApi::kDawn == GrContextFactory::ContextTypeBackend(type);
}
bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return IsGLContextType(type) && GrContextFactory::IsRenderingContext(type);
}
bool IsMockContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == GrContextFactory::kMock_ContextType;
}

void RunWithGaneshTestContexts(GrContextTestFn* testFn, GrContextTypeFilterFn* filter,
                               Reporter* reporter, const GrContextOptions& options) {
#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    static constexpr auto kNativeGLType = GrContextFactory::kGL_ContextType;
#else
    static constexpr auto kNativeGLType = GrContextFactory::kGLES_ContextType;
#endif

    for (int typeInt = 0; typeInt < GrContextFactory::kContextTypeCnt; ++typeInt) {
        GrContextFactory::ContextType contextType = (GrContextFactory::ContextType) typeInt;
        // Use "native" instead of explicitly trying OpenGL and OpenGL ES. Do not use GLES on
        // desktop since tests do not account for not fixing http://skbug.com/2809
        if (contextType == GrContextFactory::kGL_ContextType ||
            contextType == GrContextFactory::kGLES_ContextType) {
            if (contextType != kNativeGLType) {
                continue;
            }
        }
        // We destroy the factory and its associated contexts after each test. This is due to the
        // fact that the command buffer sits on top of the native GL windowing (cgl, wgl, ...) but
        // also tracks which of its contexts is current above that API and gets tripped up if the
        // native windowing API is used directly outside of the command buffer code.
        GrContextFactory factory(options);
        ContextInfo ctxInfo = factory.getContextInfo(contextType);
        if (filter && !(*filter)(contextType)) {
            continue;
        }

        ReporterContext ctx(reporter, SkString(GrContextFactory::ContextTypeName(contextType)));
        if (ctxInfo.directContext()) {
            (*testFn)(reporter, ctxInfo);
            // In case the test changed the current context make sure we move it back before
            // calling flush.
            ctxInfo.testContext()->makeCurrent();
            // Sync so any release/finished procs get called.
            ctxInfo.directContext()->flushAndSubmit(/*sync*/true);
        }
    }
}

#if defined(SK_GRAPHITE)

namespace graphite {

void RunWithGraphiteTestContexts(GraphiteTestFn* test, GrContextTypeFilterFn* filter,
                                 Reporter* reporter) {
    ContextFactory factory;
    for (int typeInt = 0; typeInt < GrContextFactory::kContextTypeCnt; ++typeInt) {
        GrContextFactory::ContextType contextType = (GrContextFactory::ContextType) typeInt;
        if (filter && !(*filter)(contextType)) {
            continue;
        }

        auto [_, context] = factory.getContextInfo(contextType);
        if (!context) {
            continue;
        }

        ReporterContext ctx(reporter, SkString(GrContextFactory::ContextTypeName(contextType)));
        (*test)(reporter, context);
    }
}

} // namespace graphite

#endif // SK_GRAPHITE

} // namespace skiatest
