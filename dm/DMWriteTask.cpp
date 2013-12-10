#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkCommandLineFlags.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkString.h"

DEFINE_string2(writePath, w, "", "If set, write GMs here as .pngs.");

namespace DM {

// Splits off the last N suffixes of name (splitting on _) and appends them to out.
// Returns the total number of characters consumed.
static int split_suffixes(int N, const char* name, SkTArray<SkString>* out) {
    SkTArray<SkString> split;
    SkStrSplit(name, "_", &split);
    int consumed = 0;
    for (int i = 0; i < N; i++) {
        // We're splitting off suffixes from the back to front.
        out->push_back(split[split.count()-i-1]);
        consumed += out->back().size() + 1;  // Add one for the _.
    }
    return consumed;
}

WriteTask::WriteTask(const Task& parent, SkBitmap bitmap) : Task(parent), fBitmap(bitmap) {
    const int suffixes = parent.depth() + 1;
    const SkString& name = parent.name();
    const int totalSuffixLength = split_suffixes(suffixes, name.c_str(), &fSuffixes);
    fGmName.set(name.c_str(), name.size()-totalSuffixLength);
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
    SkString path = SkOSPath::SkPathJoin(dir.c_str(), fGmName.c_str());
    path.append(".png");
    if (!SkImageEncoder::EncodeFile(path.c_str(),
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

static SkString path_to_expected_image(const char* root, const Task& task) {
    SkString filename = task.name();

    // We know that all names passed in here belong to top-level Tasks, which have a single suffix
    // (8888, 565, gpu, etc.) indicating what subdirectory to look in.
    SkTArray<SkString> suffixes;
    const int suffixLength = split_suffixes(1, filename.c_str(), &suffixes);
    SkASSERT(1 == suffixes.count());

    // We'll look in root/suffix for images.
    const SkString dir = SkOSPath::SkPathJoin(root, suffixes[0].c_str());

    // Remove the suffix and tack on a .png.
    filename.remove(filename.size() - suffixLength, suffixLength);
    filename.append(".png");

    //SkDebugf("dir %s, filename %s\n", dir.c_str(), filename.c_str());

    return SkOSPath::SkPathJoin(dir.c_str(), filename.c_str());
}

bool WriteTask::Expectations::check(const Task& task, SkBitmap bitmap) const {
    const SkString path = path_to_expected_image(fRoot, task);

    SkBitmap expected;
    if (SkImageDecoder::DecodeFile(path.c_str(), &expected)) {
        if (expected.config() != bitmap.config()) {
            SkBitmap converted;
            SkAssertResult(expected.copyTo(&converted, bitmap.config()));
            expected.swap(converted);
        }
        SkASSERT(expected.config() == bitmap.config());
        return BitmapsEqual(expected, bitmap);
    }

    // Couldn't read the file, etc.
    SkDebugf("Problem decoding %s to SkBitmap.\n", path.c_str());
    return false;
}

}  // namespace DM
