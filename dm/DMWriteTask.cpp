#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkCommandLineFlags.h"
#include "SkImageEncoder.h"
#include "SkString.h"

DEFINE_string2(writePath, w, "", "If set, write GMs here as .pngs.");

namespace DM {

WriteTask::WriteTask(const Task& parent, SkBitmap bitmap) : Task(parent), fBitmap(bitmap) {
    const int suffixes = parent.depth() + 1;
    const char* name = parent.name().c_str();
    SkTArray<SkString> split;
    SkStrSplit(name, "_", &split);
    int totalSuffixLength = 0;
    for (int i = 0; i < suffixes; i++) {
        // We're splitting off suffixes from the back to front.
        fSuffixes.push_back(split[split.count()-i-1]);
        totalSuffixLength += fSuffixes.back().size() + 1;
    }
    fGmName.set(name, strlen(name)-totalSuffixLength);
}

void WriteTask::makeDirOrFail(SkString dir) {
    if (!sk_mkdir(dir.c_str())) {
        this->fail();
    }
}

void WriteTask::draw() {
    SkString dir(FLAGS_writePath[0]);
    this->makeDirOrFail(dir);
    for (int i = 0; i < fSuffixes.count(); i++) {
        dir = SkOSPath::SkPathJoin(dir.c_str(), fSuffixes[i].c_str());
        this->makeDirOrFail(dir);
    }
    if (!SkImageEncoder::EncodeFile(Png(SkOSPath::SkPathJoin(dir.c_str(), fGmName.c_str())).c_str(),
                                    fBitmap,
                                    SkImageEncoder::kPNG_Type,
                                    100/*quality*/)) {
        this->fail();
    }
}

SkString WriteTask::name() const {
    SkString name("writing ");
    for (int i = 0; i < fSuffixes.count(); i++) {
        name.appendf("%s/", fSuffixes[i].c_str());
    }
    name.append(fGmName.c_str());
    return name;
}

bool WriteTask::shouldSkip() const {
    return FLAGS_writePath.isEmpty();
}

}  // namespace DM
