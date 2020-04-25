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
        // There is a matrix applied at runtime
        kVariable_Flag = 1 << 0,
        // The fExpression field contains a matrix to be applied in the vertex shader
        kVertex_Flag   = 1 << 1,
        // The fExpression field contains a matrix to be applied in the fragment shader
        kFragment_Flag = 1 << 2

        // kVertex_Flag and kFragment_Flag are mutually exclusive, but either can be combined with
        // kVariable_Flag
    };

    SampleMatrix()
    : fFlags(0)
    , fOwner(nullptr) {}

    SampleMatrix(int flags)
    : fFlags(flags) {
        SkASSERT((flags & kVertex_Flag) == 0 || (flags & kFragment_Flag) == 0);
        SkASSERT((flags & kVariable_Flag) == 0 || (flags & kVertex_Flag) == 0);
    }

    SampleMatrix(int flags, GrFragmentProcessor* owner)
    : fFlags(flags)
    , fOwner(owner) {
        SkASSERT((flags & kVertex_Flag) == 0 || (flags & kFragment_Flag) == 0);
        SkASSERT((flags & kVariable_Flag) == 0 || (flags & kVertex_Flag) == 0);
    }

    SampleMatrix(int flags, GrFragmentProcessor* owner, String expression)
    : fFlags(flags)
    , fOwner(owner)
    , fExpression(expression) {
        SkASSERT((flags & kVertex_Flag) == 0 || (flags & kFragment_Flag) == 0);
        SkASSERT((flags & kVariable_Flag) == 0 || (flags & kVertex_Flag) == 0);
    }

    SampleMatrix merge(const SampleMatrix& other);

    bool operator==(const SampleMatrix& other) const {
        return fFlags == other.fFlags && fExpression == other.fExpression;
    }

    bool operator!=(const SampleMatrix& other) const {
        return !(*this == other);
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
