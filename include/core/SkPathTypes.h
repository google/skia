/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathTypes_DEFINED
#define SkPathTypes_DEFINED

#include "SkTypes.h"

enum class SkPathDirection {
    kCW,
    kCCW,
};

enum class SkPathFillType {
    kWinding,
    kEvenOdd,
    kInverseWinding,
    kInverseEvenOdd,
};

static inline bool SkPathFillType_IsInverse(SkPathFillType ft) {
    static_assert(0 == (int)SkPathFillType::kWinding, "fill_type_mismatch");
    static_assert(1 == (int)SkPathFillType::kEvenOdd, "fill_type_mismatch");
    static_assert(2 == (int)SkPathFillType::kInverseWinding, "fill_type_mismatch");
    static_assert(3 == (int)SkPathFillType::kInverseEvenOdd, "fill_type_mismatch");
    return ((unsigned)ft & 2) != 0;
}

static SkPathFillType SkPathFillType_ConverToNonInverse(SkPathFillType ft) {
    return (SkPathFillType)((unsigned)ft & 1);
}

#endif
