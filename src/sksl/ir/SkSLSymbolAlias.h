/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLALIAS
#define SKSL_SYMBOLALIAS

#include "src/sksl/ir/SkSLSymbol.h"

namespace SkSL {

/**
 * A symbol representing a new name for an existing symbol.
 */
class SymbolAlias : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kSymbolAlias;

    SymbolAlias(int offset, StringFragment name, const Symbol* origSymbol)
    : INHERITED(offset, SymbolAliasData{name, origSymbol}) {}

    StringFragment name() const override {
        return this->symbolAliasData().fName;
    }

    const Symbol* origSymbol() const {
        return this->symbolAliasData().fOrigSymbol;
    }

    String description() const override {
        return this->name();
    }

private:
    using INHERITED = Symbol;
};

} // namespace SkSL

#endif
