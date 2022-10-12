/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#include "tools/skqp/src/skqp.h"

#include "include/core/SkStream.h"
#include "include/gpu/GrDirectContext.h"

#ifdef SK_VULKAN
#include "tools/gpu/vk/VkTestContext.h"
#include <mutex>
#endif
#ifdef SK_GRAPHITE_ENABLED
#include "include/gpu/graphite/Context.h"
#include "tools/graphite/ContextFactory.h"
#endif

using sk_gpu_test::ContextInfo;
using sk_gpu_test::GrContextFactory;
using sk_gpu_test::TestContext;

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
bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return IsGLContextType(type) && GrContextFactory::IsRenderingContext(type);
}
bool IsMockContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == GrContextFactory::kMock_ContextType;
}

// These are not supported
bool IsMetalContextType(sk_gpu_test::GrContextFactory::ContextType type) { return false; }
bool IsDirect3DContextType(sk_gpu_test::GrContextFactory::ContextType type) { return false; }
bool IsDawnContextType(sk_gpu_test::GrContextFactory::ContextType type) { return false; }

static bool vk_has_physical_devices() {
    static bool supported = false;
#ifdef SK_VULKAN
    static std::once_flag flag;
    std::call_once(flag, []() {
        // We could create a VkInstance and call vkEnumeratePhysicalDevices devices directly, but
        // CreatePlatformVkTestContext is already configured to do that and will return nullptr if
        // there are no available devices.
        std::unique_ptr<TestContext> testCtx(sk_gpu_test::CreatePlatformVkTestContext(nullptr));
        if (testCtx) {
            supported = true;
        }
    });
#endif
    return supported;
}

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
// Used for testing on desktop machines.
static constexpr auto kNativeGLType = GrContextFactory::kGL_ContextType;
#else
static constexpr auto kNativeGLType = GrContextFactory::kGLES_ContextType;
#endif

#ifdef SK_BUILD_FOR_ANDROID
static_assert(kNativeGLType == GrContextFactory::kGLES_ContextType, "CTS requires GLES");
#endif

static bool skip_context(GrContextFactory::ContextType contextType) {
    // Use "native" instead of explicitly trying both OpenGL and OpenGL ES.
    if (contextType == GrContextFactory::kGL_ContextType ||
        contextType == GrContextFactory::kGLES_ContextType) {
        if (contextType != kNativeGLType) {
            return true;
        }
    }

    // The Android CDD (https://source.android.com/compatibility/12/android-12-cdd.pdf) does not
    // require Vulkan, but if it enumerates at least one VkPhysicalDevice then it is expected that
    // Vulkan is supported
    if (contextType == GrContextFactory::kVulkan_ContextType && !vk_has_physical_devices()) {
        return true;
    }
    return false;
}

void RunWithGaneshTestContexts(GrContextTestFn* testFn,
                               GrContextTypeFilterFn* filter,
                               Reporter* reporter,
                               const GrContextOptions& options) {
    for (int typeInt = 0; typeInt < GrContextFactory::kContextTypeCnt; ++typeInt) {
        GrContextFactory::ContextType contextType = (GrContextFactory::ContextType)typeInt;
        if (skip_context(contextType)) {
            continue;
        }

        // The logic below is intended to mirror the behavior in DMGpuTestProcs.cpp
        if (filter && !(*filter)(contextType)) {
            continue;
        }

        sk_gpu_test::GrContextFactory factory(options);
        sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(contextType);

        ReporterContext ctx(reporter, SkString(GrContextFactory::ContextTypeName(contextType)));
        if (ctxInfo.directContext()) {
            ctxInfo.testContext()->makeCurrent();
            (*testFn)(reporter, ctxInfo);
            // Sync so any release/finished procs get called.
            ctxInfo.directContext()->flushAndSubmit(/*sync*/ true);
        }
    }
}

#ifdef SK_GRAPHITE_ENABLED

namespace graphite {

void RunWithGraphiteTestContexts(GraphiteTestFn* test,
                                 GrContextTypeFilterFn* filter,
                                 Reporter* reporter) { SK_ABORT(); }

}  // namespace graphite

#endif  // SK_GRAPHITE_ENABLED

}  // namespace skiatest

void SkQP::printBackendInfo(const char* dstPath) {
#ifdef SK_ENABLE_DUMP_GPU
    SkFILEWStream out(dstPath);
    out.writeText("[\n");

    GrContextFactory::ContextType contextsToDump[] = {skiatest::kNativeGLType,
                                                      GrContextFactory::kVulkan_ContextType};

    for (auto contextType : contextsToDump) {
        std::unique_ptr<TestContext> testCtx;
        switch (contextType) {
#ifdef SK_GL
            case GrContextFactory::kGL_ContextType:
                testCtx.reset(sk_gpu_test::CreatePlatformGLTestContext(kGL_GrGLStandard, nullptr));
                break;
            case GrContextFactory::kGLES_ContextType:
                testCtx.reset(
                        sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard, nullptr));
                break;
#endif
#ifdef SK_VULKAN
            case GrContextFactory::kVulkan_ContextType:
                testCtx.reset(sk_gpu_test::CreatePlatformVkTestContext(nullptr));
                break;
#endif
            default: {
            }
        }

        if (testCtx) {
            GrContextOptions options;
            testCtx->makeCurrent();
            if (sk_sp<GrDirectContext> ctx = testCtx->makeContext(options)) {
                SkString info = ctx->dump();
                // remove null
                out.write(info.c_str(), info.size());
                out.writeText(",\n");
            }
        }
    }
    out.writeText("]\n");
#endif
}
