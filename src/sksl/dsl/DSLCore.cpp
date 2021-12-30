/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCore.h"

#include "include/private/SkSLDefines.h"
#include "include/sksl/DSLSymbols.h"
#include "include/sksl/DSLVar.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/transform/SkSLTransform.h"

namespace SkSL {

namespace dsl {

void Start(SkSL::Compiler* compiler, ProgramKind kind) {
    Start(compiler, kind, ProgramSettings());
}

void Start(SkSL::Compiler* compiler, ProgramKind kind, const ProgramSettings& settings) {
    ThreadContext::SetInstance(std::make_unique<ThreadContext>(compiler, kind, settings,
            compiler->moduleForProgramKind(kind), /*isModule=*/false));
}

void StartModule(SkSL::Compiler* compiler, ProgramKind kind, const ProgramSettings& settings,
                 SkSL::ParsedModule baseModule) {
    ThreadContext::SetInstance(std::make_unique<ThreadContext>(compiler, kind, settings,
            baseModule, /*isModule=*/true));
}

void End() {
    SkASSERTF(!ThreadContext::InFragmentProcessor(),
              "more calls to StartFragmentProcessor than to EndFragmentProcessor");
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
    static std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<String> source) {
        ThreadContext& instance = ThreadContext::Instance();
        SkSL::Compiler& compiler = *instance.fCompiler;
        const SkSL::Context& context = *compiler.fContext;
        // Variables defined in the pre-includes need their declaring elements added to the program
        if (!instance.fConfig->fIsBuiltinCode && context.fBuiltins) {
            Transform::FindAndDeclareBuiltinVariables(context, instance.fConfig->fKind,
                    instance.fSharedElements);
        }
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
        if (!success) {
            ThreadContext::ReportErrors(PositionInfo());
        }
        if (pool) {
            pool->detachFromThread();
        }
        SkASSERT(instance.fProgramElements.empty());
        SkASSERT(!ThreadContext::SymbolTable());
        return success ? std::move(result) : nullptr;
    }

    static DSLGlobalVar sk_FragColor() {
        return DSLGlobalVar("sk_FragColor");
    }

    static DSLGlobalVar sk_FragCoord() {
        return DSLGlobalVar("sk_FragCoord");
    }

    static DSLExpression sk_Position() {
        return DSLExpression(Symbol("sk_Position"));
    }

    template <typename... Args>
    static DSLPossibleExpression Call(const char* name, Args... args) {
        SkSL::ExpressionArray argArray;
        argArray.reserve_back(sizeof...(args));

        // in C++17, we could just do:
        // (argArray.push_back(args.release()), ...);
        int unused[] = {0, (static_cast<void>(argArray.push_back(args.release())), 0)...};
        static_cast<void>(unused);

        return SkSL::FunctionCall::Convert(ThreadContext::Context(), /*line=*/-1,
                ThreadContext::Compiler().convertIdentifier(-1, name), std::move(argArray));
    }

    static DSLStatement Break(PositionInfo pos) {
        return SkSL::BreakStatement::Make(pos.line());
    }

    static DSLStatement Continue(PositionInfo pos) {
        return SkSL::ContinueStatement::Make(pos.line());
    }

    static void Declare(const DSLModifiers& modifiers) {
        ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::ModifiersDeclaration>(
                ThreadContext::Modifiers(modifiers.fModifiers)));
    }

    static DSLStatement Declare(DSLVar& var, PositionInfo pos) {
        if (var.fDeclared) {
            ThreadContext::ReportError("variable has already been declared", pos);
        }
        var.fDeclared = true;
        return DSLWriter::Declaration(var);
    }

    static DSLStatement Declare(SkTArray<DSLVar>& vars, PositionInfo pos) {
        StatementArray statements;
        for (DSLVar& v : vars) {
            statements.push_back(Declare(v, pos).release());
        }
        return SkSL::Block::MakeUnscoped(pos.line(), std::move(statements));
    }

