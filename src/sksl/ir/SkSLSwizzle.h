/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

#include "SkSLContext.h"
#include "SkSLExpression.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * Given a type and a swizzle component count, returns the type that will result from swizzling. For
 * instance, swizzling a vec3 with two components will result in a vec2. It is possible to swizzle
 * with more components than the source vector, as in 'vec2(1).xxxx'.
 */
static const Type& get_type(const Context& context, Expression& value, size_t count) {
    const Type& base = value.fType.componentType();
    if (count == 1) {
        return base;
    }
    if (base == *context.fFloat_Type) {
        switch (count) {
            case 2: return *context.fVec2_Type;
            case 3: return *context.fVec3_Type;
            case 4: return *context.fVec4_Type;
        }
    } else if (base == *context.fDouble_Type) {
        switch (count) {
            case 2: return *context.fDVec2_Type;
            case 3: return *context.fDVec3_Type;
            case 4: return *context.fDVec4_Type;
        }
    } else if (base == *context.fInt_Type) {
        switch (count) {
            case 2: return *context.fIVec2_Type;
            case 3: return *context.fIVec3_Type;
            case 4: return *context.fIVec4_Type;
        }
    } else if (base == *context.fUInt_Type) {
        switch (count) {
            case 2: return *context.fUVec2_Type;
            case 3: return *context.fUVec3_Type;
            case 4: return *context.fUVec4_Type;
        }
    } else if (base == *context.fBool_Type) {
        switch (count) {
            case 2: return *context.fBVec2_Type;
            case 3: return *context.fBVec3_Type;
            case 4: return *context.fBVec4_Type;
        }
    }
    ABORT("cannot swizzle %s\n", value.description().c_str());
}

/**
 * Represents a vector swizzle operation such as 'vec2(1, 2, 3).zyx'.
 */
struct Swizzle : public Expression {
    Swizzle(const Context& context, std::unique_ptr<Expression> base, std::vector<int> components)
    : INHERITED(base->fPosition, kSwizzle_Kind, get_type(context, *base, components.size()))
    , fBase(std::move(base))
    , fComponents(std::move(components)) {
        ASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
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
