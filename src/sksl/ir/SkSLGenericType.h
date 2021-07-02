/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GENERICTYPE
#define SKSL_GENERICTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * A generic type which maps to the specified types - e.g. $genType is a generic type which can
 * match float, float2, float3 or float4.
 */
class GenericType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kGeneric;

    GenericType(const char* name, std::vector<const Type*> coercibleTypes)
        : INHERITED(name, "G", kTypeKind)
        , fCoercibleTypes(std::move(coercibleTypes)) {}

    const std::vector<const Type*>& coercibleTypes() const {
        return fCoercibleTypes;
    }

private:
    using INHERITED = Type;

    std::vector<const Type*> fCoercibleTypes;
};

}  // namespace SkSL

#endif
