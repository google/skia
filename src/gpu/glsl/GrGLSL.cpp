/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrShaderCaps.h"
#include "SkString.h"
#include "../private/GrGLSL.h"

bool GrGLSLSupportsNamedFragmentShaderOutputs(GrGLSLGeneration gen) {
    switch (gen) {
        case k110_GrGLSLGeneration:
            return false;
        case k130_GrGLSLGeneration:
        case k140_GrGLSLGeneration:
        case k150_GrGLSLGeneration:
        case k330_GrGLSLGeneration:
        case k400_GrGLSLGeneration:
        case k420_GrGLSLGeneration:
        case k310es_GrGLSLGeneration:
        case k320es_GrGLSLGeneration:
            return true;
    }
    return false;
}

const char* GrGLSLTypeString(const GrShaderCaps* shaderCaps, GrSLType t) {
    switch (t) {
        case kVoid_GrSLType:
            return "void";
        case kFloat_GrSLType:
            return "float";
        case kVec2f_GrSLType:
            return "float2";
        case kVec3f_GrSLType:
            return "float3";
        case kVec4f_GrSLType:
            return "float4";
        case kVec2us_GrSLType:
            if (shaderCaps->integerSupport()) {
                return "uint2";
            } else {
                // uint2 (aka uvec2) isn't supported in GLSL ES 1.00/GLSL 1.20
                return "float2";
            }
        case kVec2i_GrSLType:
            return "int2";
        case kVec3i_GrSLType:
            return "int3";
        case kVec4i_GrSLType:
            return "int4";
        case kMat22f_GrSLType:
            return "float2x2";
        case kMat33f_GrSLType:
            return "float3x3";
        case kMat44f_GrSLType:
            return "float4x4";
        case kTexture2DSampler_GrSLType:
            return "sampler2D";
        case kITexture2DSampler_GrSLType:
            return "isampler2D";
        case kTextureExternalSampler_GrSLType:
            return "samplerExternalOES";
        case kTexture2DRectSampler_GrSLType:
            return "sampler2DRect";
        case kBufferSampler_GrSLType:
            return "samplerBuffer";
        case kBool_GrSLType:
            return "bool";
        case kInt_GrSLType:
            return "int";
        case kUint_GrSLType:
            return "uint";
        case kTexture2D_GrSLType:
            return "texture2D";
        case kSampler_GrSLType:
            return "sampler";
        case kImageStorage2D_GrSLType:
            return "image2D";
        case kIImageStorage2D_GrSLType:
            return "iimage2D";
    }
    SK_ABORT("Unknown shader var type.");
    return ""; // suppress warning
}

void GrGLSLAppendDefaultFloatPrecisionDeclaration(GrSLPrecision p,
                                                  const GrShaderCaps& shaderCaps,
                                                  SkString* out) {
    if (shaderCaps.usesPrecisionModifiers()) {
        switch (p) {
            case kHigh_GrSLPrecision:
                out->append("precision highp float;\n");
                break;
            case kMedium_GrSLPrecision:
                out->append("precision mediump float;\n");
                break;
            case kLow_GrSLPrecision:
                out->append("precision lowp float;\n");
                break;
            default:
                SK_ABORT("Unknown precision value.");
        }
    }
}
