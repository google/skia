/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTexture_DEFINED
#define skgpu_graphite_DawnTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/Texture.h"

#include "webgpu/webgpu_cpp.h"

namespace skgpu::graphite {
class DawnSharedContext;

class DawnTexture : public Texture {
public:
    static wgpu::Texture MakeDawnTexture(const DawnSharedContext*,
                                         SkISize dimensions,
                                         const TextureInfo&);

    static sk_sp<Texture> Make(const DawnSharedContext*,
                               SkISize dimensions,
                               const TextureInfo&,
                               skgpu::Budgeted);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      wgpu::Texture);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      wgpu::TextureView);

    ~DawnTexture() override {}

    const wgpu::Texture& dawnTexture() const { return fTexture; }
    const wgpu::TextureView& dawnTextureView() const { return fTextureView; }

private:
    DawnTexture(const DawnSharedContext* sharedContext,
                SkISize dimensions,
                const TextureInfo& info,
                wgpu::Texture,
                wgpu::TextureView,
                Ownership,
                skgpu::Budgeted);

    void freeGpuData() override;

    wgpu::Texture     fTexture;
    wgpu::TextureView fTextureView;
};

} // namepsace skgpu::graphite

#endif // skgpu_graphite_DawnTexture_DEFINED
