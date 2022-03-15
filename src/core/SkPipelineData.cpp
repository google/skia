/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"
#include "src/core/SkPipelineData.h"

SkPipelineData::SkPipelineData(sk_sp<SkUniformData> initial) {
    SkASSERT(initial && initial->count());
    fUniformData.push_back(std::move(initial));
}

void SkPipelineData::add(sk_sp<SkUniformData> uniforms) {
    SkASSERT(uniforms && uniforms->count());
    fUniformData.push_back(std::move(uniforms));
}

#ifdef SK_GRAPHITE_ENABLED
void SkPipelineData::addImage(const SkSamplingOptions& sampling,
                              const SkTileMode tileModes[2],
                              sk_sp<skgpu::TextureProxy> proxy) {
    fProxies.push_back({std::move(proxy), sampling, {tileModes[0], tileModes[1]}});
}
#endif

size_t SkPipelineData::totalUniformSize() const {
    size_t total = 0;

    // TODO: It seems like we need to worry about alignment between the separate sets of uniforms
    for (auto& u : fUniformData) {
        total += u->dataSize();
    }

    return total;
}

int SkPipelineData::numUniforms() const {
    int total = 0;

    for (auto& u : fUniformData) {
        total += u->count();
    }

    return total;
}

bool SkPipelineData::operator==(const SkPipelineData& other) const {
    if (fUniformData.size() != other.fUniformData.size()) {
        return false;
    }

    for (size_t i = 0; i < fUniformData.size(); ++i) {
        if (*fUniformData[i] != *other.fUniformData[i]) {
            return false;
        }
    }

    return true;
}

size_t SkPipelineData::hash() const {
    int32_t hash = 0;

    for (auto& u : fUniformData) {
        hash = SkOpts::hash_fn(u->data(), u->dataSize(), hash);
    }

    return hash;
}
