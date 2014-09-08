#include "DMWriteTask.h"

#include "DMUtil.h"
#include "SkColorPriv.h"
#include "SkCommonFlags.h"
#include "SkImageEncoder.h"
#include "SkMD5.h"
#include "SkMallocPixelRef.h"
#include "SkOSFile.h"
#include "SkStream.h"
#include "SkString.h"

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

inline static SkString find_base_name(const Task& parent, SkTArray<SkString>* suffixList) {
    const int suffixes = parent.depth() + 1;
    const SkString& name = parent.name();
    const int totalSuffixLength = split_suffixes(suffixes, name.c_str(), suffixList);
    return SkString(name.c_str(), name.size() - totalSuffixLength);
}

struct JsonData {
    SkString name;
    SkMD5::Digest md5;
};
SkTArray<JsonData> gJsonData;
SK_DECLARE_STATIC_MUTEX(gJsonDataLock);

WriteTask::WriteTask(const Task& parent, SkBitmap bitmap)
    : CpuTask(parent)
    , fFullName(parent.name())
    , fBaseName(find_base_name(parent, &fSuffixes))
    , fBitmap(bitmap)
    , fData(NULL)
    , fExtension(".png") {
}

WriteTask::WriteTask(const Task& parent, SkStreamAsset *data, const char* ext)
    : CpuTask(parent)
    , fFullName(parent.name())
    , fBaseName(find_base_name(parent, &fSuffixes))
    , fData(data)
    , fExtension(ext) {
    SkASSERT(fData.get());
    SkASSERT(fData->unique());
}

void WriteTask::makeDirOrFail(SkString dir) {
    if (!sk_mkdir(dir.c_str())) {
        this->fail();
    }
}

static bool save_bitmap_to_file(SkBitmap bitmap, const char* path) {
    SkFILEWStream stream(path);
    if (!stream.isValid() ||
        !SkImageEncoder::EncodeStream(&stream, bitmap, SkImageEncoder::kPNG_Type, 100)) {
        SkDebugf("Can't write a PNG to %s.\n", path);
        return false;
    }
    return true;
}

// Does not take ownership of data.
static bool save_data_to_file(SkStreamAsset* data, const char* path) {
    data->rewind();
    SkFILEWStream stream(path);
    if (!stream.isValid() || !stream.writeStream(data, data->getLength())) {
        SkDebugf("Can't write data to %s.\n", path);
        return false;
    }
    return true;
}

void WriteTask::draw() {
    SkString dir(FLAGS_writePath[0]);
#if SK_BUILD_FOR_IOS
    if (dir.equals("@")) {
        dir.set(FLAGS_resourcePath[0]);
    }
#endif
    this->makeDirOrFail(dir);
    for (int i = 0; i < fSuffixes.count(); i++) {
        dir = SkOSPath::Join(dir.c_str(), fSuffixes[i].c_str());
        this->makeDirOrFail(dir);
    }

    // FIXME: MD5 is really slow.  Let's use a different hash.
    SkMD5 hasher;
    if (fData.get()) {
        hasher.write(fData->getMemoryBase(), fData->getLength());
    } else {
        SkAutoLockPixels lock(fBitmap);
        hasher.write(fBitmap.getPixels(), fBitmap.getSize());
    }

    JsonData entry;
    entry.name = fFullName;
    hasher.finish(entry.md5);

    {
        SkAutoMutexAcquire lock(&gJsonDataLock);
        gJsonData.push_back(entry);
    }

    SkString path = SkOSPath::Join(dir.c_str(), fBaseName.c_str());
    path.append(fExtension);

    const bool ok = fData.get() ? save_data_to_file(fData.get(), path.c_str())
                                : save_bitmap_to_file(fBitmap, path.c_str());
    if (!ok) {
        this->fail();
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

WriteTask::Expectations* WriteTask::Expectations::Create(const char* path) {
    if (!FLAGS_writePath.isEmpty() && 0 == strcmp(FLAGS_writePath[0], path)) {
        SkDebugf("We seem to be reading and writing %s concurrently.  This won't work.\n", path);
        return NULL;
    }

    SkString jsonPath;
    if (sk_isdir(path)) {
        jsonPath = SkOSPath::Join(path, "dm.json");
    } else {
        jsonPath.set(path);
    }

    SkAutoDataUnref json(SkData::NewFromFileName(jsonPath.c_str()));
    if (NULL == json.get()) {
        SkDebugf("Can't read %s!\n", jsonPath.c_str());
        return NULL;
    }

    SkAutoTDelete<Expectations> expectations(SkNEW(Expectations));
    Json::Reader reader;
    const char* begin = (const char*)json->bytes();
    const char* end   = begin + json->size();
    if (!reader.parse(begin, end, expectations->fJson)) {
        SkDebugf("Can't read %s as JSON!\n", jsonPath.c_str());
        return NULL;
    }
    return expectations.detach();
}

bool WriteTask::Expectations::check(const Task& task, SkBitmap bitmap) const {
    const SkString name = task.name();
    if (fJson[name.c_str()].isNull()) {
        return true;  // No expectations.
    }

    const char* md5Ascii = fJson[name.c_str()].asCString();
    uint8_t md5[16];

    for (int j = 0; j < 16; j++) {
        sscanf(md5Ascii + (j*2), "%02hhx", md5 + j);
    }

    SkMD5 hasher;
    {
        SkAutoLockPixels lock(bitmap);
        hasher.write(bitmap.getPixels(), bitmap.getSize());
    }
    SkMD5::Digest digest;
    hasher.finish(digest);

    return 0 == memcmp(md5, digest.data, 16);
}

void WriteTask::DumpJson() {
    if (FLAGS_writePath.isEmpty()) {
        return;
    }

    // FIXME: This JSON format is a complete MVP strawman.
    Json::Value root;
    {
        SkAutoMutexAcquire lock(&gJsonDataLock);
        for (int i = 0; i < gJsonData.count(); i++) {
            char md5Ascii[32];
            for (int j = 0; j < 16; j++) {
                sprintf(md5Ascii + (j*2), "%02x", gJsonData[i].md5.data[j]);
            }
            root[gJsonData[i].name.c_str()] = md5Ascii;
        }
    }

    SkString path = SkOSPath::Join(FLAGS_writePath[0], "dm.json");
    SkFILEWStream stream(path.c_str());
    stream.writeText(Json::StyledWriter().write(root).c_str());
    stream.flush();
}

}  // namespace DM
