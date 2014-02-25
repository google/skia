#include "DMGpuTask.h"

#include "DMExpectationsTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkSurface.h"
#include "SkTLS.h"

namespace DM {

GpuTask::GpuTask(const char* name,
                 Reporter* reporter,
                 TaskRunner* taskRunner,
                 const Expectations& expectations,
                 skiagm::GMRegistry::Factory gmFactory,
                 SkColorType colorType,
                 GrContextFactory::GLContextType contextType,
                 int sampleCount)
    : Task(reporter, taskRunner)
    , fTaskRunner(taskRunner)
    , fGM(gmFactory(NULL))
    , fName(UnderJoin(fGM->shortName(), name))
    , fExpectations(expectations)
    , fColorType(colorType)
    , fContextType(contextType)
    , fSampleCount(sampleCount)
    {}

void GpuTask::draw() {
    SkImageInfo info = SkImageInfo::Make(SkScalarCeilToInt(fGM->width()),
                                         SkScalarCeilToInt(fGM->height()),
                                         fColorType,
                                         kPremul_SkAlphaType);
    GrContext* gr = fTaskRunner->getGrContextFactory()->get(fContextType);  // Owned by surface.
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRenderTarget(gr, info, fSampleCount));
    SkCanvas* canvas = surface->getCanvas();

    canvas->concat(fGM->getInitialTransform());
    fGM->draw(canvas);
    canvas->flush();

    SkBitmap bitmap;
    bitmap.setConfig(info);
    canvas->readPixels(&bitmap, 0, 0);

#if GR_CACHE_STATS
    gr->printCacheStats();
#endif

    this->spawnChild(SkNEW_ARGS(ExpectationsTask, (*this, fExpectations, bitmap)));
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
}

bool GpuTask::shouldSkip() const {
    return SkToBool(fGM->getFlags() & skiagm::GM::kSkipGPU_Flag);
}

}  // namespace DM
