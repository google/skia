/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOL
#define SKSL_SYMBOL

#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

/**
 * Represents a symboltable entry.
 */
struct Symbol : public IRNode {
    enum class Kind {
        kExternal = (int) ProgramElement::Kind::kLast + 1,
        kField,
        kFunctionDeclaration,
        kType,
        kUnresolvedFunction,
        kVariable,

        kFirst = kExternal,
        kLast = kVariable
    };

    Symbol(int offset, Kind kind, StringFragment name, const Type* type = nullptr)
    : INHERITED(offset, (int) kind, type)
    , fName(name) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Symbol(const Symbol&) = default;
    Symbol& operator=(const Symbol&) = default;

    ~Symbol() override {}

    Kind kind() const {
        return (Kind) fKind;
    }

    /**
     *  Use is<T> to check the type of a symbol.
     *  e.g. replace `sym.kind() == Symbol::Kind::kVariable` with `sym.is<Variable>()`.
     */
    template <typename T>
    bool is() const {
        return this->kind() == T::kSymbolKind;
    }

    /**
     *  Use as<T> to downcast symbols. e.g. replace `(Variable&) sym` with `sym.as<Variable>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->is<T>());
        return static_cast<T&>(*this);
    }

    StringFragment fName;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
