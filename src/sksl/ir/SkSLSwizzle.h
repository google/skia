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

// represents a swizzle component of constant 0, as in x.rgb0
const int SKSL_SWIZZLE_0 = -2;

// represents a swizzle component of constant 1, as in x.rgb1
const int SKSL_SWIZZLE_1 = -1;

/**
 * Given a type and a swizzle component count, returns the type that will result from swizzling. For
 * instance, swizzling a float3 with two components will result in a float2. It is possible to
 * swizzle with more components than the source vector, as in 'float2(1).xxxx'.
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
    } else if (base == *context.fByte_Type) {
        switch (count) {
            case 2: return *context.fByte2_Type;
            case 3: return *context.fByte3_Type;
            case 4: return *context.fByte4_Type;
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
    } else if (base == *context.fUByte_Type) {
        switch (count) {
            case 2: return *context.fUByte2_Type;
            case 3: return *context.fUByte3_Type;
            case 4: return *context.fUByte4_Type;
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
 * Represents a vector swizzle operation such as 'float2(1, 2, 3).zyx'.
 */
struct Swizzle : public Expression {
    Swizzle(const Context& context, std::unique_ptr<Expression> base, std::vector<int> components)
    : INHERITED(base->fOffset, kSwizzle_Kind, get_type(context, *base, components.size()))
    , fBase(std::move(base))
    , fComponents(std::move(components)) {
        SkASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
    }

    std::unique_ptr<Expression> constantPropagate(const IRGenerator& irGenerator,
                                                  const DefinitionMap& definitions) override {
        if (fBase->fKind == Expression::kConstructor_Kind && fBase->isConstant()) {
            // we're swizzling a constant vector, e.g. float4(1).x. Simplify it.
            SkASSERT(fBase->fKind == Expression::kConstructor_Kind);
            if (fType.isInteger()) {
                SkASSERT(fComponents.size() == 1);
                int64_t value = ((Constructor&) *fBase).getIVecComponent(fComponents[0]);
                return std::unique_ptr<Expression>(new IntLiteral(irGenerator.fContext,
                                                                  -1,
                                                                  value));
            } else if (fType.isFloat()) {
                SkASSERT(fComponents.size() == 1);
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

    std::unique_ptr<Expression> clone() const override {
        return std::unique_ptr<Expression>(new Swizzle(fType, fBase->clone(), fComponents));
    }

    String description() const override {
        String result = fBase->description() + ".";
        for (int x : fComponents) {
            result += "01xyzw"[x + 2];
        }
        return result;
    }

    std::unique_ptr<Expression> fBase;
    std::vector<int> fComponents;

    typedef Expression INHERITED;

private:
    Swizzle(const Type& type, std::unique_ptr<Expression> base, std::vector<int> components)
    : INHERITED(base->fOffset, kSwizzle_Kind, type)
    , fBase(std::move(base))
    , fComponents(std::move(components)) {
        SkASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
    }


};

} // namespace

#endif
