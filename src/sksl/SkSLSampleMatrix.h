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
struct Program;
struct Variable;

/**
 * Represents the matrix applied to a fragment processor by its parent's sample(child, matrix) call.
 */
struct SampleMatrix {
    enum class Kind {
        // No sample(child, matrix) call affects the FP.
        kNone,
        // The FP is sampled with a matrix whose value is fixed and based only on constants or
        // uniforms, and thus the transform can be hoisted to the vertex shader (assuming that
        // its parent can also be hoisted, i.e. not sampled explicitly).
        kConstantOrUniform,
        // The FP is sampled with a non-constant/uniform value, or sampled multiple times, and
        // thus the transform cannot be hoisted to the vertex shader.
        kVariable,
        // The FP is sampled with a constant or uniform value, *and* also inherits a variable
        // transform from an ancestor. The transform cannot be hoisted to the vertex shader, and
        // both matrices need to be applied.
        kMixed,
    };

    // Make a SampleMatrix with kNone for its kind. Will not have an expression or have perspective.
    SampleMatrix()
            : fOwner(nullptr)
            , fKind(Kind::kNone) {}

    static SampleMatrix MakeConstUniform(String expression) {
        return SampleMatrix(Kind::kConstantOrUniform, expression);
    }

    static SampleMatrix MakeVariable() {
        return SampleMatrix(Kind::kVariable, "");
    }

    static SampleMatrix Make(const Program& program, const Variable& fp);

    SampleMatrix merge(const SampleMatrix& other);

    bool operator==(const SampleMatrix& other) const {
        return fKind == other.fKind && fExpression == other.fExpression && fOwner == other.fOwner;
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

    // TODO(michaelludwig): fOwner and fBase are going away; owner is filled in automatically when
    // a matrix-sampled FP is registered as a child.
    GrFragmentProcessor* fOwner;
    Kind fKind;
    // The constant or uniform expression representing the matrix (will be the empty string when
    // kind == kNone or kVariable)
    String fExpression;
    const GrFragmentProcessor* fBase = nullptr;

private:
    SampleMatrix(Kind kind, String expression)
            : fOwner(nullptr)
            , fKind(kind)
            , fExpression(expression) {}
};

} // namespace

#endif
