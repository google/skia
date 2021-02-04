/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateDataManager_DEFINED
#define GrVkPipelineStateDataManager_DEFINED

#include "src/gpu/GrUniformDataManager.h"

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkUniformHandler.h"

class GrGpuBuffer;
class GrVkCommandBuffer;
class GrVkGpu;

class GrVkPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrVkUniformHandler::UniformInfoArray UniformInfoArray;

    GrVkPipelineStateDataManager(const UniformInfoArray&, uint32_t uniformSize,
                                 GrVkUniformHandler::Layout memLayout);

    // Returns the uniform buffer that holds all the uniform data. If there are no uniforms it
    // returns nullptr. If there was an error in creating or uploading the uniforms the value of the
    // returned bool will be false and the buffer will be nullptr. Otherwise the bool will be true.
    std::pair<sk_sp<GrGpuBuffer>, bool> uploadUniformBuffers(GrVkGpu* gpu);

    void releaseData();

    void uploadPushConstants(const GrVkGpu*, VkPipelineLayout, GrVkCommandBuffer*) const;

    // TODO: we might need more of these once std430 size/alignment issues are worked out
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;

private:
    GrVkUniformHandler::Layout fMemLayout;
    sk_sp<GrGpuBuffer> fUniformBuffer;

    using INHERITED = GrUniformDataManager;
};

#endif
