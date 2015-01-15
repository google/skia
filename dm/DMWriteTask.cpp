#include "DMWriteTask.h"

#include "DMJsonWriter.h"
#include "DMUtil.h"
#include "SkColorPriv.h"
#include "SkCommonFlags.h"
#include "SkData.h"
#include "SkImageEncoder.h"
#include "SkMD5.h"
#include "SkMallocPixelRef.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"

DEFINE_bool(nameByHash, false, "If true, write .../hash.png instead of .../mode/config/name.png");

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
        consumed += SkToInt(out->back().size() + 1);  // Add one for the _.
    }
    return consumed;
}

inline static SkString find_base_name(const Task& parent, SkTArray<SkString>* suffixList) {
    const int suffixes = parent.depth() + 1;
    const SkString& name = parent.name();
    const int totalSuffixLength = split_suffixes(suffixes, name.c_str(), suffixList);
    return SkString(name.c_str(), name.size() - totalSuffixLength);
}

WriteTask::WriteTask(const Task& parent, const char* sourceType, SkBitmap bitmap)
    : CpuTask(parent)
    , fBaseName(find_base_name(parent, &fSuffixes))
    , fSourceType(sourceType)
    , fBitmap(bitmap)
    , fData(NULL)
    , fExtension(".png") {
}

WriteTask::WriteTask(const Task& parent,
                     const char* sourceType,
                     SkStreamAsset *data,
                     const char* ext)
    : CpuTask(parent)
    , fBaseName(find_base_name(parent, &fSuffixes))
    , fSourceType(sourceType)
    , fData(data)
    , fExtension(ext) {
    SkASSERT(fData.get());
    SkASSERT(fData->unique());
}

void WriteTask::makeDirOrFail(SkString dir) {
    // This can be a little racy, so if it fails check to see if someone else succeeded.
    if (!sk_mkdir(dir.c_str()) && !sk_isdir(dir.c_str())) {
        this->fail("Can't make directory.");
    }
}

static SkString get_md5_string(SkMD5* hasher) {
    SkMD5::Digest digest;
    hasher->finish(digest);

    SkString md5;
    for (int i = 0; i < 16; i++) {
        md5.appendf("%02x", digest.data[i]);
    }
    return md5;
}

static SkString get_md5(const void* ptr, size_t len) {
    SkMD5 hasher;
    hasher.write(ptr, len);
    return get_md5_string(&hasher);
}

static bool write_asset(SkStreamAsset* input, SkWStream* output) {
    return input->rewind() && output->writeStream(input, input->getLength());
}

static SkString get_md5(SkStreamAsset* stream) {
    SkMD5 hasher;
    write_asset(stream, &hasher);
    return get_md5_string(&hasher);
}

static bool encode_png(const SkBitmap& src, SkFILEWStream* file) {
    SkBitmap bm;
    // We can't encode A8 bitmaps as PNGs.  Convert them to 8888 first.
    if (src.info().colorType() == kAlpha_8_SkColorType) {
        if (!src.copyTo(&bm, kN32_SkColorType)) {
            return false;
        }
    } else {
        bm = src;
    }
    return SkImageEncoder::EncodeStream(file, bm, SkImageEncoder::kPNG_Type, 100);
}

void WriteTask::draw() {
    SkString md5;
    {
        SkAutoLockPixels lock(fBitmap);
        md5 = fData ? get_md5(fData)
                    : get_md5(fBitmap.getPixels(), fBitmap.getSize());
    }

    SkASSERT(fSuffixes.count() > 0);
    SkString config = fSuffixes.back();
    SkString mode("direct");
    if (fSuffixes.count() > 1) {
        mode = fSuffixes.fromBack(1);
    }

    {
        const JsonWriter::BitmapResult entry = { fBaseName,
                                                 config,
                                                 mode,
                                                 fSourceType,
                                                 md5 };
        JsonWriter::AddBitmapResult(entry);
    }

    SkString dir(FLAGS_writePath[0]);
#if defined(SK_BUILD_FOR_IOS)
    if (dir.equals("@")) {
        dir.set(FLAGS_resourcePath[0]);
    }
#endif
    this->makeDirOrFail(dir);

    SkString path;
    if (FLAGS_nameByHash) {
        // Flat directory of hash-named files.
        path = SkOSPath::Join(dir.c_str(), md5.c_str());
        path.append(fExtension);
        // We're content-addressed, so it's possible two threads race to write
        // this file.  We let the first one win.  This also means we won't
        // overwrite identical files from previous runs.
        if (sk_exists(path.c_str())) {
            return;
        }
    } else {
        // Nested by mode, config, etc.
        for (int i = 0; i < fSuffixes.count(); i++) {
            dir = SkOSPath::Join(dir.c_str(), fSuffixes[i].c_str());
            this->makeDirOrFail(dir);
        }
        path = SkOSPath::Join(dir.c_str(), fBaseName.c_str());
        path.append(fExtension);
        // The path is unique, so two threads can't both write to the same file.
        // If already present we overwrite here, since the content may have changed.
    }

    SkFILEWStream file(path.c_str());
    if (!file.isValid()) {
        return this->fail("Can't open file.");
    }

    bool ok = fData ? write_asset(fData, &file)
                    : encode_png(fBitmap, &file);
    if (!ok) {
        return this->fail("Can't write to file.");
    }
}

SkString WriteTask::name() const {
    SkString name("writing ");
    for (int i = 0; i < fSuffixes.count(); i++) {
        name.appendf("%s/", fSuffixes[i].c_str());
    }
    name.append(fBaseName.c_str());
    return name;
}

bool WriteTask::shouldSkip() const {
    return FLAGS_writePath.isEmpty();
}

}  // namespace DM
