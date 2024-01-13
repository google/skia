/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERFACEBLOCK
#define SKSL_INTERFACEBLOCK

#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Context;
struct Modifiers;

/**
 * An interface block, as in:
 *
 * out sk_PerVertex {
 *   layout(builtin=0) float4 sk_Position;
 *   layout(builtin=1) float sk_PointSize;
 * };
 *
 * At the IR level, this is represented by a single variable of struct type.
 */
class InterfaceBlock final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kInterfaceBlock;

    InterfaceBlock(Position pos, Variable* var)
            : INHERITED(pos, kIRNodeKind)
            , fVariable(var) {
        SkASSERT(fVariable->type().componentType().isInterfaceBlock());
        fVariable->setInterfaceBlock(this);
    }

    ~InterfaceBlock() override;

    // Returns an InterfaceBlock; errors are reported to the ErrorReporter.
    // The caller is responsible for adding the InterfaceBlock to the program elements.
    // The program's RTAdjustData will be updated if the InterfaceBlock contains sk_RTAdjust.
    // The passed-in symbol table will be updated with a reference to the interface block variable
    // (if it is named) or each of the interface block fields (if it is anonymous).
    static std::unique_ptr<InterfaceBlock> Convert(const Context& context,
                                                   Position pos,
                                                   const Modifiers& modifiers,
                                                   std::string_view typeName,
                                                   skia_private::TArray<Field> fields,
                                                   std::string_view varName,
                                                   int arraySize);

    // Returns an InterfaceBlock; errors are reported via SkASSERT.
    // The caller is responsible for adding the InterfaceBlock to the program elements.
    // The passed-in symbol table will be updated with a reference to the interface block variable
    // (if it is named) or each of the interface block fields (if it is anonymous).
    static std::unique_ptr<InterfaceBlock> Make(const Context& context,
                                                Position pos,
                                                Variable* variable);

    Variable* var() const {
        return fVariable;
    }

    void detachDeadVariable() {
        fVariable = nullptr;
    }

    std::string_view typeName() const {
        return fVariable->type().componentType().name();
    }

    std::string_view instanceName() const {
        return fVariable->name();
    }

    int arraySize() const {
        return fVariable->type().isArray() ? fVariable->type().columns() : 0;
    }

    std::string description() const override;

private:
    Variable* fVariable;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
