/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkSLTypeShared.h"

const char* SkSLTypeString(SkSLType t) {
    switch (t) {
        case SkSLType::kVoid:                   return "void";
        case SkSLType::kBool:                   return "bool";
        case SkSLType::kBool2:                  return "bool2";
        case SkSLType::kBool3:                  return "bool3";
        case SkSLType::kBool4:                  return "bool4";
        case SkSLType::kShort:                  return "short";
        case SkSLType::kShort2:                 return "short2";
        case SkSLType::kShort3:                 return "short3";
        case SkSLType::kShort4:                 return "short4";
        case SkSLType::kUShort:                 return "ushort";
        case SkSLType::kUShort2:                return "ushort2";
        case SkSLType::kUShort3:                return "ushort3";
        case SkSLType::kUShort4:                return "ushort4";
        case SkSLType::kFloat:                  return "float";
        case SkSLType::kFloat2:                 return "float2";
        case SkSLType::kFloat3:                 return "float3";
        case SkSLType::kFloat4:                 return "float4";
        case SkSLType::kFloat2x2:               return "float2x2";
        case SkSLType::kFloat3x3:               return "float3x3";
        case SkSLType::kFloat4x4:               return "float4x4";
        case SkSLType::kHalf:                   return "half";
        case SkSLType::kHalf2:                  return "half2";
        case SkSLType::kHalf3:                  return "half3";
        case SkSLType::kHalf4:                  return "half4";
        case SkSLType::kHalf2x2:                return "half2x2";
        case SkSLType::kHalf3x3:                return "half3x3";
        case SkSLType::kHalf4x4:                return "half4x4";
        case SkSLType::kInt:                    return "int";
        case SkSLType::kInt2:                   return "int2";
        case SkSLType::kInt3:                   return "int3";
        case SkSLType::kInt4:                   return "int4";
        case SkSLType::kUInt:                   return "uint";
        case SkSLType::kUInt2:                  return "uint2";
        case SkSLType::kUInt3:                  return "uint3";
        case SkSLType::kUInt4:                  return "uint4";
        case SkSLType::kTexture2DSampler:       return "sampler2D";
        case SkSLType::kTextureExternalSampler: return "samplerExternalOES";
        case SkSLType::kTexture2DRectSampler:   return "sampler2DRect";
        case SkSLType::kTexture2D:              return "texture2D";
        case SkSLType::kSampler:                return "sampler";
        case SkSLType::kInput:                  return "subpassInput";
    }
    SkUNREACHABLE;
}

/** Is the shading language type full precision? */
bool SkSLTypeIsFullPrecisionNumericType(SkSLType type) {
    switch (type) {
        // Half-precision types:
        case SkSLType::kShort:
        case SkSLType::kShort2:
        case SkSLType::kShort3:
        case SkSLType::kShort4:
        case SkSLType::kUShort:
        case SkSLType::kUShort2:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
        case SkSLType::kHalf:
        case SkSLType::kHalf2:
        case SkSLType::kHalf3:
        case SkSLType::kHalf4:
        case SkSLType::kHalf2x2:
        case SkSLType::kHalf3x3:
        case SkSLType::kHalf4x4:
        // Non-numeric types:
        case SkSLType::kVoid:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
            return false;

        // Full-precision numeric types:
        case SkSLType::kInt:
        case SkSLType::kInt2:
        case SkSLType::kInt3:
        case SkSLType::kInt4:
        case SkSLType::kUInt:
        case SkSLType::kUInt2:
        case SkSLType::kUInt3:
        case SkSLType::kUInt4:
        case SkSLType::kFloat:
        case SkSLType::kFloat2:
        case SkSLType::kFloat3:
        case SkSLType::kFloat4:
        case SkSLType::kFloat2x2:
        case SkSLType::kFloat3x3:
        case SkSLType::kFloat4x4:
            return true;
    }
    SkUNREACHABLE;
}

int SkSLTypeMatrixSize(SkSLType type) {
    switch (type) {
        case SkSLType::kFloat2x2:
        case SkSLType::kHalf2x2:
            return 2;

        case SkSLType::kFloat3x3:
        case SkSLType::kHalf3x3:
            return 3;

        case SkSLType::kFloat4x4:
        case SkSLType::kHalf4x4:
            return 4;

        case SkSLType::kFloat:
        case SkSLType::kHalf:
        case SkSLType::kBool:
        case SkSLType::kShort:
        case SkSLType::kUShort:
        case SkSLType::kInt:
        case SkSLType::kUInt:
        case SkSLType::kFloat2:
        case SkSLType::kHalf2:
        case SkSLType::kBool2:
        case SkSLType::kShort2:
        case SkSLType::kUShort2:
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
        case SkSLType::kFloat3:
        case SkSLType::kHalf3:
        case SkSLType::kBool3:
        case SkSLType::kShort3:
        case SkSLType::kUShort3:
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
        case SkSLType::kFloat4:
        case SkSLType::kHalf4:
        case SkSLType::kBool4:
        case SkSLType::kShort4:
        case SkSLType::kUShort4:
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
        case SkSLType::kVoid:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
            return -1;
    }
    SkUNREACHABLE;
}

bool SkSLTypeIsCombinedSamplerType(SkSLType type) {
    switch (type) {
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
            return true;

        case SkSLType::kVoid:
        case SkSLType::kFloat:
        case SkSLType::kFloat2:
        case SkSLType::kFloat3:
        case SkSLType::kFloat4:
        case SkSLType::kFloat2x2:
        case SkSLType::kFloat3x3:
        case SkSLType::kFloat4x4:
        case SkSLType::kHalf:
        case SkSLType::kHalf2:
        case SkSLType::kHalf3:
        case SkSLType::kHalf4:
        case SkSLType::kHalf2x2:
        case SkSLType::kHalf3x3:
        case SkSLType::kHalf4x4:
        case SkSLType::kInt:
        case SkSLType::kInt2:
        case SkSLType::kInt3:
        case SkSLType::kInt4:
        case SkSLType::kUInt:
        case SkSLType::kUInt2:
        case SkSLType::kUInt3:
        case SkSLType::kUInt4:
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
        case SkSLType::kShort:
        case SkSLType::kShort2:
        case SkSLType::kShort3:
        case SkSLType::kShort4:
        case SkSLType::kUShort:
        case SkSLType::kUShort2:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
            return false;
    }
    SkUNREACHABLE;
}