    static void Declare(DSLGlobalVar& var, PositionInfo pos) {
        if (var.fDeclared) {
            ThreadContext::ReportError("variable has already been declared", pos);
        }
        var.fDeclared = true;
        std::unique_ptr<SkSL::Statement> stmt = DSLWriter::Declaration(var);
        if (stmt) {
            if (!stmt->isEmpty()) {
                ThreadContext::ProgramElements().push_back(
                        std::make_unique<SkSL::GlobalVarDeclaration>(std::move(stmt)));
            }
        } else if (var.fName == SkSL::Compiler::FRAGCOLOR_NAME) {
            // sk_FragColor can end up with a null declaration despite no error occurring due to
            // specific treatment in the compiler. Ignore the null and just grab the existing
            // variable from the symbol table.
            const SkSL::Symbol* alreadyDeclared = (*ThreadContext::SymbolTable())[var.fName];
            if (alreadyDeclared && alreadyDeclared->is<Variable>()) {
                var.fVar = &alreadyDeclared->as<Variable>();
                var.fInitialized = true;
            }
        }
    }

    static void Declare(SkTArray<DSLGlobalVar>& vars, PositionInfo pos) {
        for (DSLGlobalVar& v : vars) {
            Declare(v, pos);
        }
    }

    static DSLStatement Discard(PositionInfo pos) {
        return SkSL::DiscardStatement::Make(pos.line());
    }

    static DSLPossibleStatement Do(DSLStatement stmt, DSLExpression test) {
        return DoStatement::Convert(ThreadContext::Context(), stmt.release(), test.release());
    }

    static DSLPossibleStatement For(DSLStatement initializer, DSLExpression test,
                                    DSLExpression next, DSLStatement stmt, PositionInfo pos) {
        return ForStatement::Convert(ThreadContext::Context(), pos.line(),
                                     initializer.releaseIfPossible(), test.releaseIfPossible(),
                                     next.releaseIfPossible(), stmt.release(),
                                     ThreadContext::SymbolTable());
    }

