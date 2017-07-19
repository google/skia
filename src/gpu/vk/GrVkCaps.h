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
#include "vk/GrVkDefines.h"

struct GrVkInterface;
class GrShaderCaps;

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
             VkPhysicalDevice device, uint32_t featureFlags, uint32_t extensionFlags);

    int getSampleCount(int requestedCount, GrPixelConfig config) const override;

    bool isConfigTexturable(GrPixelConfig config) const override {
        return SkToBool(ConfigInfo::kTextureable_Flag & fConfigTable[config].fOptimalFlags);
    }

    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {
        return SkToBool(ConfigInfo::kRenderable_Flag & fConfigTable[config].fOptimalFlags);
    }

    bool canConfigBeImageStorage(GrPixelConfig) const override { return false; }

    bool isConfigTexturableLinearly(GrPixelConfig config) const {
        return SkToBool(ConfigInfo::kTextureable_Flag & fConfigTable[config].fLinearFlags);
    }

    bool isConfigRenderableLinearly(GrPixelConfig config, bool withMSAA) const {
        return !withMSAA && SkToBool(ConfigInfo::kRenderable_Flag &
                                     fConfigTable[config].fLinearFlags);
    }

    bool configCanBeDstofBlit(GrPixelConfig config, bool linearTiled) const {
        const uint16_t& flags = linearTiled ? fConfigTable[config].fLinearFlags :
                                              fConfigTable[config].fOptimalFlags;
        return SkToBool(ConfigInfo::kBlitDst_Flag & flags);
    }

    bool configCanBeSrcofBlit(GrPixelConfig config, bool linearTiled) const {
        const uint16_t& flags = linearTiled ? fConfigTable[config].fLinearFlags :
                                              fConfigTable[config].fOptimalFlags;
        return SkToBool(ConfigInfo::kBlitSrc_Flag & flags);
    }

    // Tells of if we can pass in straight GLSL string into vkCreateShaderModule
    bool canUseGLSLForShaderModule() const {
        return fCanUseGLSLForShaderModule;
    }

    // On Adreno vulkan, they do not respect the imageOffset parameter at least in
    // copyImageToBuffer. This flag says that we must do the copy starting from the origin always.
    bool mustDoCopiesFromOrigin() const {
        return fMustDoCopiesFromOrigin;
    }

    // Check whether we support using draws for copies.
    bool supportsCopiesAsDraws() const {
        return fSupportsCopiesAsDraws;
    }

    // On Nvidia there is a current bug where we must the current command buffer before copy
    // operations or else the copy will not happen. This includes copies, blits, resolves, and copy
    // as draws.
    bool mustSubmitCommandsBeforeCopyOp() const {
        return fMustSubmitCommandsBeforeCopyOp;
    }

    // Sometimes calls to QueueWaitIdle return before actually signalling the fences
    // on the command buffers even though they have completed. This causes an assert to fire when
    // destroying the command buffers. Therefore we add a sleep to make sure the fence signals.
    bool mustSleepOnTearDown() const {
        return fMustSleepOnTearDown;
    }

    // Returns true if while adding commands to command buffers, we must make a new command buffer
    // everytime we want to bind a new VkPipeline. This is true for both primary and secondary
    // command buffers. This is to work around a driver bug specifically on AMD.
    bool newCBOnPipelineChange() const {
        return fNewCBOnPipelineChange;
    }

    /**
     * Returns both a supported and most prefered stencil format to use in draws.
     */
    const StencilFormat& preferedStencilFormat() const {
        return fPreferedStencilFormat;
    }

    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override;

private:
    enum VkVendor {
        kAMD_VkVendor = 4098,
        kImagination_VkVendor = 4112,
        kNvidia_VkVendor = 4318,
        kQualcomm_VkVendor = 20803,
    };

    void init(const GrContextOptions& contextOptions, const GrVkInterface* vkInterface,
              VkPhysicalDevice device, uint32_t featureFlags, uint32_t extensionFlags);
    void initGrCaps(const VkPhysicalDeviceProperties&,
                    const VkPhysicalDeviceMemoryProperties&,
                    uint32_t featureFlags);
    void initShaderCaps(const VkPhysicalDeviceProperties&, uint32_t featureFlags);
    void initSampleCount(const VkPhysicalDeviceProperties& properties);


    void initConfigTable(const GrVkInterface*, VkPhysicalDevice, const VkPhysicalDeviceProperties&);
    void initStencilFormat(const GrVkInterface* iface, VkPhysicalDevice physDev);

    struct ConfigInfo {
        ConfigInfo() : fOptimalFlags(0), fLinearFlags(0) {}

        void init(const GrVkInterface*, VkPhysicalDevice, const VkPhysicalDeviceProperties&,
                  VkFormat);
        static void InitConfigFlags(VkFormatFeatureFlags, uint16_t* flags);
        void initSampleCounts(const GrVkInterface*, VkPhysicalDevice,
                              const VkPhysicalDeviceProperties&, VkFormat);

        enum {
            kTextureable_Flag = 0x1,
            kRenderable_Flag  = 0x2,
            kBlitSrc_Flag     = 0x4,
            kBlitDst_Flag     = 0x8,
        };

        uint16_t fOptimalFlags;
        uint16_t fLinearFlags;

        SkTDArray<int> fColorSampleCounts;
    };
    ConfigInfo fConfigTable[kGrPixelConfigCnt];

    StencilFormat fPreferedStencilFormat;

    bool fCanUseGLSLForShaderModule;

    bool fMustDoCopiesFromOrigin;

    bool fSupportsCopiesAsDraws;

    bool fMustSubmitCommandsBeforeCopyOp;

    bool fMustSleepOnTearDown;

    bool fNewCBOnPipelineChange;

    typedef GrCaps INHERITED;
};

#endif
