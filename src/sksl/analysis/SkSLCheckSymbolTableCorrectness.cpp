/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

using namespace skia_private;

namespace SkSL {

class Expression;
class ProgramElement;
class Symbol;

void Analysis::CheckSymbolTableCorrectness(const Program& program) {
    const Context& context = *program.fContext;

    class SymbolTableCorrectnessVisitor : public ProgramVisitor {
    public:
        SymbolTableCorrectnessVisitor(const Context& c, SymbolTable* sym)
                : fContext(c)
                , fSymbolTableStack({sym}) {}

        using ProgramVisitor::visitProgramElement;

        bool visitStatement(const Statement& stmt) override {
            Analysis::SymbolTableStackBuilder symbolTableStackBuilder(&stmt, &fSymbolTableStack);
            if (stmt.is<VarDeclaration>()) {
                // Check the top of the symbol table stack to see if it contains this exact symbol.
                const VarDeclaration& vardecl = stmt.as<VarDeclaration>();
                bool containsSymbol = false;

                // We want to do an exact Symbol* comparison in just one symbol table; we don't want
                // to look up by name, and we don't want to walk the symbol table tree. This makes
                // SymbolTable::find() an inappropriate tool for the job. Instead, we can iterate
                // the symbol table's contents directly and check for a pointer match.
                fSymbolTableStack.back()->foreach([&](std::string_view, const Symbol* symbol) {
                    if (symbol == vardecl.var()) {
                        containsSymbol = true;
                    }
                });
                if (!containsSymbol) {
                    fContext.fErrors->error(vardecl.position(), "internal error (variable '" +
                                                                std::string(vardecl.var()->name()) +
                                                                "' is incorrectly scoped)");
                }
            }
            return INHERITED::visitStatement(stmt);
        }

        bool visitExpression(const Expression&) override {
            return false;
        }

    private:
        using INHERITED = ProgramVisitor;

        const Context& fContext;
        std::vector<SymbolTable*> fSymbolTableStack;
    };

    SymbolTableCorrectnessVisitor visitor{context, program.fSymbols.get()};
    for (const std::unique_ptr<ProgramElement>& pe : program.fOwnedElements) {
        visitor.visitProgramElement(*pe);
    }
}

}  // namespace SkSL
