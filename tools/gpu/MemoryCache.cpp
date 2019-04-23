/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkBase64.h"
#include "src/core/SkMD5.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "tools/gpu/MemoryCache.h"

#if defined(SK_VULKAN)
#include "src/gpu/vk/GrVkGpu.h"
#endif

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
