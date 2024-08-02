/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnTexture_DEFINED
#define skgpu_graphite_DawnTexture_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkLRUCache.h"
#include "src/gpu/graphite/Texture.h"

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {
class DawnSharedContext;
class DawnSampler;
class DawnResourceProvider;

class DawnTexture : public Texture {
public:
    using BindGroupKey = FixedSizeKey<1>;

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
                                      const wgpu::TextureView&);

    ~DawnTexture() override {}

    // Find or create a bind group containing the given sampler for this texture.
    // NOTE: This is safe to call only from Context's thread and the `resourceProvider` should be
    // that of Context. This is because the method is not thread-safe and so should not be used from
    // different threads.
    // TODO(crbug.com/352069351): Assert that the method is called from Context's thread or make it
    // thread-safe.
    const wgpu::BindGroup& getSamplerBindGroup_callFromContextThreadOnly(
            const DawnSampler* sampler, DawnResourceProvider* resourceProvider) const;

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
                Ownership,
                skgpu::Budgeted);

    void freeGpuData() override;

    static std::pair<wgpu::TextureView, wgpu::TextureView> CreateTextureViews(
            const wgpu::Texture& texture, const TextureInfo& info);

    void setBackendLabel(char const* label) override;

    wgpu::Texture     fTexture;
    wgpu::TextureView fSampleTextureView;
    wgpu::TextureView fRenderTextureView;

    using BindGroupCache = SkLRUCache<BindGroupKey, wgpu::BindGroup, typename BindGroupKey::Hash>;
    // `fSamplerBindGroups` should only be modified on the Context's thread and thus do not need to
    // be otherwise locked.
    mutable BindGroupCache fSamplerBindGroups;
};

}  // namespace skgpu::graphite

#endif // skgpu_graphite_DawnTexture_DEFINED
