/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCore.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/DSLVar.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLModifiersPool.h"  // IWYU pragma: keep
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <vector>

namespace SkSL {

class Variable;

namespace dsl {

void Start(SkSL::Compiler* compiler, ProgramKind kind) {
    Start(compiler, kind, ProgramSettings());
}

void Start(SkSL::Compiler* compiler, ProgramKind kind, const ProgramSettings& settings) {
    ThreadContext::SetInstance(std::make_unique<ThreadContext>(compiler, kind, settings,
                                                               compiler->moduleForProgramKind(kind),
                                                               /*isModule=*/false));
}

void StartModule(SkSL::Compiler* compiler,
                 ProgramKind kind,
                 const ProgramSettings& settings,
                 const SkSL::Module* parent) {
    ThreadContext::SetInstance(std::make_unique<ThreadContext>(compiler, kind, settings,
                                                               parent, /*isModule=*/true));
}

void End() {
    ThreadContext::SetInstance(nullptr);
}

ErrorReporter& GetErrorReporter() {
    return ThreadContext::GetErrorReporter();
}

void SetErrorReporter(ErrorReporter* errorReporter) {
    SkASSERT(errorReporter);
    ThreadContext::SetErrorReporter(errorReporter);
}

class DSLCore {
public:
    static std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source) {
        ThreadContext& instance = ThreadContext::Instance();
        SkSL::Compiler& compiler = *instance.fCompiler;
        Pool* pool = instance.fPool.get();
        auto result = std::make_unique<SkSL::Program>(std::move(source),
                                                      std::move(instance.fConfig),
                                                      compiler.fContext,
                                                      std::move(instance.fProgramElements),
                                                      std::move(instance.fSharedElements),
                                                      std::move(instance.fModifiersPool),
                                                      std::move(compiler.fSymbolTable),
                                                      std::move(instance.fPool),
                                                      instance.fInputs);
        bool success = false;
        if (!compiler.finalize(*result)) {
            // Do not return programs that failed to compile.
        } else if (!compiler.optimize(*result)) {
            // Do not return programs that failed to optimize.
        } else {
            // We have a successful program!
            success = true;
        }
        if (pool) {
            pool->detachFromThread();
        }
        SkASSERT(instance.fProgramElements.empty());
        SkASSERT(!ThreadContext::SymbolTable());
        return success ? std::move(result) : nullptr;
    }

    template <typename... Args>
    static DSLExpression Call(const char* name, Position pos, Args... args) {
        SkSL::ExpressionArray argArray;
        argArray.reserve_back(sizeof...(args));
        ((void)argArray.push_back(args.release()), ...);

        return DSLExpression(SkSL::FunctionCall::Convert(ThreadContext::Context(), pos,
                ThreadContext::Compiler().convertIdentifier(Position(), name),
                std::move(argArray)));
    }

    static DSLStatement Break(Position pos) {
        return SkSL::BreakStatement::Make(pos);
    }

    static DSLStatement Continue(Position pos) {
        return SkSL::ContinueStatement::Make(pos);
    }

    static void Declare(const DSLModifiers& modifiers) {
        ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::ModifiersDeclaration>(
                ThreadContext::Modifiers(modifiers.fModifiers)));
    }

    static DSLStatement Declare(DSLVar& var, Position pos) {
        return DSLWriter::Declaration(var);
    }

    static DSLStatement Declare(SkTArray<DSLVar>& vars, Position pos) {
        StatementArray statements;
        for (DSLVar& v : vars) {
            statements.push_back(Declare(v, pos).release());
        }
        return SkSL::Block::Make(pos, std::move(statements), Block::Kind::kCompoundStatement);
    }

    static void Declare(DSLGlobalVar& var, Position pos) {
        std::unique_ptr<SkSL::Statement> stmt = DSLWriter::Declaration(var);
        if (stmt && !stmt->isEmpty()) {
            ThreadContext::ProgramElements().push_back(
                    std::make_unique<SkSL::GlobalVarDeclaration>(std::move(stmt)));
        }
    }

