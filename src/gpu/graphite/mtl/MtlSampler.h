/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlSampler_DEFINED
#define skgpu_graphite_MtlSampler_DEFINED

#include "src/gpu/graphite/Sampler.h"

#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"
#include "include/ports/SkCFObject.h"

#import <Metal/Metal.h>

struct SkSamplingOptions;

namespace skgpu::graphite {

class MtlSharedContext;

class MtlSampler : public Sampler {
public:
    static sk_sp<MtlSampler> Make(const MtlSharedContext*,
                                  const SkSamplingOptions& samplingOptions,
                                  SkTileMode xTileMode,
                                  SkTileMode yTileMode);

    ~MtlSampler() override {}

    id<MTLSamplerState> mtlSamplerState() const { return fSamplerState.get(); }

private:
    MtlSampler(const MtlSharedContext* sharedContext,
               sk_cfp<id<MTLSamplerState>>);

    void freeGpuData() override;

    sk_cfp<id<MTLSamplerState>> fSamplerState;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_MtlSampler_DEFINED
