/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCore.h"

#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"

namespace SkSL {

namespace dsl {

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
void Start(SkSL::Compiler* compiler) {
    DSLWriter::SetInstance(std::make_unique<DSLWriter>(compiler));
}

void End() {
    SkASSERTF(!DSLWriter::InFragmentProcessor(),
              "more calls to StartFragmentProcessor than to EndFragmentProcessor");
    DSLWriter::SetInstance(nullptr);
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

void SetErrorHandler(ErrorHandler* errorHandler) {
    DSLWriter::SetErrorHandler(errorHandler);
}

class DSLCore {
public:
    static DSLVar sk_FragColor() {
        return DSLVar("sk_FragColor");
    }

    static DSLVar sk_FragCoord() {
        return DSLVar("sk_FragCoord");
    }

    template <typename... Args>
    static DSLPossibleExpression Call(const char* name, Args... args) {
        SkSL::IRGenerator& ir = DSLWriter::IRGenerator();
        SkSL::ExpressionArray argArray;
        argArray.reserve_back(sizeof...(args));

        // in C++17, we could just do:
        // (argArray.push_back(args.release()), ...);
        int unused[] = {0, (static_cast<void>(argArray.push_back(args.release())), 0)...};
        static_cast<void>(unused);

        return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(argArray));
    }

    static DSLStatement Break() {
        return SkSL::BreakStatement::Make(/*offset=*/-1);
    }

    static DSLStatement Continue() {
        return SkSL::ContinueStatement::Make(/*offset=*/-1);
    }

    static DSLStatement Declare(DSLVar& var, PositionInfo pos) {
        if (var.fDeclared) {
            DSLWriter::ReportError("error: variable has already been declared\n", &pos);
        }
        var.fDeclared = true;
        return std::move(var.fDeclaration);
    }

    static DSLStatement Discard() {
        return SkSL::DiscardStatement::Make(/*offset=*/-1);
    }

    static DSLPossibleStatement Do(DSLStatement stmt, DSLExpression test) {
        return DoStatement::Convert(DSLWriter::Context(), stmt.release(), test.release());
    }

    static DSLPossibleStatement For(DSLStatement initializer, DSLExpression test,
                                    DSLExpression next, DSLStatement stmt) {
        return ForStatement::Convert(DSLWriter::Context(), /*offset=*/-1, initializer.release(),
                                     test.release(), next.release(), stmt.release(),
                                     DSLWriter::SymbolTable());
    }

    static DSLPossibleStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
        return IfStatement::Convert(DSLWriter::Context(), /*offset=*/-1, /*isStatic=*/false,
                                    test.release(), ifTrue.release(), ifFalse.release());
    }

    static DSLPossibleStatement Return(DSLExpression value, PositionInfo pos) {
        // Note that because Return is called before the function in which it resides exists, at
        // this point we do not know the function's return type. We therefore do not check for
        // errors, or coerce the value to the correct type, until the return statement is actually
        // added to a function. (This is done in IRGenerator::finalizeFunction.)
        return SkSL::ReturnStatement::Make(/*offset=*/-1, value.release());
    }

    static DSLExpression Swizzle(DSLExpression base, SkSL::SwizzleComponent::Type a,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(DSLWriter::Context(), base.release(),
                                              ComponentArray{a}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(DSLWriter::Context(), base.release(),
                                              ComponentArray{a, b}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 SkSL::SwizzleComponent::Type c,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(DSLWriter::Context(), base.release(),
                                              ComponentArray{a, b, c}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 SkSL::SwizzleComponent::Type c,
                                 SkSL::SwizzleComponent::Type d,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(DSLWriter::Context(), base.release(),
                                              ComponentArray{a,b,c,d}),
                             pos);
    }

    static DSLPossibleExpression Select(DSLExpression test, DSLExpression ifTrue,
                                        DSLExpression ifFalse) {
        return TernaryExpression::Convert(DSLWriter::Context(), test.release(),
                                          ifTrue.release(), ifFalse.release());
    }

    static DSLPossibleStatement Switch(DSLExpression value, SkSL::ExpressionArray values,
                                       SkTArray<StatementArray> statements) {
        return DSLWriter::ConvertSwitch(value.release(), std::move(values), std::move(statements));
    }

    static DSLPossibleStatement While(DSLExpression test, DSLStatement stmt) {
        return ForStatement::ConvertWhile(DSLWriter::Context(), /*offset=*/-1, test.release(),
                                          stmt.release(), DSLWriter::SymbolTable());
    }
};

DSLVar sk_FragColor() {
    return DSLCore::sk_FragColor();
}

DSLVar sk_FragCoord() {
    return DSLCore::sk_FragCoord();
}

DSLStatement Break() {
    return DSLCore::Break();
}

DSLStatement Continue() {
    return DSLCore::Continue();
}

DSLStatement Declare(DSLVar& var, PositionInfo pos) {
    return DSLCore::Declare(var, pos);
}

DSLStatement Discard() {
    return DSLCore::Discard();
}

DSLStatement Do(DSLStatement stmt, DSLExpression test, PositionInfo pos) {
    return DSLStatement(DSLCore::Do(std::move(stmt), std::move(test)), pos);
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt, PositionInfo pos) {
    return DSLStatement(DSLCore::For(std::move(initializer), std::move(test), std::move(next),
                                     std::move(stmt)), pos);
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse, PositionInfo pos) {
    return DSLStatement(DSLCore::If(std::move(test), std::move(ifTrue), std::move(ifFalse)), pos);
}

DSLStatement Return(DSLExpression expr, PositionInfo pos) {
    return DSLCore::Return(std::move(expr), pos);
}

DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
                     PositionInfo pos) {
    return DSLExpression(DSLCore::Select(std::move(test), std::move(ifTrue), std::move(ifFalse)),
                         pos);
}

DSLPossibleStatement Switch(DSLExpression value, SkSL::ExpressionArray values,
                            SkTArray<StatementArray> statements) {
    return DSLCore::Switch(std::move(value), std::move(values), std::move(statements));
}

DSLStatement While(DSLExpression test, DSLStatement stmt, PositionInfo pos) {
    return DSLStatement(DSLCore::While(std::move(test), std::move(stmt)), pos);
}

DSLExpression Abs(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("abs", std::move(x)), pos);
}

DSLExpression All(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("all", std::move(x)), pos);
}

