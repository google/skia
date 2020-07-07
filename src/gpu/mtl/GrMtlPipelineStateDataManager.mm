/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/mtl/GrMtlPipelineStateDataManager.h"

#include "src/gpu/mtl/GrMtlBuffer.h"
#include "src/gpu/mtl/GrMtlGpu.h"

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

GrMtlPipelineStateDataManager::GrMtlPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                             uint32_t uniformSize)
        : GrUniformDataManager(uniforms.count(), uniformSize) {
    fUniformData.reset(uniformSize);
    fUniforms.push_back_n(uniforms.count());
    // We must add uniforms in same order is the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    int i = 0;
    for (const auto& uniformInfo : uniforms.items()) {
        Uniform& uniform = fUniforms[i];
        SkASSERT(GrShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
            uniform.fType = uniformInfo.fVariable.getType();
        )
        uniform.fOffset = uniformInfo.fUBOffset;
        ++i;
    }
}

void GrMtlPipelineStateDataManager::uploadAndBindUniformBuffers(
        GrMtlGpu* gpu,
        id<MTLRenderCommandEncoder> renderCmdEncoder) const {
    if (fUniformSize && fUniformsDirty) {
        SkASSERT(fUniformSize < 4*1024);
        if (@available(macOS 10.11, iOS 8.3, *)) {
            [renderCmdEncoder setVertexBytes: fUniformData.get()
                                      length: fUniformSize
                                     atIndex: GrMtlUniformHandler::kUniformBinding];
            [renderCmdEncoder setFragmentBytes: fUniformData.get()
                                        length: fUniformSize
                                       atIndex: GrMtlUniformHandler::kUniformBinding];
        } else {
            size_t bufferOffset;
            id<MTLBuffer> uniformBuffer = gpu->resourceProvider().getDynamicBuffer(
                                                  fUniformSize, &bufferOffset);
            SkASSERT(uniformBuffer);
            char* bufferData = (char*) uniformBuffer.contents + bufferOffset;
            memcpy(bufferData, fUniformData.get(), fUniformSize);
            [renderCmdEncoder setVertexBuffer: uniformBuffer
                                       offset: bufferOffset
                                      atIndex: GrMtlUniformHandler::kUniformBinding];
            [renderCmdEncoder setFragmentBuffer: uniformBuffer
                                         offset: bufferOffset
                                        atIndex: GrMtlUniformHandler::kUniformBinding];
        }
        fUniformsDirty = false;
    }
}