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
 * Represents a vector swizzle operation such as 'float2(1, 2, 3).zyx'.
 */
struct Swizzle : public Expression {
    static constexpr Kind kExpressionKind = Kind::kSwizzle;

    Swizzle(const Context& context, std::unique_ptr<Expression> base, std::vector<int> components)
            : INHERITED(base->fOffset,
                        kExpressionKind,
                        &base->type().componentType().toCompound(context, components.size(), 1))
            , fBase(std::move(base))
            , fComponents(std::move(components)) {
        SkASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fBase->kind() == Expression::Kind::kConstructor) {
            Constructor& constructor = static_cast<Constructor&>(*fBase);
            if (constructor.isCompileTimeConstant()) {
                // we're swizzling a constant vector, e.g. float4(1).x. Simplify it.
                const Type& type = this->type();
                if (type.isInteger()) {
                    SkASSERT(fComponents.size() == 1);
                    int64_t value = constructor.getIVecComponent(fComponents[0]);
                    return std::make_unique<IntLiteral>(irGenerator.fContext, constructor.fOffset,
                                                        value);
                } else if (type.isFloat()) {
                    SkASSERT(fComponents.size() == 1);
                    SKSL_FLOAT value = constructor.getFVecComponent(fComponents[0]);
                    return std::make_unique<FloatLiteral>(irGenerator.fContext, constructor.fOffset,
                                                          value);
                }
            }
        }
        return nullptr;
    }

    bool hasProperty(Property property) const override {
        return fBase->hasProperty(property);
    }

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new Swizzle(&this->type(), fBase->clone(), fComponents));
    }

    String description() const override {
        String result = fBase->description() + ".";
        for (int x : fComponents) {
            result += "xyzw"[x];
        }
        return result;
    }

    std::unique_ptr<Expression> fBase;
    std::vector<int> fComponents;

    using INHERITED = Expression;

private:
    Swizzle(const Type* type, std::unique_ptr<Expression> base, std::vector<int> components)
    : INHERITED(base->fOffset, kExpressionKind, type)
    , fBase(std::move(base))
    , fComponents(std::move(components)) {
        SkASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
    }


};

}  // namespace SkSL

#endif
