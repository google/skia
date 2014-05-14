#include "DMRecordTask.h"
#include "DMSKPTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

namespace DM {

SKPTask::SKPTask(Reporter* r, TaskRunner* tr, SkPicture* pic, SkString name)
    : CpuTask(r, tr), fPicture(SkRef(pic)), fName(name) {}

void SKPTask::draw() {
    SkBitmap bitmap;
    SetupBitmap(kN32_SkColorType, *fPicture, &bitmap);
    DrawPicture(fPicture, &bitmap);

    this->spawnChild(SkNEW_ARGS(RecordTask,
                                (*this, fPicture, bitmap, RecordTask::kNoOptimize_Mode)));
    this->spawnChild(SkNEW_ARGS(RecordTask,
                                (*this, fPicture, bitmap, RecordTask::kOptimize_Mode)));
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap, WriteTask::kVerbatim_Mode)));
}

}  // namespace DM
