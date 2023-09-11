/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrAttachment.h"

#include "include/core/SkTextureCompressionType.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrGpu.h"

size_t GrAttachment::onGpuMemorySize() const {
    // The GrTexture[RenderTarget] is built up by a bunch of attachments each of which are their
    // own GrGpuResource. Ideally the GrRenderTarget would not be a GrGpuResource and the GrTexture
    // would just merge with the new GrSurface/Attachment world. Then we could just depend on each
    // attachment to give its own size since we don't have GrGpuResources owning other
    // GrGpuResources. Until we get to that point we need to live in some hybrid world. We will let
    // the msaa and stencil attachments track their own size because they do get cached separately.
    // For all GrTexture* based things we will continue to to use the GrTexture* to report size and
    // the owned attachments will have no size and be uncached.
    if (!(fSupportedUsages & UsageFlags::kTexture) && fMemoryless == GrMemoryless::kNo) {
        GrBackendFormat format = this->backendFormat();
        SkTextureCompressionType compression = GrBackendFormatToCompressionType(format);

        uint64_t size = GrNumBlocks(compression, this->dimensions());
        size *= GrBackendFormatBytesPerBlock(this->backendFormat());
        size *= this->numSamples();
        return size;
    }
    return 0;
}

static void build_key(skgpu::ResourceKey::Builder* builder,
                      const GrCaps& caps,
                      const GrBackendFormat& format,
                      SkISize dimensions,
                      GrAttachment::UsageFlags requiredUsage,
                      int sampleCnt,
                      skgpu::Mipmapped mipmapped,
                      GrProtected isProtected,
                      GrMemoryless memoryless) {
    SkASSERT(!dimensions.isEmpty());

    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(memoryless) <= 1);
    SkASSERT(static_cast<uint32_t>(requiredUsage) < (1u << 8));
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1u << (32 - 10)));

    uint64_t formatKey = caps.computeFormatKey(format);
    (*builder)[0] = dimensions.width();
    (*builder)[1] = dimensions.height();
    (*builder)[2] = formatKey & 0xFFFFFFFF;
    (*builder)[3] = (formatKey >> 32) & 0xFFFFFFFF;
    (*builder)[4] = (static_cast<uint32_t>(isProtected) << 0) |
                    (static_cast<uint32_t>(memoryless) << 1) |
                    (static_cast<uint32_t>(requiredUsage) << 2) |
                    (static_cast<uint32_t>(sampleCnt) << 10);
}

void GrAttachment::ComputeSharedAttachmentUniqueKey(const GrCaps& caps,
                                                    const GrBackendFormat& format,
                                                    SkISize dimensions,
                                                    UsageFlags requiredUsage,
                                                    int sampleCnt,
                                                    skgpu::Mipmapped mipmapped,
                                                    GrProtected isProtected,
                                                    GrMemoryless memoryless,
                                                    skgpu::UniqueKey* key) {
    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();

    skgpu::UniqueKey::Builder builder(key, kDomain, 5);
    build_key(&builder, caps, format, dimensions, requiredUsage, sampleCnt, mipmapped, isProtected,
              memoryless);
}

void GrAttachment::ComputeScratchKey(const GrCaps& caps,
                                     const GrBackendFormat& format,
                                     SkISize dimensions,
                                     UsageFlags requiredUsage,
                                     int sampleCnt,
                                     skgpu::Mipmapped mipmapped,
                                     GrProtected isProtected,
                                     GrMemoryless memoryless,
                                     skgpu::ScratchKey* key) {
    static const skgpu::ScratchKey::ResourceType kType = skgpu::ScratchKey::GenerateResourceType();

    skgpu::ScratchKey::Builder builder(key, kType, 5);
    build_key(&builder, caps, format, dimensions, requiredUsage, sampleCnt, mipmapped, isProtected,
              memoryless);
}

void GrAttachment::computeScratchKey(skgpu::ScratchKey* key) const {
    // We do don't cache GrAttachments as scratch resources when used for stencils or textures. For
    // stencils we share/cache them with unique keys so that they can be shared. Textures are in a
    // weird place on the Vulkan backend. Currently, GrVkTexture contains a GrAttachment (GrVkImage)
    // that actually holds the VkImage. The GrVkTexture is cached as a scratch resource and is
    // responsible for tracking the gpuMemorySize. Thus we set the size of the texture GrVkImage,
    // above in onGpuMemorySize, to be zero. Therefore, we can't have the GrVkImage getting cached
    // separately on its own in the GrResourceCache or we may grow forever adding them thinking they
    // contatin a memory that's size 0 and never freeing the actual VkImages.
    if (!SkToBool(fSupportedUsages & UsageFlags::kStencilAttachment) &&
        !SkToBool(fSupportedUsages & UsageFlags::kTexture)) {
        auto isProtected = this->isProtected() ? GrProtected::kYes : GrProtected::kNo;
        ComputeScratchKey(*this->getGpu()->caps(),
                          this->backendFormat(),
                          this->dimensions(),
                          fSupportedUsages,
                          this->numSamples(),
                          this->mipmapped(),
                          isProtected,
                          fMemoryless,
                          key);
    }
}
