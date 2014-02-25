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
    , fGM(gmFactory(NULL))
    , fName(UnderJoin(fGM->shortName(), name))
    , fExpectations(expectations)
    , fColorType(colorType)
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
    SkImageInfo info = SkImageInfo::Make(SkScalarCeilToInt(fGM->width()),
                                         SkScalarCeilToInt(fGM->height()),
                                         fColorType, kPremul_SkAlphaType);
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
