/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasedShaderHelpers_DEFINED
#define GrAtlasedShaderHelpers_DEFINED

#include "src/gpu/GrDrawOpAtlas.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

static void append_index_uv_varyings(GrGeometryProcessor::ProgramImpl::EmitArgs& args,
                                     int numTextureSamplers,
                                     const char* inTexCoordsName,
                                     const char* atlasDimensionsInvName,
                                     GrGLSLVarying* uv,
                                     GrGLSLVarying* texIdx,
                                     GrGLSLVarying* st) {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;
    // This extracts the texture index and texel coordinates from the same variable
    // Packing structure: texel coordinates have the 2-bit texture page encoded in bits 13 & 14 of
    // the x coordinate. It would be nice to use bits 14 and 15, but iphone6 has problem with those
    // bits when in gles. Iphone6 works fine with bits 14 and 15 in metal.
    if (args.fShaderCaps->integerSupport()) {
        if (numTextureSamplers <= 1) {
            args.fVertBuilder->codeAppendf(R"code(
                int texIdx = 0;
                float2 unormTexCoords = float2(%s.x, %s.y);
           )code", inTexCoordsName, inTexCoordsName);
        } else {
            args.fVertBuilder->codeAppendf(R"code(
                int2 coords = int2(%s.x, %s.y);
                int texIdx = coords.x >> 13;
                float2 unormTexCoords = float2(coords.x & 0x1FFF, coords.y);
            )code", inTexCoordsName, inTexCoordsName);
        }
    } else {
        if (numTextureSamplers <= 1) {
            args.fVertBuilder->codeAppendf(R"code(
                float texIdx = 0;
                float2 unormTexCoords = float2(%s.x, %s.y);
            )code", inTexCoordsName, inTexCoordsName);
        } else {
            args.fVertBuilder->codeAppendf(R"code(
                float2 coord = float2(%s.x, %s.y);
                float texIdx = floor(coord.x * exp2(-13));
                float2 unormTexCoords = float2(coord.x - texIdx * exp2(13), coord.y);
            )code", inTexCoordsName, inTexCoordsName);
        }
    }

    // Multiply by 1/atlasDimensions to get normalized texture coordinates
    uv->reset(kFloat2_GrSLType);
    args.fVaryingHandler->addVarying("TextureCoords", uv);
    args.fVertBuilder->codeAppendf(
            "%s = unormTexCoords * %s;", uv->vsOut(), atlasDimensionsInvName);

    // On ANGLE there is a significant cost to using an int varying. We don't know of any case where
    // it is worse to use a float so for now we always do.
    texIdx->reset(kFloat_GrSLType);
    // If we computed the local var "texIdx" as an int we will need to cast it to float
    const char* cast = args.fShaderCaps->integerSupport() ? "float" : "";
    args.fVaryingHandler->addVarying("TexIndex", texIdx, Interpolation::kCanBeFlat);
    args.fVertBuilder->codeAppendf("%s = %s(texIdx);", texIdx->vsOut(), cast);

    if (st) {
        st->reset(kFloat2_GrSLType);
        args.fVaryingHandler->addVarying("IntTextureCoords", st);
        args.fVertBuilder->codeAppendf("%s = unormTexCoords;", st->vsOut());
    }
}

static void append_multitexture_lookup(GrGeometryProcessor::ProgramImpl::EmitArgs& args,
                                       int numTextureSamplers,
                                       const GrGLSLVarying& texIdx,
                                       const char* coordName,
                                       const char* colorName) {
    SkASSERT(numTextureSamplers > 0);
    // This shouldn't happen, but will avoid a crash if it does
    if (numTextureSamplers <= 0) {
        args.fFragBuilder->codeAppendf("%s = float4(1, 1, 1, 1);", colorName);
        return;
    }

    // conditionally load from the indexed texture sampler
    for (int i = 0; i < numTextureSamplers-1; ++i) {
        args.fFragBuilder->codeAppendf("if (%s == %d) { %s = ", texIdx.fsIn(), i, colorName);
        args.fFragBuilder->appendTextureLookup(args.fTexSamplers[i], coordName);
        args.fFragBuilder->codeAppend("; } else ");
    }
    args.fFragBuilder->codeAppendf("{ %s = ", colorName);
    args.fFragBuilder->appendTextureLookup(args.fTexSamplers[numTextureSamplers - 1], coordName);
    args.fFragBuilder->codeAppend("; }");
}

#endif
