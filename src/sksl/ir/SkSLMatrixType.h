/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MATRIXTYPE
#define SKSL_MATRIXTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class MatrixType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kMatrix;

    MatrixType(String name, const char* abbrev, const ScalarType& componentType, int8_t columns,
               int8_t rows)
        : INHERITED(std::move(name), abbrev, kTypeKind)
        , fComponentType(componentType)
        , fColumns(columns)
        , fRows(rows) {
        SkASSERT(columns >= 2 && columns <= 4);
        SkASSERT(rows >= 2 && rows <= 4);
    }

    const ScalarType& componentType() const {
        return fComponentType;
    }

    int8_t columns() const {
        return fColumns;
    }

    int8_t rows() const {
        return fRows;
    }

private:
    using INHERITED = Type;

    const ScalarType& fComponentType;
    int8_t fColumns;
    int8_t fRows;
};

}  // namespace SkSL

#endif
