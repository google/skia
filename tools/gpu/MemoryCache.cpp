/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkBase64.h"
#include "src/core/SkMD5.h"
#include "src/core/SkReadBuffer.h"
#include "src/gpu/ganesh/GrPersistentCacheUtils.h"
#include "tools/gpu/MemoryCache.h"

#if defined(SK_VULKAN)
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#endif

// Change this to 1 to log cache hits/misses/stores using SkDebugf.
#define LOG_MEMORY_CACHE 0

static SkString data_to_str(const SkData& data) {
    size_t encodeLength = SkBase64::EncodedSize(data.size());
    SkString str;
    str.resize(encodeLength);
    SkBase64::Encode(data.data(), data.size(), str.data());
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

void MemoryCache::store(const SkData& key, const SkData& data, const SkString& description) {
    if (LOG_MEMORY_CACHE) {
        SkDebugf("Store Key: %s\n\tData: %s\n\n", data_to_str(key).c_str(),
                 data_to_str(data).c_str());
    }
    ++fCacheStoreCnt;
    fMap[Key(key)] = Value(data, description);
}

void MemoryCache::writeShadersToDisk(const char* path, GrBackendApi api) {
    if (GrBackendApi::kOpenGL != api && GrBackendApi::kVulkan != api) {
        return;
    }

    for (auto it = fMap.begin(); it != fMap.end(); ++it) {
        SkMD5 hash;
        size_t bytesToHash = it->first.fKey->size();
#if defined(SK_VULKAN)
        if (GrBackendApi::kVulkan == api) {
            // Vulkan stores two kinds of data in the cache (shaders and pipelines). The last four
            // bytes of the key identify which one we have. We only want to extract shaders.
            // Additionally, we don't want to hash the tag bytes, so we get the same keys as GL,
            // which is good for cross-checking code generation and performance.
            GrVkGpu::PersistentCacheKeyType vkKeyType;
            SkASSERT(bytesToHash >= sizeof(vkKeyType));
            bytesToHash -= sizeof(vkKeyType);
            memcpy(&vkKeyType, it->first.fKey->bytes() + bytesToHash, sizeof(vkKeyType));
            if (vkKeyType != GrVkGpu::kShader_PersistentCacheKeyType) {
                continue;
            }
        }
#endif
        hash.write(it->first.fKey->bytes(), bytesToHash);
        SkMD5::Digest digest = hash.finish();
        SkString md5 = digest.toLowercaseHexString();

        SkSL::Program::Interface interfacesIgnored[kGrShaderTypeCount];
        std::string shaders[kGrShaderTypeCount];
        const SkData* data = it->second.fData.get();
        const SkString& description = it->second.fDescription;
        SkReadBuffer reader(data->data(), data->size());
        GrPersistentCacheUtils::GetType(&reader); // Shader type tag
        GrPersistentCacheUtils::UnpackCachedShaders(&reader, shaders,
                                                    interfacesIgnored, kGrShaderTypeCount);

        // Even with the SPIR-V switches, it seems like we must use .spv, or malisc tries to
        // run glslang on the input.
        {
            const char* ext = GrBackendApi::kOpenGL == api ? "frag" : "frag.spv";
            SkString filename = SkStringPrintf("%s/%s.%s", path, md5.c_str(), ext);
            SkFILEWStream file(filename.c_str());
            file.write(shaders[kFragment_GrShaderType].c_str(),
                       shaders[kFragment_GrShaderType].size());
        }
        {
            const char* ext = GrBackendApi::kOpenGL == api ? "vert" : "vert.spv";
            SkString filename = SkStringPrintf("%s/%s.%s", path, md5.c_str(), ext);
            SkFILEWStream file(filename.c_str());
            file.write(shaders[kVertex_GrShaderType].c_str(),
                       shaders[kVertex_GrShaderType].size());
        }

        if (!description.isEmpty()) {
            const char* ext = "key";
            SkString filename = SkStringPrintf("%s/%s.%s", path, md5.c_str(), ext);
            SkFILEWStream file(filename.c_str());
            file.write(description.c_str(), description.size());
        }
    }
}

}  // namespace sk_gpu_test
