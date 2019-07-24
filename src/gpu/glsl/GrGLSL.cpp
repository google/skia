/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/glsl/GrGLSL.h"

const char* GrGLSLTypeString(GrSLType t) {
    switch (t) {
        case kVoid_GrSLType:
            return "void";
        case kHalf_GrSLType:
            return "half";
        case kHalf2_GrSLType:
            return "half2";
        case kHalf3_GrSLType:
            return "half3";
        case kHalf4_GrSLType:
            return "half4";
        case kFloat_GrSLType:
            return "float";
        case kFloat2_GrSLType:
            return "float2";
        case kFloat3_GrSLType:
            return "float3";
        case kFloat4_GrSLType:
            return "float4";
        case kUint2_GrSLType:
            return "uint2";
        case kInt2_GrSLType:
            return "int2";
        case kInt3_GrSLType:
            return "int3";
        case kInt4_GrSLType:
            return "int4";
        case kFloat2x2_GrSLType:
            return "float2x2";
        case kFloat3x3_GrSLType:
            return "float3x3";
        case kFloat4x4_GrSLType:
            return "float4x4";
        case kHalf2x2_GrSLType:
            return "half2x2";
        case kHalf3x3_GrSLType:
            return "half3x3";
        case kHalf4x4_GrSLType:
            return "half4x4";
        case kTexture2DSampler_GrSLType:
            return "sampler2D";
        case kTextureExternalSampler_GrSLType:
            return "samplerExternalOES";
        case kTexture2DRectSampler_GrSLType:
            return "sampler2DRect";
        case kBool_GrSLType:
            return "bool";
        case kInt_GrSLType:
            return "int";
        case kUint_GrSLType:
            return "uint";
        case kShort_GrSLType:
            return "short";
        case kShort2_GrSLType:
            return "short2";
        case kShort3_GrSLType:
            return "short3";
        case kShort4_GrSLType:
            return "short4";
        case kUShort_GrSLType:
            return "ushort";
        case kUShort2_GrSLType:
            return "ushort2";
        case kUShort3_GrSLType:
            return "ushort3";
        case kUShort4_GrSLType:
            return "ushort4";
        case kByte_GrSLType:
            return "byte";
        case kByte2_GrSLType:
            return "byte2";
        case kByte3_GrSLType:
            return "byte3";
        case kByte4_GrSLType:
            return "byte4";
        case kUByte_GrSLType:
            return "ubyte";
        case kUByte2_GrSLType:
            return "ubyte2";
        case kUByte3_GrSLType:
            return "ubyte3";
        case kUByte4_GrSLType:
            return "ubyte4";
        case kTexture2D_GrSLType:
            return "texture2D";
        case kSampler_GrSLType:
            return "sampler";
    }
    SK_ABORT("Unknown shader var type.");
    return ""; // suppress warning
}
