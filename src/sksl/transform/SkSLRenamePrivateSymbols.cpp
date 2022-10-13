/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/private/SkSLSymbol.h"
#include "include/private/SkStringView.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLModifiersPool.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
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

class ProgramUsage;

static void strip_export_flag(Context& context,
                              const FunctionDeclaration* funcDecl,
                              SymbolTable* symbols) {
    // Remove `$export` from every overload of this function.
    Symbol* mutableSym = symbols->findMutable(funcDecl->name());
    while (mutableSym) {
        FunctionDeclaration* mutableDecl = &mutableSym->as<FunctionDeclaration>();

        Modifiers modifiers = mutableDecl->modifiers();
        modifiers.fFlags &= ~Modifiers::kExport_Flag;
        mutableDecl->setModifiers(context.fModifiersPool->add(modifiers));

        mutableSym = mutableDecl->mutableNextOverload();
    }
}

void Transform::RenamePrivateSymbols(Context& context, LoadedModule& module, ProgramUsage* usage) {

    class SymbolRenamer : public ProgramWriter {
    public:
        SymbolRenamer(Context& context,
                      ProgramUsage* usage,
                      std::shared_ptr<SymbolTable> symbolBase)
                : fContext(context), fUsage(usage), fSymbolTableStack({std::move(symbolBase)}) {}

        static std::string FindShortNameForSymbol(const Symbol* sym,
                                                  const SymbolTable* symbolTable,
                                                  std::string namePrefix) {
            static constexpr std::string_view kLetters[] = {
                    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
                    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
                    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

            // Try any single-letter option.
            for (std::string_view letter : kLetters) {
                std::string name = namePrefix + std::string(letter);
                if (symbolTable->find(name) == nullptr) {
                    return name;
                }
            }

            // Try every two-letter option.
            for (std::string_view letterA : kLetters) {
                for (std::string_view letterB : kLetters) {
                    std::string name = namePrefix + std::string(letterA) + std::string(letterB);
                    if (symbolTable->find(name) == nullptr) {
                        return name;
                    }
                }
            }

            // We struck out. Somehow, all 2700 two-letter names have been claimed.
            SkDEBUGFAILF("Unable to find unique name for '%s'", std::string(sym->name()).c_str());
            return std::string(sym->name());
        }

        void minifyVariableName(const Variable* var) {
            // Look for a new name for this symbol.
            // Note: we always rename _every_ variable, even ones with single-letter names. This is
            // a safeguard: if we claimed a name like `i`, and then the program itself contained an
            // `i` later on, in a nested SymbolTable, the two names would clash. By always renaming
            // everything, we can ignore that problem.
            SymbolTable* symbols = fSymbolTableStack.back().get();
            std::string shortName = FindShortNameForSymbol(var, symbols, "");
            SkASSERT(symbols->findMutable(shortName) == nullptr);

            // Update the symbol's name.
            Symbol* mutableSym = symbols->findMutable(var->name());
            SkASSERT(mutableSym == var);
            const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
            symbols->renameSymbol(mutableSym, *ownedName);
        }

        void minifyPrivateFunctionName(const FunctionDeclaration* funcDecl) {
            // Look for a new name for this function.
            SymbolTable* symbols = fSymbolTableStack.back().get();
            std::string shortName = FindShortNameForSymbol(funcDecl, symbols, "$");
            SkASSERT(symbols->findMutable(shortName) == nullptr);

            if (shortName.size() < funcDecl->name().size()) {
                // Update the function's name. (If the function has overloads, this will rename all
                // of them at once.)
                Symbol* mutableSym = symbols->findMutable(funcDecl->name());
                const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
                symbols->renameSymbol(mutableSym, *ownedName);
            }
        }

        void minifyFunction(FunctionDefinition& def) {
            // If the function is private, minify its name.
            const FunctionDeclaration* funcDecl = &def.declaration();
            if (skstd::starts_with(def.declaration().name(), '$') &&
                !(funcDecl->modifiers().fFlags & Modifiers::kExport_Flag)) {
                this->minifyPrivateFunctionName(funcDecl);
            }

            // Minify the names of each function parameter.
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(def.body().get(),
                                                                      &fSymbolTableStack);
            for (Variable* param : funcDecl->parameters()) {
                this->minifyVariableName(param);
            }
        }

        void minifyPrototype(FunctionPrototype& proto) {
            const FunctionDeclaration* funcDecl = &proto.declaration();
            if (funcDecl->definition()) {
                // This function is defined somewhere; this isn't just a loose prototype.
                return;
            }

            // Eliminate the names of each function parameter.
            // The parameter names aren't in the symbol table's name lookup map at all.
            // All we need to do is blank out their names.
            for (Variable* param : funcDecl->parameters()) {
                param->setName("");
            }
        }

        bool visitProgramElement(ProgramElement& elem) override {
            switch (elem.kind()) {
                case ProgramElement::Kind::kFunction:
                    this->minifyFunction(elem.as<FunctionDefinition>());
                    return INHERITED::visitProgramElement(elem);

               case ProgramElement::Kind::kFunctionPrototype:
                   this->minifyPrototype(elem.as<FunctionPrototype>());
                    return INHERITED::visitProgramElement(elem);

                default:
                    return false;
            }
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(stmt.get(),
                                                                      &fSymbolTableStack);
            if (stmt->is<VarDeclaration>()) {
                // Minify the variable's name.
                VarDeclaration& decl = stmt->as<VarDeclaration>();
                this->minifyVariableName(decl.var());
            }

            return INHERITED::visitStatementPtr(stmt);
        }

        Context& fContext;
        ProgramUsage* fUsage;
        std::vector<std::shared_ptr<SymbolTable>> fSymbolTableStack;
        using INHERITED = ProgramWriter;
    };

    // Rename local variables and private functions.
    SymbolRenamer renamer{context, usage, module.fSymbols};
    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        renamer.visitProgramElement(*pe);
    }

    // Strip off modifier `$export` from every function. (Only the minifier checks this flag, so we
    // can remove it without affecting the meaning of the code.)
    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        if (pe->is<FunctionDefinition>()) {
            const FunctionDeclaration* funcDecl = &pe->as<FunctionDefinition>().declaration();
            if (funcDecl->modifiers().fFlags & Modifiers::kExport_Flag) {
                strip_export_flag(context, funcDecl, module.fSymbols.get());
            }
        }
    }
}

}  // namespace SkSL
