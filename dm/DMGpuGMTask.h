#ifndef DMGpuGMTask_DEFINED
#define DMGpuGMTask_DEFINED

#include "DMGpuSupport.h"
#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// This is the main entry point for drawing GMs with the GPU.

namespace DM {

class GpuGMTask : public GpuTask {
public:
    GpuGMTask(const char* config,
              Reporter*,
              TaskRunner*,
              skiagm::GMRegistry::Factory,
              GrContextFactory::GLContextType,
              GrGLStandard gpuAPI,
              int sampleCount);

    virtual void draw(GrContextFactory*) SK_OVERRIDE;
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTDelete<skiagm::GM> fGM;
    const SkString fName;
    const GrContextFactory::GLContextType fContextType;
    GrGLStandard fGpuAPI;
    const int fSampleCount;
};

}  // namespace DM

#endif  // DMGpuGMTask_DEFINED
