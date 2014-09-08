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
    SkString md5;  // In ASCII, so 32 bytes long.
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

static SkString finish_hash(SkMD5* hasher) {
    SkMD5::Digest digest;
    hasher->finish(digest);

    SkString out;
    for (int i = 0; i < 16; i++) {
        out.appendf("%02x", digest.data[i]);
    }
    return out;
}

static SkString hash(const SkBitmap& src) {
    SkMD5 hasher;
    {
        SkAutoLockPixels lock(src);
        hasher.write(src.getPixels(), src.getSize());
    }
    return finish_hash(&hasher);
}

static SkString hash(SkStreamAsset* src) {
    SkMD5 hasher;
    hasher.write(src->getMemoryBase(), src->getLength());
    return finish_hash(&hasher);
}

void WriteTask::draw() {
    JsonData entry = {
        fFullName,
        fData ? hash(fData) : hash(fBitmap),
    };

    {
        SkAutoMutexAcquire lock(&gJsonDataLock);
        gJsonData.push_back(entry);
    }

    SkString dir(FLAGS_writePath[0]);
#if SK_BUILD_FOR_IOS
    if (dir.equals("@")) {
        dir.set(FLAGS_resourcePath[0]);
    }
#endif
    this->makeDirOrFail(dir);

    SkString path;
    if (FLAGS_nameByHash) {
        // Flat directory of hash-named files.
        path = SkOSPath::Join(dir.c_str(), entry.md5.c_str());
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

    const char* expected = fJson[name.c_str()].asCString();
    SkString actual = hash(bitmap);
    return actual.equals(expected);
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
            root[gJsonData[i].name.c_str()] = gJsonData[i].md5.c_str();
        }
    }

    SkString path = SkOSPath::Join(FLAGS_writePath[0], "dm.json");
    SkFILEWStream stream(path.c_str());
    stream.writeText(Json::StyledWriter().write(root).c_str());
    stream.flush();
}

}  // namespace DM
