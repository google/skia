/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSurfac2_DEFINED
#define GrSurfac2_DEFINED

class GrSurfac2 : public GrGpuResource {
public:
    /**
     * Retrieves the width of the surface.
     */
    int width() const { return fWidth; }

    /**
     * Retrieves the height of the surface.
     */
    int height() const { return fHeight; }

    /**
     * Returns the number of samples/pixels in the surface (Zero if non-MSAA).
     */
    int sampleCnt() const { return fSampleCnt; }

    /**
     * Retrieves the pixel config specified when the surface was created. For render targets this
     * can be kUnknown_GrPixelConfig if client asked us to render to a target that has a pixel
     * config that isn't equivalent with one of our configs. It will also be set to
     * kUnknown_GrPixelConfig for stencils.
     *
     * TODO: Remove the knownledge of GrPixelConfig once it is removed from use in the rest of
     * Ganesh.
     */
    GrPixelConfig config() const { return fConfig; }

    /**
     * Flags used to describe the valid usage for the surface.
     */
    enum UsageFlags : uint32_t {
        kTexturable_UsageFlag = 0x1,
        kRenderable_UsageFlag = 0x2,
        kStencil_UsageFlag    = 0x4,
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(UsageFlags);

    bool isTexturable() const {
        return SkToBool(fUsageFlags & kTexturable_UsageFlag);
    }

    bool isRenderable() const {
        return SkToBool(fUsageFlags & kRenderable_UsageFlag);
    }

    bool isStencil() const {
        return SkToBool(fUsageFlags & kStencil_UsageFlag);
    }

    // These match the definitions in SkImage, for whence they came
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

private:
    void computeScratchKey(GrScratchKey*) const override;
    size_t onGpuMemorySize() const override;
    int                  fWidth;
    int                  fHeight;
    GrPixelConfig        fConfig;

    UsageFlags           fUsageFlags;

    // Only valid when kTexturable_Usage flag is set
    void dirtyMipMaps(bool mipMapsDirty);

    enum MipMapsStatus {
        kNotAllocated_MipMapsStatus,
        kAllocated_MipMapsStatus,
        kValid_MipMapsStatus
    };
    MipMapsStatus                 fMipMapsStatus;
    int                           fNumMipLevels; // Unmipped will be 1, otherwise fully mipped
    SkDestinationSurfaceColorMode fMipColorMode;

    // Only valid when kRenderable_Usage flag is set
    int                  fSampleCnt;

    typedef GrGpuResource INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrSurfac2::UsageFlags);

#endif
