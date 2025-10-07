/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/GraphiteMemoryPipelineStorage.h"

// Define this to log data loads and stores using SkDebugf.
// #define LOG_MEMORY_CACHE

#if defined(LOG_MEMORY_CACHE)
#include "include/core/SkString.h"
#include "src/base/SkBase64.h"

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
#endif

namespace sk_gpu_test {

sk_sp<SkData> GraphiteMemoryPipelineStorage::load() {
    if (!fData) {
#if defined(LOG_MEMORY_CACHE)
        SkDebugf("No data to load\n");
#endif
        return nullptr;
    }

#if defined(LOG_MEMORY_CACHE)
    SkDebugf("Loading data: %zu %s\n", fData->size(), data_to_str(*fData).c_str());
#endif

    ++fLoadCount;
    return fData;
}

void GraphiteMemoryPipelineStorage::store(const SkData& data) {
#if defined(LOG_MEMORY_CACHE)
    SkDebugf("Storing data: %zu %s\n", data.size(), data_to_str(data).c_str());
#endif

    ++fStoreCount;
    fData = SkData::MakeWithCopy(data.data(), data.size());
}

}  // namespace sk_gpu_test
