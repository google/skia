#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkImageEncoder.h"
#include "SkMallocPixelRef.h"
#include "SkStream.h"
#include "SkString.h"

DEFINE_string2(writePath, w, "", "If set, write GMs here as .pngs.");
DEFINE_bool(writePngOnly, false, "If true, don't encode raw bitmap after .png data.  "
                                 "This means -r won't work, but skdiff will still work fine.");

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

inline static SkString find_gm_name(const Task& parent, SkTArray<SkString>* suffixList) {
    const int suffixes = parent.depth() + 1;
    const SkString& name = parent.name();
    const int totalSuffixLength = split_suffixes(suffixes, name.c_str(), suffixList);
    return SkString(name.c_str(), name.size() - totalSuffixLength);
}

WriteTask::WriteTask(const Task& parent, SkBitmap bitmap)
    : CpuTask(parent)
    , fGmName(find_gm_name(parent, &fSuffixes))
    , fBitmap(bitmap)
    , fData(NULL)
    , fExtension(".png") {}

WriteTask::WriteTask(const Task& parent, SkData *data, const char* ext)
    : CpuTask(parent)
    , fGmName(find_gm_name(parent, &fSuffixes))
    , fData(SkRef(data))
    , fExtension(ext) {}

void WriteTask::makeDirOrFail(SkString dir) {
    if (!sk_mkdir(dir.c_str())) {
        this->fail();
    }
}

namespace {

// One file that first contains a .png of an SkBitmap, then its raw pixels.
// We use this custom format to avoid premultiplied/unpremultiplied pixel conversions.
struct PngAndRaw {
    static bool Encode(SkBitmap bitmap, const char* path) {
        SkFILEWStream stream(path);
        if (!stream.isValid()) {
            SkDebugf("Can't write %s.\n", path);
            return false;
        }

        // Write a PNG first for humans and other tools to look at.
        if (!SkImageEncoder::EncodeStream(&stream, bitmap, SkImageEncoder::kPNG_Type, 100)) {
            SkDebugf("Can't encode a PNG.\n");
            return false;
        }
        if (FLAGS_writePngOnly) {
            return true;
        }

        // Pad out so the raw pixels start 4-byte aligned.
        const uint32_t maxPadding = 0;
        const size_t pos = stream.bytesWritten();
        stream.write(&maxPadding, SkAlign4(pos) - pos);

        // Then write our secret raw pixels that only DM reads.
        SkAutoLockPixels lock(bitmap);
        return stream.write(bitmap.getPixels(), bitmap.getSize());
    }

    // This assumes bitmap already has allocated pixels of the correct size.
    static bool Decode(const char* path, SkImageInfo info, SkBitmap* bitmap) {
        SkAutoTUnref<SkData> data(SkData::NewFromFileName(path));
        if (!data) {
            SkDebugf("Can't read %s.\n", path);
            return false;
        }

        // The raw pixels are at the end of the file.  We'll skip the encoded PNG at the front.
        const size_t rowBytes = info.minRowBytes();  // Assume densely packed.
        const size_t bitmapBytes = info.getSafeSize(rowBytes);
        if (data->size() < bitmapBytes) {
            SkDebugf("%s is too small to contain the bitmap we're looking for.\n", path);
            return false;
        }

        const size_t offset = data->size() - bitmapBytes;
        SkAutoTUnref<SkData> subset(
                SkData::NewSubset(data, offset, bitmapBytes));
        SkAutoTUnref<SkPixelRef> pixels(
            SkMallocPixelRef::NewWithData(
                    info, rowBytes, NULL/*ctable*/, subset));
        SkASSERT(pixels);

        bitmap->setInfo(info, rowBytes);
        bitmap->setPixelRef(pixels);
        return true;
    }
};

// Does not take ownership of data.
bool save_data_to_file(const SkData* data, const char* path) {
    SkFILEWStream stream(path);
    if (!stream.isValid() || !stream.write(data->data(), data->size())) {
        SkDebugf("Can't write %s.\n", path);
        return false;
    }
    return true;
}

}  // namespace

void WriteTask::draw() {
    SkString dir(FLAGS_writePath[0]);
    this->makeDirOrFail(dir);
    for (int i = 0; i < fSuffixes.count(); i++) {
        dir = SkOSPath::SkPathJoin(dir.c_str(), fSuffixes[i].c_str());
        this->makeDirOrFail(dir);
    }

    SkString path = SkOSPath::SkPathJoin(dir.c_str(), fGmName.c_str());
    path.append(fExtension);

    const bool ok = fData.get() ? save_data_to_file(fData,   path.c_str())
                                : PngAndRaw::Encode(fBitmap, path.c_str());
    if (!ok) {
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

    return SkOSPath::SkPathJoin(dir.c_str(), filename.c_str());
}

bool WriteTask::Expectations::check(const Task& task, SkBitmap bitmap) const {
    if (!FLAGS_writePath.isEmpty() && 0 == strcmp(FLAGS_writePath[0], fRoot)) {
        SkDebugf("We seem to be reading and writing %s concurrently.  This won't work.\n", fRoot);
        return false;
    }

    const SkString path = path_to_expected_image(fRoot, task);
    SkBitmap expected;
    if (!PngAndRaw::Decode(path.c_str(), bitmap.info(), &expected)) {
        return false;
    }

    return BitmapsEqual(expected, bitmap);
}

}  // namespace DM
