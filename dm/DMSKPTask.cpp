#include "DMSKPTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPictureRecorder.h"

DEFINE_int32(skpMaxWidth,  1000, "Max SKPTask viewport width.");
DEFINE_int32(skpMaxHeight, 1000, "Max SKPTask viewport height.");

namespace DM {

SKPTask::SKPTask(Reporter* r,
                 TaskRunner* tr,
                 const SkPicture* pic,
                 SkString filename)
    : CpuTask(r, tr)
    , fPicture(SkRef(pic))
    , fName(FileToTaskName(filename)) {}

void SKPTask::draw() {
    const int width  = SkTMin(SkScalarCeilToInt(fPicture->cullRect().width()),  FLAGS_skpMaxWidth),
              height = SkTMin(SkScalarCeilToInt(fPicture->cullRect().height()), FLAGS_skpMaxHeight);
    SkBitmap bitmap;
    AllocatePixels(kN32_SkColorType, width, height, &bitmap);
    DrawPicture(*fPicture, &bitmap);

    this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "SKP", bitmap)));
}

}  // namespace DM
