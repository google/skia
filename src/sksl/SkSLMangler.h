/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MANGLER
#define SKSL_MANGLER

#include "include/private/SkSLString.h"

namespace SkSL {

class SymbolTable;

class Mangler {
public:
    /**
     * Mangles baseName to create a name that is unique within symbolTable.
     */
    String uniqueName(skstd::string_view baseName, SymbolTable* symbolTable);

    void reset() {
        fCounter = 0;
    }

private:
    int fCounter = 0;
};

} // namespace SkSL

#endif
