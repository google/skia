/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkTHash.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <memory>
#include <string_view>
#include <vector>

namespace SkSL {
namespace Transform {
namespace {

class BuiltinVariableScanner : public ProgramVisitor {
public:
    BuiltinVariableScanner(const Context& context, const SymbolTable& symbols)
            : fContext(context)
            , fSymbols(symbols) {}

    void addDeclaringElement(const ProgramElement* decl) {
        // Make sure we only add a built-in variable once. We only have a small handful of built-in
        // variables to declare, so linear search here is good enough.
        if (std::find(fNewElements.begin(), fNewElements.end(), decl) == fNewElements.end()) {
            fNewElements.push_back(decl);
        }
    }

    void addDeclaringElement(const Symbol* symbol) {
        if (!symbol) {
            return;
        }
        const Variable& var = symbol->as<Variable>();
        if (const GlobalVarDeclaration* decl = var.globalVarDeclaration()) {
            this->addDeclaringElement(decl);
        } else if (const InterfaceBlock* block = var.interfaceBlock()) {
            this->addDeclaringElement(block);
        } else {
            // Double-check that this variable isn't associated with a global or an interface block.
            // (Locals and parameters will come along naturally as part of the associated function.)
            SkASSERTF(var.storage() != VariableStorage::kGlobal &&
                      var.storage() != VariableStorage::kInterfaceBlock,
                      "%.*s", (int)var.name().size(), var.name().data());
        }
    }

    bool visitProgramElement(const ProgramElement& pe) override {
        if (pe.is<FunctionDefinition>()) {
            const FunctionDefinition& funcDef = pe.as<FunctionDefinition>();
            // We synthesize writes to sk_FragColor if main() returns a color, even if it's
            // otherwise unreferenced. Check main's return type to see if it's half4.
            if (funcDef.declaration().isMain()) {
                if (funcDef.declaration().returnType().matches(*fContext.fTypes.fHalf4)) {
                    // main() returns a half4, so make sure we include sk_FragColor in the output.
                    this->addDeclaringElement(fSymbols.find(Compiler::FRAGCOLOR_NAME));
                }
                // Once we find main(), we can stop scanning.
                return true;
            }
        }
        return false;
    }

    static std::string_view GlobalVarBuiltinName(const ProgramElement& elem) {
        return elem.as<GlobalVarDeclaration>().varDeclaration().var()->name();
    }

    static std::string_view InterfaceBlockName(const ProgramElement& elem) {
        return elem.as<InterfaceBlock>().instanceName();
    }

    void sortNewElements() {
        std::sort(fNewElements.begin(),
                  fNewElements.end(),
                  [](const ProgramElement* a, const ProgramElement* b) {
                      if (a->kind() != b->kind()) {
                          return a->kind() < b->kind();
                      }
                      switch (a->kind()) {
                          case ProgramElement::Kind::kGlobalVar:
                              SkASSERT(GlobalVarBuiltinName(*a) != GlobalVarBuiltinName(*b));
                              return GlobalVarBuiltinName(*a) < GlobalVarBuiltinName(*b);

                          case ProgramElement::Kind::kInterfaceBlock:
                              SkASSERT(InterfaceBlockName(*a) != InterfaceBlockName(*b));
                              return InterfaceBlockName(*a) < InterfaceBlockName(*b);

                          default:
                              SkUNREACHABLE;
                      }
                  });
    }

    const Context& fContext;
    const SymbolTable& fSymbols;
    std::vector<const ProgramElement*> fNewElements;
};

}  // namespace

void FindAndDeclareBuiltinVariables(Program& program) {
    const Context& context = *program.fContext;
    const SymbolTable& symbols = *program.fSymbols;
    BuiltinVariableScanner scanner(context, symbols);

    // Find main() in the program and check its return type.
    for (auto& e : program.fOwnedElements) {
        scanner.visitProgramElement(*e);
    }

    if (ProgramConfig::IsFragment(program.fConfig->fKind)) {
        // Vulkan requires certain builtin variables be present, even if they're unused. At one
        // time, validation errors would result if sk_Clockwise was missing. Now, it's just (Adreno)
        // driver bugs that drop or corrupt draws if they're missing.
        scanner.addDeclaringElement(symbols.find("sk_Clockwise"));
    }

    // Scan all the variables used by the program and declare any built-ins.
    for (const auto& [var, counts] : program.fUsage->fVariableCounts) {
        if (var->isBuiltin()) {
            scanner.addDeclaringElement(var);

            // Set the FlipRT program input if we find sk_FragCoord or sk_Clockwise.
            switch (var->modifiers().fLayout.fBuiltin) {
                case SK_FRAGCOORD_BUILTIN:
                    if (context.fCaps->fCanUseFragCoord) {
                        program.fInputs.fUseFlipRTUniform =
                                !context.fConfig->fSettings.fForceNoRTFlip;
                    }
                    break;

                case SK_CLOCKWISE_BUILTIN:
                    program.fInputs.fUseFlipRTUniform = !context.fConfig->fSettings.fForceNoRTFlip;
                    break;
            }
        }
    }

    // Sort the referenced builtin functions into a consistent order; otherwise our output will
    // become non-deterministic. The exact order isn't particularly important.
    scanner.sortNewElements();

    // Add all the newly-declared elements to the program, and update ProgramUsage to match.
    program.fSharedElements.insert(program.fSharedElements.begin(),
                                   scanner.fNewElements.begin(),
                                   scanner.fNewElements.end());

    for (const ProgramElement* element : scanner.fNewElements) {
        program.fUsage->add(*element);
    }
}

}  // namespace Transform
}  // namespace SkSL
