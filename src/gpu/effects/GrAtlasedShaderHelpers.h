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
                                     const char* inTexCoordsName,
                                     const char* atlasSizeInvName,
                                     GrGLSLVarying *uv,
                                     GrGLSLVarying *texIdx,
                                     GrGLSLVarying *st) {
    using Interpolation = GrGLSLVaryingHandler::Interpolation;

    // This extracts the texture index and texel coordinates from the same variable
    // Packing structure: texel coordinates are multiplied by 2 (or shifted left 1)
    //                    texture index is stored as lower bits of both x and y
    if (args.fShaderCaps->integerSupport()) {
        args.fVertBuilder->codeAppendf("int2 signedCoords = int2(%s.x, %s.y);",
                                       inTexCoordsName, inTexCoordsName);
        args.fVertBuilder->codeAppend("int texIdx = 2*(signedCoords.x & 0x1) + (signedCoords.y & 0x1);");
        args.fVertBuilder->codeAppend("float2 unormTexCoords = float2(signedCoords.x/2, signedCoords.y/2);");
    } else {
        args.fVertBuilder->codeAppendf("float2 indexTexCoords = float2(%s.x, %s.y);",
                                       inTexCoordsName, inTexCoordsName);
        args.fVertBuilder->codeAppend("float2 unormTexCoords = floor(0.5*indexTexCoords);");
        args.fVertBuilder->codeAppend("float2 diff = indexTexCoords - 2.0*unormTexCoords;");
        args.fVertBuilder->codeAppend("float texIdx = 2.0*diff.x + diff.y;");
    }

    // Multiply by 1/atlasSize to get normalized texture coordinates
    args.fVaryingHandler->addVarying("TextureCoords", uv);
    args.fVertBuilder->codeAppendf("%s = unormTexCoords * %s;", uv->vsOut(), atlasSizeInvName);

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
    // conditionally load from the indexed texture sampler
    for (int i = 0; i < numTextureSamplers-1; ++i) {
        args.fFragBuilder->codeAppendf("if (%s == %d) { %s = ", texIdx.fsIn(), i, colorName);
        args.fFragBuilder->appendTextureLookup(args.fTexSamplers[i], coordName,
                                               kFloat2_GrSLType);
        args.fFragBuilder->codeAppend("; } else ");
    }
    args.fFragBuilder->codeAppendf("{ %s = ", colorName);
    args.fFragBuilder->appendTextureLookup(args.fTexSamplers[numTextureSamplers-1], coordName,
                                           kFloat2_GrSLType);
    args.fFragBuilder->codeAppend("; }");
}

#endif
