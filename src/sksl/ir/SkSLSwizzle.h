/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_SWIZZLE
#define SKSL_SWIZZLE

#include "SkSLExpression.h"
#include "SkSLUtil.h"

namespace SkSL {

/**
 * Given a type and a swizzle component count, returns the type that will result from swizzling. For 
 * instance, swizzling a vec3 with two components will result in a vec2. It is possible to swizzle
 * with more components than the source vector, as in 'vec2(1).xxxx'.
 */
static std::shared_ptr<Type> get_type(Expression& value, 
                                      size_t count) {
    std::shared_ptr<Type> base = value.fType->componentType();
    if (count == 1) {
        return base;
    }
    if (base == kFloat_Type) {
        switch (count) {
            case 2: return kVec2_Type;
            case 3: return kVec3_Type;
            case 4: return kVec4_Type;
        }
    } else if (base == kDouble_Type) {
        switch (count) {
            case 2: return kDVec2_Type;
            case 3: return kDVec3_Type;
            case 4: return kDVec4_Type;
        }
    } else if (base == kInt_Type) {
        switch (count) {
            case 2: return kIVec2_Type;
            case 3: return kIVec3_Type;
            case 4: return kIVec4_Type;
        }
    } else if (base == kUInt_Type) {
        switch (count) {
            case 2: return kUVec2_Type;
            case 3: return kUVec3_Type;
            case 4: return kUVec4_Type;
        }
    } else if (base == kBool_Type) {
        switch (count) {
            case 2: return kBVec2_Type;
            case 3: return kBVec3_Type;
            case 4: return kBVec4_Type;
        }
    }
    ABORT("cannot swizzle %s\n", value.description().c_str());
}

/**
 * Represents a vector swizzle operation such as 'vec2(1, 2, 3).zyx'.
 */
struct Swizzle : public Expression {
    Swizzle(std::unique_ptr<Expression> base, std::vector<int> components)
    : INHERITED(base->fPosition, kSwizzle_Kind, get_type(*base, components.size()))
    , fBase(std::move(base))
    , fComponents(std::move(components)) {
        ASSERT(fComponents.size() >= 1 && fComponents.size() <= 4);
    }

    std::string description() const override {
        std::string result = fBase->description() + ".";
        for (int x : fComponents) {
            result += "xyzw"[x];
        }
        return result;
    }

    const std::unique_ptr<Expression> fBase;
    const std::vector<int> fComponents;

    typedef Expression INHERITED;
};

} // namespace

#endif
