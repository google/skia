/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrAttachment.h"

#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrGpu.h"

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
        SkImage::CompressionType compression = GrBackendFormatToCompressionType(format);

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
                      GrMipmapped mipmapped,
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
                                                    GrMipmapped mipmapped,
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
                                     GrMipmapped mipmapped,
                                     GrProtected isProtected,
                                     GrMemoryless memoryless,
                                     skgpu::ScratchKey* key) {
    static const skgpu::ScratchKey::ResourceType kType = skgpu::ScratchKey::GenerateResourceType();

    skgpu::ScratchKey::Builder builder(key, kType, 5);
    build_key(&builder, caps, format, dimensions, requiredUsage, sampleCnt, mipmapped, isProtected,
              memoryless);
}

void GrAttachment::computeScratchKey(skgpu::ScratchKey* key) const {
    if (!SkToBool(fSupportedUsages & UsageFlags::kStencilAttachment)) {
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
