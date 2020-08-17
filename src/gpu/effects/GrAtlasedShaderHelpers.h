/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasedShaderHelpers_DEFINED
#define GrAtlasedShaderHelpers_DEFINED

#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLPrimitiveProcessor.h"
#include "src/gpu/glsl/GrGLSLVarying.h"
#include "src/gpu/glsl/GrGLSLVertexGeoBuilder.h"

static void append_index_uv_varyings(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                     int numTextureSamplers,
                                     const char* inTexCoordsName,
                                     const char* atlasDimensionsInvName,
                                     GrGLSLVarying* uv,
                                     GrGLSLVarying* texIdx,
                                     GrGLSLVarying* st) {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

    args.fVaryingHandler->addVarying("TextureCoords", uv);
    // This extracts the texture index and texel coordinates from the same variable
    // Packing structure: texel coordinates are multiplied by 2 (or shifted left 1)
    //                    texture index is stored as lower bits of both x and y
    if (args.fShaderCaps->integerSupport()) {
        args.fVertBuilder->codeAppendf("int2 signedCoords = int2(%s.x, %s.y);",
                                       inTexCoordsName, inTexCoordsName);
        args.fVertBuilder->codeAppend(
                "float2 unormTexCoords = float2(signedCoords.x & 32767, signedCoords.y & 32767);");
        if (numTextureSamplers <= 1) {
            args.fVertBuilder->codeAppend("int texIdx = 0;");
        } else {
            args.fVertBuilder->codeAppend("int texIdx = 2*(signedCoords.x >> 15) + (signedCoords.y >> 15);");
        }
        // Multiply by 1/atlasDimensions to get normalized texture coordinates
        args.fVertBuilder->codeAppendf("%s = unormTexCoords * %s;", uv->vsOut(),
                                       atlasDimensionsInvName);
    } else {
        args.fVertBuilder->codeAppendf("float2 indexTexCoords = float2(%s.x, %s.y);",
                                       inTexCoordsName, inTexCoordsName);
        constexpr float f = 1.0f / 32768.0f;
        args.fVertBuilder->codeAppendf("float2 unitTexCoords = %g*indexTexCoords;", f);
        args.fVertBuilder->codeAppend("float2 unormTexCoords = fract(unitTexCoords)*32768;");
        if (numTextureSamplers <= 1) {
            args.fVertBuilder->codeAppend("float texIdx = 0;");
        } else {
            args.fVertBuilder->codeAppend("float2 highBits = floor(unitTexCoords);");
            args.fVertBuilder->codeAppend("float texIdx = highBits.x * 2.0 + highBits.y;");
        }
        // Multiply by 1/atlasDimensions to get normalized texture coordinates
        args.fVertBuilder->codeAppendf("%s = unormTexCoords * %s;", uv->vsOut(),
                                       atlasDimensionsInvName);
    }

    args.fVaryingHandler->addVarying("TexIndex", texIdx, args.fShaderCaps->integerSupport()
                                                                 ? Interpolation::kMustBeFlat
                                                                 : Interpolation::kCanBeFlat);
    args.fVertBuilder->codeAppendf("%s = texIdx;", texIdx->vsOut());

    if (st) {
        args.fVaryingHandler->addVarying("IntTextureCoords", st);
        args.fVertBuilder->codeAppendf("%s = unormTexCoords;", st->vsOut());
    }
}

static void append_multitexture_lookup(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                       int numTextureSamplers,
                                       const GrGLSLVarying &texIdx,
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
