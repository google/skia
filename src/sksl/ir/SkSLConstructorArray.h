/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_ARRAY
#define SKSL_CONSTRUCTOR_ARRAY

#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>
#include <utility>

namespace SkSL {

class Context;
class Type;

/**
 * Represents the construction of an array type, such as "float[5](x, y, z, w, 1)".
 */
class ConstructorArray final : public MultiArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorArray;

    ConstructorArray(Position pos, const Type& type, ExpressionArray arguments)
        : INHERITED(pos, kIRNodeKind, &type, std::move(arguments)) {}

    // ConstructorArray::Convert will typecheck and create array-constructor expressions.
    // Reports errors via the ErrorReporter; returns null on error.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               const Type& type,
                                               ExpressionArray args);

    // ConstructorArray::Make creates array-constructor expressions; errors reported via SkASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            ExpressionArray args);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorArray>(pos, this->type(), this->arguments().clone());
    }

private:
    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
