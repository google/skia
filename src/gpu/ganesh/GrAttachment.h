/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAttachment_DEFINED
#define GrAttachment_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/SkMacros.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrSurface.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

class GrCaps;
class GrGpu;

namespace skgpu {
class ScratchKey;
class UniqueKey;
enum class Mipmapped : bool;
}  // namespace skgpu

/**
 * This is a generic attachment class for out GrSurfaces. It always represents a single gpu
 * allocation. It contains usage flags so that we know what the attachment can be used for.
 *
 * TODO: Once we can pull out GrRenderTarget to be more of a framebuffer and break apart our
 * texture render target diamond, we will merge this class with GrSurface. Until then this will
 * act as the staging class for the new surface and framebuffer world.
 */
class GrAttachment : public GrSurface {
public:
    enum class UsageFlags : uint8_t {
        kStencilAttachment = 0x1,
        kColorAttachment   = 0x2,
        kTexture           = 0x4,
    };
    SK_DECL_BITFIELD_CLASS_OPS_FRIENDS(UsageFlags);

    ~GrAttachment() override {}

    UsageFlags supportedUsages() const { return fSupportedUsages; }

    int numSamples() const { return fSampleCnt; }

    skgpu::Mipmapped mipmapped() const { return fMipmapped; }

    SkIRect clearedArea() const { return fClearedArea; }
    bool hasAreaBeenCleared(SkIRect attachmentArea) const {
        return fClearedArea.contains(attachmentArea);
    }
    void markAreaCleared(SkIRect area) { fClearedArea = area; }

    // This unique key is used for attachments of the same dimensions, usage, and sample cnt which
    // are shared between multiple render targets at the same time. Only one usage flag may be
    // passed in.
    // TODO: Once attachments start having multiple usages, we'll need to figure out how to search
    // the cache for an attachment that simply contains the requested usage instead of equaling it.
    static void ComputeSharedAttachmentUniqueKey(const GrCaps& caps,
                                                 const GrBackendFormat& format,
                                                 SkISize dimensions,
                                                 UsageFlags requiredUsage,
                                                 int sampleCnt,
                                                 skgpu::Mipmapped mipmapped,
                                                 GrProtected isProtected,
                                                 GrMemoryless memoryless,
                                                 skgpu::UniqueKey* key);

    // TODO: Once attachments start having multiple usages, we'll need to figure out how to search
    // the cache for an attachment that simply contains the requested usage instead of equaling it.
    static void ComputeScratchKey(const GrCaps& caps,
                                  const GrBackendFormat& format,
                                  SkISize dimensions,
                                  UsageFlags requiredUsage,
                                  int sampleCnt,
                                  skgpu::Mipmapped mipmapped,
                                  GrProtected,
                                  GrMemoryless,
                                  skgpu::ScratchKey* key);

protected:
    GrAttachment(GrGpu* gpu,
                 SkISize dimensions,
                 UsageFlags supportedUsages,
                 int sampleCnt,
                 skgpu::Mipmapped mipmapped,
                 GrProtected isProtected,
                 std::string_view label,
                 GrMemoryless memoryless = GrMemoryless::kNo)
            : INHERITED(gpu, dimensions, isProtected, label)
            , fSupportedUsages(supportedUsages)
            , fSampleCnt(sampleCnt)
            , fMipmapped(mipmapped)
            , fMemoryless(memoryless) {}

private:
    size_t onGpuMemorySize() const final;

    void onSetLabel() override{}

    void computeScratchKey(skgpu::ScratchKey*) const final;

    const char* getResourceType() const override {
        if (fSupportedUsages == UsageFlags::kStencilAttachment) {
            return "StencilAttachment";
        }

        // This is a general grouping of all textures and color attachments.
        return "Surface";
    }

    UsageFlags fSupportedUsages;
    int fSampleCnt;
    skgpu::Mipmapped fMipmapped;
    // Track which area of the attachment has already been cleared to cut down on unnecessary clear
    // operations, which can be more expensive on desktop GPUs than loads.
    // NOTE: stored in native (backend) coordinate space, NOT surface-origin space -- this
    // attachment can be shared by render targets with different GrSurfaceOrigin (origin is not
    // part of the shared-attachment UniqueKey).
    SkIRect fClearedArea = SkIRect::MakeEmpty();
    GrMemoryless fMemoryless;

    using INHERITED = GrSurface;
};

SK_MAKE_BITFIELD_CLASS_OPS(GrAttachment::UsageFlags)

#endif
