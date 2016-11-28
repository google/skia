/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include <stack>
#include <tuple>
#include <unordered_map>

#include "SkStream.h"
#include "SkSLCodeGenerator.h"
#include "SkSLMemoryLayout.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLBoolLiteral.h"
#include "ir/SkSLConstructor.h"
#include "ir/SkSLFloatLiteral.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLInterfaceBlock.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLFieldAccess.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionDeclaration.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLProgramElement.h"
#include "ir/SkSLReturnStatement.h"
#include "ir/SkSLStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLVarDeclarations.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLVariableReference.h"
#include "spirv.h"

namespace SkSL {

#define kLast_Capability SpvCapabilityMultiViewport

/**
 * Converts a Program into a SPIR-V binary.
 */
class SPIRVCodeGenerator : public CodeGenerator {
public:
    class LValue {
    public:
        virtual ~LValue() {}

        // returns a pointer to the lvalue, if possible. If the lvalue cannot be directly referenced
        // by a pointer (e.g. vector swizzles), returns 0.
        virtual SpvId getPointer() = 0;

        virtual SpvId load(SkWStream& out) = 0;

        virtual void store(SpvId value, SkWStream& out) = 0;
    };

    SPIRVCodeGenerator(const Context* context)
    : fContext(*context)
    , fDefaultLayout(MemoryLayout::k140_Standard)
    , fCapabilities(1 << SpvCapabilityShader)
    , fIdCount(1)
    , fBoolTrue(0)
    , fBoolFalse(0)
    , fCurrentBlock(0) {
        this->setupIntrinsics();
    }

    void generateCode(const Program& program, ErrorReporter& errors, SkWStream& out) override;

private:
    enum IntrinsicKind {
        kGLSL_STD_450_IntrinsicKind,
        kSPIRV_IntrinsicKind,
        kSpecial_IntrinsicKind
    };

    enum SpecialIntrinsic {
        kAtan_SpecialIntrinsic,
        kTexture_SpecialIntrinsic,
        kSubpassLoad_SpecialIntrinsic,
    };

    void setupIntrinsics();

    SpvId nextId();

    SpvId getType(const Type& type);

    SpvId getType(const Type& type, const MemoryLayout& layout);

    SpvId getFunctionType(const FunctionDeclaration& function);

    SpvId getPointerType(const Type& type, SpvStorageClass_ storageClass);

    SpvId getPointerType(const Type& type, const MemoryLayout& layout, 
                         SpvStorageClass_ storageClass);

    std::vector<SpvId> getAccessChain(const Expression& expr, SkWStream& out);

    void writeLayout(const Layout& layout, SpvId target);

    void writeLayout(const Layout& layout, SpvId target, int member);

    void writeStruct(const Type& type, const MemoryLayout& layout, SpvId resultId);

    void writeProgramElement(const ProgramElement& pe, SkWStream& out);

    SpvId writeInterfaceBlock(const InterfaceBlock& intf);

    SpvId writeFunctionStart(const FunctionDeclaration& f, SkWStream& out);
    
    SpvId writeFunctionDeclaration(const FunctionDeclaration& f, SkWStream& out);

    SpvId writeFunction(const FunctionDefinition& f, SkWStream& out);

    void writeGlobalVars(Program::Kind kind, const VarDeclarations& v, SkWStream& out);

    void writeVarDeclarations(const VarDeclarations& decl, SkWStream& out);

    SpvId writeVariableReference(const VariableReference& ref, SkWStream& out);

    std::unique_ptr<LValue> getLValue(const Expression& value, SkWStream& out);

    SpvId writeExpression(const Expression& expr, SkWStream& out);
    
    SpvId writeIntrinsicCall(const FunctionCall& c, SkWStream& out);

    SpvId writeFunctionCall(const FunctionCall& c, SkWStream& out);

    SpvId writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind, SkWStream& out);

    SpvId writeConstantVector(const Constructor& c);

    SpvId writeFloatConstructor(const Constructor& c, SkWStream& out);

    SpvId writeIntConstructor(const Constructor& c, SkWStream& out);
    
    SpvId writeMatrixConstructor(const Constructor& c, SkWStream& out);

