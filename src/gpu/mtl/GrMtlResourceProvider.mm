/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlResourceProvider.h"

#include "GrMtlCopyManager.h"
#include "GrMtlGpu.h"
#include "GrMtlUtil.h"

#include "SkSLCompiler.h"

GrMtlCopyPipelineState* GrMtlResourceProvider::findOrCreateCopyPipelineState(
        MTLPixelFormat dstPixelFormat,
        id<MTLFunction> vertexFunction,
        id<MTLFunction> fragmentFunction,
        MTLVertexDescriptor* vertexDescriptor) {

    for (const auto& copyPipelineState: fCopyPipelineStateCache) {
        if (GrMtlCopyManager::IsCompatible(copyPipelineState.get(), dstPixelFormat)) {
            return copyPipelineState.get();
        }
    }

    fCopyPipelineStateCache.emplace_back(GrMtlCopyPipelineState::CreateCopyPipelineState(
             fGpu, dstPixelFormat, vertexFunction, fragmentFunction, vertexDescriptor));
    return fCopyPipelineStateCache.back().get();
}