    static void Declare(SkTArray<DSLGlobalVar>& vars, Position pos) {
        for (DSLGlobalVar& v : vars) {
            Declare(v, pos);
        }
    }

    static DSLStatement Discard(Position pos) {
        return DSLStatement(SkSL::DiscardStatement::Convert(ThreadContext::Context(), pos), pos);
    }

    static DSLStatement Do(DSLStatement stmt, DSLExpression test, Position pos) {
        return DSLStatement(DoStatement::Convert(ThreadContext::Context(), pos, stmt.release(),
                                                 test.release()), pos);
    }

    static DSLStatement For(DSLStatement initializer, DSLExpression test,
                                    DSLExpression next, DSLStatement stmt, Position pos,
                                    const ForLoopPositions& forLoopPositions) {
        return DSLStatement(ForStatement::Convert(ThreadContext::Context(), pos, forLoopPositions,
                                                  initializer.releaseIfPossible(),
                                                  test.releaseIfPossible(),
                                                  next.releaseIfPossible(),
                                                  stmt.release(),
                                                  ThreadContext::SymbolTable()), pos);
    }

    static DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse,
                           Position pos) {
        return DSLStatement(IfStatement::Convert(ThreadContext::Context(),
                                                 pos,
                                                 test.release(),
                                                 ifTrue.release(),
                                                 ifFalse.releaseIfPossible()), pos);
    }

    static DSLExpression InterfaceBlock(const DSLModifiers& modifiers, std::string_view typeName,
                                        SkTArray<DSLField> fields, std::string_view varName,
                                        int arraySize, Position pos) {
        // Build a struct type corresponding to the passed-in fields and array size.
        DSLType varType = StructType(typeName, fields, /*interfaceBlock=*/true, pos);
        if (arraySize > 0) {
            varType = Array(varType, arraySize);
        }

        // Create a global variable to attach our interface block to. (The variable doesn't actually
        // get a program element, though; the interface block does instead.)
        DSLGlobalVar var(modifiers, varType, varName, DSLExpression(), pos);
        if (SkSL::Variable* skslVar = DSLWriter::Var(var)) {
            // Add an InterfaceBlock program element to the program.
            if (std::unique_ptr<SkSL::InterfaceBlock> intf = SkSL::InterfaceBlock::Convert(
                        ThreadContext::Context(), pos, skslVar, ThreadContext::SymbolTable())) {
                ThreadContext::ProgramElements().push_back(std::move(intf));
                // Return a VariableReference to the global variable tied to the interface block.
                return DSLExpression(var);
            }
        }

        // The InterfaceBlock couldn't be created; return poison.
        return DSLExpression(nullptr);
    }

    static DSLStatement Return(DSLExpression value, Position pos) {
        // Note that because Return is called before the function in which it resides exists, at
        // this point we do not know the function's return type. We therefore do not check for
        // errors, or coerce the value to the correct type, until the return statement is actually
        // added to a function. (This is done in FunctionDefinition::Convert.)
        return SkSL::ReturnStatement::Make(pos, value.releaseIfPossible());
    }

    static DSLExpression Swizzle(DSLExpression base, SkSL::SwizzleComponent::Type a,
                                 Position pos, Position maskPos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), pos, maskPos,
                                              base.release(), ComponentArray{a}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 Position pos,
                                 Position maskPos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), pos, maskPos,
                                              base.release(), ComponentArray{a, b}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 SkSL::SwizzleComponent::Type c,
                                 Position pos,
                                 Position maskPos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), pos, maskPos,
                                              base.release(), ComponentArray{a, b, c}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 SkSL::SwizzleComponent::Type c,
                                 SkSL::SwizzleComponent::Type d,
                                 Position pos,
                                 Position maskPos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), pos, maskPos,
                                              base.release(), ComponentArray{a, b, c, d}),
                             pos);
    }

    static DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
            Position pos) {
        auto result = TernaryExpression::Convert(ThreadContext::Context(), pos, test.release(),
                                          ifTrue.release(), ifFalse.release());
        SkASSERT(!result || result->fPosition == pos);
        return DSLExpression(std::move(result), pos);
    }

    static DSLStatement Switch(DSLExpression value, SkTArray<DSLCase> cases, Position pos) {
        ExpressionArray values;
        values.reserve_back(cases.size());
        StatementArray caseBlocks;
        caseBlocks.reserve_back(cases.size());
        for (DSLCase& c : cases) {
            values.push_back(c.fValue.releaseIfPossible());
            caseBlocks.push_back(SkSL::Block::Make(Position(), std::move(c.fStatements),
                                                   Block::Kind::kUnbracedBlock));
        }
        return DSLStatement(SwitchStatement::Convert(ThreadContext::Context(), pos,
                                                     value.release(),
                                                     std::move(values),
                                                     std::move(caseBlocks),
                                                     ThreadContext::SymbolTable()), pos);
    }

    static DSLStatement While(DSLExpression test, DSLStatement stmt, Position pos) {
        return DSLStatement(ForStatement::ConvertWhile(ThreadContext::Context(), pos,
                                                       test.release(),
                                                       stmt.release(),
                                                       ThreadContext::SymbolTable()), pos);
    }
};

