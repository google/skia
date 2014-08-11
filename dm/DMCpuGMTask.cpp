#include "DMCpuGMTask.h"
#include "DMExpectationsTask.h"
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
                     const Expectations& expectations,
                     SkColorType colorType)
    : CpuTask(reporter, taskRunner)
    , fGMFactory(gmFactory)
    , fGM(fGMFactory(NULL))
    , fName(UnderJoin(fGM->getName(), config))
    , fExpectations(expectations)
    , fColorType(colorType)
    {}

void CpuGMTask::draw() {
    SkBitmap bm;
    AllocatePixels(fColorType, fGM->getISize().width(), fGM->getISize().height(), &bm);

    SkCanvas canvas(bm);
    canvas.concat(fGM->getInitialTransform());
    fGM->draw(&canvas);
    canvas.flush();

#define SPAWN(ChildTask, ...) this->spawnChild(SkNEW_ARGS(ChildTask, (*this, __VA_ARGS__)))
    SPAWN(ExpectationsTask, fExpectations, bm);

    SPAWN(PipeTask, fGMFactory(NULL), bm, PipeTask::kInProcess_Mode);
    SPAWN(PipeTask, fGMFactory(NULL), bm, PipeTask::kCrossProcess_Mode);
    SPAWN(PipeTask, fGMFactory(NULL), bm, PipeTask::kSharedAddress_Mode);

    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kNone_BBH,     QuiltTask::kDefault_Backend);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kRTree_BBH,    QuiltTask::kDefault_Backend);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kQuadTree_BBH, QuiltTask::kDefault_Backend);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kTileGrid_BBH, QuiltTask::kDefault_Backend);

    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kNone_BBH,     QuiltTask::kSkRecord_Backend);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kRTree_BBH,    QuiltTask::kSkRecord_Backend);
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kQuadTree_BBH, QuiltTask::kSkRecord_Backend);
    /*  TODO: Failing, not sure why.  Enable these when passing.
    SPAWN(QuiltTask, fGMFactory(NULL), bm, QuiltTask::kTileGrid_BBH, QuiltTask::kSkRecord_Backend);
    */

    SPAWN(SerializeTask, fGMFactory(NULL), bm, SerializeTask::kNormal_Mode);
    SPAWN(SerializeTask, fGMFactory(NULL), bm, SerializeTask::kSkRecord_Mode);

    SPAWN(WriteTask, bm);
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
