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
    enum class Kind {
        // No sample(child, matrix) call affects the FP.
        kNone,
        // The FP is sampled with a matrix whose value is fixed and based only on constants or
        // uniforms, and thus the transform can be hoisted to the vertex shader.
        kConstantOrUniform,
        // The FP is sampled with a non-constant/uniform value, or sampled multiple times, and
        // thus the transform cannot be hoisted to the vertex shader.
        kVariable,
        // The FP is sampled with a constant or uniform value, *and* also inherits a variable
        // transform from an ancestor. The transform cannot be hoisted to the vertex shader, and
        // both matrices need to be applied.
        kMixed,
    };

    SampleMatrix()
    : fOwner(nullptr)
    , fKind(Kind::kNone) {}

    SampleMatrix(Kind kind)
    : fOwner(nullptr)
    , fKind(kind) {
        SkASSERT(kind == Kind::kNone || kind == Kind::kVariable);
    }

    SampleMatrix(Kind kind, GrFragmentProcessor* owner, String expression)
    : fOwner(owner)
    , fKind(kind)
    , fExpression(expression) {}

    SampleMatrix merge(const SampleMatrix& other);

    bool operator==(const SampleMatrix& other) const {
        return fKind == other.fKind && fExpression == other.fExpression;
    }

#ifdef SK_DEBUG
    String description() {
        switch (fKind) {
            case Kind::kNone:
                return "SampleMatrix<None>";
            case Kind::kConstantOrUniform:
                return "SampleMatrix<ConstantOrUniform(" + fExpression + ")>";
            case Kind::kVariable:
                return "SampleMatrix<Variable>";
            case Kind::kMixed:
                return "SampleMatrix<Mixed(" + fExpression + ")>";
        }
    }
#endif

    GrFragmentProcessor* fOwner;
    Kind fKind;
    // The constant or uniform expression representing the matrix (will be the empty string when
    // kind == kNone or kVariable)
    String fExpression;
};

} // namespace

#endif
