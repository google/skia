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
class GrMtlRenderCommandEncoder;

class GrMtlPipelineStateDataManager : public GrUniformDataManager {
public:
    typedef GrMtlUniformHandler::UniformInfoArray UniformInfoArray;

    GrMtlPipelineStateDataManager(const UniformInfoArray&,
                                  uint32_t uniformSize);

    void set1iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set1fv(UniformHandle, int arrayCount, const float v[]) const override;
    void set2iv(UniformHandle, int arrayCount, const int32_t v[]) const override;
    void set2fv(UniformHandle, int arrayCount, const float v[]) const override;
    // matrices are column-major, the first one uploads a single matrix, the latter uploads
    // arrayCount matrices into a uniform array.
    void setMatrix2f(UniformHandle, const float matrix[]) const override;
    void setMatrix2fv(UniformHandle, int arrayCount, const float matrices[]) const override;

    void uploadAndBindUniformBuffers(GrMtlGpu* gpu,
                                     GrMtlRenderCommandEncoder* renderCmdEncoder) const;
    void resetDirtyBits();

private:
    typedef GrUniformDataManager INHERITED;
};

#endif
