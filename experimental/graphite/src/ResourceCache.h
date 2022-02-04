/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ResourceCache_DEFINED
#define skgpu_ResourceCache_DEFINED

#include "experimental/graphite/src/Resource.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "src/core/SkTDPQueue.h"

namespace skgpu {

class SingleOwner;

class ResourceCache {
public:
    ResourceCache(SingleOwner*);

    ResourceCache(const ResourceCache&) = delete;
    ResourceCache(ResourceCache&&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;
    ResourceCache& operator=(ResourceCache&&) = delete;

    void insertResource(Resource*);

private:
    static bool CompareTimestamp(Resource* const& a, Resource* const& b) {
        return a->timestamp() < b->timestamp();
    }

    static int* AccessResourceIndex(Resource* const& res) {
        return res->accessCacheIndex();
    }

    using PurgeableQueue = SkTDPQueue<Resource*, CompareTimestamp, AccessResourceIndex>;
    using ResourceArray = SkTDArray<Resource*>;

    PurgeableQueue fPurgeableQueue;
    ResourceArray fNonpurgeableResources;

    SingleOwner* fSingleOwner = nullptr;
};

} // namespace skgpu

#endif // skgpu_ResourceCache_DEFINED
