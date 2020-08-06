/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLSampleUsage_DEFINED
#define SkSLSampleUsage_DEFINED

#include <string>

namespace SkSL {

/**
 * Represents all of the ways that a fragment processor is sampled by its parent.
 */
struct SampleUsage {
    enum class Kind {
        // No sample(child, matrix) call affects the FP.
        kNone,
        // The FP is sampled with a matrix whose value is fixed and based only on literals or
        // uniforms, and thus the transform can be hoisted to the vertex shader (assuming that
        // its parent can also be hoisted, i.e. not sampled explicitly).
        kUniform,
        // The FP is sampled with a non-literal/uniform value, or matrix-sampled multiple times,
        // and thus the transform cannot be hoisted to the vertex shader.
        kVariable
    };

    // Make a SampleUsage that corresponds to no sampling of the child at all
    SampleUsage() = default;

    // This corresponds to sample(child, color, matrix) calls where every call site in the FP has
    // the same matrix, and that matrix's value is uniform (some expression only involving literals
    // and uniform variables).
    static SampleUsage UniformMatrix(std::string expression, bool hasPerspective = true) {
        return SampleUsage(Kind::kUniform, std::move(expression), hasPerspective, false, false);
    }

    // This corresponds to sample(child, color, matrix) where the 3rd argument is an expression that
    // can't be hoisted to the vertex shader, or where the expression used is not the same at all
    // call sites in the FP.
    static SampleUsage VariableMatrix(bool hasPerspective = true) {
        return SampleUsage(Kind::kVariable, "", hasPerspective, false, false);
    }

    static SampleUsage Explicit() {
        return SampleUsage(Kind::kNone, "", false, true, false);
    }

    static SampleUsage PassThrough() {
        return SampleUsage(Kind::kNone, "", false, false, true);
    }

    SampleUsage merge(const SampleUsage& other);

    bool isSampled() const {
        return this->hasMatrix() || fExplicitCoords || fPassThrough;
    }

    bool hasMatrix()         const { return fKind != Kind::kNone; }
    bool hasUniformMatrix()  const { return fKind == Kind::kUniform; }
    bool hasVariableMatrix() const { return fKind == Kind::kVariable; }

    Kind fKind = Kind::kNone;
    // The uniform expression representing the matrix (only valid when kind == kUniform)
    std::string fExpression;
    // FIXME: We can expand this to track a more general matrix type to allow for optimizations on
    // identity or scale+translate matrices too.
    bool fHasPerspective = false;

    bool fExplicitCoords = false;
    bool fPassThrough    = false;

    SampleUsage(Kind kind,
                std::string expression,
                bool hasPerspective,
                bool explicitCoords,
                bool passThrough)
            : fKind(kind)
            , fExpression(expression)
            , fHasPerspective(hasPerspective)
            , fExplicitCoords(explicitCoords)
            , fPassThrough(passThrough) {}

    std::string constructor(std::string perspectiveExpression) const;
};

}  // namespace SkSL

#endif
