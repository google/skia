#include "DMRecordTask.h"
#include "DMSKPTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

namespace DM {

SKPTask::SKPTask(Reporter* r, TaskRunner* tr, SkPicture* pic, SkString filename)
    : CpuTask(r, tr), fPicture(SkRef(pic)), fName(FileToTaskName(filename)) {}

void SKPTask::draw() {
    SkBitmap bitmap;
    AllocatePixels(kN32_SkColorType, fPicture->width(), fPicture->height(), &bitmap);
    DrawPicture(fPicture, &bitmap);

    this->spawnChild(SkNEW_ARGS(RecordTask,
                                (*this, fPicture, bitmap, RecordTask::kNoOptimize_Mode)));
    this->spawnChild(SkNEW_ARGS(RecordTask,
                                (*this, fPicture, bitmap, RecordTask::kOptimize_Mode)));
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
}

}  // namespace DM