std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source) {
    return DSLCore::ReleaseProgram(std::move(source));
}

void AddExtension(std::string_view name, Position pos) {
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::Extension>(pos, name));
}

DSLStatement Break(Position pos) {
    return DSLCore::Break(pos);
}

DSLStatement Continue(Position pos) {
    return DSLCore::Continue(pos);
}

void Declare(const DSLModifiers& modifiers, Position pos) {
    SkSL::ProgramKind kind = ThreadContext::GetProgramConfig()->fKind;
    if (!ProgramConfig::IsFragment(kind) &&
        !ProgramConfig::IsVertex(kind)) {
        ThreadContext::ReportError("layout qualifiers are not allowed in this kind of program",
                                   pos);
        return;
    }
    DSLCore::Declare(modifiers);
}

// Logically, we'd want the variable's initial value to appear on here in Declare, since that
// matches how we actually write code (and in fact that was what our first attempt looked like).
// Unfortunately, C++ doesn't guarantee execution order between arguments, and Declare() can appear
// as a function argument in constructs like Block(Declare(x, 0), foo(x)). If these are executed out
// of order, we will evaluate the reference to x before we evaluate Declare(x, 0), and thus the
// variable's initial value is unknown at the point of reference. There are probably some other
// issues with this as well, but it is particularly dangerous when x is const, since SkSL will
// expect its value to be known when it is referenced and will end up asserting, dereferencing a
// null pointer, or possibly doing something else awful.
//
// So, we put the initial value onto the Var itself instead of the Declare to guarantee that it is
// always executed in the correct order.
DSLStatement Declare(DSLVar& var, Position pos) {
    return DSLCore::Declare(var, pos);
}

DSLStatement Declare(SkTArray<DSLVar>& vars, Position pos) {
    return DSLCore::Declare(vars, pos);
}

void Declare(DSLGlobalVar& var, Position pos) {
    DSLCore::Declare(var, pos);
}

void Declare(SkTArray<DSLGlobalVar>& vars, Position pos) {
    DSLCore::Declare(vars, pos);
}

DSLStatement Discard(Position pos) {
    return DSLCore::Discard(pos);
}

DSLStatement Do(DSLStatement stmt, DSLExpression test, Position pos) {
    return DSLCore::Do(std::move(stmt), std::move(test), pos);
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt, Position pos, ForLoopPositions forLoopPositions) {
    return DSLCore::For(std::move(initializer), std::move(test), std::move(next),
                        std::move(stmt), pos, forLoopPositions);
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse, Position pos) {
    return DSLCore::If(std::move(test), std::move(ifTrue), std::move(ifFalse), pos);
}

