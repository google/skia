/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLCore.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLModifiersPool.h"  // IWYU pragma: keep
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/DSLType.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

#include <type_traits>
#include <utility>
#include <vector>

using namespace skia_private;

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
                                                      std::move(compiler.fContext->fSymbolTable),
                                                      std::move(instance.fPool),
                                                      instance.fInterface);
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
        SkASSERT(!compiler.fContext->fSymbolTable);
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

    static DSLStatement Declare(DSLVar& var, Position pos) {
        return DSLWriter::Declaration(var);
    }

    static DSLStatement Declare(TArray<DSLVar>& vars, Position pos) {
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

    static void Declare(TArray<DSLGlobalVar>& vars, Position pos) {
        for (DSLGlobalVar& v : vars) {
            Declare(v, pos);
        }
    }

    static DSLExpression InterfaceBlock(const DSLModifiers& modifiers, std::string_view typeName,
                                        TArray<DSLField> fields, std::string_view varName,
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
            if (std::unique_ptr<SkSL::InterfaceBlock> intf =
                        SkSL::InterfaceBlock::Convert(ThreadContext::Context(), pos, skslVar)) {
                ThreadContext::ProgramElements().push_back(std::move(intf));
                // Return a VariableReference to the global variable tied to the interface block.
                return DSLExpression(var);
            }
        }

        // The InterfaceBlock couldn't be created; return poison.
        return DSLExpression(nullptr);
    }

    static DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
            Position pos) {
        auto result = TernaryExpression::Convert(ThreadContext::Context(), pos, test.release(),
                                                 ifTrue.release(), ifFalse.release());
        SkASSERT(!result || result->fPosition == pos);
        return DSLExpression(std::move(result), pos);
    }
};

std::unique_ptr<SkSL::Program> ReleaseProgram(std::unique_ptr<std::string> source) {
    return DSLCore::ReleaseProgram(std::move(source));
}

void AddExtension(std::string_view name, Position pos) {
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::Extension>(pos, name));
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

DSLStatement Declare(TArray<DSLVar>& vars, Position pos) {
    return DSLCore::Declare(vars, pos);
}

void Declare(DSLGlobalVar& var, Position pos) {
    DSLCore::Declare(var, pos);
}

void Declare(TArray<DSLGlobalVar>& vars, Position pos) {
    DSLCore::Declare(vars, pos);
}

DSLExpression InterfaceBlock(const DSLModifiers& modifiers, std::string_view typeName,
                             TArray<DSLField> fields, std::string_view varName, int arraySize,
                             Position pos) {
    return DSLCore::InterfaceBlock(modifiers, typeName, std::move(fields), varName, arraySize, pos);
}

DSLExpression Select(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse,
                     Position pos) {
    return DSLCore::Select(std::move(test), std::move(ifTrue), std::move(ifFalse), pos);
}

} // namespace dsl

} // namespace SkSL
