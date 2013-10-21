#include "DMCpuTask.h"
#include "DMReplayTask.h"
#include "DMSerializeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

namespace DM {

CpuTask::CpuTask(const char* name,
                 Reporter* reporter,
                 TaskRunner* taskRunner,
                 const skiagm::ExpectationsSource& expectations,
                 skiagm::GMRegistry::Factory gmFactory,
                 SkBitmap::Config config)
    : Task(reporter, taskRunner)
    , fGMFactory(gmFactory)
    , fGM(fGMFactory(NULL))
    , fName(UnderJoin(fGM->shortName(), name))
    , fExpectations(expectations.get(Png(fName).c_str()))
    , fConfig(config)
    {}

void CpuTask::draw() {
    SkBitmap bitmap;
    SetupBitmap(fConfig, fGM.get(), &bitmap);

    SkCanvas canvas(bitmap);
    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

    if (!MeetsExpectations(fExpectations, bitmap)) {
        this->fail();
    }

    this->spawnChild(SkNEW_ARGS(ReplayTask, (*this, fGMFactory(NULL), bitmap)));
    this->spawnChild(SkNEW_ARGS(SerializeTask, (*this, fGMFactory(NULL), bitmap)));
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
}

bool CpuTask::shouldSkip() const {
    if (SkBitmap::kRGB_565_Config == fConfig && (fGM->getFlags() & skiagm::GM::kSkip565_Flag)) {
        return true;
    }
    if (fGM->getFlags() & skiagm::GM::kGPUOnly_Flag) {
        return true;
    }
    return false;
}

}  // namespace DM
