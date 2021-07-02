/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ARRAYTYPE
#define SKSL_ARRAYTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class ArrayType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kArray;

    ArrayType(String name, const char* abbrev, const Type& componentType, int count)
        : INHERITED(std::move(name), abbrev, kTypeKind)
        , fComponentType(componentType)
        , fCount(count) {
        // Allow either explicitly-sized or unsized arrays.
        SkASSERT(count > 0 || count == kUnsizedArray);
        // Disallow multi-dimensional arrays.
        SkASSERT(!componentType.is<ArrayType>());
    }

    const Type& componentType() const {
        return fComponentType;
    }

    int count() const {
        return fCount;
    }

private:
    using INHERITED = Type;

    const Type& fComponentType;
    int fCount;
};

}  // namespace SkSL

#endif
