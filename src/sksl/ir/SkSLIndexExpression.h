/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INDEX
#define SKSL_INDEX

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class Type;
enum class OperatorPrecedence : uint8_t;

/**
 * An expression which extracts a value from an array, vector or matrix, as in 'm[2]'.
 */
class IndexExpression final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kIndex;

    IndexExpression(const Context& context, Position pos, std::unique_ptr<Expression> base,
                    std::unique_ptr<Expression> index)
        : INHERITED(pos, kIRNodeKind, &IndexType(context, base->type()))
        , fBase(std::move(base))
        , fIndex(std::move(index)) {}

    // Returns a simplified index-expression; reports errors via the ErrorReporter.
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               std::unique_ptr<Expression> base,
                                               std::unique_ptr<Expression> index);

    // Returns a simplified index-expression; reports errors via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
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

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::unique_ptr<Expression>(new IndexExpression(pos, this->base()->clone(),
                                                               this->index()->clone(),
                                                               &this->type()));
    }

    std::string description(OperatorPrecedence) const override;

    using INHERITED = Expression;

private:
    IndexExpression(Position pos, std::unique_ptr<Expression> base,
                    std::unique_ptr<Expression> index, const Type* type)
        : INHERITED(pos, Kind::kIndex, type)
        , fBase(std::move(base))
        , fIndex(std::move(index)) {}

    std::unique_ptr<Expression> fBase;
    std::unique_ptr<Expression> fIndex;
};

}  // namespace SkSL

#endif
