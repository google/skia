/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_UniformCache_DEFINED
#define skgpu_UniformCache_DEFINED

#include <unordered_set>
#include <vector>
#include "include/core/SkRefCnt.h"

namespace skgpu {

class UniformData;

class UniformCache {
public:
    UniformCache();

    sk_sp<UniformData> findOrCreate(sk_sp<UniformData>);

    sk_sp<UniformData> lookup(uint32_t uniqueID);

    // The number of unique uniformdata objects in the cache
    size_t count() const {
        SkASSERT(fUniformDataHash.size()+1 == fUniformDataVector.size());
        return fUniformDataHash.size();
    }

private:
    struct Hash {
        size_t operator()(sk_sp<UniformData>) const;
    };
    struct Eq {
        // This equality operator doesn't compare the UniformData's ids
        bool operator()(sk_sp<UniformData>, sk_sp<UniformData>) const;
    };

    std::unordered_set<sk_sp<UniformData>, Hash, Eq> fUniformDataHash;
    std::vector<sk_sp<UniformData>> fUniformDataVector;
    // The UniformData's unique ID is only unique w/in a Recorder _not_ globally
    uint32_t fNextUniqueID = 1;
};

} // namespace skgpu

#endif // skgpu_UniformCache_DEFINED
