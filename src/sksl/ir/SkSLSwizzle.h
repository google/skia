/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Represents a vector swizzle operation such as 'float3(1, 2, 3).zyx'.
 */
struct Swizzle final : public Expression {
    inline static constexpr Kind kExpressionKind = Kind::kSwizzle;

    Swizzle(const Context& context, std::unique_ptr<Expression> base,
            const ComponentArray& components)
            : INHERITED(base->fLine, kExpressionKind,
                        &base->type().componentType().toCompound(context, components.size(), 1))
            , fBase(std::move(base))
            , fComponents(components) {
        SkASSERT(this->components().size() >= 1 && this->components().size() <= 4);
    }

    // Swizzle::Convert permits component arrays containing ZERO or ONE, does typechecking, reports
    // errors via ErrorReporter, and returns an expression that combines constructors and native
    // swizzles (comprised solely of X/Y/W/Z).
    static std::unique_ptr<Expression> Convert(const Context& context,
                                               std::unique_ptr<Expression> base,
                                               ComponentArray inComponents);

    static std::unique_ptr<Expression> Convert(const Context& context,
                                               std::unique_ptr<Expression> base,
                                               skstd::string_view maskString);

    // Swizzle::Make does not permit ZERO or ONE in the component array, just X/Y/Z/W; errors are
    // reported via ASSERT.
    static std::unique_ptr<Expression> Make(const Context& context,
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

    bool hasProperty(Property property) const override {
        return this->base()->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new Swizzle(&this->type(), this->base()->clone(),
                                                       this->components()));
    }

    String description() const override {
        String result = this->base()->description() + ".";
        for (int x : this->components()) {
            result += "xyzw"[x];
        }
        return result;
    }

private:
    Swizzle(const Type* type, std::unique_ptr<Expression> base, const ComponentArray& components)
        : INHERITED(base->fLine, kExpressionKind, type)
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
