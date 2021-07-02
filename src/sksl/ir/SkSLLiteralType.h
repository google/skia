/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_LITERALTYPE
#define SKSL_LITERALTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class ScalarType;

class LiteralType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kLiteral;

    LiteralType(const char* name, const ScalarType& scalarType, int8_t priority)
        : INHERITED(name, "L", kTypeKind)
        , fScalarType(scalarType)
        , fPriority(priority) {}

    const ScalarType& scalarType() const {
        return fScalarType;
    }

    int8_t priority() const {
        return fPriority;
    }

private:
    using INHERITED = Type;

    const ScalarType& fScalarType;
    int8_t fPriority;
};

}  // namespace SkSL

#endif
