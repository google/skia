/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MemoryCache.h"
#include "SkBase64.h"

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
                 data_to_str(*result->second).c_str());
    }
    return result->second;
}

void MemoryCache::store(const SkData& key, const SkData& data) {
    if (LOG_MEMORY_CACHE) {
        SkDebugf("Store Key: %s\n\tData: %s\n\n", data_to_str(key).c_str(),
                 data_to_str(data).c_str());
    }
    fMap[Key(key)] = SkData::MakeWithCopy(data.data(), data.size());
}

}  // namespace sk_gpu_test
