/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h" // IWYU pragma: keep

#include <utility>

namespace SkSL {

Program::Program(std::unique_ptr<std::string> source,
                 std::unique_ptr<ProgramConfig> config,
                 std::shared_ptr<Context> context,
                 std::vector<std::unique_ptr<ProgramElement>> elements,
                 std::unique_ptr<SymbolTable> symbols,
                 std::unique_ptr<Pool> pool)
        : fSource(std::move(source))
        , fConfig(std::move(config))
        , fContext(context)
        , fSymbols(std::move(symbols))
        , fPool(std::move(pool))
        , fOwnedElements(std::move(elements)) {
    fUsage = Analysis::GetUsage(*this);
}

Program::~Program() {
    // Some or all of the program elements are in the pool. To free them safely, we must attach
    // the pool before destroying any program elements. (Otherwise, we may accidentally call
    // delete on a pooled node.)
    AutoAttachPoolToThread attach(fPool.get());

    fOwnedElements.clear();
    fContext.reset();
    fSymbols.reset();
}

std::string Program::description() const {
    std::string result = fConfig->versionDescription();
    for (const ProgramElement* e : this->elements()) {
        result += e->description();
    }
    return result;
}

const FunctionDeclaration* Program::getFunction(const char* functionName) const {
    const Symbol* symbol = fSymbols->find(functionName);
    bool valid = symbol && symbol->is<FunctionDeclaration>() &&
                 symbol->as<FunctionDeclaration>().definition();
    return valid ? &symbol->as<FunctionDeclaration>() : nullptr;
}

}  // namespace SkSL
