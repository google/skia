/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTexture_DEFINED
#define GrVkTexture_DEFINED

#include "GrGpu.h"
#include "GrTexture.h"
#include "GrVkImage.h"

class GrVkGpu;
class GrVkImageView;
struct GrVkTextureInfo;

class GrVkTexture : public GrTexture, public virtual GrVkImage {
public:
    static GrVkTexture* CreateNewTexture(GrVkGpu*, SkBudgeted budgeted, const GrSurfaceDesc&,
                                         const GrVkImage::ImageDesc&);

    static GrVkTexture* CreateWrappedTexture(GrVkGpu*, const GrSurfaceDesc&,
                                             GrWrapOwnership,
                                             VkFormat, const GrVkTextureInfo*);

    ~GrVkTexture() override;

    GrBackendObject getTextureHandle() const override;

    void textureParamsModified() override {}

    const GrVkImageView* textureView() const { return fTextureView; }

    bool reallocForMipmap(const GrVkGpu* gpu, uint32_t mipLevels);

protected:
    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&,
                const GrVkImage::Resource*, const GrVkImageView* imageView);

    template<typename ResourceType>
    static GrVkTexture* Create(GrVkGpu*, ResourceType, const GrSurfaceDesc&, VkFormat,
                               uint32_t levels, const GrVkImage::Resource* texImpl);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

private:
    enum Wrapped { kWrapped };
    GrVkTexture(GrVkGpu*, SkBudgeted, const GrSurfaceDesc&,
                const GrVkImage::Resource*, const GrVkImageView* imageView);
    GrVkTexture(GrVkGpu*, Wrapped, const GrSurfaceDesc&,
                const GrVkImage::Resource*, const GrVkImageView* imageView);

    const GrVkImageView*     fTextureView;

    typedef GrTexture INHERITED;
};

#endif
