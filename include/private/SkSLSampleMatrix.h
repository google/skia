/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLSampleMatrix_DEFINED
#define SkSLSampleMatrix_DEFINED

#include <string>

namespace SkSL {

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
        kVariable
    };

    // Make a SampleMatrix with kNone for its kind. Will not have an expression or have perspective.
    // Represents sample(child, color) and sample(child, color, float2) calls.
    SampleMatrix()
            : fKind(Kind::kNone)
            , fHasPerspective(false) {}

    // This corresponds to sample(child, color, matrix) calls where every call site in the FP has
    // the same constant or uniform.
    static SampleMatrix MakeConstUniform(std::string expression, bool hasPerspective=true) {
        return SampleMatrix(Kind::kConstantOrUniform, std::move(expression), hasPerspective);
    }

    // This corresponds to sample(child, color, matrix) where the 3rd argument is an expression,
    // or where the constants/uniforms are not the same at all call sites in the FP.
    static SampleMatrix MakeVariable(bool hasPerspective=true) {
        return SampleMatrix(Kind::kVariable, "", hasPerspective);
    }

    static SampleMatrix Make(const Program& program, const Variable& fp);

    SampleMatrix merge(const SampleMatrix& other);

    bool operator==(const SampleMatrix& other) const {
        return fKind == other.fKind && fExpression == other.fExpression &&
               fHasPerspective == other.fHasPerspective;
    }

    bool isNoOp() const { return fKind == Kind::kNone; }
    bool isConstUniform() const { return fKind == Kind::kConstantOrUniform; }
    bool isVariable() const { return fKind == Kind::kVariable; }

    Kind fKind;
    // The constant or uniform expression representing the matrix (will be the empty string when
    // kind == kNone or kVariable)
    std::string fExpression;

    // FIXME: We can expand this to track a more general matrix type to allow for optimizations on
    // identity or scale+translate matrices too.
    bool fHasPerspective;

private:
    SampleMatrix(Kind kind, std::string expression, bool hasPerspective)
            : fKind(kind)
            , fExpression(expression)
            , fHasPerspective(hasPerspective) {}
};

} // namespace

#endif
