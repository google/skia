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

    // This extracts the texture index and texel coordinates from the same variable
    // Packing structure: to store an index bit, texel coordinate [0,N] is mapped to [-1,-N-1]
    args.fVertBuilder->codeAppendf("float2 unormTexCoords = float2(%s.x, %s.y);",
                                   inTexCoordsName, inTexCoordsName);
    if (numTextureSamplers < 2) {
        if (args.fShaderCaps->integerSupport()) {
            args.fVertBuilder->codeAppend("int texIdx = 0;");
        } else {
            args.fVertBuilder->codeAppend("float texIdx = 0;");
        }
    } else {
        if (args.fShaderCaps->integerSupport()) {
            args.fVertBuilder->codeAppend("int indexX = 0; int indexY = 0;");
        } else {
            args.fVertBuilder->codeAppend("float indexX = 0; float indexY = 0;");
        }
        // Could possibly do this with mix() but not clear it's worth it.
        args.fVertBuilder->codeAppend("if (unormTexCoords.x < 0) {");
        args.fVertBuilder->codeAppend("  unormTexCoords.x = -unormTexCoords.x-1;");
        args.fVertBuilder->codeAppend("  indexX = 2;");
        args.fVertBuilder->codeAppend("}");
        args.fVertBuilder->codeAppend("if (unormTexCoords.y < 0) {");
        args.fVertBuilder->codeAppend("  unormTexCoords.y = -unormTexCoords.y-1;");
        args.fVertBuilder->codeAppend("  indexY = 1;");
        args.fVertBuilder->codeAppend("}");
        if (args.fShaderCaps->integerSupport()) {
            args.fVertBuilder->codeAppend("int texIdx = indexX + indexY;");
        } else {
            args.fVertBuilder->codeAppend("float texIdx = indexX + indexY;");
        }
    }

    // Multiply by 1/atlasDimensions to get normalized texture coordinates
    args.fVaryingHandler->addVarying("TextureCoords", uv);
    args.fVertBuilder->codeAppendf("%s = unormTexCoords * %s;", uv->vsOut(),
                                   atlasDimensionsInvName);

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
