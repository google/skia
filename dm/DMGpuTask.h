#ifndef DMGpuTask_DEFINED
#define DMGpuTask_DEFINED

#include "DMExpectations.h"
#include "DMReporter.h"
#include "DMTask.h"
#include "DMTaskRunner.h"
#include "GrContextFactory.h"
#include "SkBitmap.h"
#include "SkString.h"
#include "SkTemplates.h"
#include "gm.h"

// This is the main entry point for drawing GMs with the GPU.

namespace DM {

class GpuTask : public Task {
public:
    GpuTask(const char* name,
            Reporter*,
            TaskRunner*,
            const Expectations&,
            skiagm::GMRegistry::Factory,
            SkBitmap::Config,
            GrContextFactory::GLContextType,
            int sampleCount);

    virtual void draw() SK_OVERRIDE;
    virtual bool usesGpu() const SK_OVERRIDE { return true; }
    virtual bool shouldSkip() const SK_OVERRIDE;
    virtual SkString name() const SK_OVERRIDE { return fName; }

private:
    SkAutoTDelete<skiagm::GM> fGM;
    const SkString fName;
    const Expectations& fExpectations;
    const SkBitmap::Config fConfig;
    const GrContextFactory::GLContextType fContextType;
    const int fSampleCount;
};

}  // namespace DM

#endif  // DMGpuTask_DEFINED
