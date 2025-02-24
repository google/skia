/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTexture_DEFINED
#define skgpu_graphite_MtlTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/graphite/mtl/MtlGraphiteTypes.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureInfoPriv.h"

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
                               const TextureInfo&);

    static sk_sp<Texture> MakeWrapped(const MtlSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      sk_cfp<id<MTLTexture>>);

    ~MtlTexture() override {}

    const MtlTextureInfo& mtlTextureInfo() const {
        return TextureInfoPriv::Get<MtlTextureInfo>(this->textureInfo());
    }
    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

private:
    MtlTexture(const MtlSharedContext* sharedContext,
               SkISize dimensions,
               const TextureInfo& info,
               sk_cfp<id<MTLTexture>>,
               Ownership);

    void freeGpuData() override;

    void setBackendLabel(char const* label) override;

    sk_cfp<id<MTLTexture>> fTexture;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_MtlTexture_DEFINED