DSLExpression InterfaceBlock(const DSLModifiers& modifiers, std::string_view typeName,
                             SkTArray<DSLField> fields, std::string_view varName, int arraySize,
                             Position pos) {
    return DSLCore::InterfaceBlock(modifiers, typeName, std::move(fields), varName, arraySize, pos);
}

DSLStatement Return(DSLExpression expr, Position pos) {
    return DSLCore::Return(std::move(expr), pos);
}

DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
                     Position pos) {
    return DSLCore::Select(std::move(test), std::move(ifTrue), std::move(ifFalse), pos);
}

DSLStatement Switch(DSLExpression value, SkTArray<DSLCase> cases, Position pos) {
    return DSLCore::Switch(std::move(value), std::move(cases), pos);
}

DSLStatement While(DSLExpression test, DSLStatement stmt, Position pos) {
    return DSLCore::While(std::move(test), std::move(stmt), pos);
}

DSLExpression Abs(DSLExpression x, Position pos) {
    return DSLCore::Call("abs", pos, std::move(x));
}

DSLExpression All(DSLExpression x, Position pos) {
    return DSLCore::Call("all", pos, std::move(x));
}

DSLExpression Any(DSLExpression x, Position pos) {
    return DSLCore::Call("any", pos, std::move(x));
}

DSLExpression Atan(DSLExpression y_over_x, Position pos) {
    return DSLCore::Call("atan", pos, std::move(y_over_x));
}

DSLExpression Atan(DSLExpression y, DSLExpression x, Position pos) {
    return DSLCore::Call("atan", pos, std::move(y), std::move(x));
}

DSLExpression Ceil(DSLExpression x, Position pos) {
    return DSLCore::Call("ceil", pos, std::move(x));
}

DSLExpression Clamp(DSLExpression x, DSLExpression min, DSLExpression max, Position pos) {
    return DSLCore::Call("clamp", pos, std::move(x), std::move(min), std::move(max));
}

DSLExpression Cos(DSLExpression x, Position pos) {
    return DSLCore::Call("cos", pos, std::move(x));
}

DSLExpression Cross(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("cross", pos, std::move(x), std::move(y));
}

DSLExpression Degrees(DSLExpression x, Position pos) {
    return DSLCore::Call("degrees", pos, std::move(x));
}

DSLExpression Distance(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("distance", pos, std::move(x), std::move(y));
}

DSLExpression Dot(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("dot", pos, std::move(x), std::move(y));
}

DSLExpression Equal(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("equal", pos, std::move(x), std::move(y));
}

DSLExpression Exp(DSLExpression x, Position pos) {
    return DSLCore::Call("exp", pos, std::move(x));
}

DSLExpression Exp2(DSLExpression x, Position pos) {
    return DSLCore::Call("exp2", pos, std::move(x));
}

DSLExpression Faceforward(DSLExpression n, DSLExpression i, DSLExpression nref, Position pos) {
    return DSLCore::Call("faceforward", pos, std::move(n), std::move(i), std::move(nref));
}

DSLExpression Fract(DSLExpression x, Position pos) {
    return DSLCore::Call("fract", pos, std::move(x));
}

DSLExpression Floor(DSLExpression x, Position pos) {
    return DSLCore::Call("floor", pos, std::move(x));
}

DSLExpression GreaterThan(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("greaterThan", pos, std::move(x), std::move(y));
}

DSLExpression GreaterThanEqual(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("greaterThanEqual", pos, std::move(x), std::move(y));
}

DSLExpression Inverse(DSLExpression x, Position pos) {
    return DSLCore::Call("inverse", pos, std::move(x));
}

DSLExpression Inversesqrt(DSLExpression x, Position pos) {
    return DSLCore::Call("inversesqrt", pos, std::move(x));
}

DSLExpression Length(DSLExpression x, Position pos) {
    return DSLCore::Call("length", pos, std::move(x));
}

DSLExpression LessThan(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("lessThan", pos, std::move(x), std::move(y));
}

