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

class SymbolTable;
class IRIntrinsicMap;

struct ParsedModule {
    std::shared_ptr<SymbolTable>    fSymbols;
    std::shared_ptr<IRIntrinsicMap> fIntrinsics;
};

} //namespace SkSL

#endif
