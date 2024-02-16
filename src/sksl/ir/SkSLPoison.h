/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSLPoison_DEFINED
#define SkSLPoison_DEFINED

#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

class Poison : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kPoison;

    static std::unique_ptr<Expression> Make(Position pos, const Context& context) {
        return std::make_unique<Poison>(pos, context.fTypes.fPoison.get());
    }

    Poison(Position pos, const Type* type)
        : INHERITED(pos, kIRNodeKind, type) {}

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::make_unique<Poison>(pos, &this->type());
    }

    std::string description(OperatorPrecedence) const override {
        return Compiler::POISON_TAG;
    }

private:
    using INHERITED = Expression;
};

} // namespace SkSL

#endif  // SkSLPoison_DEFINED
