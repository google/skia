/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/UniformCache.h"

#include "src/core/SkOpts.h"
#include "src/core/SkUniformData.h"

namespace skgpu {

size_t UniformCache::Hash::operator()(SkUniformBlock* ub) const {
    if (!ub) {
        return 0;
    }

    return ub->hash();
}

bool UniformCache::Eq::operator()(SkUniformBlock* a, SkUniformBlock* b) const {
    if (!a || !b) {
        return !a && !b;
    }

    return *a == *b;
};

UniformCache::UniformCache() {
    // kInvalidUniformID is reserved
    static_assert(kInvalidUniformID == 0);
    fUniformBlock.push_back(nullptr);
    fUniformBlockIDs.insert({nullptr, 0});
}

#ifdef SK_DEBUG
void UniformCache::validate() const {
    for (size_t i = 0; i < fUniformBlock.size(); ++i) {
        auto kv = fUniformBlockIDs.find(fUniformBlock[i].get());
        SkASSERT(kv != fUniformBlockIDs.end());
        SkASSERT(kv->first == fUniformBlock[i].get());
        SkASSERT(SkTo<uint32_t>(i) == kv->second);
    }
}
#endif

uint32_t UniformCache::insert(std::unique_ptr<SkUniformBlock> block) {
    auto kv = fUniformBlockIDs.find(block.get());
    if (kv != fUniformBlockIDs.end()) {
        return kv->second;
    }

    uint32_t id = SkTo<uint32_t>(fUniformBlock.size());
    SkASSERT(block && id != kInvalidUniformID);

    fUniformBlockIDs.insert({block.get(), id});
    fUniformBlock.push_back(std::move(block));
    this->validate();
    return id;
}

SkUniformBlock* UniformCache::lookup(uint32_t uniqueID) {
    SkASSERT(uniqueID < fUniformBlock.size());
    return fUniformBlock[uniqueID].get();
}

} // namespace skgpu
