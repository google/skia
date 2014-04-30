#include "DMCpuGMTask.h"
#include "DMExpectationsTask.h"
#include "DMPipeTask.h"
#include "DMQuiltTask.h"
#include "DMRecordTask.h"
#include "DMReplayTask.h"
#include "DMSerializeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

namespace DM {

CpuGMTask::CpuGMTask(const char* config,
                     Reporter* reporter,
                     TaskRunner* taskRunner,
                     const Expectations& expectations,
                     skiagm::GMRegistry::Factory gmFactory,
                     SkColorType colorType)
    : CpuTask(reporter, taskRunner)
    , fGMFactory(gmFactory)
    , fGM(fGMFactory(NULL))
    , fName(UnderJoin(fGM->getName(), config))
    , fExpectations(expectations)
    , fColorType(colorType)
    {}

void CpuGMTask::draw() {
    SkBitmap bitmap;
    SetupBitmap(fColorType, fGM.get(), &bitmap);

    SkCanvas canvas(bitmap);
    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

#define SPAWN(ChildTask, ...) this->spawnChild(SkNEW_ARGS(ChildTask, (*this, __VA_ARGS__)))
    SPAWN(ExpectationsTask, fExpectations, bitmap);

    SPAWN(PipeTask, fGMFactory(NULL), bitmap, false, false);
    SPAWN(PipeTask, fGMFactory(NULL), bitmap, true, false);
    SPAWN(PipeTask, fGMFactory(NULL), bitmap, true, true);
    SPAWN(QuiltTask, fGMFactory(NULL), bitmap);
    SPAWN(RecordTask, fGMFactory(NULL), bitmap);
    SPAWN(ReplayTask, fGMFactory(NULL), bitmap, false);
    SPAWN(ReplayTask, fGMFactory(NULL), bitmap, true);
    SPAWN(SerializeTask, fGMFactory(NULL), bitmap);

    SPAWN(WriteTask, bitmap);
#undef SPAWN
}

bool CpuGMTask::shouldSkip() const {
    if (kRGB_565_SkColorType == fColorType && (fGM->getFlags() & skiagm::GM::kSkip565_Flag)) {
        return true;
    }
    if (fGM->getFlags() & skiagm::GM::kGPUOnly_Flag) {
        return true;
    }
    return false;
}

}  // namespace DM
