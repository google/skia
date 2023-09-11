/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONSTRUCTOR_STRUCT
#define SKSL_CONSTRUCTOR_STRUCT

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
 * Represents the construction of an struct object, such as "Color(red, green, blue, 1)".
 */
class ConstructorStruct final : public MultiArgumentConstructor {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kConstructorStruct;

    ConstructorStruct(Position pos, const Type& type, ExpressionArray arguments)
        : INHERITED(pos, kIRNodeKind, &type, std::move(arguments)) {}

    // ConstructorStruct::Convert will typecheck and create struct-constructor expressions.
    // Reports errors via the ErrorReporter; returns null on error.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               const Type& type,
                                               ExpressionArray args);

    // ConstructorStruct::Make creates struct-constructor expressions; errors reported via SkASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            const Type& type,
                                            ExpressionArray args);

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<ConstructorStruct>(pos, this->type(), this->arguments().clone());
    }

private:
    using INHERITED = MultiArgumentConstructor;
};

}  // namespace SkSL

#endif
