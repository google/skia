/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrImpl_DEFINED
#define GrImpl_DEFINED

#include "GrTypes.h"

// compile time versions of min/max
#define GR_CT_MAX(a, b) (((b) < (a)) ? (a) : (b))
#define GR_CT_MIN(a, b) (((b) < (a)) ? (b) : (a))

/**
 *  divide, rounding up
 */
static inline int32_t GrIDivRoundUp(int x, int y) {
    SkASSERT(y > 0);
    return (x + (y-1)) / y;
}
static inline uint32_t GrUIDivRoundUp(uint32_t x, uint32_t y) {
    return (x + (y-1)) / y;
}
static inline size_t GrSizeDivRoundUp(size_t x, size_t y) {
    return (x + (y-1)) / y;
}

// compile time, evaluates Y multiple times
#define GR_CT_DIV_ROUND_UP(X, Y) (((X) + ((Y)-1)) / (Y))

/**
 *  align up
 */
static inline uint32_t GrUIAlignUp(uint32_t x, uint32_t alignment) {
    return GrUIDivRoundUp(x, alignment) * alignment;
}
static inline size_t GrSizeAlignUp(size_t x, size_t alignment) {
    return GrSizeDivRoundUp(x, alignment) * alignment;
}

// compile time, evaluates A multiple times
#define GR_CT_ALIGN_UP(X, A) (GR_CT_DIV_ROUND_UP((X),(A)) * (A))

/**
 * amount of pad needed to align up
 */
static inline uint32_t GrUIAlignUpPad(uint32_t x, uint32_t alignment) {
    return (alignment - x % alignment) % alignment;
}
static inline size_t GrSizeAlignUpPad(size_t x, size_t alignment) {
    return (alignment - x % alignment) % alignment;
}

/**
 *  align down
 */
static inline uint32_t GrUIAlignDown(uint32_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}
static inline size_t GrSizeAlignDown(size_t x, uint32_t alignment) {
    return (x / alignment) * alignment;
}

/**
 * Geometric primitives used for drawing.
 */
enum class GrPrimitiveType {
    kTriangles,
    kTriangleStrip,
    kTriangleFan,
    kPoints,
    kLines,     // 1 pix wide only
    kLineStrip, // 1 pix wide only
    kLinesAdjacency // requires geometry shader support.
};
static constexpr int kNumGrPrimitiveTypes = (int) GrPrimitiveType::kLinesAdjacency + 1;

static constexpr bool GrIsPrimTypeLines(GrPrimitiveType type) {
    return GrPrimitiveType::kLines == type ||
    GrPrimitiveType::kLineStrip == type ||
    GrPrimitiveType::kLinesAdjacency == type;
}

static constexpr bool GrPrimTypeRequiresGeometryShaderSupport(GrPrimitiveType type) {
    return GrPrimitiveType::kLinesAdjacency == type;
}

struct GrMipLevel {
    const void* fPixels;
    size_t fRowBytes;
};

#endif
