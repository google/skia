#include "DMGpuGMTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommonFlags.h"
#include "SkSurface.h"
#include "SkTLS.h"

namespace DM {

GpuGMTask::GpuGMTask(const char* config,
                     Reporter* reporter,
                     TaskRunner* taskRunner,
                     skiagm::GMRegistry::Factory gmFactory,
                     GrContextFactory::GLContextType contextType,
                     GrGLStandard gpuAPI,
                     int sampleCount,
                     bool useDFText)
    : GpuTask(reporter, taskRunner)
    , fGM(gmFactory(NULL))
    , fName(UnderJoin(fGM->getName(), config))
    , fContextType(contextType)
    , fGpuAPI(gpuAPI)
    , fSampleCount(sampleCount)
    , fUseDFText(useDFText)
    {}

static bool gAlreadyWarned[GrContextFactory::kGLContextTypeCnt][kGrGLStandardCnt];

void GpuGMTask::draw(GrContextFactory* grFactory) {
    SkImageInfo info = SkImageInfo::Make(SkScalarCeilToInt(fGM->width()),
                                         SkScalarCeilToInt(fGM->height()),
                                         kN32_SkColorType,
                                         kPremul_SkAlphaType);
    SkAutoTUnref<SkSurface> surface(NewGpuSurface(grFactory, fContextType, fGpuAPI, info,
                                                  fSampleCount, fUseDFText));
    if (!surface) {
        if (!gAlreadyWarned[fContextType][fGpuAPI]) {
            SkDebugf("FYI: couldn't create GPU context, type %d API %d.  Will skip.\n",
                     fContextType, fGpuAPI);
            gAlreadyWarned[fContextType][fGpuAPI] = true;
        }
        return;
    }
    SkCanvas* canvas = surface->getCanvas();
    CanvasPreflight(canvas);

    canvas->concat(fGM->getInitialTransform());
    fGM->draw(canvas);
    canvas->flush();
#if GR_CACHE_STATS && SK_SUPPORT_GPU
    if (FLAGS_veryVerbose) {
        grFactory->get(fContextType)->printCacheStats();
    }
#endif

    SkBitmap bitmap;
    bitmap.setInfo(info);
    canvas->readPixels(&bitmap, 0, 0);

    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "GM", bitmap)));
}

bool GpuGMTask::shouldSkip() const {
    return kGPUDisabled || SkToBool(fGM->getFlags() & skiagm::GM::kSkipGPU_Flag);
}

}  // namespace DM
