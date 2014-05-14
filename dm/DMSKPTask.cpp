#include "DMRecordTask.h"
#include "DMSKPTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

namespace DM {

// foo_bar.skp -> foo-bar_skp
static SkString filename_to_task_name(SkString filename) {
    for (size_t i = 0; i < filename.size(); i++) {
        if ('_' == filename[i]) { filename[i] = '-'; }
        if ('.' == filename[i]) { filename[i] = '_'; }
    }
    return filename;
}

SKPTask::SKPTask(Reporter* r, TaskRunner* tr, SkPicture* pic, SkString filename)
    : CpuTask(r, tr), fPicture(SkRef(pic)), fName(filename_to_task_name(filename)) {}

void SKPTask::draw() {
    SkBitmap bitmap;
    SetupBitmap(kN32_SkColorType, *fPicture, &bitmap);
    DrawPicture(fPicture, &bitmap);

    this->spawnChild(SkNEW_ARGS(RecordTask,
                                (*this, fPicture, bitmap, RecordTask::kNoOptimize_Mode)));
    this->spawnChild(SkNEW_ARGS(RecordTask,
                                (*this, fPicture, bitmap, RecordTask::kOptimize_Mode)));
    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, bitmap)));
}

}  // namespace DM
