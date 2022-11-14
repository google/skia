/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLProgramElement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <utility>

namespace SkSL {

Program::Program(std::unique_ptr<std::string> source,
                 std::unique_ptr<ProgramConfig> config,
                 std::shared_ptr<Context> context,
                 std::vector<std::unique_ptr<ProgramElement>> elements,
                 std::vector<const ProgramElement*> sharedElements,
                 std::unique_ptr<ModifiersPool> modifiers,
                 std::shared_ptr<SymbolTable> symbols,
                 std::unique_ptr<Pool> pool,
                 Inputs inputs)
        : fSource(std::move(source))
        , fConfig(std::move(config))
        , fContext(context)
        , fModifiers(std::move(modifiers))
        , fSymbols(symbols)
        , fPool(std::move(pool))
        , fOwnedElements(std::move(elements))
        , fSharedElements(std::move(sharedElements))
        , fInputs(inputs) {
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
    fModifiers.reset();
}

std::string Program::description() const {
    std::string result = fConfig->versionDescription();
    for (const ProgramElement* e : this->elements()) {
        result += e->description();
    }
    return result;
}

}  // namespace SkSL
