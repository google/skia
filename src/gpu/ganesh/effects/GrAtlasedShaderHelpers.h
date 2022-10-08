/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasedShaderHelpers_DEFINED
#define GrAtlasedShaderHelpers_DEFINED

#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVarying.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"

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
    if (args.fShaderCaps->fIntegerSupport) {
        if (numTextureSamplers <= 1) {
            args.fVertBuilder->codeAppendf(
                "int texIdx = 0;"
                "float2 unormTexCoords = float2(%s.x, %s.y);"
           , inTexCoordsName, inTexCoordsName);
        } else {
            args.fVertBuilder->codeAppendf(
                "int2 coords = int2(%s.x, %s.y);"
                "int texIdx = coords.x >> 13;"
                "float2 unormTexCoords = float2(coords.x & 0x1FFF, coords.y);"
            , inTexCoordsName, inTexCoordsName);
        }
    } else {
        if (numTextureSamplers <= 1) {
            args.fVertBuilder->codeAppendf(
                "float texIdx = 0;"
                "float2 unormTexCoords = float2(%s.x, %s.y);"
            , inTexCoordsName, inTexCoordsName);
        } else {
            args.fVertBuilder->codeAppendf(
                "float2 coord = float2(%s.x, %s.y);"
                "float texIdx = floor(coord.x * exp2(-13));"
                "float2 unormTexCoords = float2(coord.x - texIdx * exp2(13), coord.y);"
            , inTexCoordsName, inTexCoordsName);
        }
    }

    // Multiply by 1/atlasDimensions to get normalized texture coordinates
    uv->reset(SkSLType::kFloat2);
    args.fVaryingHandler->addVarying("TextureCoords", uv);
    args.fVertBuilder->codeAppendf(
            "%s = unormTexCoords * %s;", uv->vsOut(), atlasDimensionsInvName);

    // On ANGLE there is a significant cost to using an int varying. We don't know of any case where
    // it is worse to use a float so for now we always do.
    texIdx->reset(SkSLType::kFloat);
    // If we computed the local var "texIdx" as an int we will need to cast it to float
    const char* cast = args.fShaderCaps->fIntegerSupport ? "float" : "";
    args.fVaryingHandler->addVarying("TexIndex", texIdx, Interpolation::kCanBeFlat);
    args.fVertBuilder->codeAppendf("%s = %s(texIdx);", texIdx->vsOut(), cast);

    if (st) {
        st->reset(SkSLType::kFloat2);
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

    // Conditionally load from the indexed texture sampler. We intentionally sample without implicit
    // derivatives to avoid undefined behavior in branches (and satisfy Dawn's uniformity analysis).
    // Atlas textures are not mipped so it's OK to always use mip-level 0.
    //
    // On OpenGL versions that lack explicit LOD support (GLSL <1.3, GLSL ES <3.0, WebGL 1) we
    // fallback to sampling with implicit derivatives in branches. For mip-mapepd textures this
    // would still be considered undefined behavior by the GLSL ES 1.00 specification, but we don't
    // worry about this because the texture is assumed to be not mip-mapped and we expect shader
    // compilers to flatten these branches.
    auto lookup = [&](int idx) {
        if (args.fShaderCaps->fExplicitTextureLodSupport) {
            args.fFragBuilder->appendTextureLookupExplicitLod(
                    args.fTexSamplers[idx], coordName, "0.0");
        } else {
            args.fFragBuilder->appendTextureLookup(args.fTexSamplers[idx], coordName);
        }
    };
    for (int i = 0; i < numTextureSamplers-1; ++i) {
        args.fFragBuilder->codeAppendf("if (%s == %d) { %s = ", texIdx.fsIn(), i, colorName);
        lookup(i);
        args.fFragBuilder->codeAppend("; } else ");
    }
    args.fFragBuilder->codeAppendf("{ %s = ", colorName);
    lookup(numTextureSamplers - 1);
    args.fFragBuilder->codeAppend("; }");
}

#endif