    static DSLPossibleStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse,
                                   bool isStatic) {
        return IfStatement::Convert(ThreadContext::Context(), /*line=*/-1, isStatic, test.release(),
                                    ifTrue.release(), ifFalse.releaseIfPossible());
    }

    static void FindRTAdjust(SkSL::InterfaceBlock& intf, PositionInfo pos) {
        const std::vector<SkSL::Type::Field>& fields =
                intf.variable().type().componentType().fields();
        const Context& context = ThreadContext::Context();
        for (size_t i = 0; i < fields.size(); ++i) {
            const SkSL::Type::Field& f = fields[i];
            if (f.fName == SkSL::Compiler::RTADJUST_NAME) {
                if (f.fType->matches(*context.fTypes.fFloat4)) {
                    ThreadContext::RTAdjustData& rtAdjust = ThreadContext::RTAdjustState();
                    rtAdjust.fInterfaceBlock = &intf.variable();
                    rtAdjust.fFieldIndex = i;
                } else {
                    ThreadContext::ReportError("sk_RTAdjust must have type 'float4'", pos);
                }
                break;
            }
        }
    }

    static DSLGlobalVar InterfaceBlock(const DSLModifiers& modifiers, skstd::string_view typeName,
                                       SkTArray<DSLField> fields, skstd::string_view varName,
                                       int arraySize, PositionInfo pos) {
        // We need to create a new struct type for the interface block, but we don't want it in the
        // symbol table. Since dsl::Struct automatically sticks it in the symbol table, we create it
        // the old fashioned way with MakeStructType.
        std::vector<SkSL::Type::Field> skslFields;
        skslFields.reserve(fields.count());
        for (const DSLField& field : fields) {
            const SkSL::Type* baseType = &field.fType.skslType();
            if (baseType->isArray()) {
                baseType = &baseType->componentType();
            }
            SkSL::VarDeclaration::ErrorCheck(ThreadContext::Context(), field.fPosition.line(),
                    field.fModifiers.fModifiers, baseType, Variable::Storage::kInterfaceBlock);
            GetErrorReporter().reportPendingErrors(field.fPosition);
            skslFields.push_back(SkSL::Type::Field(field.fModifiers.fModifiers, field.fName,
                                                   &field.fType.skslType()));
        }
        const SkSL::Type* structType =
                ThreadContext::SymbolTable()->takeOwnershipOfSymbol(SkSL::Type::MakeStructType(
                        pos.line(), typeName, std::move(skslFields), /*interfaceBlock=*/true));
        DSLType varType = arraySize > 0 ? Array(structType, arraySize) : DSLType(structType);
        DSLGlobalVar var(modifiers, varType, !varName.empty() ? varName : typeName, DSLExpression(),
                pos);
        // Interface blocks can't be declared, so we always need to mark the var declared ourselves.
        // We do this only when fDSLMarkVarDeclared is false, so we don't double-declare it.
        if (!ThreadContext::Settings().fDSLMarkVarsDeclared) {
            DSLWriter::MarkDeclared(var);
        }
        const SkSL::Variable* skslVar = DSLWriter::Var(var);
        if (skslVar) {
            auto intf = std::make_unique<SkSL::InterfaceBlock>(pos.line(),
                    *skslVar, typeName, varName, arraySize, ThreadContext::SymbolTable());
            FindRTAdjust(*intf, pos);
            ThreadContext::ProgramElements().push_back(std::move(intf));
            if (varName.empty()) {
                const std::vector<SkSL::Type::Field>& structFields = structType->fields();
                for (size_t i = 0; i < structFields.size(); ++i) {
                    ThreadContext::SymbolTable()->add(std::make_unique<SkSL::Field>(pos.line(),
                                                                                    skslVar,
                                                                                    i));
                }
            } else {
                AddToSymbolTable(var);
            }
        }
        GetErrorReporter().reportPendingErrors(pos);
        return var;
    }

    static DSLStatement Return(DSLExpression value, PositionInfo pos) {
        // Note that because Return is called before the function in which it resides exists, at
        // this point we do not know the function's return type. We therefore do not check for
        // errors, or coerce the value to the correct type, until the return statement is actually
        // added to a function. (This is done in FunctionDefinition::Convert.)
        return SkSL::ReturnStatement::Make(pos.line(), value.releaseIfPossible());
    }

    static DSLExpression Swizzle(DSLExpression base, SkSL::SwizzleComponent::Type a,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), base.release(),
                                              ComponentArray{a}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), base.release(),
                                              ComponentArray{a, b}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 SkSL::SwizzleComponent::Type c,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), base.release(),
                                              ComponentArray{a, b, c}),
                             pos);
    }

    static DSLExpression Swizzle(DSLExpression base,
                                 SkSL::SwizzleComponent::Type a,
                                 SkSL::SwizzleComponent::Type b,
                                 SkSL::SwizzleComponent::Type c,
                                 SkSL::SwizzleComponent::Type d,
                                 PositionInfo pos) {
        return DSLExpression(Swizzle::Convert(ThreadContext::Context(), base.release(),
                                              ComponentArray{a,b,c,d}),
                             pos);
    }

    static DSLPossibleExpression Select(DSLExpression test, DSLExpression ifTrue,
                                        DSLExpression ifFalse) {
        return TernaryExpression::Convert(ThreadContext::Context(), test.release(),
                                          ifTrue.release(), ifFalse.release());
    }

    static DSLPossibleStatement Switch(DSLExpression value, SkTArray<DSLCase> cases,
                                       bool isStatic) {
        ExpressionArray values;
        values.reserve_back(cases.count());
        StatementArray caseBlocks;
        caseBlocks.reserve_back(cases.count());
        for (DSLCase& c : cases) {
            values.push_back(c.fValue.releaseIfPossible());
            caseBlocks.push_back(SkSL::Block::Make(/*line=*/-1,
                    std::move(c.fStatements), /*symbols=*/nullptr, /*isScope=*/false));
        }
        return SwitchStatement::Convert(ThreadContext::Context(), /*line=*/-1, isStatic,
                value.release(), std::move(values), std::move(caseBlocks),
                ThreadContext::SymbolTable());
    }

    static DSLPossibleStatement While(DSLExpression test, DSLStatement stmt) {
        return ForStatement::ConvertWhile(ThreadContext::Context(), /*line=*/-1, test.release(),
                                          stmt.release(), ThreadContext::SymbolTable());
    }
};

std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<String> source) {
    return DSLCore::ReleaseProgram(std::move(source));
}

DSLGlobalVar sk_FragColor() {
    return DSLCore::sk_FragColor();
}

DSLGlobalVar sk_FragCoord() {
    return DSLCore::sk_FragCoord();
}

DSLExpression sk_Position() {
    return DSLCore::sk_Position();
}

void AddExtension(skstd::string_view name, PositionInfo pos) {
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::Extension>(pos.line(), name));
    ThreadContext::ReportErrors(pos);
}

