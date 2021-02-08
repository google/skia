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
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"

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

static char swizzle_component(SwizzleComponent c) {
    switch (c) {
        case X:
            return 'x';
        case Y:
            return 'y';
        case Z:
            return 'z';
        case W:
            return 'w';
        case ZERO:
            return '0';
        case ONE:
            return '1';
        default:
            SkUNREACHABLE;
    }
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
    static DSLExpression Call(const char* name, Args... args) {
        SkSL::IRGenerator& ir = DSLWriter::IRGenerator();
        SkSL::ExpressionArray argArray;
        argArray.reserve_back(sizeof...(args));

        // in C++17, we could just do:
        // (argArray.push_back(args.release()), ...);
        int unused[] = {0, (ignore(argArray.push_back(args.release())), 0)...};
        static_cast<void>(unused);

        return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(argArray));
    }

    static DSLStatement Break() {
        return std::unique_ptr<SkSL::Statement>(new SkSL::BreakStatement(/*offset=*/-1));
    }

    static DSLStatement Continue() {
        return std::unique_ptr<SkSL::Statement>(new SkSL::ContinueStatement(/*offset=*/-1));
    }

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

    static DSLStatement Discard() {
        return std::unique_ptr<SkSL::Statement>(new SkSL::DiscardStatement(/*offset=*/-1));
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

    static DSLStatement Return(DSLExpression value) {
        // note that because Return is called before the function in which it resides exists, at
        // this point we do not know the function's return type. We therefore do not check for
        // errors, or coerce the value to the correct type, until the return statement is actually
        // added to a function
        std::unique_ptr<SkSL::Expression> expr = value.release();
        if (expr) {
            return std::unique_ptr<SkSL::Statement>(new ReturnStatement(std::move(expr)));
        } else {
            return std::unique_ptr<SkSL::Statement>(new ReturnStatement(/*offset=*/-1));
        }
    }

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a) {
        char mask[] = { swizzle_component(a), 0 };
        return DSLWriter::IRGenerator().convertSwizzle(base.release(), mask);
    }

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b) {
        char mask[] = { swizzle_component(a), swizzle_component(b), 0 };
        return DSLWriter::IRGenerator().convertSwizzle(base.release(), mask);
    }

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                                 SwizzleComponent c) {
        char mask[] = { swizzle_component(a), swizzle_component(b), swizzle_component(c), 0 };
        return DSLWriter::IRGenerator().convertSwizzle(base.release(), mask);
    }

    static DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                                 SwizzleComponent c, SwizzleComponent d) {
        char mask[] = { swizzle_component(a), swizzle_component(b), swizzle_component(c),
                        swizzle_component(d), 0 };
        return DSLWriter::IRGenerator().convertSwizzle(base.release(), mask);
    }

    static DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
        return DSLWriter::IRGenerator().convertTernaryExpression(test.release(), ifTrue.release(),
                                                                 ifFalse.release());
    }

    static DSLStatement While(DSLExpression test, DSLStatement stmt) {
        return DSLWriter::IRGenerator().convertWhile(/*offset=*/-1, test.release(), stmt.release());
    }

private:
    static void ignore(std::unique_ptr<SkSL::Expression>&) {}
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

DSLStatement Declare(DSLVar& var, DSLExpression initialValue) {
    return DSLCore::Declare(var, std::move(initialValue));
}

DSLStatement Discard() {
    return DSLCore::Discard();
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

DSLStatement Return(DSLExpression expr) {
    return DSLCore::Return(std::move(expr));
}

DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
    return DSLCore::Ternary(std::move(test), std::move(ifTrue), std::move(ifFalse));
}

DSLStatement While(DSLExpression test, DSLStatement stmt) {
    return DSLCore::While(std::move(test), std::move(stmt));
}

DSLExpression Abs(DSLExpression x) {
    return DSLCore::Call("abs", std::move(x));
}

DSLExpression All(DSLExpression x) {
    return DSLCore::Call("all", std::move(x));
}

DSLExpression Any(DSLExpression x) {
    return DSLCore::Call("any", std::move(x));
}

DSLExpression Ceil(DSLExpression x) {
    return DSLCore::Call("ceil", std::move(x));
}

DSLExpression Clamp(DSLExpression x, DSLExpression min, DSLExpression max) {
    return DSLCore::Call("clamp", std::move(x), std::move(min), std::move(max));
}

DSLExpression Cos(DSLExpression x) {
    return DSLCore::Call("cos", std::move(x));
}

DSLExpression Cross(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("cross", std::move(x), std::move(y));
}