    SpvId writeVectorConstructor(const Constructor& c, SkWStream& out);

    SpvId writeConstructor(const Constructor& c, SkWStream& out);

    SpvId writeFieldAccess(const FieldAccess& f, SkWStream& out);

    SpvId writeSwizzle(const Swizzle& swizzle, SkWStream& out);

    SpvId writeBinaryOperation(const Type& resultType, const Type& operandType, SpvId lhs, 
                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt, 
                               SpvOp_ ifBool, SkWStream& out);

    SpvId writeBinaryOperation(const BinaryExpression& expr, SpvOp_ ifFloat, SpvOp_ ifInt, 
                               SpvOp_ ifUInt, SkWStream& out);

    SpvId writeBinaryExpression(const BinaryExpression& b, SkWStream& out);

    SpvId writeTernaryExpression(const TernaryExpression& t, SkWStream& out);

    SpvId writeIndexExpression(const IndexExpression& expr, SkWStream& out);

    SpvId writeLogicalAnd(const BinaryExpression& b, SkWStream& out);

    SpvId writeLogicalOr(const BinaryExpression& o, SkWStream& out);

    SpvId writePrefixExpression(const PrefixExpression& p, SkWStream& out);

    SpvId writePostfixExpression(const PostfixExpression& p, SkWStream& out);

    SpvId writeBoolLiteral(const BoolLiteral& b);

    SpvId writeIntLiteral(const IntLiteral& i);

    SpvId writeFloatLiteral(const FloatLiteral& f);

    void writeStatement(const Statement& s, SkWStream& out);

    void writeBlock(const Block& b, SkWStream& out);

    void writeIfStatement(const IfStatement& stmt, SkWStream& out);

    void writeForStatement(const ForStatement& f, SkWStream& out);

    void writeReturnStatement(const ReturnStatement& r, SkWStream& out);

    void writeCapabilities(SkWStream& out);

    void writeInstructions(const Program& program, SkWStream& out);

    void writeOpCode(SpvOp_ opCode, int length, SkWStream& out);

    void writeWord(int32_t word, SkWStream& out);

    void writeString(const char* string, SkWStream& out);

    void writeLabel(SpvId id, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, const char* string, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, const char* string, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, const char* string,
                          SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, 
                          SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, int32_t word7, SkWStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, int32_t word7, int32_t word8, 
                          SkWStream& out);

    const Context& fContext;
    const MemoryLayout fDefaultLayout;
    ErrorReporter* fErrors;

    uint64_t fCapabilities;
    SpvId fIdCount;
    SpvId fGLSLExtendedInstructions;
    typedef std::tuple<IntrinsicKind, int32_t, int32_t, int32_t, int32_t> Intrinsic;
    std::unordered_map<SkString, Intrinsic> fIntrinsicMap;
    std::unordered_map<const FunctionDeclaration*, SpvId> fFunctionMap;
    std::unordered_map<const Variable*, SpvId> fVariableMap;
    std::unordered_map<const Variable*, int32_t> fInterfaceBlockMap;
    std::unordered_map<SkString, SpvId> fTypeMap;
    SkDynamicMemoryWStream fCapabilitiesBuffer;
    SkDynamicMemoryWStream fGlobalInitializersBuffer;
    SkDynamicMemoryWStream fConstantBuffer;
    SkDynamicMemoryWStream fExternalFunctionsBuffer;
    SkDynamicMemoryWStream fVariableBuffer;
    SkDynamicMemoryWStream fNameBuffer;
    SkDynamicMemoryWStream fDecorationBuffer;

    SpvId fBoolTrue;
    SpvId fBoolFalse;
    std::unordered_map<int64_t, SpvId> fIntConstants;
    std::unordered_map<uint64_t, SpvId> fUIntConstants;
    std::unordered_map<float, SpvId> fFloatConstants;
    std::unordered_map<double, SpvId> fDoubleConstants;
    // label of the current block, or 0 if we are not in a block
    SpvId fCurrentBlock;
    std::stack<SpvId> fBreakTarget;
    std::stack<SpvId> fContinueTarget;

    friend class PointerLValue;
    friend class SwizzleLValue;
};

}

#endif
