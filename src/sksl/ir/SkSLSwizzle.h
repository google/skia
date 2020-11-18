/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

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
    static constexpr Kind kExpressionKind = Kind::kSwizzle;

    Swizzle(const Context& context, std::unique_ptr<Expression> base,
            const ComponentArray& components)
            : INHERITED(base->fOffset, kExpressionKind,
                        &base->type().componentType().toCompound(context, components.size(), 1))
            , fBase(std::move(base))
            , fComponents(components) {
        SkASSERT(this->components().size() >= 1 && this->components().size() <= 4);
    }

    std::unique_ptr<Expression>& base() {
        return fBase;
    }

    const std::unique_ptr<Expression>& base() const {
        return fBase;
    }

    const ComponentArray& components() const {
        return fComponents;
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (this->base()->is<Constructor>()) {
            Constructor& constructor = this->base()->as<Constructor>();
            if (constructor.isCompileTimeConstant()) {
                // we're swizzling a constant vector, e.g. float4(1).x. Simplify it.
                const Type& type = this->type();
                if (type.isInteger()) {
                    SkASSERT(this->components().size() == 1);
                    int64_t value = constructor.getIVecComponent(this->components()[0]);
                    return std::make_unique<IntLiteral>(irGenerator.fContext, constructor.fOffset,
                                                        value);
                } else if (type.isFloat()) {
                    SkASSERT(this->components().size() == 1);
                    SKSL_FLOAT value = constructor.getFVecComponent(this->components()[0]);
                    return std::make_unique<FloatLiteral>(irGenerator.fContext, constructor.fOffset,
                                                          value);
                }
            }
        }
        return nullptr;
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
        : INHERITED(base->fOffset, kExpressionKind, type)
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
