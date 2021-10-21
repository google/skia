/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/AtlasInstancedHelper.h"

#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

namespace skgpu::v1 {

void AtlasInstancedHelper::appendInstanceAttribs(
        SkTArray<GrGeometryProcessor::Attribute>* instanceAttribs) const {
    instanceAttribs->emplace_back("locations", kFloat4_GrVertexAttribType, kFloat4_GrSLType);
    if (fShaderFlags & ShaderFlags::kCheckBounds) {
        instanceAttribs->emplace_back("sizeInAtlas", kFloat2_GrVertexAttribType, kFloat2_GrSLType);
    }
}

void AtlasInstancedHelper::writeInstanceData(GrVertexWriter* instanceWriter,
                                             const Instance* i) const {
    SkASSERT(i->fLocationInAtlas.x() >= 0);
    SkASSERT(i->fLocationInAtlas.y() >= 0);
    *instanceWriter <<
            // A negative x coordinate in the atlas indicates that the path is transposed.
            // Also add 1 since we can't negate zero.
            ((float)(i->fTransposedInAtlas ? -i->fLocationInAtlas.x() - 1
                     : i->fLocationInAtlas.x() + 1)) <<
            (float)i->fLocationInAtlas.y() <<
            (float)i->fPathDevIBounds.left() <<
            (float)i->fPathDevIBounds.top() <<
            GrVertexWriter::If(fShaderFlags & ShaderFlags::kCheckBounds,
                               SkSize::Make(i->fPathDevIBounds.size()));
}

void AtlasInstancedHelper::injectShaderCode(
        const GrGeometryProcessor::ProgramImpl::EmitArgs& args,
        const GrShaderVar& devCoord,
        GrGLSLUniformHandler::UniformHandle* atlasAdjustUniformHandle) const {
    GrGLSLVarying atlasCoord(kFloat2_GrSLType);
    args.fVaryingHandler->addVarying("atlasCoord", &atlasCoord);

    const char* atlasAdjustName;
    *atlasAdjustUniformHandle = args.fUniformHandler->addUniform(
            nullptr, kVertex_GrShaderFlag, kFloat2_GrSLType, "atlas_adjust", &atlasAdjustName);

    args.fVertBuilder->codeAppendf(R"(
    // A negative x coordinate in the atlas indicates that the path is transposed.
    // We also added 1 since we can't negate zero.
    float2 atlasTopLeft = float2(abs(locations.x) - 1, locations.y);
    float2 devTopLeft = locations.zw;
    bool transposed = locations.x < 0;
    float2 atlasCoord = %s - devTopLeft;
    if (transposed) {
        atlasCoord = atlasCoord.yx;
    }
    atlasCoord += atlasTopLeft;
    %s = atlasCoord * %s;)", devCoord.c_str(), atlasCoord.vsOut(), atlasAdjustName);

    if (fShaderFlags & ShaderFlags::kCheckBounds) {
        GrGLSLVarying atlasBounds(kFloat4_GrSLType);
        args.fVaryingHandler->addVarying("atlasbounds", &atlasBounds,
                                         GrGLSLVaryingHandler::Interpolation::kCanBeFlat);
        args.fVertBuilder->codeAppendf(R"(
        float4 atlasBounds = atlasTopLeft.xyxy + (transposed ? sizeInAtlas.00yx
                                                             : sizeInAtlas.00xy);
        %s = atlasBounds * %s.xyxy;)", atlasBounds.vsOut(), atlasAdjustName);

        args.fFragBuilder->codeAppendf(R"(
        half atlasCoverage = 0;
        float2 atlasCoord = %s;
        float4 atlasBounds = %s;
        if (all(greaterThan(atlasCoord, atlasBounds.xy)) &&
            all(lessThan(atlasCoord, atlasBounds.zw))) {
            atlasCoverage = )", atlasCoord.fsIn(), atlasBounds.fsIn());
        args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], "atlasCoord");
        args.fFragBuilder->codeAppendf(R"(.a;
        })");
    } else {
        args.fFragBuilder->codeAppendf("half atlasCoverage = ");
        args.fFragBuilder->appendTextureLookup(args.fTexSamplers[0], atlasCoord.fsIn());
        args.fFragBuilder->codeAppendf(".a;");
    }

    if (fShaderFlags & ShaderFlags::kInvertCoverage) {
        args.fFragBuilder->codeAppendf("%s *= (1 - atlasCoverage);", args.fOutputCoverage);
    } else {
        args.fFragBuilder->codeAppendf("%s *= atlasCoverage;", args.fOutputCoverage);
    }
}

void AtlasInstancedHelper::setUniformData(
        const GrGLSLProgramDataManager& pdman,
        const GrGLSLUniformHandler::UniformHandle& atlasAdjustUniformHandle) const {
    SkASSERT(fAtlasProxy->isInstantiated());
    SkISize dimensions = fAtlasProxy->backingStoreDimensions();
    pdman.set2f(atlasAdjustUniformHandle, 1.f / dimensions.width(), 1.f / dimensions.height());
}

} // namespace skgpu::v1
