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
    enum class Kind {
        // Child is never sampled
        kNone,
        // Child is only sampled at the same coordinates as the parent
        kPassThrough,
        // Child is sampled with a matrix whose value is uniform
        kUniformMatrix,
        // Child is sampled using explicit coordinates
        kExplicit,
    };

    // Make a SampleUsage that corresponds to no sampling of the child at all
    SampleUsage() = default;

    // Child is sampled with a matrix whose value is uniform (some expression only involving
    // literals and uniform variables).
    static SampleUsage UniformMatrix(std::string expression, bool hasPerspective = true) {
        return SampleUsage(Kind::kUniformMatrix, std::move(expression), hasPerspective);
    }

    static SampleUsage Explicit() {
        return SampleUsage(Kind::kExplicit, "", false);
    }

    static SampleUsage PassThrough() {
        return SampleUsage(Kind::kPassThrough, "", false);
    }

    SampleUsage merge(const SampleUsage& other);

    bool isSampled()       const { return fKind != Kind::kNone; }
    bool isPassThrough()   const { return fKind == Kind::kPassThrough; }
    bool isExplicit()      const { return fKind == Kind::kExplicit; }
    bool isUniformMatrix() const { return fKind == Kind::kUniformMatrix; }

    Kind fKind = Kind::kNone;
    // The uniform expression representing the matrix, or empty for non-matrix sampling
    std::string fExpression;
    bool fHasPerspective = false;

    SampleUsage(Kind kind, std::string expression, bool hasPerspective)
            : fKind(kind), fExpression(expression), fHasPerspective(hasPerspective) {
        if (kind == Kind::kUniformMatrix) {
            SkASSERT(!fExpression.empty());
        } else {
            SkASSERT(fExpression.empty() && !fHasPerspective);
        }
    }

    std::string constructor() const;
};

}  // namespace SkSL

#endif
