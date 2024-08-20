/*
 * Copyright 2021 Google LLC
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
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SkSL {

bool Analysis::CheckProgramStructure(const Program& program) {
    const Context& context = *program.fContext;

    static constexpr size_t kProgramStackDepthLimit = 50;

    class ProgramStructureVisitor : public ProgramVisitor {
    public:
        ProgramStructureVisitor(const Context& c) : fContext(c) {}

        using ProgramVisitor::visitProgramElement;

        bool visitProgramElement(const ProgramElement& pe) override {
            if (pe.is<FunctionDefinition>()) {
                // Check the function map first. We don't need to visit this function if we already
                // processed it before.
                const FunctionDeclaration* decl = &pe.as<FunctionDefinition>().declaration();
                if (FunctionState *funcState = fFunctionMap.find(decl)) {
                    // We already have this function in our map. We don't need to check it again.
                    if (*funcState == FunctionState::kVisiting) {
                        // If the function is present in the map with with the `kVisiting` state,
                        // we're recursively processing it -- in other words, we found a cycle in
                        // the code. Unwind our stack into a string.
                        std::string msg = "\n\t" + decl->description();
                        for (auto unwind = fStack.rbegin(); unwind != fStack.rend(); ++unwind) {
                            msg = "\n\t" + (*unwind)->description() + msg;
                            if (*unwind == decl) {
                                break;
                            }
                        }
                        msg = "potential recursion (function call cycle) not allowed:" + msg;
                        fContext.fErrors->error(pe.fPosition, std::move(msg));
                        *funcState = FunctionState::kVisited;
                        return true;
                    }
                    return false;
                }

                // If the function-call stack has gotten too deep, stop the analysis.
                if (fStack.size() >= kProgramStackDepthLimit) {
                    std::string msg = "exceeded max function call depth:";
                    for (auto unwind = fStack.begin(); unwind != fStack.end(); ++unwind) {
                        msg += "\n\t" + (*unwind)->description();
                    }
                    msg += "\n\t" + decl->description();
                    fContext.fErrors->error(pe.fPosition, std::move(msg));
                    fFunctionMap.set(decl, FunctionState::kVisited);
                    return true;
                }

                fFunctionMap.set(decl, FunctionState::kVisiting);
                fStack.push_back(decl);
                bool result = INHERITED::visitProgramElement(pe);
                fFunctionMap.set(decl, FunctionState::kVisited);
                fStack.pop_back();

                return result;
            }

            return INHERITED::visitProgramElement(pe);
        }

        bool visitExpression(const Expression& expr) override {
            bool earlyExit = false;

            if (expr.is<FunctionCall>()) {
                const FunctionCall& call = expr.as<FunctionCall>();
                const FunctionDeclaration* decl = &call.function();
                if (decl->definition() && !decl->isIntrinsic()) {
                    earlyExit = this->visitProgramElement(*decl->definition());
                }
            }

            return earlyExit || INHERITED::visitExpression(expr);
        }

    private:
        using INHERITED = ProgramVisitor;

        enum class FunctionState {
            kVisiting,
            kVisited,
        };

        const Context& fContext;
        skia_private::THashMap<const FunctionDeclaration*, FunctionState> fFunctionMap;
        std::vector<const FunctionDeclaration*> fStack;
    };

    // Process every function in our program.
    ProgramStructureVisitor visitor{context};
    for (const std::unique_ptr<ProgramElement>& element : program.fOwnedElements) {
        if (element->is<FunctionDefinition>()) {
            // Visit every function--we want to detect static recursion and report it as an error,
            // even in unreferenced functions.
            visitor.visitProgramElement(*element);
        }
    }

    return true;
}

}  // namespace SkSL
