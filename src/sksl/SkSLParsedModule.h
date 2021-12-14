/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PARSEDMODULE
#define SKSL_PARSEDMODULE

#include <memory>

namespace SkSL {

class BuiltinMap;
class SymbolTable;

struct ParsedModule {
    std::shared_ptr<SymbolTable> fSymbols;
    std::shared_ptr<BuiltinMap>  fElements;
};

} //namespace SkSL

#endif
