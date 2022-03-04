/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/PipelineDataCache.h"

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"

namespace skgpu {

size_t PipelineDataCache::Hash::operator()(SkPipelineData* pd) const {
    if (!pd) {
        return 0;
    }

    return pd->hash();
}

bool PipelineDataCache::Eq::operator()(SkPipelineData* a, SkPipelineData* b) const {
    if (!a || !b) {
        return !a && !b;
    }

    return *a == *b;
};

PipelineDataCache::PipelineDataCache() {
    // kInvalidUniformID is reserved
    static_assert(kInvalidUniformID == 0);
    fUniformBlock.push_back(nullptr);
    fUniformBlockIDs.insert({nullptr, 0});
}

#ifdef SK_DEBUG
void PipelineDataCache::validate() const {
    for (size_t i = 0; i < fUniformBlock.size(); ++i) {
        auto kv = fUniformBlockIDs.find(fUniformBlock[i].get());
        SkASSERT(kv != fUniformBlockIDs.end());
        SkASSERT(kv->first == fUniformBlock[i].get());
        SkASSERT(SkTo<uint32_t>(i) == kv->second);
    }
}
#endif

uint32_t PipelineDataCache::insert(std::unique_ptr<SkPipelineData> pipelineData) {
    auto kv = fUniformBlockIDs.find(pipelineData.get());
    if (kv != fUniformBlockIDs.end()) {
        return kv->second;
    }

    uint32_t id = SkTo<uint32_t>(fUniformBlock.size());
    SkASSERT(pipelineData && id != kInvalidUniformID);

    fUniformBlockIDs.insert({pipelineData.get(), id});
    fUniformBlock.push_back(std::move(pipelineData));
    this->validate();
    return id;
}

SkPipelineData* PipelineDataCache::lookup(uint32_t uniqueID) {
    SkASSERT(uniqueID < fUniformBlock.size());
    return fUniformBlock[uniqueID].get();
}

} // namespace skgpu
