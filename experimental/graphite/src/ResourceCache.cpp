/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ResourceCache.h"

#include "experimental/graphite/src/Resource.h"
#include "include/private/SingleOwner.h"

namespace skgpu {

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

ResourceCache::ResourceCache(SingleOwner* singleOwner) : fSingleOwner(singleOwner) {
// TODO: Maybe when things start using ResourceCache, then like Ganesh the compiler won't complain
// about not using fSingleOwner in Release builds and we can delete this.
#ifndef SK_DEBUG
    (void)fSingleOwner;
#endif
}

void ResourceCache::insertResource(Resource* resource) {
    ASSERT_SINGLE_OWNER
    SkASSERT(resource);
}

} // namespace skgpu
