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

class GrVkCommandBuffer;
class GrVkGpu;
class GrVkUniformBuffer;

class GrVkPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrVkUniformHandler::UniformInfoArray UniformInfoArray;

    GrVkPipelineStateDataManager(const UniformInfoArray&, uint32_t uniformSize,
                                 GrVkUniformHandler::Layout layout);

    // Returns true if either the geometry or fragment buffers needed to generate a new underlying
    // VkBuffer object in order upload data. If true is returned, this is a signal to the caller
    // that they will need to update the descriptor set that is using these buffers.
    bool uploadUniformBuffers(GrVkGpu* gpu, GrVkUniformBuffer* buffer) const;

    void uploadPushConstants(const GrVkGpu*, VkPipelineLayout, GrVkCommandBuffer*);

    void set1iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set3iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set3fv(UniformHandle, int arrayCount, const float v[]) const override;
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;
    void setMatrix3fv(UniformHandle, int arrayCount, const float matrices[]) const override;

private:
    template<int N> inline void setStd430Matrices(UniformHandle, int arrayCount,
                                                  const float matrices[]) const;

    GrVkUniformHandler::Layout fLayout;

    using INHERITED = GrUniformDataManager;
};

#endif
