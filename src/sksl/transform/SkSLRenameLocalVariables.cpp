/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SkSL {

class Context;
class ProgramUsage;

void Transform::RenameLocalVariables(Context& context, LoadedModule& module, ProgramUsage* usage) {

    class VariableRenamer : public ProgramWriter {
    public:
        VariableRenamer(Context& context, ProgramUsage* usage) : fContext(context), fUsage(usage) {}

        static std::string FindShortNameForSymbol(const Symbol* sym,
                                                  const SymbolTable* symbolTable) {
            static constexpr std::string_view kLetters[] = {
                    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
                    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

            // Try any single-letter option.
            for (std::string_view letter : kLetters) {
                if ((*symbolTable)[letter] == nullptr) {
                    return std::string(letter);
                }
            }

            // Try every two-letter option.
            for (std::string_view letterA : kLetters) {
                for (std::string_view letterB : kLetters) {
                    std::string name = std::string(letterA) + std::string(letterB);
                    if ((*symbolTable)[name] == nullptr) {
                        return name;
                    }
                }
            }

            // We struck out. Somehow, all 2700 two-letter names have been claimed.
            SkDEBUGFAILF("Unable to find unique name for '%s'", std::string(sym->name()).c_str());
            return std::string(sym->name());
        }

        void minifyName(const Symbol* sym) {
            // Look for a new name for this symbol.
            // Note: we always rename _every_ variable, even ones with single-letter names. This is
            // a safeguard: if we claimed a name like `i`, and then the program itself contained an
            // `i` later on, in a nested SymbolTable, the two names would clash. By always renaming
            // everything, we can ignore that problem.
            SymbolTable* symbols = fSymbolTableStack.back().get();
            std::string shortName = FindShortNameForSymbol(sym, symbols);

            // Update the symbol's name.
            SkASSERT(symbols->getMutableSymbol(shortName) == nullptr);
            Symbol* mutableSym = symbols->getMutableSymbol(sym->name());
            SkASSERT(mutableSym == sym);
            const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
            symbols->renameSymbol(mutableSym, *ownedName);
        }

        bool visitProgramElement(ProgramElement& elem) override {
            if (!elem.is<FunctionDefinition>()) {
                // This transform only looks at function definitions.
                return false;
            }
            // Minify the names of each function parameter.
            const FunctionDefinition& def = elem.as<FunctionDefinition>();
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(def.body().get(),
                                                                      &fSymbolTableStack);
            for (const Variable* param : def.declaration().parameters()) {
                this->minifyName(param);
            }
            return INHERITED::visitProgramElement(elem);
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(stmt.get(),
                                                                      &fSymbolTableStack);
            if (stmt->is<VarDeclaration>()) {
                // Minify the variable's name.
                VarDeclaration& decl = stmt->as<VarDeclaration>();
                this->minifyName(&decl.var());
            }

            return INHERITED::visitStatementPtr(stmt);
        }

        Context& fContext;
        ProgramUsage* fUsage;
        std::vector<std::shared_ptr<SymbolTable>> fSymbolTableStack;
        using INHERITED = ProgramWriter;
    };

    VariableRenamer visitor{context, usage};

    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        visitor.visitProgramElement(*pe);
    }
}

}  // namespace SkSL
