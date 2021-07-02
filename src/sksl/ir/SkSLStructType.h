/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRUCTTYPE
#define SKSL_STRUCTTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class StructType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kStruct;

    StructType(int offset, String name, std::vector<Field> fields)
        : INHERITED(std::move(name), "S", kTypeKind, offset)
        , fFields(std::move(fields)) {}

    const std::vector<Field>& fields() const {
        return fFields;
    }

private:
    using INHERITED = Type;

    std::vector<Field> fFields;
};

}  // namespace SkSL

#endif
