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
    static GrVkTexture* CreateNewTexture(GrVkGpu*, const GrSurfaceDesc&,
                                         GrGpuResource::LifeCycle,
                                         const GrVkImage::ImageDesc&);

    static GrVkTexture* CreateWrappedTexture(GrVkGpu*, const GrSurfaceDesc&,
                                             GrGpuResource::LifeCycle,
                                             VkFormat, const GrVkTextureInfo*);

    ~GrVkTexture() override;

    GrBackendObject getTextureHandle() const override;

    void textureParamsModified() override {}

    const GrVkImageView* textureView() const { return fTextureView; }

protected:
    enum Derived { kDerived };

    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&, GrGpuResource::LifeCycle,
                const GrVkImage::Resource*, const GrVkImageView* imageView);

    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&, GrGpuResource::LifeCycle,
                const GrVkImage::Resource*, const GrVkImageView* imageView, Derived);

    static GrVkTexture* Create(GrVkGpu*, const GrSurfaceDesc&,
                               GrGpuResource::LifeCycle, VkFormat,
                               const GrVkImage::Resource* texImpl);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;

private:
    const GrVkImageView* fTextureView;

    typedef GrTexture INHERITED;
};

#endif
