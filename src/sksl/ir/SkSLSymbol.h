/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOL
#define SKSL_SYMBOL

#include "src/sksl/ir/SkSLIRNode.h"

namespace SkSL {

/**
 * Represents a symboltable entry.
 */
struct Symbol : public IRNode {
    enum Kind {
        kFunctionDeclaration_Kind,
        kUnresolvedFunction_Kind,
        kType_Kind,
        kVariable_Kind,
        kField_Kind,
        kExternal_Kind
    };

    Symbol(int offset, Kind kind, StringFragment name)
    : INHERITED(offset)
    , fKind(kind)
    , fName(name) {}

    ~Symbol() override {}

    /**
     *  Use is<T> to check the type of a symbol.
     *  e.g. replace `sym.fKind == Symbol::kVariable_Kind` with `sym.is<Variable>()`.
     */
    template <typename T>
    bool is() const {
        return this->fKind == T::kSymbolKind;
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

    Kind fKind;
    StringFragment fName;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
