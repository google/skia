/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAtlasedShaderHelpers_DEFINED
#define GrAtlasedShaderHelpers_DEFINED

#include "glsl/GrGLSLPrimitiveProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLVarying.h"
#include "glsl/GrGLSLVertexGeoBuilder.h"

static void append_index_uv_varyings(GrGLSLPrimitiveProcessor::EmitArgs& args,
                                     const char* inTexCoordsName,
                                     const char* atlasSizeInvName,
                                     GrGLSLVarying *uv,
                                     GrGLSLVarying *texIdx,
                                     GrGLSLVarying *st) {
    // This extracts the texture index and texel coordinates from the same variable
    // Packing structure: texel coordinates are multiplied by 2 (or shifted left 1)
    //                    texture index is stored as lower bits of both x and y
    args.fVertBuilder->codeAppendf("half2 indexTexCoords = half2(%s.x, %s.y);",
                                   inTexCoordsName, inTexCoordsName);
    args.fVertBuilder->codeAppend("half2 intCoords = floor(0.5*indexTexCoords);");
    args.fVertBuilder->codeAppend("half2 diff = indexTexCoords - 2.0*intCoords;");
    args.fVertBuilder->codeAppend("half texIdx = 2.0*diff.x + diff.y;");

    // Multiply by 1/atlasSize to get normalized texture coordinates
    args.fVaryingHandler->addVarying("TextureCoords", uv);
    args.fVertBuilder->codeAppendf("%s = intCoords * %s;", uv->vsOut(), atlasSizeInvName);

    args.fVaryingHandler->addVarying("TexIndex", texIdx);
    args.fVertBuilder->codeAppendf("%s = texIdx;", texIdx->vsOut());

    if (st) {
        args.fVaryingHandler->addVarying("IntTextureCoords", st);
        args.fVertBuilder->codeAppendf("%s = intCoords;", st->vsOut());
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
