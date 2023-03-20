/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLIRNode.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace SkSL {

class Context;
enum class OperatorPrecedence : uint8_t;

/**
 * Represents a vector swizzle operation such as 'float3(1, 2, 3).zyx'.
 */
class Swizzle final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSwizzle;

    Swizzle(const Context& context, Position pos, std::unique_ptr<Expression> base,
            const ComponentArray& components)
            : INHERITED(pos, kIRNodeKind,
                        &base->type().componentType().toCompound(context, components.size(), 1))
            , fBase(std::move(base))
            , fComponents(components) {
        SkASSERT(this->components().size() >= 1 && this->components().size() <= 4);
    }

    // Swizzle::Convert permits component arrays containing ZERO or ONE, does typechecking, reports
    // errors via ErrorReporter, and returns an expression that combines constructors and native
    // swizzles (comprised solely of X/Y/W/Z).
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               Position maskPos,
                                               std::unique_ptr<Expression> base,
                                               ComponentArray inComponents);

    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               Position maskPos,
                                               std::unique_ptr<Expression> base,
                                               std::string_view maskString);

    // Swizzle::Make does not permit ZERO or ONE in the component array, just X/Y/Z/W; errors are
    // reported via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
                                            Position pos,
                                            std::unique_ptr<Expression> expr,
                                            ComponentArray inComponents);

    std::unique_ptr<Expression>& base() {
        return fBase;
    }

    const std::unique_ptr<Expression>& base() const {
        return fBase;
    }

    const ComponentArray& components() const {
        return fComponents;
    }

    std::unique_ptr<Expression> clone(Position pos) const override {
        return std::unique_ptr<Expression>(new Swizzle(pos, &this->type(), this->base()->clone(),
                                                       this->components()));
    }

    std::string description(OperatorPrecedence) const override;

private:
    Swizzle(Position pos, const Type* type, std::unique_ptr<Expression> base,
            const ComponentArray& components)
        : INHERITED(pos, kIRNodeKind, type)
        , fBase(std::move(base))
        , fComponents(components) {
        SkASSERT(this->components().size() >= 1 && this->components().size() <= 4);
    }

    std::unique_ptr<Expression> fBase;
    ComponentArray fComponents;

    using INHERITED = Expression;
};

}  // namespace SkSL

#endif