DSLStatement Break(PositionInfo pos) {
    return DSLCore::Break(pos);
}

DSLStatement Continue(PositionInfo pos) {
    return DSLCore::Continue(pos);
}

void Declare(const DSLModifiers& modifiers, PositionInfo pos) {
    SkSL::ProgramKind kind = ThreadContext::GetProgramConfig()->fKind;
    if (kind != ProgramKind::kFragment &&
        kind != ProgramKind::kVertex) {
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
DSLStatement Declare(DSLVar& var, PositionInfo pos) {
    return DSLCore::Declare(var, pos);
}

DSLStatement Declare(SkTArray<DSLVar>& vars, PositionInfo pos) {
    return DSLCore::Declare(vars, pos);
}

void Declare(DSLGlobalVar& var, PositionInfo pos) {
    DSLCore::Declare(var, pos);
}

void Declare(SkTArray<DSLGlobalVar>& vars, PositionInfo pos) {
    DSLCore::Declare(vars, pos);
}

DSLStatement Discard(PositionInfo pos) {
    if (ThreadContext::GetProgramConfig()->fKind != ProgramKind::kFragment) {
        ThreadContext::ReportError("discard statement is only permitted in fragment shaders", pos);
    }
    return DSLCore::Discard(pos);
}

DSLStatement Do(DSLStatement stmt, DSLExpression test, PositionInfo pos) {
    return DSLStatement(DSLCore::Do(std::move(stmt), std::move(test)), pos);
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt, PositionInfo pos) {
    return DSLStatement(DSLCore::For(std::move(initializer), std::move(test), std::move(next),
                                     std::move(stmt), pos), pos);
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse, PositionInfo pos) {
    return DSLStatement(DSLCore::If(std::move(test), std::move(ifTrue), std::move(ifFalse),
                                    /*isStatic=*/false),
                        pos);
}

DSLGlobalVar InterfaceBlock(const DSLModifiers& modifiers,  skstd::string_view typeName,
                            SkTArray<DSLField> fields, skstd::string_view varName, int arraySize,
                            PositionInfo pos) {
    SkSL::ProgramKind kind = ThreadContext::GetProgramConfig()->fKind;
    if (kind != ProgramKind::kFragment &&
        kind != ProgramKind::kVertex) {
        ThreadContext::ReportError("interface blocks are not allowed in this kind of program", pos);
        return DSLGlobalVar();
    }
    return DSLCore::InterfaceBlock(modifiers, typeName, std::move(fields), varName, arraySize, pos);
}

DSLStatement Return(DSLExpression expr, PositionInfo pos) {
    return DSLCore::Return(std::move(expr), pos);
}

DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
                     PositionInfo pos) {
    return DSLExpression(DSLCore::Select(std::move(test), std::move(ifTrue), std::move(ifFalse)),
                         pos);
}

DSLStatement StaticIf(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse,
                      PositionInfo pos) {
    return DSLStatement(DSLCore::If(std::move(test), std::move(ifTrue), std::move(ifFalse),
                                    /*isStatic=*/true),
                         pos);
}

DSLPossibleStatement PossibleStaticSwitch(DSLExpression value, SkTArray<DSLCase> cases) {
    return DSLCore::Switch(std::move(value), std::move(cases), /*isStatic=*/true);
}

DSLStatement StaticSwitch(DSLExpression value, SkTArray<DSLCase> cases, PositionInfo pos) {
    return DSLStatement(PossibleStaticSwitch(std::move(value), std::move(cases)), pos);
}

DSLPossibleStatement PossibleSwitch(DSLExpression value, SkTArray<DSLCase> cases) {
    return DSLCore::Switch(std::move(value), std::move(cases), /*isStatic=*/false);
}

DSLStatement Switch(DSLExpression value, SkTArray<DSLCase> cases, PositionInfo pos) {
    return DSLStatement(PossibleSwitch(std::move(value), std::move(cases)), pos);
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

DSLExpression Atan(DSLExpression y_over_x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("atan", std::move(y_over_x)), pos);
}

DSLExpression Atan(DSLExpression y, DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("atan", std::move(y), std::move(x)), pos);
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

DSLExpression Round(DSLExpression x, PositionInfo pos) {
    return DSLExpression(DSLCore::Call("round", std::move(x)), pos);
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
