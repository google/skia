#include "DMGpuGMTask.h"

#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkSurface.h"
#include "SkTLS.h"

namespace DM {

GpuGMTask::GpuGMTask(const char* config,
                     Reporter* reporter,
                     TaskRunner* taskRunner,
                     skiagm::GMRegistry::Factory gmFactory,
                     GrContextFactory::GLContextType contextType,
                     GrGLStandard gpuAPI,
                     int sampleCount)
    : GpuTask(reporter, taskRunner)
    , fGM(gmFactory(NULL))
    , fName(UnderJoin(fGM->getName(), config))
    , fContextType(contextType)
    , fGpuAPI(gpuAPI)
    , fSampleCount(sampleCount)
    {}

void GpuGMTask::draw(GrContextFactory* grFactory) {
    SkImageInfo info = SkImageInfo::Make(SkScalarCeilToInt(fGM->width()),
                                         SkScalarCeilToInt(fGM->height()),
                                         kN32_SkColorType,
                                         kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(NewGpuSurface(grFactory, fContextType, fGpuAPI, info,
                                                  fSampleCount));
    if (!surface) {
        this->fail("Could not create context for the config and the api.");
        return;
    }
    SkCanvas* canvas = surface->getCanvas();

    canvas->concat(fGM->getInitialTransform());
    fGM->draw(canvas);
    canvas->flush();

    SkBitmap bitmap;
    bitmap.setInfo(info);
    canvas->readPixels(&bitmap, 0, 0);

    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "GM", bitmap)));
}

bool GpuGMTask::shouldSkip() const {
    return kGPUDisabled || SkToBool(fGM->getFlags() & skiagm::GM::kSkipGPU_Flag);
}

}  // namespace DM
