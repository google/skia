/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/ProgramCache.h"

namespace {

static uint32_t next_id() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == skgpu::ProgramCache::kInvalidProgramID);
    return id;
}

} // anonymous namespace

namespace skgpu {

size_t ProgramCache::Hash::operator()(Combination c) const {
    return static_cast<int>(c.fShaderType) +
           static_cast<int>(c.fTileMode) +
           static_cast<int>(c.fBlendMode);
}

ProgramCache::ProgramInfo::ProgramInfo(Combination c)
    : fID(next_id())
    , fCombination(c) {
}

ProgramCache::ProgramInfo::~ProgramInfo() {}

sk_sp<ProgramCache::ProgramInfo> ProgramCache::findOrCreateProgram(Combination c) {
    auto iter = fPrograms.find(c);
    if (iter != fPrograms.end()) {
        SkASSERT(iter->second->id() != kInvalidProgramID);
        return iter->second;
    }

    sk_sp<ProgramInfo> pi(new ProgramInfo(c));
    fPrograms.insert(std::make_pair(c, pi));
    return pi;
}

} // namespace skgpu
