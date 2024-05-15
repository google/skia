/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTexture_DEFINED
#define skgpu_graphite_MtlTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/Texture.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {
class MtlSharedContext;

class MtlTexture : public Texture {
public:
    static sk_cfp<id<MTLTexture>> MakeMtlTexture(const MtlSharedContext*,
                                                 SkISize dimensions,
                                                 const TextureInfo&);

    static sk_sp<Texture> Make(const MtlSharedContext*,
                               SkISize dimensions,
                               const TextureInfo&,
                               std::string_view label,
                               skgpu::Budgeted);

    static sk_sp<Texture> MakeWrapped(const MtlSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_cfp<id<MTLTexture>>,
                                      std::string_view label);

    ~MtlTexture() override {}

    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

private:
    MtlTexture(const MtlSharedContext* sharedContext,
               SkISize dimensions,
               const TextureInfo& info,
               sk_cfp<id<MTLTexture>>,
               std::string_view label,
               Ownership,
               skgpu::Budgeted);

    void freeGpuData() override;

    void setBackendLabel(char const* label) override;

    sk_cfp<id<MTLTexture>> fTexture;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_MtlTexture_DEFINED
