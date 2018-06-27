/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

#include "SkSLConstructor.h"
#include "SkSLContext.h"
#include "SkSLExpression.h"
#include "SkSLIRGenerator.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * Given a type and a swizzle component count, returns the type that will result from swizzling. For
 * instance, swizzling a float3with two components will result in a float2 It is possible to swizzle
 * with more components than the source vector, as in 'float21).xxxx'.
 */
static const Type& get_type(const Context& context, Expression& value, size_t count) {
    const Type& base = value.fType.componentType();
    if (count == 1) {
        return base;
    }
    if (base == *context.fFloat_Type) {
        switch (count) {
            case 2: return *context.fFloat2_Type;
            case 3: return *context.fFloat3_Type;
            case 4: return *context.fFloat4_Type;
        }
    } else if (base == *context.fHalf_Type) {
        switch (count) {
            case 2: return *context.fHalf2_Type;
            case 3: return *context.fHalf3_Type;
            case 4: return *context.fHalf4_Type;
        }
    } else if (base == *context.fDouble_Type) {
        switch (count) {
            case 2: return *context.fDouble2_Type;
            case 3: return *context.fDouble3_Type;
            case 4: return *context.fDouble4_Type;
        }
    } else if (base == *context.fInt_Type) {
        switch (count) {
            case 2: return *context.fInt2_Type;
            case 3: return *context.fInt3_Type;
            case 4: return *context.fInt4_Type;
        }
    } else if (base == *context.fShort_Type) {
        switch (count) {
            case 2: return *context.fShort2_Type;
            case 3: return *context.fShort3_Type;
            case 4: return *context.fShort4_Type;
        }
    } else if (base == *context.fUInt_Type) {
        switch (count) {
            case 2: return *context.fUInt2_Type;
            case 3: return *context.fUInt3_Type;
            case 4: return *context.fUInt4_Type;
        }
    } else if (base == *context.fUShort_Type) {
        switch (count) {
            case 2: return *context.fUShort2_Type;
            case 3: return *context.fUShort3_Type;
            case 4: return *context.fUShort4_Type;
        }
    } else if (base == *context.fBool_Type) {
        switch (count) {
            case 2: return *context.fBool2_Type;
            case 3: return *context.fBool3_Type;
            case 4: return *context.fBool4_Type;
        }
    }
    ABORT("cannot swizzle %s\n", value.description().c_str());
}

/**
 * Represents a vector swizzle operation such as 'float21, 2, 3).zyx'.
 */
struct Swizzle : public Expression {
    Swizzle(const Context& context, std::unique_ptr<Expression> base, std::vector<int> components)
    : INHERITED(base->fOffset, kSwizzle_Kind, get_type(context, *base, components.size()))
    , fBase(std::move(base))
    , fComponents(std::move(components)) {
        ASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fBase->fKind == Expression::kConstructor_Kind && fBase->isConstant()) {
            // we're swizzling a constant vector, e.g. float4(1).x. Simplify it.
            ASSERT(fBase->fKind == Expression::kConstructor_Kind);
            if (fType == *irGenerator.fContext.fInt_Type) {
                ASSERT(fComponents.size() == 1);
                int64_t value = ((Constructor&) *fBase).getIVecComponent(fComponents[0]);
                return std::unique_ptr<Expression>(new IntLiteral(irGenerator.fContext,
                                                                  -1,
                                                                  value));
            } else if (fType == *irGenerator.fContext.fFloat_Type) {
                ASSERT(fComponents.size() == 1);
                double value = ((Constructor&) *fBase).getFVecComponent(fComponents[0]);
                return std::unique_ptr<Expression>(new FloatLiteral(irGenerator.fContext,
                                                                    -1,
                                                                    value));
            }
        }
        return nullptr;
    }

    bool hasSideEffects() const override {
        return fBase->hasSideEffects();
    }

    String description() const override {
        String result = fBase->description() + ".";
        for (int x : fComponents) {
            result += "xyzw"[x];
        }
        return result;
    }

    std::unique_ptr<Expression> fBase;
    const std::vector<int> fComponents;

    typedef Expression INHERITED;
};

} // namespace

#endif
