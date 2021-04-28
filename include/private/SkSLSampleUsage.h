/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSLSampleUsage_DEFINED
#define SkSLSampleUsage_DEFINED

#include "include/core/SkTypes.h"

#include <string>

namespace SkSL {

/**
 * Represents all of the ways that a fragment processor is sampled by its parent.
 */
struct SampleUsage {
    // Make a SampleUsage that corresponds to no sampling of the child at all
    SampleUsage() = default;

    // Child is sampled with a matrix whose value is uniform (some expression only involving
    // literals and uniform variables).
    static SampleUsage UniformMatrix(std::string expression, bool hasPerspective = true) {
        SkASSERT(!expression.empty());
        return SampleUsage(std::move(expression), hasPerspective, false, false);
    }

    static SampleUsage Explicit() {
        return SampleUsage("", false, true, false);
    }

    static SampleUsage PassThrough() {
        return SampleUsage("", false, false, true);
    }

    SampleUsage merge(const SampleUsage& other);

    bool isSampled() const {
        return this->hasUniformMatrix() || fExplicitCoords || fPassThrough;
    }

    bool hasUniformMatrix()  const { return !fExpression.empty(); }

    // The uniform expression representing the matrix, or empty for non-matrix sampling
    std::string fExpression;
    bool fHasPerspective = false;

    bool fExplicitCoords = false;
    bool fPassThrough    = false;

    SampleUsage(std::string expression,
                bool hasPerspective,
                bool explicitCoords,
                bool passThrough)
            : fExpression(expression)
            , fHasPerspective(hasPerspective)
            , fExplicitCoords(explicitCoords)
            , fPassThrough(passThrough) {}

    std::string constructor() const;
};

}  // namespace SkSL

#endif
