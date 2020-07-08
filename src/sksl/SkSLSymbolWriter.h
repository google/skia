/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SYMBOLWRITER
#define SKSL_SYMBOLWRITER

#ifdef SKSL_STANDALONE

#include "src/sksl/SkSLOutputStream.h"

#include <set>

namespace SkSL {

struct FunctionDeclaration;
struct Symbol;
class SymbolTable;

class SymbolWriter {
public:
    static String runInFunction(const FunctionDeclaration& f, const String& code);

    static String runInSymbolTable(const SymbolTable& symbols, const String& code);

    static String symbolCode(const Symbol& symbol);
};

} // namespace

#endif

#endif
