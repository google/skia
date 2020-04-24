/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLSampleMatrix_DEFINED
#define SkSLSampleMatrix_DEFINED

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLString.h"

class GrFragmentProcessor;

namespace SkSL {

struct Expression;

/**
 * Represents the matrix applied to a fragment processor by its parent's sample(child, matrix) call.
 */
struct SampleMatrix {
    enum Flags {
        kVariable_Flag = 1 << 0,
        kVertex_Flag   = 1 << 1,
        kFragment_Flag = 1 << 2
    };

    SampleMatrix()
    : fFlags(0)
    , fOwner(nullptr) {}

    SampleMatrix(int flags)
    : fFlags(flags) {}

    SampleMatrix(int flags, GrFragmentProcessor* owner)
    : fFlags(flags)
    , fOwner(owner) {}

    SampleMatrix(int flags, GrFragmentProcessor* owner, String expression)
    : fFlags(flags)
    , fOwner(owner)
    , fExpression(expression) {
        SkASSERT((flags & kVertex_Flag) == 0 || (flags & kFragment_Flag) == 0);
    }

    SampleMatrix merge(const SampleMatrix& other);

    bool operator==(const SampleMatrix& other) const {
        return fFlags == other.fFlags && fExpression == other.fExpression;
    }

    String description() const {
        return String::printf("SampleMatrix<%d, %p, '%s'>", fFlags, fOwner, fExpression.c_str());
    }

    int fFlags;
    GrFragmentProcessor* fOwner;
    String fExpression;
};

} // namespace

#endif
