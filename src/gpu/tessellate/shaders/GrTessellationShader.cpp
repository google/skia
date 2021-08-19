/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/shaders/GrTessellationShader.h"

const GrPipeline* GrTessellationShader::MakePipeline(const ProgramArgs& args,
                                                     GrAAType aaType,
                                                     GrAppliedClip&& appliedClip,
                                                     GrProcessorSet&& processors) {
    GrPipeline::InitArgs pipelineArgs;

    pipelineArgs.fCaps = args.fCaps;
    pipelineArgs.fDstProxyView = *args.fDstProxyView;
    pipelineArgs.fWriteSwizzle = args.fWriteView.swizzle();

    return args.fArena->make<GrPipeline>(pipelineArgs,
                                         std::move(processors),
                                         std::move(appliedClip));
}
