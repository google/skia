/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

#include "include/core/SkTypes.h"
#include "src/base/SkFixedArray.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace SkSL {

class Context;
enum class OperatorPrecedence : uint8_t;

namespace SwizzleComponent {

enum Type : int8_t {
    X  =  0,  Y =  1,  Z =  2,  W =  3,
    R  =  4,  G =  5,  B =  6,  A =  7,
    S  =  8,  T =  9,  P = 10,  Q = 11,
    UL = 12, UT = 13, UR = 14, UB = 15,
    ZERO,
    ONE
};

}  // namespace SwizzleComponent

/**
 * Represents the components of a vector swizzle.
 */
using ComponentArray = skia_private::FixedArray<4, int8_t>;

/**
 * Represents a vector swizzle operation such as 'float3(1, 2, 3).zyx'.
 */
class Swizzle final : public Expression {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kSwizzle;

    using Component = SwizzleComponent::Type;

    Swizzle(const Context& context, Position pos, std::unique_ptr<Expression> base,
            const ComponentArray& components)
            : INHERITED(pos, kIRNodeKind,
                        &base->type().componentType().toCompound(context, components.size(), 1))
            , fBase(std::move(base))
            , fComponents(components) {
        SkASSERT(this->components().size() >= 1 && this->components().size() <= 4);
    }

    // Swizzle::Convert permits component strings containing ZERO or ONE, does typechecking, reports
    // errors via ErrorReporter, and returns an expression that combines constructors and native
    // swizzles (comprised solely of X/Y/W/Z).
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               Position pos,
                                               Position maskPos,
                                               std::unique_ptr<Expression> base,
                                               std::string_view componentString);

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

    // Converts an array of swizzle components into a string.
    static std::string MaskString(const ComponentArray& inComponents);

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
