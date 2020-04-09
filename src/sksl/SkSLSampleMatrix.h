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

struct SampleMatrix {
    enum class Kind {
        kNone,
        kConstantOrUniform,
        kVariable,
        // naturally ConstantOrUniform, but inherited Variable from a parent
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
    // the empty string when kind == kNone or kVariable
    String fExpression;
};

} // namespace

#endif