DSLExpression Any(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("any", std::move(x)), pos);
}

DSLExpression Ceil(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("ceil", std::move(x)), pos);
}

DSLExpression Clamp(DSLExpression x, DSLExpression min, DSLExpression max, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("clamp", std::move(x), std::move(min), std::move(max)), pos);
}

DSLExpression Cos(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("cos", std::move(x)), pos);
}

DSLExpression Cross(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("cross", std::move(x), std::move(y)), pos);
}

DSLExpression Degrees(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("degrees", std::move(x)), pos);
}

DSLExpression Distance(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("distance", std::move(x), std::move(y)), pos);
}

DSLExpression Dot(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("dot", std::move(x), std::move(y)), pos);
}

DSLExpression Equal(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("equal", std::move(x), std::move(y)), pos);
}

DSLExpression Exp(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("exp", std::move(x)), pos);
}

DSLExpression Exp2(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("exp2", std::move(x)), pos);
}

DSLExpression Faceforward(DSLExpression n, DSLExpression i, DSLExpression nref, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("faceforward", std::move(n), std::move(i), std::move(nref)),
                         pos);
}

DSLExpression Fract(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("fract", std::move(x)), pos);
}

DSLExpression Floor(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("floor", std::move(x)), pos);
}

DSLExpression GreaterThan(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("greaterThan", std::move(x), std::move(y)), pos);
}

DSLExpression GreaterThanEqual(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("greaterThanEqual", std::move(x), std::move(y)), pos);
}

DSLExpression Inverse(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("inverse", std::move(x)), pos);
}

DSLExpression Inversesqrt(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("inversesqrt", std::move(x)), pos);
}

DSLExpression Length(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("length", std::move(x)), pos);
}

DSLExpression LessThan(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("lessThan", std::move(x), std::move(y)), pos);
}

DSLExpression LessThanEqual(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("lessThanEqual", std::move(x), std::move(y)), pos);
}

DSLExpression Log(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("log", std::move(x)), pos);
}

DSLExpression Log2(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("log2", std::move(x)), pos);
}

DSLExpression Max(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("max", std::move(x), std::move(y)), pos);
}

DSLExpression Min(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("min", std::move(x), std::move(y)), pos);
}

DSLExpression Mix(DSLExpression x, DSLExpression y, DSLExpression a, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("mix", std::move(x), std::move(y), std::move(a)), pos);
}

DSLExpression Mod(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("mod", std::move(x), std::move(y)), pos);
}

DSLExpression Normalize(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("normalize", std::move(x)), pos);
}

DSLExpression NotEqual(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("notEqual", std::move(x), std::move(y)), pos);
}

DSLExpression Pow(DSLExpression x, DSLExpression y, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("pow", std::move(x), std::move(y)), pos);
}

DSLExpression Radians(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("radians", std::move(x)), pos);
}

DSLExpression Reflect(DSLExpression i, DSLExpression n, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("reflect", std::move(i), std::move(n)), pos);
}

DSLExpression Refract(DSLExpression i, DSLExpression n, DSLExpression eta, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("refract", std::move(i), std::move(n), std::move(eta)), pos);
}

DSLExpression Saturate(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("saturate", std::move(x)), pos);
}

DSLExpression Sign(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("sign", std::move(x)), pos);
}

DSLExpression Sin(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("sin", std::move(x)), pos);
}

DSLExpression Smoothstep(DSLExpression edge1, DSLExpression edge2, DSLExpression x,
                         PositionInfo pos) {
    return DSLExpression(DSLCore::Call("smoothstep", std::move(edge1), std::move(edge2),
                                       std::move(x)),
                         pos);
}

DSLExpression Sqrt(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("sqrt", std::move(x)), pos);
}

DSLExpression Step(DSLExpression edge, DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("step", std::move(edge), std::move(x)), pos);
}

DSLExpression Swizzle(DSLExpression base, SkSL::SwizzleComponent::Type a,
                      PositionInfo pos) {
    return DSLCore::Swizzle(std::move(base), a, pos);
}

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      PositionInfo pos) {
    return DSLCore::Swizzle(std::move(base), a, b, pos);
}

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      SkSL::SwizzleComponent::Type c,
                      PositionInfo pos) {
    return DSLCore::Swizzle(std::move(base), a, b, c, pos);
}

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      SkSL::SwizzleComponent::Type c,
                      SkSL::SwizzleComponent::Type d,
                      PositionInfo pos) {
    return DSLCore::Swizzle(std::move(base), a, b, c, d, pos);
}

DSLExpression Tan(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("tan", std::move(x)), pos);
}

DSLExpression Unpremul(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("unpremul", std::move(x)), pos);
}

} // namespace dsl

} // namespace SkSL
