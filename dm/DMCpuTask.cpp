#include "DMCpuTask.h"
#include "DMReplayTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"
#include "SkCommandLineFlags.h"

DEFINE_bool(replay, false, "If true, run replay tests for each CpuTask.");
// TODO(mtklein): add the other various options

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
    , fName(underJoin(fGM->shortName(), name))
    , fExpectations(expectations.get(png(fName).c_str()))
    , fConfig(config)
    {}

void CpuTask::draw() {
    SkBitmap bitmap;
    bitmap.setConfig(fConfig, SkScalarCeilToInt(fGM->width()), SkScalarCeilToInt(fGM->height()));
    bitmap.allocPixels();
    bitmap.eraseColor(0x00000000);
    SkCanvas canvas(bitmap);

    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

    if (!meetsExpectations(fExpectations, bitmap)) {
        this->fail();
    }

    if (FLAGS_replay) {
        this->spawnChild(SkNEW_ARGS(ReplayTask,
                                   ("replay", *this, fGMFactory(NULL), bitmap)));
    }
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
