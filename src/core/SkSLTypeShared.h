/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLTypeShared_DEFINED
#define SkSLTypeShared_DEFINED

// TODO: replace GrSLType with SkSLType to minimize duplication

/**
 * Types of shader-language-specific boxed variables we can create.
 */
enum class SkSLType {
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

#endif // SkSLTypeShared_DEFINED
