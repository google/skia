/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformCache.h"

#include "experimental/graphite/src/ContextUtils.h"
#include "src/core/SkOpts.h"

namespace skgpu {

size_t UniformCache::Hash::operator()(sk_sp<UniformData> ud) const {
    return SkOpts::hash_fn(ud->data(), ud->dataSize(), 0);
}

bool UniformCache::Eq::operator()(sk_sp<UniformData> a, sk_sp<UniformData> b) const {
    if (a->count() != b->count() ||
        a->uniforms() != b->uniforms() ||
        a->dataSize() != b->dataSize()) {
        return false;
    }

    return !memcmp(a->data(), b->data(), a->dataSize()) &&
           !memcmp(a->offsets(), b->offsets(), a->count()*sizeof(uint32_t));
};

UniformCache::UniformCache() {
    // kInvalidUniformID (aka 0) is reserved
    fUniformDataVector.push_back(nullptr);
}

sk_sp<UniformData> UniformCache::findOrCreate(sk_sp<UniformData> ud) {

    auto iter = fUniformDataHash.find(ud);
    if (iter != fUniformDataHash.end()) {
        SkASSERT((*iter)->id() != UniformData::kInvalidUniformID);
        return *iter;
    }

    ud->setID(fNextUniqueID++);
    fUniformDataHash.insert(ud);
    fUniformDataVector.push_back(ud);
    SkASSERT(fUniformDataVector[ud->id()] == ud);
    return ud;
}

sk_sp<UniformData> UniformCache::lookup(uint32_t uniqueID) {
    SkASSERT(uniqueID < fUniformDataVector.size());
    return fUniformDataVector[uniqueID];
}

} // namespace skgpu
