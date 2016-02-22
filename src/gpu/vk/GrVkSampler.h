/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkSampler_DEFINED
#define GrVkSampler_DEFINED

#include "GrVkResource.h"

#include "vulkan/vulkan.h"

class GrTextureAccess;
class GrVkGpu;


class GrVkSampler : public GrVkResource {
public:
    static GrVkSampler* Create(const GrVkGpu* gpu, const GrTextureAccess& textureAccess);

    VkSampler sampler() const { return fSampler; }

private:
    GrVkSampler(VkSampler sampler) : INHERITED(), fSampler(sampler) {}

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkSampler  fSampler;

    typedef GrVkResource INHERITED;
};

#endif