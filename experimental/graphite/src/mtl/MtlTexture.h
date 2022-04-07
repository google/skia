/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTexture_DEFINED
#define skgpu_graphite_MtlTexture_DEFINED

#include "experimental/graphite/src/Texture.h"
#include "include/core/SkRefCnt.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {
class MtlGpu;

class MtlTexture : public Texture {
public:
    static sk_cfp<id<MTLTexture>> MakeMtlTexture(const MtlGpu*,
                                                 SkISize dimensions,
                                                 const TextureInfo&);

    static sk_sp<Texture> Make(const MtlGpu*,
                               SkISize dimensions,
                               const TextureInfo&);

    static sk_sp<Texture> MakeWrapped(const MtlGpu*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_cfp<id<MTLTexture>>);

    ~MtlTexture() override {}

    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

private:
    MtlTexture(const MtlGpu* gpu,
               SkISize dimensions,
               const TextureInfo& info,
               sk_cfp<id<MTLTexture>>,
               Ownership);

    void freeGpuData() override;

    sk_cfp<id<MTLTexture>> fTexture;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_MtlTexture_DEFINED
