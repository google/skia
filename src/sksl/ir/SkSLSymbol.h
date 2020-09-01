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
    enum class Kind {
        kFunctionDeclaration,
        kUnresolvedFunction,
        kType,
        kVariable,
        kField,
        kExternal
    };

    Symbol(int offset, Kind kind, StringFragment name)
    : INHERITED(offset, (int) kind)
    , fName(name) {}

    ~Symbol() override {}

    Kind kind() const {
        return (Kind) fKind;
    }

    /**
     *  Use as<T> to downcast symbols. e.g. replace `(Variable&) sym` with `sym.as<Variable>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->kind() == T::kSymbolKind);
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->kind() == T::kSymbolKind);
        return static_cast<T&>(*this);
    }

    StringFragment fName;

    typedef IRNode INHERITED;
};

}  // namespace SkSL

#endif
