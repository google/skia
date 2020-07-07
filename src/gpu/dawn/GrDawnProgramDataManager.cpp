/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnProgramDataManager.h"

#include "src/gpu/dawn/GrDawnGpu.h"

GrDawnProgramDataManager::GrDawnProgramDataManager(const UniformInfoArray& uniforms,
                                                   uint32_t uniformBufferSize)
    : GrUniformDataManager(uniforms.count(), uniformBufferSize) {
    memset(fUniformData.get(), 0, uniformBufferSize);
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    int i = 0;
    for (const auto& uniformInfo : uniforms.items()) {
        Uniform& uniform = fUniforms[i];
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
            uniform.fType = uniformInfo.fVariable.getType();
        )
        uniform.fOffset = uniformInfo.fUBOOffset;
        ++i;
    }
}

void GrDawnProgramDataManager::uploadUniformBuffers(void* dest) const {
    if (fUniformsDirty) {
        memcpy(dest, fUniformData.get(), fUniformSize);
    }
}
