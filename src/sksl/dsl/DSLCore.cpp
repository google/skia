/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLCore.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"

namespace SkSL {

namespace dsl {

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
void Start(SkSL::Compiler* compiler) {
    DSLWriter::SetInstance(std::make_unique<DSLWriter>(compiler));
}

void End() {
    DSLWriter::SetInstance(nullptr);
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

void SetErrorHandler(ErrorHandler* errorHandler) {
    DSLWriter::SetErrorHandler(errorHandler);
}
class DSLCore {
public:
    static DSLStatement Declare(DSLVar& var, DSLExpression initialValue) {
        if (!var.fDeclaration) {
            DSLWriter::ReportError("Declare failed (was the variable already declared?)");
            return DSLStatement();
        }
        VarDeclaration& decl = var.fDeclaration->as<SkSL::VarDeclaration>();
        std::unique_ptr<Expression> expr = initialValue.coerceAndRelease(decl.var().type());
        if (expr) {
            decl.fValue = std::move(expr);
        }
        return DSLStatement(std::move(var.fDeclaration));
    }

    static DSLStatement Do(DSLStatement stmt, DSLExpression test) {
        return DSLWriter::IRGenerator().convertDo(stmt.release(), test.release());
    }

    static DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                            DSLStatement stmt) {
        return DSLWriter::IRGenerator().convertFor(/*offset=*/-1, initializer.release(),
                                                   test.release(), next.release(), stmt.release());
    }

    static DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
        return DSLWriter::IRGenerator().convertIf(/*offset=*/-1, /*isStatic=*/false, test.release(),
                                                  ifTrue.release(), ifFalse.release());
    }

    static DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
        return DSLWriter::IRGenerator().convertTernaryExpression(test.release(), ifTrue.release(),
                                                                 ifFalse.release());
    }

    static DSLStatement While(DSLExpression test, DSLStatement stmt) {
        return DSLWriter::IRGenerator().convertWhile(/*offset=*/-1, test.release(), stmt.release());
    }
};

DSLStatement Declare(DSLVar& var, DSLExpression initialValue) {
    return DSLCore::Declare(var, std::move(initialValue));
}

DSLStatement Do(DSLStatement stmt, DSLExpression test) {
    return DSLCore::Do(std::move(stmt), std::move(test));
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt) {
    return DSLCore::For(std::move(initializer), std::move(test), std::move(next), std::move(stmt));
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
    return DSLCore::If(std::move(test), std::move(ifTrue), std::move(ifFalse));
}

DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
    return DSLCore::Ternary(std::move(test), std::move(ifTrue), std::move(ifFalse));
}

DSLStatement While(DSLExpression test, DSLStatement stmt) {
    return DSLCore::While(std::move(test), std::move(stmt));
}

} // namespace dsl

} // namespace SkSL
