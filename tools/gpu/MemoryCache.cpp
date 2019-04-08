/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPersistentCacheUtils.h"
#include "MemoryCache.h"
#include "SkBase64.h"
#include "SkJSONWriter.h"
#include "SkMD5.h"
#include "SkTHash.h"

// Change this to 1 to log cache hits/misses/stores using SkDebugf.
#define LOG_MEMORY_CACHE 0

static SkString data_to_str(const SkData& data) {
    size_t encodeLength = SkBase64::Encode(data.data(), data.size(), nullptr);
    SkString str;
    str.resize(encodeLength);
    SkBase64::Encode(data.data(), data.size(), str.writable_str());
    static constexpr size_t kMaxLength = 60;
    static constexpr char kTail[] = "...";
    static const size_t kTailLen = strlen(kTail);
    bool overlength = encodeLength > kMaxLength;
    if (overlength) {
        str = SkString(str.c_str(), kMaxLength - kTailLen);
        str.append(kTail);
    }
    return str;
}

namespace sk_gpu_test {

sk_sp<SkData> MemoryCache::load(const SkData& key) {
    auto result = fMap.find(key);
    if (result == fMap.end()) {
        if (LOG_MEMORY_CACHE) {
            SkDebugf("Load Key: %s\n\tNot Found.\n\n", data_to_str(key).c_str());
        }
        ++fCacheMissCnt;
        return nullptr;
    }
    if (LOG_MEMORY_CACHE) {
        SkDebugf("Load Key: %s\n\tFound Data: %s\n\n", data_to_str(key).c_str(),
                 data_to_str(*result->second.fData).c_str());
    }
    result->second.fHitCount++;
    return result->second.fData;
}

void MemoryCache::store(const SkData& key, const SkData& data) {
    if (LOG_MEMORY_CACHE) {
        SkDebugf("Store Key: %s\n\tData: %s\n\n", data_to_str(key).c_str(),
                 data_to_str(data).c_str());
    }
    fMap[Key(key)] = Value(data);
}

void MemoryCache::writeShadersToDisk(const char* path, GrBackendApi api) {
    SkTHashMap<SkString, std::pair<SkString, int>> shaders[kGrShaderTypeCount];
    for (auto it = fMap.begin(); it != fMap.end(); ++it) {
        auto data = it->second.fData;
        int hitCount = it->second.fHitCount;

        SkMD5 hash;
        hash.write(it->first.fKey->bytes(), it->first.fKey->size());
        SkMD5::Digest digest = hash.finish();
        SkString md5;
        for (int i = 0; i < 16; ++i) {
            md5.appendf("%02x", digest.data[i]);
        }

        SkReader32 reader(data->data(), data->size());
        SkSL::Program::Inputs inputsIgnored;
        SkSL::String glsl[kGrShaderTypeCount];
        GrPersistentCacheUtils::UnpackCachedGLSL(reader, &inputsIgnored, glsl);
        for (int shaderType = 0; shaderType < kGrShaderTypeCount; ++shaderType) {
            if (!glsl[shaderType].empty()) {
                SkString shader(glsl[shaderType].c_str());
                shaders[shaderType].set(md5, std::make_pair(shader, hitCount));
            }
        }
    }

    // Default extensions detected by the Mali Offline Compiler
    const char* extensions[kGrShaderTypeCount] = { "vert", "geom", "frag" };

    for (int shaderType = 0; shaderType < kGrShaderTypeCount; ++shaderType) {
        // For now, only dump fragment shaders
        if (shaderType != kFragment_GrShaderType) {
            continue;
        }

        const char* ext = extensions[shaderType];
        SkString jsonPath = SkStringPrintf("%s/%s.json", path, ext);
        SkFILEWStream jsonFile(jsonPath.c_str());
        SkJSONWriter writer(&jsonFile, SkJSONWriter::Mode::kPretty);
        writer.beginArray();

        auto dumpShader = [path, ext, &writer]
        (const SkString& key, std::pair<SkString, int>* shaderAndCount) {
            SkString filename = SkStringPrintf("%s/%s.%s", path, key.c_str(), ext);
            SkFILEWStream file(filename.c_str());
            file.write(shaderAndCount->first.c_str(), shaderAndCount->first.size());
            writer.beginArray(nullptr, false);
            writer.appendString(key.c_str());
            writer.appendS32(shaderAndCount->second);
            writer.endArray();
        };

        shaders[shaderType].foreach(dumpShader);
        writer.endArray();
    }
}

}  // namespace sk_gpu_test
