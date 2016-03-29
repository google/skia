/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkImageView_DEFINED
#define GrVkImageView_DEFINED

#include "GrTypes.h"

#include "GrVkResource.h"

#include "vk/GrVkDefines.h"

class GrVkImageView : public GrVkResource {
public:
    enum Type {
        kColor_Type,
        kStencil_Type
    };

    static const GrVkImageView* Create(GrVkGpu* gpu, VkImage image, VkFormat format, Type viewType);

    VkImageView imageView() const { return fImageView; }

private:
    GrVkImageView(VkImageView imageView) : INHERITED(), fImageView(imageView) {}

    GrVkImageView(const GrVkImageView&);
    GrVkImageView& operator=(const GrVkImageView&);

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkImageView  fImageView;

    typedef GrVkResource INHERITED;
};

#endif
