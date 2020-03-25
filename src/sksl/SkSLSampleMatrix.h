/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLString.h"

class GrFragmentProcessor;

namespace SkSL {

struct Expression;

struct SampleMatrix {
    enum class Kind {
        kNone,
        kConstantOrUniform,
        kVariable
    };

    SampleMatrix()
    : fOwner(nullptr)
    , fKind(Kind::kNone) {}

    SampleMatrix(Kind kind)
    : fOwner(nullptr)
    , fKind(kind) {
        SkASSERT(kind == Kind::kNone || kind == Kind::kVariable);
    }

    SampleMatrix(GrFragmentProcessor* owner, String expression)
    : fOwner(owner)
    , fKind(Kind::kConstantOrUniform)
    , fExpression(expression) {}

    SampleMatrix merge(const SampleMatrix& other);

    GrFragmentProcessor* fOwner;
    Kind fKind;
    // defined only when fKind is kConstant or kUniform
    String fExpression;
};

} // namespace