DSLExpression Degrees(DSLExpression x) {
    return DSLCore::Call("degrees", std::move(x));
}

DSLExpression Distance(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("distance", std::move(x), std::move(y));
}

DSLExpression Dot(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("dot", std::move(x), std::move(y));
}

DSLExpression Equal(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("equal", std::move(x), std::move(y));
}

DSLExpression Exp(DSLExpression x) {
    return DSLCore::Call("exp", std::move(x));
}

DSLExpression Exp2(DSLExpression x) {
    return DSLCore::Call("exp2", std::move(x));
}

DSLExpression Faceforward(DSLExpression n, DSLExpression i, DSLExpression nref) {
    return DSLCore::Call("faceforward", std::move(n), std::move(i), std::move(nref));
}

DSLExpression Fract(DSLExpression x) {
    return DSLCore::Call("fract", std::move(x));
}

DSLExpression Floor(DSLExpression x) {
    return DSLCore::Call("floor", std::move(x));
}

DSLExpression GreaterThan(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("greaterThan", std::move(x), std::move(y));
}

DSLExpression GreaterThanEqual(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("greaterThanEqual", std::move(x), std::move(y));
}

DSLExpression Inverse(DSLExpression x) {
    return DSLCore::Call("inverse", std::move(x));
}

DSLExpression Inversesqrt(DSLExpression x) {
    return DSLCore::Call("inversesqrt", std::move(x));
}

DSLExpression Length(DSLExpression x) {
    return DSLCore::Call("length", std::move(x));
}

DSLExpression LessThan(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("lessThan", std::move(x), std::move(y));
}

DSLExpression LessThanEqual(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("lessThanEqual", std::move(x), std::move(y));
}

DSLExpression Log(DSLExpression x) {
    return DSLCore::Call("log", std::move(x));
}

DSLExpression Log2(DSLExpression x) {
    return DSLCore::Call("log2", std::move(x));
}

DSLExpression Max(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("max", std::move(x), std::move(y));
}

DSLExpression Min(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("min", std::move(x), std::move(y));
}

DSLExpression Mix(DSLExpression x, DSLExpression y, DSLExpression a) {
    return DSLCore::Call("mix", std::move(x), std::move(y), std::move(a));
}

DSLExpression Mod(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("mod", std::move(x), std::move(y));
}

DSLExpression Normalize(DSLExpression x) {
    return DSLCore::Call("normalize", std::move(x));
}

DSLExpression NotEqual(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("notEqual", std::move(x), std::move(y));
}

DSLExpression Pow(DSLExpression x, DSLExpression y) {
    return DSLCore::Call("pow", std::move(x), std::move(y));
}

DSLExpression Radians(DSLExpression x) {
    return DSLCore::Call("radians", std::move(x));
}

DSLExpression Reflect(DSLExpression i, DSLExpression n) {
    return DSLCore::Call("reflect", std::move(i), std::move(n));
}

DSLExpression Refract(DSLExpression i, DSLExpression n, DSLExpression eta) {
    return DSLCore::Call("refract", std::move(i), std::move(n), std::move(eta));
}

DSLExpression Saturate(DSLExpression x) {
    return DSLCore::Call("saturate", std::move(x));
}

DSLExpression Sign(DSLExpression x) {
    return DSLCore::Call("sign", std::move(x));
}

DSLExpression Sin(DSLExpression x) {
    return DSLCore::Call("sin", std::move(x));
}

DSLExpression Smoothstep(DSLExpression edge1, DSLExpression edge2, DSLExpression x) {
    return DSLCore::Call("smoothstep", std::move(edge1), std::move(edge2), std::move(x));
}

DSLExpression Sqrt(DSLExpression x) {
    return DSLCore::Call("sqrt", std::move(x));
}

DSLExpression Step(DSLExpression edge, DSLExpression x) {
    return DSLCore::Call("step", std::move(edge), std::move(x));
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a) {
    return DSLCore::Swizzle(std::move(base), a);
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b) {
    return DSLCore::Swizzle(std::move(base), a, b);
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                      SwizzleComponent c) {
    return DSLCore::Swizzle(std::move(base), a, b, c);
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                      SwizzleComponent c, SwizzleComponent d) {
    return DSLCore::Swizzle(std::move(base), a, b, c, d);
}

DSLExpression Tan(DSLExpression x) {
    return DSLCore::Call("tan", std::move(x));
}

DSLExpression Unpremul(DSLExpression x) {
    return DSLCore::Call("unpremul", std::move(x));
}

} // namespace dsl

} // namespace SkSL
