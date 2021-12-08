/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlCaps_DEFINED
#define skgpu_MtlCaps_DEFINED

#include "experimental/graphite/src/Caps.h"

#import <Metal/Metal.h>

namespace skgpu::mtl {

class Caps final : public skgpu::Caps {
public:
    Caps(const id<MTLDevice>);
    ~Caps() override {}

    skgpu::TextureInfo getDefaultSampledTextureInfo(SkColorType,
                                                    uint32_t levelCount,
                                                    Protected,
                                                    Renderable) const override;

    skgpu::TextureInfo getDefaultMSAATextureInfo(SkColorType,
                                                 uint32_t sampleCount,
                                                 Protected) const override;

    skgpu::TextureInfo getDefaultDepthStencilTextureInfo(Mask<DepthStencilFlags>,
                                                         uint32_t sampleCount,
                                                         Protected) const override;

    bool isMac() const { return fGPUFamily == GPUFamily::kMac; }
    bool isApple()const  { return fGPUFamily == GPUFamily::kApple; }

    size_t getMinBufferAlignment() const { return this->isMac() ? 4 : 1; }

    bool isTexturable(const skgpu::TextureInfo&) const override;
    bool isRenderable(const skgpu::TextureInfo&) const override;

private:
    void initGPUFamily(const id<MTLDevice>);

    void initCaps(const id<MTLDevice>);
    void initShaderCaps();
    void initFormatTable();

    enum class GPUFamily {
        kMac,
        kApple,
    };
    static bool GetGPUFamily(id<MTLDevice> device, GPUFamily* gpuFamily, int* group);
    static bool GetGPUFamilyFromFeatureSet(id<MTLDevice> device, GPUFamily* gpuFamily,
                                           int* group);

    bool onAreColorTypeAndTextureInfoCompatible(SkColorType,
                                                const skgpu::TextureInfo&) const override;
    bool isTexturable(MTLPixelFormat) const;
    bool isRenderable(MTLPixelFormat, uint32_t numSamples) const;

    GPUFamily fGPUFamily;
    int fFamilyGroup;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlCaps_DEFINED
