/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkStringView.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace SkSL {

class Context;
class ProgramUsage;
enum class ProgramKind : int8_t;

static void strip_export_flag(Context& context,
                              const FunctionDeclaration* funcDecl,
                              SymbolTable* symbols) {
    // Remove `$export` from every overload of this function.
    Symbol* mutableSym = symbols->findMutable(funcDecl->name());
    while (mutableSym) {
        FunctionDeclaration* mutableDecl = &mutableSym->as<FunctionDeclaration>();

        ModifierFlags flags = mutableDecl->modifierFlags();
        flags &= ~ModifierFlag::kExport;
        mutableDecl->setModifierFlags(flags);

        mutableSym = mutableDecl->mutableNextOverload();
    }
}

void Transform::RenamePrivateSymbols(Context& context,
                                     Module& module,
                                     ProgramUsage* usage,
                                     ProgramKind kind) {
    class SymbolRenamer : public ProgramWriter {
    public:
        SymbolRenamer(Context& context,
                      ProgramUsage* usage,
                      std::shared_ptr<SymbolTable> symbolBase,
                      ProgramKind kind)
                : fContext(context)
                , fUsage(usage)
                , fSymbolTableStack({std::move(symbolBase)})
                , fKind(kind) {}

        static std::string FindShortNameForSymbol(const Symbol* sym,
                                                  const SymbolTable* symbolTable,
                                                  const std::string& namePrefix) {
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
            // Some variables are associated with anonymous parameters--these don't have names and
            // aren't present in the symbol table. Their names are already empty so there's no way
            // to shrink them further.
            if (var->name().empty()) {
                return;
            }

            // Ensure that this variable is properly set up in the symbol table.
            SymbolTable* symbols = fSymbolTableStack.back().get();
            Symbol* mutableSym = symbols->findMutable(var->name());
            SkASSERTF(mutableSym != nullptr,
                      "symbol table missing '%.*s'", (int)var->name().size(), var->name().data());
            SkASSERTF(mutableSym == var,
                      "wrong symbol found for '%.*s'", (int)var->name().size(), var->name().data());

            // Look for a new name for this symbol.
            // Note: we always rename _every_ variable, even ones with single-letter names. This is
            // a safeguard: if we claimed a name like `i`, and then the program itself contained an
            // `i` later on, in a nested SymbolTable, the two names would clash. By always renaming
            // everything, we can ignore that problem.
            std::string shortName = FindShortNameForSymbol(var, symbols, "");
            SkASSERT(symbols->findMutable(shortName) == nullptr);

            // Update the symbol's name.
            const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
            symbols->renameSymbol(mutableSym, *ownedName);
        }

        void minifyFunctionName(const FunctionDeclaration* funcDecl) {
            // Look for a new name for this function.
            std::string namePrefix = ProgramConfig::IsRuntimeEffect(fKind) ? "" : "$";
            SymbolTable* symbols = fSymbolTableStack.back().get();
            std::string shortName = FindShortNameForSymbol(funcDecl, symbols,
                                                           std::move(namePrefix));
            SkASSERT(symbols->findMutable(shortName) == nullptr);

            if (shortName.size() < funcDecl->name().size()) {
                // Update the function's name. (If the function has overloads, this will rename all
                // of them at once.)
                Symbol* mutableSym = symbols->findMutable(funcDecl->name());
                const std::string* ownedName = symbols->takeOwnershipOfString(std::move(shortName));
                symbols->renameSymbol(mutableSym, *ownedName);
            }
        }

        bool functionNameCanBeMinifiedSafely(const FunctionDeclaration& funcDecl) const {
            if (ProgramConfig::IsRuntimeEffect(fKind)) {
                // The only externally-accessible function in a runtime effect is main().
                return !funcDecl.isMain();
            } else {
                // We will only minify $private_functions, and only ones not marked as $export.
                return skstd::starts_with(funcDecl.name(), '$') &&
                       !funcDecl.modifierFlags().isExport();
            }
        }

        void minifyFunction(FunctionDefinition& def) {
            // If the function is private, minify its name.
            const FunctionDeclaration* funcDecl = &def.declaration();
            if (this->functionNameCanBeMinifiedSafely(*funcDecl)) {
                this->minifyFunctionName(funcDecl);
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
        ProgramKind fKind;
        using INHERITED = ProgramWriter;
    };

    // Rename local variables and private functions.
    SymbolRenamer renamer{context, usage, module.fSymbols, kind};
    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        renamer.visitProgramElement(*pe);
    }

    // Strip off modifier `$export` from every function. (Only the minifier checks this flag, so we
    // can remove it without affecting the meaning of the code.)
    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        if (pe->is<FunctionDefinition>()) {
            const FunctionDeclaration* funcDecl = &pe->as<FunctionDefinition>().declaration();
            if (funcDecl->modifierFlags().isExport()) {
                strip_export_flag(context, funcDecl, module.fSymbols.get());
            }
        }
    }
}

}  // namespace SkSL
