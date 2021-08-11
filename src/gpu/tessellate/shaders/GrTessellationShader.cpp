/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

const GrPipeline* GrTessellationShader::MakePipeline(const ProgramArgs& args,
                                                     GrAAType aaType,
                                                     GrAppliedClip&& appliedClip,
                                                     GrProcessorSet&& processors) {
    auto pipelineFlags = GrPipeline::InputFlags::kNone;
    if (aaType == GrAAType::kMSAA) {
        pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    return GrSimpleMeshDrawOpHelper::CreatePipeline(
            args.fCaps, args.fArena, args.fWriteView.swizzle(), std::move(appliedClip),
            *args.fDstProxyView, std::move(processors), pipelineFlags);
}
