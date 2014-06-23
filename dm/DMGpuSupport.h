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
                                       int samples) {
    return SkSurface::NewRenderTarget(grFactory->get(type, gpuAPI), info, samples);
}

}  // namespace DM

#else// !SK_SUPPORT_GPU

// Ganesh is not available.  Fake it.

enum GrGLStandard {
    kNone_GrGLStandard,
    kGL_GrGLStandard,
    kGLES_GrGLStandard
};

class GrContextFactory {
public:
    typedef int GLContextType;

    static const GLContextType kANGLE_GLContextType  = 0,
                               kDebug_GLContextType  = 0,
                               kMESA_GLContextType   = 0,
                               kNVPR_GLContextType   = 0,
                               kNative_GLContextType = 0,
                               kNull_GLContextType   = 0;
};

namespace DM {

static const bool kGPUDisabled = true;

static inline SkSurface* NewGpuSurface(GrContextFactory*,
                                       GrContextFactory::GLContextType,
                                       GrGLStandard,
                                       SkImageInfo,
                                       int) {
    return NULL;
}

}  // namespace DM

#endif//SK_SUPPORT_GPU

#endif//DMGpuSupport_DEFINED
