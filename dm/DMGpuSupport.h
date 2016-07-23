/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DMGpuSupport_DEFINED
#define DMGpuSupport_DEFINED

// Provides Ganesh to DM,
// or if it's not available, fakes it enough so most code doesn't have to know that.

#include "SkSurface.h"

// This should be safe to include even in no-gpu builds. Include by relative path so it
// can be found in non-gpu builds.
#include "../include/gpu/GrContextOptions.h"

#if SK_SUPPORT_GPU

// Ganesh is available.  Yippee!

#  include "GrContext.h"
#  include "GrContextFactory.h"

namespace DM {

static const bool kGPUDisabled = false;

static inline sk_sp<SkSurface> NewGpuSurface(
        sk_gpu_test::GrContextFactory* grFactory,
        sk_gpu_test::GrContextFactory::ContextType type,
        sk_gpu_test::GrContextFactory::ContextOptions options,
        SkImageInfo info,
        int samples,
        bool useDIText) {
    uint32_t flags = useDIText ? SkSurfaceProps::kUseDeviceIndependentFonts_Flag : 0;
    if (SkImageInfoIsGammaCorrect(info)) {
        flags |= SkSurfaceProps::kGammaCorrect_Flag;
    }
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    return SkSurface::MakeRenderTarget(grFactory->get(type, options), SkBudgeted::kNo,
                                       info, samples, &props);
}

}  // namespace DM

#else// !SK_SUPPORT_GPU

// Ganesh is not available.  Fake it.

enum GrGLStandard {
    kNone_GrGLStandard,
    kGL_GrGLStandard,
    kGLES_GrGLStandard
};
static const int kGrGLStandardCnt = 3;

class GrContext {
public:
    void dumpCacheStats(SkString*) const {}
    void dumpGpuStats(SkString*) const {}
};

namespace sk_gpu_test {
class GrContextFactory {
public:
    GrContextFactory() {};
    explicit GrContextFactory(const GrContextOptions&) {}

    typedef int ContextType;

    static const ContextType kANGLE_ContextType         = 0,
                             kANGLE_GL_ContextType      = 0,
                             kCommandBuffer_ContextType = 0,
                             kDebugGL_ContextType       = 0,
                             kMESA_ContextType          = 0,
                             kNVPR_ContextType          = 0,
                             kNativeGL_ContextType      = 0,
                             kGL_ContextType            = 0,
                             kGLES_ContextType          = 0,
                             kNullGL_ContextType        = 0,
                             kVulkan_ContextType        = 0;
    static const int kContextTypeCnt = 1;
    enum ContextOptions {
        kNone_ContextOptions = 0,
        kEnableNVPR_ContextOptions = 0x1,
    };
    void destroyContexts() {}

    void abandonContexts() {}

    void releaseResourcesAndAbandonContexts() {}
};
}  // namespace sk_gpu_test

namespace DM {

static const bool kGPUDisabled = true;

static inline SkSurface* NewGpuSurface(sk_gpu_test::GrContextFactory*,
                                       sk_gpu_test::GrContextFactory::ContextType,
                                       sk_gpu_test::GrContextFactory::ContextOptions,
                                       SkImageInfo,
                                       int,
                                       bool) {
    return nullptr;
}

}  // namespace DM

#endif//SK_SUPPORT_GPU

#endif//DMGpuSupport_DEFINED
