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
        case kVoid_GrSLType:                   return "void";
        case kBool_GrSLType:                   return "bool";
        case kBool2_GrSLType:                  return "bool2";
        case kBool3_GrSLType:                  return "bool3";
        case kBool4_GrSLType:                  return "bool4";
        case kShort_GrSLType:                  return "short";
        case kShort2_GrSLType:                 return "short2";
        case kShort3_GrSLType:                 return "short3";
        case kShort4_GrSLType:                 return "short4";
        case kUShort_GrSLType:                 return "ushort";
        case kUShort2_GrSLType:                return "ushort2";
        case kUShort3_GrSLType:                return "ushort3";
        case kUShort4_GrSLType:                return "ushort4";
        case kFloat_GrSLType:                  return "float";
        case kFloat2_GrSLType:                 return "float2";
        case kFloat3_GrSLType:                 return "float3";
        case kFloat4_GrSLType:                 return "float4";
        case kFloat2x2_GrSLType:               return "float2x2";
        case kFloat3x3_GrSLType:               return "float3x3";
        case kFloat4x4_GrSLType:               return "float4x4";
        case kHalf_GrSLType:                   return "half";
        case kHalf2_GrSLType:                  return "half2";
        case kHalf3_GrSLType:                  return "half3";
        case kHalf4_GrSLType:                  return "half4";
        case kHalf2x2_GrSLType:                return "half2x2";
        case kHalf3x3_GrSLType:                return "half3x3";
        case kHalf4x4_GrSLType:                return "half4x4";
        case kInt_GrSLType:                    return "int";
        case kInt2_GrSLType:                   return "int2";
        case kInt3_GrSLType:                   return "int3";
        case kInt4_GrSLType:                   return "int4";
        case kUint_GrSLType:                   return "uint";
        case kUint2_GrSLType:                  return "uint2";
        case kUint3_GrSLType:                  return "uint3";
        case kUint4_GrSLType:                  return "uint4";
        case kTexture2DSampler_GrSLType:       return "sampler2D";
        case kTextureExternalSampler_GrSLType: return "samplerExternalOES";
        case kTexture2DRectSampler_GrSLType:   return "sampler2DRect";
        case kTexture2D_GrSLType:              return "texture2D";
        case kSampler_GrSLType:                return "sampler";
        case kInput_GrSLType:                  return "subpassInput";
    }
    SK_ABORT("Unknown shader var type.");
}
