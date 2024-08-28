/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/vk/GrVkPipelineStateDataManager.h"

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkCommandBuffer.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"

#include <cstring>

GrVkPipelineStateDataManager::GrVkPipelineStateDataManager(const UniformInfoArray& uniforms,
                                                           uint32_t uniformSize,
                                                           bool usePushConstants)
    : INHERITED(uniforms.count(), uniformSize)
    , fUsePushConstants(usePushConstants) {
    // We must add uniforms in same order as the UniformInfoArray so that UniformHandles already
    // owned by other objects will still match up here.
    int i = 0;
    GrVkUniformHandler::Layout memLayout = usePushConstants ? GrVkUniformHandler::kStd430Layout
                                                            : GrVkUniformHandler::kStd140Layout;
    for (const auto& uniformInfo : uniforms.items()) {
        Uniform& uniform = fUniforms[i];
        SkASSERT(GrShaderVar::kNonArray == uniformInfo.fVariable.getArrayCount() ||
                 uniformInfo.fVariable.getArrayCount() > 0);
        SkDEBUGCODE(
            uniform.fArrayCount = uniformInfo.fVariable.getArrayCount();
        )

        uniform.fOffset = uniformInfo.fOffsets[memLayout];
        uniform.fType = uniformInfo.fVariable.getType();
        ++i;
    }
}

std::pair<sk_sp<GrGpuBuffer>, bool> GrVkPipelineStateDataManager::uploadUniforms(
        GrVkGpu* gpu, VkPipelineLayout layout, GrVkCommandBuffer* commandBuffer) {
    if (fUniformSize == 0) {
        return std::make_pair(nullptr, true);
    }
    if (fUsePushConstants) {
        commandBuffer->pushConstants(gpu, layout, gpu->vkCaps().getPushConstantStageFlags(),
                                     0, fUniformSize, fUniformData.get());
        fUniformBuffer = nullptr;
    } else {
        if (fUniformsDirty) {
            GrResourceProvider* resourceProvider = gpu->getContext()->priv().resourceProvider();
            fUniformBuffer = resourceProvider->createBuffer(fUniformData.get(),
                                                            fUniformSize,
                                                            GrGpuBufferType::kUniform,
                                                            kDynamic_GrAccessPattern);
            if (!fUniformBuffer) {
                return std::make_pair(nullptr, false);
            }
            fUniformsDirty = false;
        }
    }

    return std::make_pair(fUniformBuffer, true);
}

void GrVkPipelineStateDataManager::set1iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    if (fUsePushConstants) {
        const Uniform& uni = fUniforms[u.toIndex()];
        SkASSERT(uni.fType == SkSLType::kInt || uni.fType == SkSLType::kShort);
        SkASSERT(arrayCount > 0);
        SkASSERT(arrayCount <= uni.fArrayCount ||
                 (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

        void* buffer = this->getBufferPtrAndMarkDirty(uni);
        SkASSERT(sizeof(int32_t) == 4);
        memcpy(buffer, v, arrayCount * sizeof(int32_t));
    } else {
        return this->INHERITED::set1iv(u, arrayCount, v);
    }
}

void GrVkPipelineStateDataManager::set1fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    if (fUsePushConstants) {
        const Uniform& uni = fUniforms[u.toIndex()];
        SkASSERT(uni.fType == SkSLType::kFloat || uni.fType == SkSLType::kHalf);
        SkASSERT(arrayCount > 0);
        SkASSERT(arrayCount <= uni.fArrayCount ||
                 (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

        void* buffer = this->getBufferPtrAndMarkDirty(uni);
        SkASSERT(sizeof(float) == 4);
        memcpy(buffer, v, arrayCount * sizeof(float));
    } else {
        return this->INHERITED::set1fv(u, arrayCount, v);
    }
}

void GrVkPipelineStateDataManager::set2iv(UniformHandle u,
                                          int arrayCount,
                                          const int32_t v[]) const {
    if (fUsePushConstants) {
        const Uniform& uni = fUniforms[u.toIndex()];
        SkASSERT(uni.fType == SkSLType::kInt2 || uni.fType == SkSLType::kShort2);
        SkASSERT(arrayCount > 0);
        SkASSERT(arrayCount <= uni.fArrayCount ||
                 (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

        void* buffer = this->getBufferPtrAndMarkDirty(uni);
        SkASSERT(sizeof(int32_t) == 4);
        memcpy(buffer, v, arrayCount * 2 * sizeof(int32_t));
    } else {
        return this->INHERITED::set2iv(u, arrayCount, v);
    }
}

void GrVkPipelineStateDataManager::set2fv(UniformHandle u,
                                          int arrayCount,
                                          const float v[]) const {
    if (fUsePushConstants) {
        const Uniform& uni = fUniforms[u.toIndex()];
        SkASSERT(uni.fType == SkSLType::kFloat2 || uni.fType == SkSLType::kHalf2);
        SkASSERT(arrayCount > 0);
        SkASSERT(arrayCount <= uni.fArrayCount ||
                 (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

        void* buffer = this->getBufferPtrAndMarkDirty(uni);
        SkASSERT(sizeof(float) == 4);
        memcpy(buffer, v, arrayCount * 2 * sizeof(float));
    } else {
        return this->INHERITED::set2fv(u, arrayCount, v);
    }
}

void GrVkPipelineStateDataManager::setMatrix2fv(UniformHandle u,
                                                int arrayCount,
                                                const float m[]) const {
    if (fUsePushConstants) {
        // upload as std430
        const Uniform& uni = fUniforms[u.toIndex()];
        SkASSERT(uni.fType == SkSLType::kFloat2x2 || uni.fType == SkSLType::kHalf2x2);
        SkASSERT(arrayCount > 0);
        SkASSERT(arrayCount <= uni.fArrayCount ||
                 (1 == arrayCount && GrShaderVar::kNonArray == uni.fArrayCount));

        void* buffer = fUniformData.get();
        fUniformsDirty = true;

        static_assert(sizeof(float) == 4);
        buffer = static_cast<char*>(buffer) + uni.fOffset;
        memcpy(buffer, m, arrayCount * 2 * 2 * sizeof(float));
    } else {
        this->INHERITED::setMatrix2fv(u, arrayCount, m);
    }
}

void GrVkPipelineStateDataManager::releaseData() { fUniformBuffer.reset(); }
