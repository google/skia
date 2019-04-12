/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPersistentCacheUtils.h"
#include "MemoryCache.h"
#include "SkBase64.h"
#include "SkMD5.h"

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
    if (GrBackendApi::kOpenGL != api && GrBackendApi::kVulkan != api) {
        return;
    }

    for (auto it = fMap.begin(); it != fMap.end(); ++it) {
        SkMD5 hash;
        size_t bytesToHash = it->first.fKey->size();
        if (GrBackendApi::kVulkan == api) {
            // Hack: GrVkPipelineStateBuilder::Desc appends an integer constant to the key (to
            // differentiate shader vs. pipeline). If we omit that, we often get the same keys
            // on Vulkan and GL, which is good for cross-checking code generation and performance.
            SkASSERT(bytesToHash >= 4);
            bytesToHash -= 4;
        }
        hash.write(it->first.fKey->bytes(), bytesToHash);
        SkMD5::Digest digest = hash.finish();
        SkString md5;
        for (int i = 0; i < 16; ++i) {
            md5.appendf("%02x", digest.data[i]);
        }

        SkSL::Program::Inputs inputsIgnored[kGrShaderTypeCount];
        SkSL::String shaders[kGrShaderTypeCount];
        const SkData* data = it->second.fData.get();
        const char* ext;
        if (GrBackendApi::kOpenGL == api) {
            ext = "frag";
            GrPersistentCacheUtils::UnpackCachedGLSL(data, inputsIgnored, shaders);
        } else if (GrBackendApi::kVulkan == api) {
            // Even with the SPIR-V switches, it seems like we must use .spv, or malisc tries to
            // run glslang on the input.
            ext = "spv";
            GrPersistentCacheUtils::UnpackCachedSPIRV(data, shaders, inputsIgnored);
        }

        SkString filename = SkStringPrintf("%s/%s.%s", path, md5.c_str(), ext);
        SkFILEWStream file(filename.c_str());
        file.write(shaders[kFragment_GrShaderType].c_str(), shaders[kFragment_GrShaderType].size());
    }
}

}  // namespace sk_gpu_test
