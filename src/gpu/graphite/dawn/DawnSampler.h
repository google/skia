/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnSampler_DEFINED
#define skgpu_graphite_DawnSampler_DEFINED

#include "src/gpu/graphite/Sampler.h"

#include "include/core/SkTileMode.h"
#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

struct SkSamplingOptions;

namespace skgpu::graphite {

class DawnSharedContext;

class DawnSampler : public Sampler {
public:
    static sk_sp<DawnSampler> Make(const DawnSharedContext*,
                                   const SkSamplingOptions& samplingOptions,
                                   SkTileMode xTileMode,
                                   SkTileMode yTileMode);
    ~DawnSampler() override {}

    const wgpu::Sampler& dawnSampler() const { return fSampler; }

private:
    DawnSampler(const DawnSharedContext* sharedContext,
                wgpu::Sampler sampler);

    void freeGpuData() override;

    wgpu::Sampler fSampler;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_DawnSampler_DEFINED
