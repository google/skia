/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRGENERATOR
#define SKSL_IRGENERATOR

#include <unordered_map>
#include <unordered_set>

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace dsl {
    class DSLCore;
    class DSLExpression;
    class DSLFunction;
    class DSLGlobalVar;
    class DSLVar;
    class DSLWriter;
}

class ExternalFunction;
class FunctionCall;
class StructDefinition;
struct ParsedModule;
struct Swizzle;

/**
 * Performs semantic analysis on an abstract syntax tree (AST) and produces the corresponding
 * (unoptimized) intermediate representation (IR).
 */
class IRGenerator {
public:
    IRGenerator(const Context* context);

    struct IRBundle {
        std::vector<std::unique_ptr<ProgramElement>> fElements;
        std::vector<const ProgramElement*>           fSharedElements;
        std::shared_ptr<SymbolTable>                 fSymbolTable;
        Program::Inputs                              fInputs;
    };

    void start(const ParsedModule& base,
               std::vector<std::unique_ptr<ProgramElement>>* elements,
               std::vector<const ProgramElement*>* sharedElements);

    /**
     * If externalFunctions is supplied, those values are registered in the symbol table of the
     * Program, but ownership is *not* transferred. It is up to the caller to keep them alive.
     */
    IRBundle convertProgram(
            const ParsedModule& base,
            bool isBuiltinCode,
            skstd::string_view text);

    const Program::Settings& settings() const { return fContext.fConfig->fSettings; }
    ProgramKind programKind() const { return fContext.fConfig->fKind; }

    ErrorReporter& errorReporter() const { return *fContext.fErrors; }

    std::shared_ptr<SymbolTable>& symbolTable() {
        return fSymbolTable;
    }

    void setSymbolTable(std::shared_ptr<SymbolTable>& symbolTable) {
        fSymbolTable = symbolTable;
    }

    static void CheckModifiers(const Context& context,
                               int line,
                               const Modifiers& modifiers,
                               int permittedModifierFlags,
                               int permittedLayoutFlags);

    std::unique_ptr<Expression> convertIdentifier(int line, skstd::string_view identifier);

    const Context& fContext;

private:
    IRGenerator::IRBundle finish();

    /** Appends sk_Position fixup to the bottom of main() if this is a vertex program. */
    void appendRTAdjustFixupToVertexMain(const FunctionDeclaration& decl, Block* body);

    // Runtime effects (and the interpreter, which uses the same CPU runtime) require adherence to
    // the strict rules from The OpenGL ES Shading Language Version 1.00. (Including Appendix A).
    bool strictES2Mode() const {
        return fContext.fConfig->strictES2Mode();
    }

    bool isRuntimeEffect() const {
        return ProgramConfig::IsRuntimeEffect(fContext.fConfig->fKind);
    }

    const ShaderCapsClass& caps() const {
        return fContext.fCaps;
    }

    ModifiersPool& modifiersPool() const {
        return *fContext.fModifiersPool;
    }

    Program::Inputs fInputs;

    std::shared_ptr<SymbolTable> fSymbolTable = nullptr;
    std::unordered_set<const Type*> fDefinedStructs;
    std::vector<std::unique_ptr<ProgramElement>>* fProgramElements = nullptr;
    std::vector<const ProgramElement*>*           fSharedElements = nullptr;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class AutoSwitchLevel;
    friend class AutoDisableInline;
    friend class Compiler;
    friend class DSLParser;
    friend class ThreadContext;
    friend class dsl::DSLCore;
    friend class dsl::DSLExpression;
    friend class dsl::DSLFunction;
    friend class dsl::DSLGlobalVar;
    friend class dsl::DSLVar;
    friend class dsl::DSLWriter;
};

}  // namespace SkSL

#endif
