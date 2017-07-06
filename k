[1mdiff --git a/BUILD.gn b/BUILD.gn[m
[1mindex 463e0b5246..b47d5dba50 100644[m
[1m--- a/BUILD.gn[m
[1m+++ b/BUILD.gn[m
[36m@@ -943,6 +943,7 @@[m [mif (skia_enable_tools) {[m
         "tools/gpu/gl/debug/GrTextureObj.cpp",[m
         "tools/gpu/gl/debug/GrTextureUnitObj.cpp",[m
         "tools/gpu/gl/null/NullGLTestContext.cpp",[m
[32m+[m[32m        "tools/gpu/mock/MockTestContext.cpp",[m
       ][m
       libs = [][m
 [m
[1mdiff --git a/src/gpu/mock/GrMockGpu.h b/src/gpu/mock/GrMockGpu.h[m
[1mindex eb0a74a7f3..ec6ceb3ab7 100644[m
[1m--- a/src/gpu/mock/GrMockGpu.h[m
[1m+++ b/src/gpu/mock/GrMockGpu.h[m
[36m@@ -10,13 +10,32 @@[m
 [m
 class GrPipeline;[m
 [m
[32m+[m[32mstruct GrMockOptions {[m
[32m+[m[32m    GrMockOptions() {[m
[32m+[m[32m        // By default the only valid config is RGBA_8888 with no MSAA support.[m
[32m+[m[32m        fConfigOptions[kRGBA_8888_GrPixelConfig].fRenderable[0] = true;[m
[32m+[m[32m        fConfigOptions[kRGBA_8888_GrPixelConfig].fTexturable = true;[m
[32m+[m[32m    }[m
[32m+[m
[32m+[m
[32m+[m[32m    struct ConfigOptions {[m
[32m+[m[32m        bool fRenderable[2] = {false, false};[m
[32m+[m[32m        bool fTexturable = false;[m
[32m+[m[32m    };[m
[32m+[m[32m    ConfigOptions fConfigOptions[kGrPixelConfigCnt];[m
[32m+[m[32m};[m
[32m+[m
 class GrMockCaps : public GrCaps {[m
 public:[m
[31m-    explicit GrMockCaps(const GrContextOptions& options) : INHERITED(options) {[m
[32m+[m[32m    GrMockCaps(const GrContextOptions& contextOptions, const GrMockOptions& options) : INHERITED(contextOptions), fOptions(options) {[m
         fBufferMapThreshold = SK_MaxS32;[m
     }[m
[31m-    bool isConfigTexturable(GrPixelConfig) const override { return false; }[m
[31m-    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override { return false; }[m
[32m+[m[32m    bool isConfigTexturable(GrPixelConfig config) const override {[m
[32m+[m[32m        return fOptions.fConfigOptions[config].fTexturable;[m
[32m+[m[32m    }[m
[32m+[m[32m    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {[m
[32m+[m[32m        return fOptions.fConfigOptions[config].fTexturable;[m
[32m+[m[32m    }[m
     bool canConfigBeImageStorage(GrPixelConfig) const override { return false; }[m
     bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,[m
                             bool* rectsMustMatch, bool* disallowSubrect) const override {[m
[36m@@ -24,15 +43,17 @@[m [mpublic:[m
     }[m
 [m
 private:[m
[32m+[m[32m    GrMockOptions fOptions;[m
     typedef GrCaps INHERITED;[m
 };[m
 [m
 class GrMockGpu : public GrGpu {[m
 public:[m
[31m-    static GrGpu* Create(GrBackendContext backendContext, const GrContextOptions& options,[m
[32m+[m[32m    static GrGpu* Create(GrBackendContext backendContext, const GrContextOptions& contextOptions,[m
                          GrContext* context) {[m
         SkASSERT((void*)backendContext == nullptr);[m
[31m-        return new GrMockGpu(context, options);[m
[32m+[m[32m        static const GrMockOptions kOptions = GrMockOptions();[m
[32m+[m[32m        return new GrMockGpu(context, kOptions, contextOptions);[m
     }[m
 [m
     ~GrMockGpu() override {}[m
[36m@@ -74,8 +95,8 @@[m [mpublic:[m
     sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }[m
 [m
 private:[m
[31m-    GrMockGpu(GrContext* context, const GrContextOptions& options) : INHERITED(context) {[m
[31m-        fCaps.reset(new GrMockCaps(options));[m
[32m+[m[32m    GrMockGpu(GrContext* context, const GrMockOptions& options, const GrContextOptions& contextOptions) : INHERITED(context) {[m
[32m+[m[32m        fCaps.reset(new GrMockCaps(contextOptions, options));[m
     }[m
 [m
     void onResetContext(uint32_t resetBits) override {}[m
[1mdiff --git a/tools/gpu/GrContextFactory.cpp b/tools/gpu/GrContextFactory.cpp[m
[1mindex 23ffad155f..d614b0643e 100644[m
[1m--- a/tools/gpu/GrContextFactory.cpp[m
[1m+++ b/tools/gpu/GrContextFactory.cpp[m
[36m@@ -22,6 +22,7 @@[m
 #endif[m
 #include "gl/null/NullGLTestContext.h"[m
 #include "gl/GrGLGpu.h"[m
[32m+[m[32m#include "mock/MockTestContext.h"[m
 #include "GrCaps.h"[m
 [m
 #if defined(SK_BUILD_FOR_WIN32) && defined(SK_ENABLE_DISCRETE_GPU)[m
[36m@@ -220,6 +221,19 @@[m [mContextInfo GrContextFactory::getContextInfoInternal(ContextType type, ContextOv[m
             break;[m
         }[m
 #endif[m
[32m+[m[32m        case kMock_GrBackend: {[m
[32m+[m[32m            TestContext* sharedContext = masterContext ? masterContext->fTestContext : nullptr;[m
[32m+[m[32m            SkASSERT(kMock_ContextType == type);[m
[32m+[m[32m            if (ContextOverrides::kRequireNVPRSupport & overrides) {[m
[32m+[m[32m                return ContextInfo();[m
[32m+[m[32m            }[m
[32m+[m[32m            testCtx.reset(CreateMockTestContext(sharedContext));[m
[32m+[m[32m            if (!testCtx) {[m
[32m+[m[32m                return ContextInfo();[m
[32m+[m[32m            }[m
[32m+[m[32m            backendContext = testCtx->backendContext();[m
[32m+[m[32m            break;[m
[32m+[m[32m        }[m
         default:[m
             return ContextInfo();[m
     }[m
[1mdiff --git a/tools/gpu/GrContextFactory.h b/tools/gpu/GrContextFactory.h[m
[1mindex 508249b8d8..1b142d873c 100644[m
[1m--- a/tools/gpu/GrContextFactory.h[m
[1m+++ b/tools/gpu/GrContextFactory.h[m
[36m@@ -44,7 +44,8 @@[m [mpublic:[m
         kNullGL_ContextType,         //! Non-rendering OpenGL mock context.[m
         kDebugGL_ContextType,        //! Non-rendering, state verifying OpenGL context.[m
         kVulkan_ContextType,         //! Vulkan[m
[31m-        kLastContextType = kVulkan_ContextType[m
[32m+[m[32m        kMock_ContextType,[m
[32m+[m[32m        kLastContextType = kMock_ContextType[m
     };[m
 [m
     static const int kContextTypeCnt = kLastContextType + 1;[m
[36m@@ -68,6 +69,7 @@[m [mpublic:[m
         switch (type) {[m
             case kNullGL_ContextType:[m
             case kDebugGL_ContextType:[m
[32m+[m[32m            case kMock_ContextType:[m
                 return false;[m
             default:[m
                 return true;[m
[36m@@ -78,6 +80,8 @@[m [mpublic:[m
         switch (type) {[m
             case kVulkan_ContextType:[m
                 return kVulkan_GrBackend;[m
[32m+[m[32m            case kMock_ContextType:[m
[32m+[m[32m                return kMock_GrBackend;[m
             default:[m
                 return kOpenGL_GrBackend;[m
         }[m
[36m@@ -109,8 +113,10 @@[m [mpublic:[m
                 return "Debug GL";[m
             case kVulkan_ContextType:[m
                 return "Vulkan";[m
[32m+[m[32m            case kMock_ContextType:[m
[32m+[m[32m                return "Mock";[m
         }[m
[31m-        SkDEBUGFAIL("Unreachable");[m
[32m+[m[32m        SkFAIL("Unreachable");[m
         return "Unknown";[m
     }[m
 [m
[1mdiff --git a/tools/gpu/mock/MockTestContext.cpp b/tools/gpu/mock/MockTestContext.cpp[m
[1mnew file mode 100644[m
[1mindex 0000000000..c47fff5de3[m
[1m--- /dev/null[m
[1m+++ b/tools/gpu/mock/MockTestContext.cpp[m
[36m@@ -0,0 +1,45 @@[m
[32m+[m
[32m+[m[32m/*[m
[32m+[m[32m * Copyright 2017 Google Inc.[m
[32m+[m[32m *[m
[32m+[m[32m * Use of this source code is governed by a BSD-style license that can be[m
[32m+[m[32m * found in the LICENSE file.[m
[32m+[m[32m */[m
[32m+[m[32m#ifndef GLTestContext_DEFINED[m
[32m+[m[32m#define GLTestContext_DEFINED[m
[32m+[m
[32m+[m[32m#include "MockTestContext.h"[m
[32m+[m
[32m+[m[32mnamespace {[m
[32m+[m
[32m+[m[32mclass MockTestContext : public sk_gpu_test::TestContext {[m
[32m+[m[32mpublic:[m
[32m+[m[32m    MockTestContext() {}[m
[32m+[m[32m    ~MockTestContext() override {}[m
[32m+[m
[32m+[m[32m    virtual GrBackend backend() override { return kMock_GrBackend; }[m
[32m+[m[32m    virtual GrBackendContext backendContext() override {[m
[32m+[m[32m        return reinterpret_cast<GrBackendContext>(nullptr);[m
[32m+[m[32m    }[m
[32m+[m[32m    bool isValid() const override { return true; }[m
[32m+[m[32m    void testAbandon() override {}[m
[32m+[m[32m    void submit() override {}[m
[32m+[m[32m    void finish() override {}[m
[32m+[m
[32m+[m[32mprotected:[m
[32m+[m[32m    void teardown() override {}[m
[32m+[m[32m    void onPlatformMakeCurrent() const override {}[m
[32m+[m[32m    void onPlatformSwapBuffers() const override {}[m
[32m+[m
[32m+[m[32mprivate:[m
[32m+[m[32m    typedef sk_gpu_test::TestContext INHERITED;[m
[32m+[m[32m};[m
[32m+[m
[32m+[m[32m} // anonymous namespace[m
[32m+[m
[32m+[m[32mnamespace sk_gpu_test {[m
[32m+[m
[32m+[m[32mTestContext* CreateMockTestContext(TestContext*) { return new MockTestContext(); }[m
[32m+[m
[32m+[m[32m}  // namespace sk_gpu_test[m
[32m+[m[32m#endif[m
[1mdiff --git a/tools/gpu/mock/MockTestContext.h b/tools/gpu/mock/MockTestContext.h[m
[1mnew file mode 100644[m
[1mindex 0000000000..4aac2483e6[m
[1m--- /dev/null[m
[1m+++ b/tools/gpu/mock/MockTestContext.h[m
[36m@@ -0,0 +1,22 @@[m
[32m+[m
[32m+[m[32m/*[m
[32m+[m[32m * Copyright 2017 Google Inc.[m
[32m+[m[32m *[m
[32m+[m[32m * Use of this source code is governed by a BSD-style license that can be[m
[32m+[m[32m * found in the LICENSE file.[m
[32m+[m[32m */[m
[32m+[m[32m#ifndef MockTestContext_DEFINED[m
[32m+[m[32m#define MockTestContext_DEFINED[m
[32m+[m
[32m+[m[32m#include "TestContext.h"[m
[32m+[m
[32m+[m[32mnamespace sk_gpu_test {[m
[32m+[m
[32m+[m[32m/**[m
[32m+[m[32m * Creates mock context object for use with GrContexts created with kMock_GrBackend. It will[m
[32m+[m[32m * trivially succeed at everything.[m
[32m+[m[32m */[m
[32m+[m[32mTestContext* CreateMockTestContext(TestContext* shareContext = nullptr);[m
[32m+[m
[32m+[m[32m}  // namespace sk_gpu_test[m
[32m+[m[32m#endif[m
