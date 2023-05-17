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

namespace SkSL {
namespace dsl {

struct DSLVarBase {
    /** Holds the data needed to declare a new variable with the specified type and name. */
    DSLVarBase(VariableStorage storage,
               const DSLModifiers& modifiers,
               DSLType type,
               std::string_view name,
               DSLExpression initialValue,
               Position pos,
               Position namePos)
            : fModifiersPos(modifiers.fPosition)
            , fModifiers(modifiers.fModifiers)
            , fType(std::move(type))
            , fNamePosition(namePos)
            , fName(name)
            , fInitialValue(std::move(initialValue))
            , fPosition(pos)
            , fStorage(storage) {}

    Position fModifiersPos;
    SkSL::Modifiers fModifiers;
    DSLType fType;
    std::unique_ptr<SkSL::Statement> fDeclaration;
    SkSL::Variable* fVar = nullptr;
    Position fNamePosition;
    std::string_view fName;
    DSLExpression fInitialValue;
    Position fPosition;
    VariableStorage fStorage;
};

/**
 * A local variable.
 */
struct DSLVar : public DSLVarBase {
    DSLVar(const DSLModifiers& modifiers,
           DSLType type,
           std::string_view name,
           DSLExpression initialValue = DSLExpression(),
           Position pos = {},
           Position namePos = {})
            : DSLVarBase(SkSL::VariableStorage::kLocal,
                         modifiers,
                         type,
                         name,
                         std::move(initialValue),
                         pos,
                         namePos) {}

    DSLVar(DSLVar&&) = default;
};

/**
 * A global variable.
 */
struct DSLGlobalVar : public DSLVarBase {
    DSLGlobalVar(const DSLModifiers& modifiers,
                 DSLType type,
                 std::string_view name,
                 DSLExpression initialValue = DSLExpression(),
                 Position pos = {},
                 Position namePos = {})
            : DSLVarBase(SkSL::VariableStorage::kGlobal,
                         modifiers,
                         type,
                         name,
                         std::move(initialValue),
                         pos,
                         namePos) {}

    DSLGlobalVar(DSLGlobalVar&&) = default;
};

/**
 * A function parameter.
 */
struct DSLParameter : public DSLVarBase {
    DSLParameter(const DSLModifiers& modifiers,
                 DSLType type,
                 std::string_view name,
                 Position pos = {},
                 Position namePos = {})
            : DSLVarBase(SkSL::VariableStorage::kParameter,
                         modifiers,
                         type,
                         name,
                         DSLExpression(),
                         pos,
                         namePos) {}

    DSLParameter(DSLParameter&&) = default;
};

} // namespace dsl
} // namespace SkSL

#endif