DSLExpression LessThanEqual(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("lessThanEqual", pos, std::move(x), std::move(y));
}

DSLExpression Log(DSLExpression x, Position pos) {
    return DSLCore::Call("log", pos, std::move(x));
}

DSLExpression Log2(DSLExpression x, Position pos) {
    return DSLCore::Call("log2", pos, std::move(x));
}

DSLExpression Max(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("max", pos, std::move(x), std::move(y));
}

DSLExpression Min(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("min", pos, std::move(x), std::move(y));
}

DSLExpression Mix(DSLExpression x, DSLExpression y, DSLExpression a, Position pos) {
    return DSLCore::Call("mix", pos, std::move(x), std::move(y), std::move(a));
}

DSLExpression Mod(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("mod", pos, std::move(x), std::move(y));
}

DSLExpression Normalize(DSLExpression x, Position pos) {
    return DSLCore::Call("normalize", pos, std::move(x));
}

DSLExpression NotEqual(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("notEqual", pos, std::move(x), std::move(y));
}

DSLExpression Pow(DSLExpression x, DSLExpression y, Position pos) {
    return DSLCore::Call("pow", pos, std::move(x), std::move(y));
}

DSLExpression Radians(DSLExpression x, Position pos) {
    return DSLCore::Call("radians", pos, std::move(x));
}

DSLExpression Reflect(DSLExpression i, DSLExpression n, Position pos) {
    return DSLCore::Call("reflect", pos, std::move(i), std::move(n));
}

DSLExpression Refract(DSLExpression i, DSLExpression n, DSLExpression eta, Position pos) {
    return DSLCore::Call("refract", pos, std::move(i), std::move(n), std::move(eta));
}

DSLExpression Round(DSLExpression x, Position pos) {
    return DSLCore::Call("round", pos, std::move(x));
}

DSLExpression Saturate(DSLExpression x, Position pos) {
    return DSLCore::Call("saturate", pos, std::move(x));
}

DSLExpression Sign(DSLExpression x, Position pos) {
    return DSLCore::Call("sign", pos, std::move(x));
}

DSLExpression Sin(DSLExpression x, Position pos) {
    return DSLCore::Call("sin", pos, std::move(x));
}

DSLExpression Smoothstep(DSLExpression edge1, DSLExpression edge2, DSLExpression x,
                         Position pos) {
    return DSLCore::Call("smoothstep", pos, std::move(edge1), std::move(edge2), std::move(x));
}

DSLExpression Sqrt(DSLExpression x, Position pos) {
    return DSLCore::Call("sqrt", pos, std::move(x));
}

DSLExpression Step(DSLExpression edge, DSLExpression x, Position pos) {
    return DSLCore::Call("step", pos, std::move(edge), std::move(x));
}

DSLExpression Swizzle(DSLExpression base, SkSL::SwizzleComponent::Type a,
                      Position pos, Position maskPos) {
    return DSLCore::Swizzle(std::move(base), a, pos, maskPos);
}

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      Position pos,
                      Position maskPos) {
    return DSLCore::Swizzle(std::move(base), a, b, pos, maskPos);
}

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      SkSL::SwizzleComponent::Type c,
                      Position pos,
                      Position maskPos) {
    return DSLCore::Swizzle(std::move(base), a, b, c, pos, maskPos);
}

DSLExpression Swizzle(DSLExpression base,
                      SkSL::SwizzleComponent::Type a,
                      SkSL::SwizzleComponent::Type b,
                      SkSL::SwizzleComponent::Type c,
                      SkSL::SwizzleComponent::Type d,
                      Position pos,
                      Position maskPos) {
    return DSLCore::Swizzle(std::move(base), a, b, c, d, pos, maskPos);
}

DSLExpression Tan(DSLExpression x, Position pos) {
    return DSLCore::Call("tan", pos, std::move(x));
}

DSLExpression Unpremul(DSLExpression x, Position pos) {
    return DSLCore::Call("unpremul", pos, std::move(x));
}

} // namespace dsl

} // namespace SkSL
