/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnProgramDataManager_DEFINED
#define GrDawnProgramDataManager_DEFINED

#include "src/gpu/GrUniformDataManager.h"

#include "src/gpu/GrSPIRVUniformHandler.h"
#include "src/gpu/dawn/GrDawnRingBuffer.h"
#include "dawn/webgpu_cpp.h"

#include "src/core/SkAutoMalloc.h"

class GrDawnGpu;
class GrDawnUniformBuffer;

class GrDawnProgramDataManager : public GrUniformDataManager {
public:
    typedef GrSPIRVUniformHandler::UniformInfoArray UniformInfoArray;

    GrDawnProgramDataManager(const UniformInfoArray&, uint32_t uniformBufferSize);

    uint32_t uniformBufferSize() const { return fUniformSize; }

    wgpu::BindGroup uploadUniformBuffers(GrDawnGpu* gpu, wgpu::BindGroupLayout layout);

private:
    wgpu::BindGroup fBindGroup;
};

#endif
