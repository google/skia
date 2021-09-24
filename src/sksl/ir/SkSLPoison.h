/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"

namespace SkSL {

class Poison : public Expression {
public:
    static constexpr Kind kExpressionKind = Kind::kPoison;

    static std::unique_ptr<Expression> Make(int line, const Context& context) {
        return std::make_unique<Poison>(line, context.fTypes.fPoison.get());
    }

    Poison(int line, const Type* type)
        : INHERITED(line, kExpressionKind, type) {}

    bool hasProperty(Property property) const override {
        return false;
    }

    std::unique_ptr<Expression> clone() const override {
        return std::make_unique<Poison>(fLine, &this->type());
    }

    String description() const override {
        return Compiler::POISON_TAG;
    }

private:
    using INHERITED = Expression;
};

} // namespace SkSL
