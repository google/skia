/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INDEX
#define SKSL_INDEX

#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * An expression which extracts a value from an array or matrix, as in 'm[2]'.
 */
struct IndexExpression final : public Expression {
    inline static constexpr Kind kExpressionKind = Kind::kIndex;

    IndexExpression(const Context& context, std::unique_ptr<Expression> base,
                    std::unique_ptr<Expression> index)
        : INHERITED(base->fLine, kExpressionKind, &IndexType(context, base->type()))
        , fBase(std::move(base))
        , fIndex(std::move(index)) {}

    // Returns a simplified index-expression; reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               SymbolTable& symbolTable,
                                               std::unique_ptr<Expression> base,
                                               std::unique_ptr<Expression> index);

    // Returns a simplified index-expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            std::unique_ptr<Expression> base,
                                            std::unique_ptr<Expression> index);

    /**
     * Given a type, returns the type that will result from extracting an array value from it.
     */
    static const Type& IndexType(const Context& context, const Type& type);

    std::unique_ptr<Expression>& base() {
        return fBase;
    }

    const std::unique_ptr<Expression>& base() const {
        return fBase;
    }

    std::unique_ptr<Expression>& index() {
        return fIndex;
    }

    const std::unique_ptr<Expression>& index() const {
        return fIndex;
    }

    bool hasProperty(Property property) const override {
        return this->base()->hasProperty(property) || this->index()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new IndexExpression(this->base()->clone(),
                                                               this->index()->clone(),
                                                               &this->type()));
    }

    String description() const override {
        return this->base()->description() + "[" + this->index()->description() + "]";
    }

    using INHERITED = Expression;

private:
    IndexExpression(std::unique_ptr<Expression> base, std::unique_ptr<Expression> index,
                    const Type* type)
        : INHERITED(base->fLine, Kind::kIndex, type)
        , fBase(std::move(base))
        , fIndex(std::move(index)) {}

    std::unique_ptr<Expression> fBase;
    std::unique_ptr<Expression> fIndex;
};

}  // namespace SkSL

#endif
