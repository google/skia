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

#if SK_SUPPORT_GPU

// Ganesh is available.  Yippee!

#  include "GrContext.h"
#  include "GrContextFactory.h"

namespace DM {

static const bool kGPUDisabled = false;

static inline SkSurface* NewGpuSurface(GrContextFactory* grFactory,
                                       GrContextFactory::GLContextType type,
                                       GrGLStandard gpuAPI,
                                       SkImageInfo info,
                                       int samples,
                                       bool useDFText) {
    uint32_t flags = useDFText ? SkSurfaceProps::kUseDistanceFieldFonts_Flag : 0;
    SkSurfaceProps props(flags, SkSurfaceProps::kLegacyFontHost_InitType);
    return SkSurface::NewRenderTarget(grFactory->get(type, gpuAPI), SkSurface::kNo_Budgeted,
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

struct GrContextOptions {};

class GrContextFactory {
public:
    GrContextFactory() {};
    explicit GrContextFactory(const GrContextOptions&) {}

    typedef int GLContextType;

    static const GLContextType kANGLE_GLContextType  = 0,
                               kDebug_GLContextType  = 0,
                               kMESA_GLContextType   = 0,
                               kNVPR_GLContextType   = 0,
                               kNative_GLContextType = 0,
                               kNull_GLContextType   = 0;
    static const int kGLContextTypeCnt = 1;
    void destroyContexts() {}

    void abandonContexts() {}
};

namespace DM {

static const bool kGPUDisabled = true;

static inline SkSurface* NewGpuSurface(GrContextFactory*,
                                       GrContextFactory::GLContextType,
                                       GrGLStandard,
                                       SkImageInfo,
                                       int,
                                       bool) {
    return NULL;
}

}  // namespace DM

#endif//SK_SUPPORT_GPU

#endif//DMGpuSupport_DEFINED
