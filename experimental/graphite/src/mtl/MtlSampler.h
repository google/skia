/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlSampler_DEFINED
#define skgpu_MtlSampler_DEFINED

#include "experimental/graphite/src/Sampler.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

struct SkSamplingOptions;

namespace skgpu::mtl {

class Gpu;

class Sampler : public skgpu::Sampler {
public:
    static sk_sp<Sampler> Make(const Gpu*,
                               const SkSamplingOptions& samplingOptions,
                               SkTileMode xTileMode,
                               SkTileMode yTileMode);

    ~Sampler() override {}

    id<MTLSamplerState> mtlSamplerState() const { return fSamplerState.get(); }

private:
    Sampler(const Gpu* gpu,
            sk_cfp<id<MTLSamplerState>>);

    void onFreeGpuData() override;

    sk_cfp<id<MTLSamplerState>> fSamplerState;
};

} // namepsace skgpu::mtl

#endif // skgpu_MtlSampler_DEFINED
