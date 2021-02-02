/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/gpu/GrDirectContext.h"

using sk_gpu_test::GrContextFactory;
using sk_gpu_test::GLTestContext;
using sk_gpu_test::ContextInfo;

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
bool IsRenderingGLOrMetalContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return (IsGLContextType(type) || IsMetalContextType(type)) &&
           GrContextFactory::IsRenderingContext(type);
}
bool IsMockContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == GrContextFactory::kMock_ContextType;
}

void RunWithGPUTestContexts(GrContextTestFn* test, GrContextTypeFilterFn* contextTypeFilter,
                            Reporter* reporter, const GrContextOptions& options) {
    for (int typeInt = 0; typeInt < GrContextFactory::kContextTypeCnt; ++typeInt) {
        auto contextType = static_cast<GrContextFactory::ContextType>(typeInt);
        // We destroy the factory and its associated contexts after each test. This is due to the
        // fact that the command buffer sits on top of the native GL windowing (cgl, wgl, ...) but
        // also tracks which of its contexts is current above that API and gets tripped up if the
        // native windowing API is used directly outside of the command buffer code.
        GrContextFactory factory(options);
        ContextInfo ctxInfo = factory.getContextInfo(contextType);
        if (contextTypeFilter && !(*contextTypeFilter)(contextType)) {
            continue;
        }

        ReporterContext ctx(reporter, SkString(GrContextFactory::ContextTypeName(contextType)));
        if (ctxInfo.directContext()) {
            (*test)(reporter, ctxInfo);
            // In case the test changed the current context make sure we move it back before
            // calling flush.
            ctxInfo.testContext()->makeCurrent();
            // Sync so any release/finished procs get called.
            ctxInfo.directContext()->flushAndSubmit(/*sync*/true);
        }
    }
}
} // namespace skiatest
