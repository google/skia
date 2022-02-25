/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_REHYDRATOR
#define SKSL_REHYDRATOR

#include "include/private/SkSLDefines.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLSymbol.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <vector>

namespace SkSL {

class Compiler;
class Context;
class ErrorReporter;
class Expression;
class ProgramElement;
class Statement;
class SymbolTable;
class Type;

/**
 * Interprets a simple bytecode format that encodes the structure of an SkSL IR tree. This is used
 * to process the .sksl files representing SkSL's core include files, so that they can be quickly
 * reconstituted at runtime.
 */
class Rehydrator {
public:
    static constexpr uint16_t kVersion = 8;

    // see binary_format.md for a description of the command data
    enum Command {
        kArrayType_Command,
        kBinary_Command,
        kBlock_Command,
        kBoolLiteral_Command,
        kBreak_Command,
        kBuiltinLayout_Command,
        kConstructorArray_Command,
        kConstructorArrayCast_Command,
        kConstructorCompound_Command,
        kConstructorCompoundCast_Command,
        kConstructorDiagonalMatrix_Command,
        kConstructorMatrixResize_Command,
        kConstructorScalarCast_Command,
        kConstructorSplat_Command,
        kConstructorStruct_Command,
        kContinue_Command,
        kDefaultLayout_Command,
        kDefaultModifiers_Command,
        kDiscard_Command,
        kDo_Command,
        kElements_Command,
        kElementsComplete_Command,
        kExpressionStatement_Command,
        kField_Command,
        kFieldAccess_Command,
        kFloatLiteral_Command,
        kFor_Command,
        kFunctionCall_Command,
        kFunctionDeclaration_Command,
        kFunctionDefinition_Command,
        kFunctionPrototype_Command,
        kGlobalVar_Command,
        kIf_Command,
        kIndex_Command,
        kInlineMarker_Command,
        kInterfaceBlock_Command,
        kIntLiteral_Command,
        kLayout_Command,
        kModifiers8Bit_Command,
        kModifiers_Command,
        kNop_Command,
        kPostfix_Command,
        kPrefix_Command,
        kProgram_Command,
        kReturn_Command,
        kSetting_Command,
        kSharedFunction_Command,
        kStructDefinition_Command,
        kStructType_Command,
        kSwitch_Command,
        kSwizzle_Command,
        kSymbolRef_Command,
        kSymbolTable_Command,
        kTernary_Command,
        kUnresolvedFunction_Command,
        kVariable_Command,
        kVarDeclaration_Command,
        kVariableReference_Command,
        kVoid_Command,
    };

    // src must remain in memory as long as the objects created from it do
    Rehydrator(Compiler& compiler, const uint8_t* src, size_t length,
            std::shared_ptr<SymbolTable> base = nullptr);

#ifdef SK_DEBUG
    ~Rehydrator();
#endif

    // Reads a symbol table and makes it current (inheriting from the previous current table)
    std::shared_ptr<SymbolTable> symbolTable();

    // Reads a collection of program elements and returns it
    std::vector<std::unique_ptr<ProgramElement>> elements();

    // Reads an entire program.
    //
    // NOTE: The program is initialized using a new ProgramConfig that may differ from the one that
    // was assigned to the context of the Compiler this Rehydrator was constructed with.
    std::unique_ptr<Program> program();

private:
    // If this ID appears in place of a symbol ID, it means the corresponding symbol isn't actually
    // present in the file as it's a builtin. The string name of the symbol follows.
    static constexpr uint16_t kBuiltin_Symbol = 65535;

    int8_t readS8() {
        SkASSERT(fIP < fEnd);
        return *(fIP++);
    }

    uint8_t readU8() {
        return this->readS8();
    }

    int16_t readS16() {
        uint8_t b1 = this->readU8();
        uint8_t b2 = this->readU8();
        return (b2 << 8) + b1;
    }

    uint16_t readU16() {
        return this->readS16();
    }

    int32_t readS32() {
        uint8_t b1 = this->readU8();
        uint8_t b2 = this->readU8();
        uint8_t b3 = this->readU8();
        uint8_t b4 = this->readU8();
        return (b4 << 24) + (b3 << 16) + (b2 << 8) + b1;
    }

    uint32_t readU32() {
        return this->readS32();
    }

    std::string_view readString() {
        uint16_t offset = this->readU16();
        uint8_t length = *(uint8_t*) (fStringStart + offset);
        const char* chars = (const char*) fStringStart + offset + 1;
        return std::string_view(chars, length);
    }

    void addSymbol(int id, const Symbol* symbol) {
        while ((size_t) id >= fSymbols.size()) {
            fSymbols.push_back(nullptr);
        }
        fSymbols[id] = symbol;
    }

    template<typename T>
    T* symbolRef(Symbol::Kind kind) {
        uint16_t result = this->readU16();
        SkASSERT(fSymbols.size() > result);
        return (T*) fSymbols[result];
    }

    /**
     * Reads either a symbol belonging to this program, or a named reference to a builtin symbol.
     * This has to be a separate method from symbolRef() because builtin symbols can be const, and
     * thus this method must have a const return, but there is at least one case in which we
     * specifically require a non-const return value.
     */
    const Symbol* possiblyBuiltinSymbolRef() {
        uint16_t id = this->readU16();
        if (id == kBuiltin_Symbol) {
            std::string_view name = this->readString();
            const Symbol* result = (*fSymbolTable)[name];
            SkASSERTF(result, "symbol '%s' not found", std::string(name).c_str());
            return result;
        }
        SkASSERT(fSymbols.size() > id);
        return fSymbols[id];
    }

    Layout layout();

    Modifiers modifiers();

    const Symbol* symbol();

    std::unique_ptr<ProgramElement> element();

    std::unique_ptr<Statement> statement();

    std::unique_ptr<Expression> expression();

    ExpressionArray expressionArray();

    const Type* type();

    Context& context() const;

    ErrorReporter* errorReporter() const { return this->context().fErrors; }

    ModifiersPool& modifiersPool() const { return *this->context().fModifiersPool; }

    Compiler& fCompiler;
    std::shared_ptr<SymbolTable> fSymbolTable;
    std::vector<const Symbol*> fSymbols;

    const uint8_t* fStringStart;
    const uint8_t* fIP;
    SkDEBUGCODE(const uint8_t* fEnd;)

    friend class AutoRehydratorSymbolTable;
    friend class Dehydrator;
};

}  // namespace SkSL

#endif
