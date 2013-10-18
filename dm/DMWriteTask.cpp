#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkCommandLineFlags.h"
#include "SkImageEncoder.h"

#include <string.h>

DEFINE_string2(writePath, w, "", "If set, write GMs here as .pngs.");

namespace DM {

WriteTask::WriteTask(const Task& parent, SkBitmap bitmap)
    : Task(parent)
    , fBitmap(bitmap) {
    // Split parent's name <gmName>_<config> into gmName and config.
    const char* parentName = parent.name().c_str();
    const char* fromLastUnderscore = strrchr(parentName, '_');
    const ptrdiff_t gmNameLength = fromLastUnderscore - parentName;

    fConfig.set(fromLastUnderscore+1);
    fGmName.set(parentName, gmNameLength);
}

void WriteTask::draw() {
    const char* root = FLAGS_writePath[0];
    const SkString dir = SkOSPath::SkPathJoin(root, fConfig.c_str());
    if (!sk_mkdir(root) ||
        !sk_mkdir(dir.c_str())  ||
        !SkImageEncoder::EncodeFile(png(SkOSPath::SkPathJoin(dir.c_str(), fGmName.c_str())).c_str(),
                                    fBitmap,
                                    SkImageEncoder::kPNG_Type,
                                    100/*quality*/))
    {
        this->fail();
    }
}

SkString WriteTask::name() const {
    return SkStringPrintf("writing %s/%s.png", fConfig.c_str(), fGmName.c_str());
}

bool WriteTask::shouldSkip() const {
    return FLAGS_writePath.isEmpty();
}

}  // namespace DM
