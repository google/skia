#include "DMGpuTask.h"

#include "DMExpectationsTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"
#include "SkGpuDevice.h"
#include "SkTLS.h"

namespace DM {

GpuTask::GpuTask(const char* name,
                 Reporter* reporter,
                 TaskRunner* taskRunner,
                 const Expectations& expectations,
                 skiagm::GMRegistry::Factory gmFactory,
                 SkBitmap::Config config,
                 GrContextFactory::GLContextType contextType,
                 int sampleCount)
    : Task(reporter, taskRunner)
    , fGM(gmFactory(NULL))
    , fName(UnderJoin(fGM->shortName(), name))
    , fExpectations(expectations)
    , fConfig(config)
    , fContextType(contextType)
    , fSampleCount(sampleCount)
    {}

static void* new_gr_context_factory() {
    return SkNEW(GrContextFactory);
}

static void delete_gr_context_factory(void* factory) {
    SkDELETE((GrContextFactory*) factory);
}

static GrContextFactory* get_gr_factory() {
    return reinterpret_cast<GrContextFactory*>(SkTLS::Get(&new_gr_context_factory,
                                                          &delete_gr_context_factory));
}

void GpuTask::draw() {
    GrContext* gr = get_gr_factory()->get(fContextType);  // Will be owned by device.
    SkGpuDevice device(gr,
                       fConfig,
                       SkScalarCeilToInt(fGM->width()),
                       SkScalarCeilToInt(fGM->height()),
                       fSampleCount);
    SkCanvas canvas(&device);

    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

    SkBitmap bitmap;
    bitmap.setConfig(fConfig, SkScalarCeilToInt(fGM->width()), SkScalarCeilToInt(fGM->height()));
    canvas.readPixels(&bitmap, 0, 0);

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
