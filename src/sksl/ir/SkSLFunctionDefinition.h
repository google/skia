/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDEFINITION
#define SKSL_FUNCTIONDEFINITION

#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;

/**
 * A function definition (a declaration plus an associated block of code).
 */
class FunctionDefinition final : public ProgramElement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kFunction;

    FunctionDefinition(Position pos, const FunctionDeclaration* declaration, bool builtin,
                       std::unique_ptr<Statement> body)
        : INHERITED(pos, kIRNodeKind)
        , fDeclaration(declaration)
        , fBuiltin(builtin)
        , fBody(std::move(body)) {}

    /**
     * Coerces `return` statements to the return type of the function, and reports errors in the
     * function that can't be detected at the individual statement level:
     *     - `break` and `continue` statements must be in reasonable places.
     *     - non-void functions are required to return a value on all paths.
     *     - vertex main() functions don't allow early returns.
     *
     * This will return a FunctionDefinition even if an error is detected; this leads to better
     * diagnostics overall. (Returning null here leads to spurious "function 'f()' was not defined"
     * errors when trying to call a function with an error in it.)
     */
    static std::unique_ptr<FunctionDefinition> Convert(const Context& context,
                                                       Position pos,
                                                       const FunctionDeclaration& function,
                                                       std::unique_ptr<Statement> body,
                                                       bool builtin);

    const FunctionDeclaration& declaration() const {
        return *fDeclaration;
    }

    bool isBuiltin() const {
        return fBuiltin;
    }

    std::unique_ptr<Statement>& body() {
        return fBody;
    }

    const std::unique_ptr<Statement>& body() const {
        return fBody;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<FunctionDefinition>(fPosition, &this->declaration(),
                                                    /*builtin=*/false, this->body()->clone());
    }

    std::string description() const override {
        return this->declaration().description() + " " + this->body()->description();
    }

private:
    const FunctionDeclaration* fDeclaration;
    bool fBuiltin;
    std::unique_ptr<Statement> fBody;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
