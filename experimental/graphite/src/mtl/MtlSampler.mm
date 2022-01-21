/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/mtl/MtlSampler.h"

#include "experimental/graphite/src/mtl/MtlGpu.h"

namespace skgpu::mtl {

sk_cfp<id<MTLSamplerState>> Sampler::MakeMtlSamplerState(const Gpu* gpu) {
    sk_cfp<MTLSamplerDescriptor*> desc([[MTLSamplerDescriptor alloc] init]);

    sk_cfp<id<MTLSamplerState>> sampler([gpu->device() newSamplerStateWithDescriptor:desc.get()]);
    // TODO: fill in fields
#ifdef SK_ENABLE_MTL_DEBUG_INFO
    // TODO: add label
#endif

    return sampler;
}

Sampler::Sampler(const Gpu* gpu,
                 sk_cfp<id<MTLSamplerState>> samplerState)
        : skgpu::Sampler(gpu)
        , fSamplerState(std::move(samplerState)) {}

sk_sp<Sampler> Sampler::Make(const Gpu* gpu) {
    sk_cfp<id<MTLSamplerState>> samplerState = MakeMtlSamplerState(gpu);
    if (!samplerState) {
        return nullptr;
    }
    return sk_sp<Sampler>(new Sampler(gpu,
                                      std::move(samplerState)));
}

void Sampler::onFreeGpuData() {
    fSamplerState.reset();
}

} // namespace skgpu::mtl

