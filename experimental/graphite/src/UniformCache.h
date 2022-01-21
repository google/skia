/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformCache_DEFINED
#define skgpu_UniformCache_DEFINED

#include "include/core/SkRefCnt.h"

#include <unordered_map>
#include <vector>

class SkUniformData;

namespace skgpu {


class UniformCache {
public:
    static constexpr uint32_t kInvalidUniformID = 0;

    UniformCache();

    // TODO: Revisit the UniformCache::insert and UniformData::Make APIs:
    // 1. UniformData::Make requires knowing the data size up front, which involves two invocations
    //    of the UniformManager. Ideally, we could align uniforms on the fly into a dynamic buffer.
    // 2. UniformData stores the offsets for each uniform, but these aren't needed after we've
    //    filled out the buffer. If we remember layout offsets, it should be stored per Combination
    //    or RenderStep that defines the uniform set.
    // 3. UniformCache's ids are only fundamentally limited by the number of draws that can be
    //    recorded into a DrawPass, which means a very large recording with multiple passes could
    //    exceed uint32_t across all the passes.
    // 4. The check to know if a UniformData is present in the cache is practically the same for
    //    checking if the data needs to be uploaded to the GPU, so UniformCache could remember the
    //    associated BufferBindInfos as well.
    // 5. Because UniformCache only cares about the content byte hash/equality, and can memcpy to
    //    the GPU buffer, the cached data contents could all go into a shared byte array, instead of
    //    needing to extend SkRefCnt.
    // 6. insert() as a name can imply that the value is always added, so we may want a better one.
    //    It can be a little less generic if UniformCache returns id and bind buffer info. On the
    //    other hand unordered_map::insert has the same semantics as this insert, so maybe it's fine

    // Add the block of uniform data to the cache and return a unique ID that corresponds to its
    // contents. If an identical block of data is already in the cache, that unique ID is returned.
    uint32_t insert(sk_sp<SkUniformData>);

    sk_sp<SkUniformData> lookup(uint32_t uniqueID);

    // The number of unique uniformdata objects in the cache
    size_t count() const {
        SkASSERT(fUniformData.size() == fUniformDataIDs.size() && fUniformData.size() > 0);
        return fUniformData.size() - 1;
    }

private:
    struct Hash {
        // This hash operator de-references and hashes the data contents
        size_t operator()(SkUniformData*) const;
    };
    struct Eq {
        // This equality operator de-references and compares the actual data contents
        bool operator()(SkUniformData*, SkUniformData*) const;
    };

    // The UniformData's unique ID is only unique w/in a Recorder _not_ globally
    std::unordered_map<SkUniformData*, uint32_t, Hash, Eq> fUniformDataIDs;
    std::vector<sk_sp<SkUniformData>> fUniformData;

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

} // namespace skgpu

#endif // skgpu_UniformCache_DEFINED
