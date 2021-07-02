/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SCALARTYPE
#define SKSL_SCALARTYPE

#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

class ScalarType : public Type {
public:
    static constexpr TypeKind kTypeKind = TypeKind::kScalar;

    ScalarType(const char* name, const char* abbrev, NumberKind numberKind, int8_t priority,
               int8_t bitWidth)
        : INHERITED(name, abbrev, kTypeKind)
        , fNumberKind(numberKind)
        , fPriority(priority)
        , fBitWidth(bitWidth) {}

    NumberKind numberKind() const {
        return fNumberKind;
    }

    int8_t priority() const {
        return fPriority;
    }

    int8_t bitWidth() const {
        return fBitWidth;
    }

private:
    using INHERITED = Type;

    NumberKind fNumberKind;
    int8_t fPriority;
    int8_t fBitWidth;
};

}  // namespace SkSL

#endif
