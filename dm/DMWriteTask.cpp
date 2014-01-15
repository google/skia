#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkColorPriv.h"
#include "SkCommandLineFlags.h"
#include "SkImageDecoder.h"
#include "SkImageEncoder.h"
#include "SkString.h"
#include "SkUnPreMultiply.h"

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
    // PNG is stored unpremultiplied, and going from premul to unpremul to premul is lossy.  To
    // skirt this problem, we decode the PNG into an unpremul bitmap, convert our bitmap to unpremul
    // if needed, and compare those.  Each image goes once from premul to unpremul, never back.
    const SkString path = path_to_expected_image(fRoot, task);

    SkAutoTUnref<SkStreamRewindable> stream(SkStream::NewFromFile(path.c_str()));
    if (NULL == stream.get()) {
        SkDebugf("Could not read %s.\n", path.c_str());
        return false;
    }

    SkAutoTDelete<SkImageDecoder> decoder(SkImageDecoder::Factory(stream));
    if (NULL == decoder.get()) {
        SkDebugf("Could not find a decoder for %s.\n", path.c_str());
        return false;
    }

    SkImageInfo info;
    SkAssertResult(bitmap.asImageInfo(&info));

    SkBitmap expected;
    expected.setConfig(info);
    expected.allocPixels();

    // expected will be unpremultiplied.
    decoder->setRequireUnpremultipliedColors(true);
    if (!decoder->decode(stream, &expected, SkImageDecoder::kDecodePixels_Mode)) {
        SkDebugf("Could not decode %s.\n", path.c_str());
        return false;
    }

    // We always seem to decode to 8888.  This puts 565 back in 565.
    if (expected.config() != bitmap.config()) {
        SkBitmap converted;
        SkAssertResult(expected.copyTo(&converted, bitmap.config()));
        expected.swap(converted);
    }
    SkASSERT(expected.config() == bitmap.config());

    // Manually unpremultiply 8888 bitmaps to match expected.
    // Their pixels are shared, concurrently even, so we must copy them.
    if (info.fColorType == kPMColor_SkColorType) {
        SkBitmap unpremul;
        unpremul.setConfig(info);
        unpremul.allocPixels();

        // Unpremultiply without changing native byte order.
        SkAutoLockPixels lockSrc(bitmap), lockDst(unpremul);
        const SkPMColor* src = (SkPMColor*)bitmap.getPixels();
        uint32_t* dst = (uint32_t*)unpremul.getPixels();
        for (size_t i = 0; i < bitmap.getSize()/4; i++) {
            const U8CPU a = SkGetPackedA32(src[i]);
            const SkUnPreMultiply::Scale s = SkUnPreMultiply::GetScale(a);
            dst[i] = SkPackARGB32NoCheck(a,
                                         SkUnPreMultiply::ApplyScale(s, SkGetPackedR32(src[i])),
                                         SkUnPreMultiply::ApplyScale(s, SkGetPackedG32(src[i])),
                                         SkUnPreMultiply::ApplyScale(s, SkGetPackedB32(src[i])));
        }
        bitmap.swap(unpremul);
    }

    return BitmapsEqual(expected, bitmap);
}

}  // namespace DM
