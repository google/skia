#include "DMCpuGMTask.h"
#include "DMPipeTask.h"
#include "DMQuiltTask.h"
#include "DMSerializeTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

namespace DM {

CpuGMTask::CpuGMTask(const char* config,
                     Reporter* reporter,
                     TaskRunner* taskRunner,
                     skiagm::GMRegistry::Factory gmFactory,
                     SkColorType colorType)
    : CpuTask(reporter, taskRunner)
    , fGMFactory(gmFactory)
    , fGM(fGMFactory(NULL))
    , fName(UnderJoin(fGM->getName(), config))
    , fColorType(colorType)
    {}

void CpuGMTask::draw() {
    SkBitmap bm;
    AllocatePixels(fColorType, fGM->getISize().width(), fGM->getISize().height(), &bm);

    SkCanvas canvas(bm);
    CanvasPreflight(&canvas);
    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

#define SPAWN(ChildTask, ...) this->spawnChild(SkNEW_ARGS(ChildTask, (*this, __VA_ARGS__)))
    SPAWN(PipeTask, fGMFactory(NULL), bm, PipeTask::kInProcess_Mode);
    SPAWN(PipeTask, fGMFactory(NULL), bm, PipeTask::kCrossProcess_Mode);
    SPAWN(PipeTask, fGMFactory(NULL), bm, PipeTask::kSharedAddress_Mode);

    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kNone_BBH);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kRTree_BBH);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kTileGrid_BBH);

    SPAWN(SerializeTask, fGMFactory(NULL), bm);

    SPAWN(WriteTask, "GM", bm);
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
