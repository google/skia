/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VECTORTYPE
#define SKSL_VECTORTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class VectorType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kVector;

    VectorType(String name, const char* abbrev, const ScalarType& componentType, int8_t columns)
        : INHERITED(std::move(name), abbrev, kTypeKind)
        , fComponentType(componentType)
        , fColumns(columns) {
        SkASSERT(columns >= 2 && columns <= 4);
    }

    const ScalarType& componentType() const {
        return fComponentType;
    }

    int8_t columns() const {
        return fColumns;
    }

private:
    using INHERITED = Type;

    const ScalarType& fComponentType;
    int8_t fColumns;
};

}  // namespace SkSL

#endif
