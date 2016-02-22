/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkCaps_DEFINED
#define GrVkCaps_DEFINED

#include "GrCaps.h"
#include "GrVkStencilAttachment.h"
#include "vulkan/vulkan.h"

struct GrVkInterface;
class GrGLSLCaps;

/**
 * Stores some capabilities of a Vk backend.
 */
class GrVkCaps : public GrCaps {
public:    
    typedef GrVkStencilAttachment::Format StencilFormat;

    /**
     * Creates a GrVkCaps that is set such that nothing is supported. The init function should
     * be called to fill out the caps.
     */
    GrVkCaps(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
             VkPhysicalDevice device);

    bool isConfigTexturable(GrPixelConfig config) const override {
        SkASSERT(kGrPixelConfigCnt > config);
        return fConfigTextureSupport[config];
    }

    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {
        SkASSERT(kGrPixelConfigCnt > config);
        return fConfigRenderSupport[config][withMSAA];
    }

    bool isConfigRenderableLinearly(GrPixelConfig config, bool withMSAA) const {
        SkASSERT(kGrPixelConfigCnt > config);
        return fConfigLinearRenderSupport[config][withMSAA];
    }

    bool isConfigTexurableLinearly(GrPixelConfig config) const {
        SkASSERT(kGrPixelConfigCnt > config);
        return fConfigLinearTextureSupport[config];
    }

    /**
     * Gets an array of legal stencil formats. These formats are not guaranteed to be supported by
     * the driver but are legal VK_TEXTURE_FORMATs.
     */
    const SkTArray<StencilFormat, true>& stencilFormats() const {
        return fStencilFormats;
    }

    /**
     * Gets an array of legal stencil formats. These formats are not guaranteed to be supported by
     * the driver but are legal VK_TEXTURE_FORMATs.
     */
    const SkTArray<StencilFormat, true>& linearStencilFormats() const {
        return fLinearStencilFormats;
    }

    /**
     * Returns the max number of sampled textures we can use in a program. This number is the max of
     * max samplers and max sampled images. This number is technically the max sampled textures we
     * can have per stage, but we'll use it for the whole program since for now we only do texture
     * lookups in the fragment shader.
     */
    int maxSampledTextures() const {
        return fMaxSampledTextures;
    }


    GrGLSLCaps* glslCaps() const { return reinterpret_cast<GrGLSLCaps*>(fShaderCaps.get()); }

private:
    void init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
              VkPhysicalDevice device);
    void initSampleCount(const VkPhysicalDeviceProperties& properties);
    void initGLSLCaps(const GrVkInterface* interface, VkPhysicalDevice physDev);
    void initConfigRenderableTable(const GrVkInterface* interface, VkPhysicalDevice physDev);
    void initConfigTexturableTable(const GrVkInterface* interface, VkPhysicalDevice physDev);
    void initStencilFormats(const GrVkInterface* interface, VkPhysicalDevice physDev);


    bool fConfigTextureSupport[kGrPixelConfigCnt];
    // For Vulkan we track whether a config is supported linearly (without need for swizzling)
    bool fConfigLinearTextureSupport[kGrPixelConfigCnt];

    // The first entry for each config is without msaa and the second is with.
    bool fConfigRenderSupport[kGrPixelConfigCnt][2];
    // The first entry for each config is without msaa and the second is with.
    bool fConfigLinearRenderSupport[kGrPixelConfigCnt][2];

    SkTArray<StencilFormat, true> fLinearStencilFormats;
    SkTArray<StencilFormat, true> fStencilFormats;

    int fMaxSampledTextures;

    typedef GrCaps INHERITED;
};

#endif
