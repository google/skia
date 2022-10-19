/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTERFACEBLOCK
#define SKSL_INTERFACEBLOCK

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace SkSL {

class SymbolTable;

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

    InterfaceBlock(Position pos,
                   Variable* var,
                   std::shared_ptr<SymbolTable> typeOwner)
            : INHERITED(pos, kIRNodeKind)
            , fVariable(var)
            , fTypeOwner(std::move(typeOwner)) {
        SkASSERT(fVariable->type().componentType().isInterfaceBlock());
        fVariable->setInterfaceBlock(this);
    }

    ~InterfaceBlock() override;

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

    const std::shared_ptr<SymbolTable>& typeOwner() const {
        return fTypeOwner;
    }

    int arraySize() const {
        return fVariable->type().isArray() ? fVariable->type().columns() : 0;
    }

    std::unique_ptr<ProgramElement> clone() const override;

    std::string description() const override;

private:
    Variable* fVariable;
    std::shared_ptr<SymbolTable> fTypeOwner;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
