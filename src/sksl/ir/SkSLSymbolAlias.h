/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLALIAS
#define SKSL_SYMBOLALIAS

#include "include/private/SkSLSymbol.h"

namespace SkSL {

/**
 * A symbol representing a new name for an existing symbol.
 */
class SymbolAlias final : public Symbol {
public:
    static constexpr Kind kSymbolKind = Kind::kSymbolAlias;

    SymbolAlias(int offset, StringFragment name, const Symbol* origSymbol)
        : INHERITED(offset, kSymbolKind, name)
        , fOrigSymbol(origSymbol) {}

    const Symbol* origSymbol() const {
        return fOrigSymbol;
    }

    String description() const override {
        return this->name();
    }

private:
    const Symbol* fOrigSymbol;

    using INHERITED = Symbol;
};

} // namespace SkSL

#endif
