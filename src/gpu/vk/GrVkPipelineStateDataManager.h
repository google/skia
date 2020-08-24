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

class GrVkGpu;
class GrVkUniformBuffer;

class GrVkPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrVkUniformHandler::UniformInfoArray UniformInfoArray;

    GrVkPipelineStateDataManager(const UniformInfoArray&, uint32_t uniformSize);

    void uploadUniformBuffers(void* dst) const;

private:
    typedef GrUniformDataManager INHERITED;
};

#endif
