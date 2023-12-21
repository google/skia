/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLTypeShared_DEFINED
#define SkSLTypeShared_DEFINED

#include "include/core/SkTypes.h"

/**
 * Types of shader-language-specific boxed variables we can create.
 */
enum class SkSLType : char {
    kVoid,
    kBool,
    kBool2,
    kBool3,
    kBool4,
    kShort,
    kShort2,
    kShort3,
    kShort4,
    kUShort,
    kUShort2,
    kUShort3,
    kUShort4,
    kFloat,
    kFloat2,
    kFloat3,
    kFloat4,
    kFloat2x2,
    kFloat3x3,
    kFloat4x4,
    kHalf,
    kHalf2,
    kHalf3,
    kHalf4,
    kHalf2x2,
    kHalf3x3,
    kHalf4x4,
    kInt,
    kInt2,
    kInt3,
    kInt4,
    kUInt,
    kUInt2,
    kUInt3,
    kUInt4,
    kTexture2DSampler,
    kTextureExternalSampler,
    kTexture2DRectSampler,
    kTexture2D,
    kSampler,
    kInput,

    kLast = kInput
};
static const int kSkSLTypeCount = static_cast<int>(SkSLType::kLast) + 1;

/** Returns the SkSL typename for this type. */
const char* SkSLTypeString(SkSLType t);

/** Is the shading language type float (including vectors/matrices)? */
static constexpr bool SkSLTypeIsFloatType(SkSLType type) {
    switch (type) {
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
            return true;

        case SkSLType::kVoid:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
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
        case SkSLType::kInt:
        case SkSLType::kInt2:
        case SkSLType::kInt3:
        case SkSLType::kInt4:
        case SkSLType::kUInt:
        case SkSLType::kUInt2:
        case SkSLType::kUInt3:
        case SkSLType::kUInt4:
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
            return false;
    }
    SkUNREACHABLE;
}

/** Is the shading language type integral (including vectors)? */
static constexpr bool SkSLTypeIsIntegralType(SkSLType type) {
    switch (type) {
        case SkSLType::kShort:
        case SkSLType::kShort2:
        case SkSLType::kShort3:
        case SkSLType::kShort4:
        case SkSLType::kUShort:
        case SkSLType::kUShort2:
        case SkSLType::kUShort3:
        case SkSLType::kUShort4:
        case SkSLType::kInt:
        case SkSLType::kInt2:
        case SkSLType::kInt3:
        case SkSLType::kInt4:
        case SkSLType::kUInt:
        case SkSLType::kUInt2:
        case SkSLType::kUInt3:
        case SkSLType::kUInt4:
            return true;

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
        case SkSLType::kVoid:
        case SkSLType::kTexture2DSampler:
        case SkSLType::kTextureExternalSampler:
        case SkSLType::kTexture2DRectSampler:
        case SkSLType::kBool:
        case SkSLType::kBool2:
        case SkSLType::kBool3:
        case SkSLType::kBool4:
        case SkSLType::kTexture2D:
        case SkSLType::kSampler:
        case SkSLType::kInput:
            return false;
    }
    SkUNREACHABLE;
}

/** If the type represents a single value or vector return the vector length; otherwise, -1. */
static constexpr int SkSLTypeVecLength(SkSLType type) {
    switch (type) {
        case SkSLType::kFloat:
        case SkSLType::kHalf:
        case SkSLType::kBool:
        case SkSLType::kShort:
        case SkSLType::kUShort:
        case SkSLType::kInt:
        case SkSLType::kUInt:
            return 1;

        case SkSLType::kFloat2:
        case SkSLType::kHalf2:
        case SkSLType::kBool2:
        case SkSLType::kShort2:
        case SkSLType::kUShort2:
        case SkSLType::kInt2:
        case SkSLType::kUInt2:
            return 2;

        case SkSLType::kFloat3:
        case SkSLType::kHalf3:
        case SkSLType::kBool3:
        case SkSLType::kShort3:
        case SkSLType::kUShort3:
        case SkSLType::kInt3:
        case SkSLType::kUInt3:
            return 3;

        case SkSLType::kFloat4:
        case SkSLType::kHalf4:
        case SkSLType::kBool4:
        case SkSLType::kShort4:
        case SkSLType::kUShort4:
        case SkSLType::kInt4:
        case SkSLType::kUInt4:
            return 4;

        case SkSLType::kFloat2x2:
        case SkSLType::kFloat3x3:
        case SkSLType::kFloat4x4:
        case SkSLType::kHalf2x2:
        case SkSLType::kHalf3x3:
        case SkSLType::kHalf4x4:
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

/**
 * Is the shading language type supported as a uniform (ie, does it have a corresponding set
 * function on GrGLSLProgramDataManager)?
 */
static constexpr bool SkSLTypeCanBeUniformValue(SkSLType type) {
    // This is almost "IsFloatType || IsIntegralType" but excludes non-full precision int types.
    switch(type) {
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
            return true;

        default:
            return false;
    }
}

/** Is the shading language type full precision? */
bool SkSLTypeIsFullPrecisionNumericType(SkSLType type);

/** If the type represents a square matrix, return its size; otherwise, -1. */
int SkSLTypeMatrixSize(SkSLType type);

/** If the type represents a square matrix, return its size; otherwise, -1. */
bool SkSLTypeIsCombinedSamplerType(SkSLType type);

#endif // SkSLTypeShared_DEFINED
