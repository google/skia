#include "DMGpuGMTask.h"

#include "DMExpectationsTask.h"
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
                     const Expectations& expectations,
                     GrContextFactory::GLContextType contextType,
                     int sampleCount)
    : GpuTask(reporter, taskRunner)
    , fGM(gmFactory(NULL))
    , fName(UnderJoin(fGM->getName(), config))
    , fExpectations(expectations)
    , fContextType(contextType)
    , fSampleCount(sampleCount)
    {}

void GpuGMTask::draw(GrContextFactory* grFactory) {
    SkImageInfo info = SkImageInfo::Make(SkScalarCeilToInt(fGM->width()),
                                         SkScalarCeilToInt(fGM->height()),
                                         kN32_SkColorType,
                                         kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(NewGpuSurface(grFactory, fContextType, info, fSampleCount));
    SkCanvas* canvas = surface->getCanvas();

    canvas->concat(fGM->getInitialTransform());
    fGM->draw(canvas);
    canvas->flush();

    SkBitmap bitmap;
    bitmap.setInfo(info);
    canvas->readPixels(&bitmap, 0, 0);

    this->spawnChild(SkNEW_ARGS(ExpectationsTask, (*this, fExpectations, bitmap)));
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
}

bool GpuGMTask::shouldSkip() const {
    return kGPUDisabled || SkToBool(fGM->getFlags() & skiagm::GM::kSkipGPU_Flag);
}

}  // namespace DM
