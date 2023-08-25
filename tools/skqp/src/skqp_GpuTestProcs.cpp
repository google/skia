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
#if defined(SK_GRAPHITE)
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

bool IsGLContextType(skgpu::ContextType type) {
    return skgpu::ganesh::ContextTypeBackend(type) == GrBackendApi::kOpenGL;
}
bool IsVulkanContextType(skgpu::ContextType type) {
    return skgpu::ganesh::ContextTypeBackend(type) == GrBackendApi::kVulkan;
}
bool IsMockContextType(skgpu::ContextType type) {
    return type == skgpu::ContextType::kMock;
}

// These are not supported
bool IsMetalContextType(skgpu::ContextType type) { return false; }
bool IsDirect3DContextType(skgpu::ContextType type) { return false; }
bool IsDawnContextType(skgpu::ContextType type) { return false; }

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
static constexpr auto kNativeGLType = skgpu::ContextType::kGL;
#else
static constexpr auto kNativeGLType = skgpu::ContextType::kGLES;
#endif

#ifdef SK_BUILD_FOR_ANDROID
static_assert(kNativeGLType == skgpu::ContextType::kGLES, "CTS requires GLES");
#endif

static bool skip_context(skgpu::ContextType contextType) {
    // Use "native" instead of explicitly trying both OpenGL and OpenGL ES.
    if (contextType == skgpu::ContextType::kGL || contextType == skgpu::ContextType::kGLES) {
        if (contextType != kNativeGLType) {
            return true;
        }
    }

    // The Android CDD (https://source.android.com/compatibility/12/android-12-cdd.pdf) does not
    // require Vulkan, but if it enumerates at least one VkPhysicalDevice then it is expected that
    // Vulkan is supported
    if (contextType == skgpu::ContextType::kVulkan && !vk_has_physical_devices()) {
        return true;
    }
    return false;
}

void RunWithGaneshTestContexts(GrContextTestFn* testFn,
                               ContextTypeFilterFn* filter,
                               Reporter* reporter,
                               const GrContextOptions& options) {
    for (int typeInt = 0; typeInt < skgpu::kContextTypeCount; ++typeInt) {
        skgpu::ContextType contextType = static_cast<skgpu::ContextType>(typeInt);
        if (skip_context(contextType)) {
            continue;
        }

        // The logic below is intended to mirror the behavior in DMGpuTestProcs.cpp
        if (filter && !(*filter)(contextType)) {
            continue;
        }

        sk_gpu_test::GrContextFactory factory(options);
        sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(contextType);

        ReporterContext ctx(reporter, SkString(skgpu::ContextTypeName(contextType)));
        if (ctxInfo.directContext()) {
            ctxInfo.testContext()->makeCurrent();
            (*testFn)(reporter, ctxInfo);
            // Sync so any release/finished procs get called.
            ctxInfo.directContext()->flushAndSubmit(/*sync*/ true);
        }
    }
}

#if defined(SK_GRAPHITE)

namespace graphite {

void RunWithGraphiteTestContexts(GraphiteTestFn* test,
                                 ContextTypeFilterFn* filter,
                                 Reporter* reporter,
                                 const skgpu::graphite::ContextOptions&) {
    SK_ABORT();
}

}  // namespace graphite

#endif  // SK_GRAPHITE

}  // namespace skiatest

void SkQP::printBackendInfo(const char* dstPath) {
#ifdef SK_ENABLE_DUMP_GPU
    SkFILEWStream out(dstPath);
    out.writeText("[\n");

    skgpu::ContextType contextsToDump[] = {skiatest::kNativeGLType, skgpu::ContextType::kVulkan};

    for (auto contextType : contextsToDump) {
        std::unique_ptr<TestContext> testCtx;
        switch (contextType) {
#ifdef SK_GL
            case skgpu::ContextType::kGL:
                testCtx.reset(sk_gpu_test::CreatePlatformGLTestContext(kGL_GrGLStandard, nullptr));
                break;
            case skgpu::ContextType::kGLES:
                testCtx.reset(
                        sk_gpu_test::CreatePlatformGLTestContext(kGLES_GrGLStandard, nullptr));
                break;
#endif
#ifdef SK_VULKAN
            case skgpu::ContextType::kVulkan:
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
