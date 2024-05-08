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

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

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
                               std::string_view label,
                               skgpu::Budgeted);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      wgpu::Texture,
                                      std::string_view label);

    static sk_sp<Texture> MakeWrapped(const DawnSharedContext*,
                                      SkISize dimensions,
                                      const TextureInfo&,
                                      const wgpu::TextureView&,
                                      std::string_view label);

    ~DawnTexture() override {}

    const wgpu::Texture& dawnTexture() const { return fTexture; }
    const wgpu::TextureView& sampleTextureView() const { return fSampleTextureView; }
    const wgpu::TextureView& renderTextureView() const { return fRenderTextureView; }

private:
    DawnTexture(const DawnSharedContext*,
                SkISize dimensions,
                const TextureInfo&,
                wgpu::Texture,
                wgpu::TextureView sampleTextureView,
                wgpu::TextureView renderTextureView,
                std::string_view label,
                Ownership,
                skgpu::Budgeted);

    void freeGpuData() override;

    static std::pair<wgpu::TextureView, wgpu::TextureView> CreateTextureViews(
            const wgpu::Texture& texture, const TextureInfo& info);

    void setBackendLabel(char const* label) override;

    wgpu::Texture     fTexture;
    wgpu::TextureView fSampleTextureView;
    wgpu::TextureView fRenderTextureView;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTexture_DEFINED
