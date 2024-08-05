/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkPipelineStateDataManager_DEFINED
#define GrVkPipelineStateDataManager_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/gpu/vk/SkiaVulkan.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrUniformDataManager.h"
#include "src/gpu/ganesh/vk/GrVkUniformHandler.h"

#include <cstdint>
#include <utility>

class GrVkCommandBuffer;
class GrVkGpu;

class GrVkPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrVkUniformHandler::UniformInfoArray UniformInfoArray;

    GrVkPipelineStateDataManager(const UniformInfoArray&, uint32_t uniformSize,
                                 bool usePushConstants);

    // Returns the uniform buffer that holds all the uniform data. If there are no uniforms it
    // returns nullptr. If there was an error in creating or uploading the uniforms the value of the
    // returned bool will be false and the buffer will be nullptr. Otherwise the bool will be true.
    std::pair<sk_sp<GrGpuBuffer>, bool> uploadUniforms(GrVkGpu* gpu, VkPipelineLayout,
                                                       GrVkCommandBuffer* commandBuffer);

    void releaseData();

    // TODO: we might need more of these once std430 size/alignment issues are worked out
    void set1iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;

private:
    sk_sp<GrGpuBuffer> fUniformBuffer;
    bool fUsePushConstants;

    using INHERITED = GrUniformDataManager;
};

#endif
