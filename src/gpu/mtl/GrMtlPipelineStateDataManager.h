/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlPipelineStateDataManager_DEFINED
#define GrMtlPipelineStateDataManager_DEFINED

#include "src/core/SkAutoMalloc.h"
#include "src/gpu/GrUniformDataManager.h"
#include "src/gpu/mtl/GrMtlUniformHandler.h"

#import <Metal/Metal.h>

class GrMtlBuffer;
class GrMtlGpu;

class GrMtlPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrMtlUniformHandler::UniformInfoArray UniformInfoArray;

    GrMtlPipelineStateDataManager(const UniformInfoArray&, uint32_t uniformSize);

    void uploadAndBindUniformBuffers(GrMtlGpu* gpu,
                                     id<MTLRenderCommandEncoder> renderCmdEncoder) const;
};

#endif
