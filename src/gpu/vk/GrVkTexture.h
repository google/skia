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
struct GrVkImageInfo;

class GrVkTexture : public GrTexture, public virtual GrVkImage {
public:
    static GrVkTexture* CreateNewTexture(GrVkGpu*, SkBudgeted budgeted, const GrSurfaceDesc&,
                                         const GrVkImage::ImageDesc&);

    static sk_sp<GrVkTexture> MakeWrappedTexture(GrVkGpu*, const GrSurfaceDesc&,
                                                 GrWrapOwnership, const GrVkImageInfo*);

    ~GrVkTexture() override;

    GrBackendObject getTextureHandle() const override;

    void textureParamsModified() override {}

    const GrVkImageView* textureView(bool allowSRGB);

    bool reallocForMipmap(GrVkGpu* gpu, uint32_t mipLevels);

protected:
    GrVkTexture(GrVkGpu*, const GrSurfaceDesc&, const GrVkImageInfo&, const GrVkImageView*,
                GrVkImage::Wrapped wrapped);

    GrVkGpu* getVkGpu() const;

    void onAbandon() override;
    void onRelease() override;
    std::unique_ptr<GrExternalTextureData> detachBackendTexture() override;

private:
    enum Wrapped { kWrapped };
    GrVkTexture(GrVkGpu*, SkBudgeted, const GrSurfaceDesc&,
                const GrVkImageInfo&, const GrVkImageView* imageView);
    GrVkTexture(GrVkGpu*, Wrapped, const GrSurfaceDesc&,
                const GrVkImageInfo&, const GrVkImageView* imageView, GrVkImage::Wrapped wrapped);

    const GrVkImageView*     fTextureView;
    const GrVkImageView*     fLinearTextureView;

    typedef GrTexture INHERITED;
};

#endif
