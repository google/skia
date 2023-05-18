/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_VAR
#define SKSL_DSL_VAR

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLModifiers.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <cstdint>
#include <memory>
#include <string_view>

namespace SkSL::dsl {

/**
 * A function parameter.
 */
struct DSLParameter {
    Position fModifiersPos;
    SkSL::Modifiers fModifiers;
    const SkSL::Type* fType;
    Position fNamePosition;
    std::string_view fName;
    Position fPosition;
};

}  // namespace SkSL::dsl

#endif
