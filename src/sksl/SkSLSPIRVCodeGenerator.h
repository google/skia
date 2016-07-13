/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include <sstream>
#include <stack>
#include <tuple>
#include <unordered_map>

#include "SkSLCodeGenerator.h"
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
#include "ir/SkSLVarDeclaration.h"
#include "ir/SkSLVarDeclarationStatement.h"
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

        virtual SpvId load(std::ostream& out) = 0;

        virtual void store(SpvId value, std::ostream& out) = 0;
    };

    SPIRVCodeGenerator()
    : fCapabilities(1 << SpvCapabilityShader)
    , fIdCount(1)
    , fBoolTrue(0)
    , fBoolFalse(0)
    , fCurrentBlock(0) {
        this->setupIntrinsics();
    }

    void generateCode(Program& program, std::ostream& out) override;

private:
    enum IntrinsicKind {
        kGLSL_STD_450_IntrinsicKind,
        kSPIRV_IntrinsicKind,
        kSpecial_IntrinsicKind
    };

    enum SpecialIntrinsic {
        kAtan_SpecialIntrinsic,
        kTexture_SpecialIntrinsic,
        kTexture2D_SpecialIntrinsic,
        kTextureProj_SpecialIntrinsic
    };

    void setupIntrinsics();

    SpvId nextId();

    SpvId getType(const Type& type);

    SpvId getFunctionType(std::shared_ptr<FunctionDeclaration> function);

    SpvId getPointerType(std::shared_ptr<Type> type, SpvStorageClass_ storageClass);

    std::vector<SpvId> getAccessChain(Expression& expr, std::ostream& out);

    void writeLayout(const Layout& layout, SpvId target);

    void writeLayout(const Layout& layout, SpvId target, int member);

    void writeStruct(const Type& type, SpvId resultId);

    void writeProgramElement(ProgramElement& pe, std::ostream& out);

    SpvId writeInterfaceBlock(InterfaceBlock& intf);

    SpvId writeFunctionStart(std::shared_ptr<FunctionDeclaration> f, std::ostream& out);
    
    SpvId writeFunctionDeclaration(std::shared_ptr<FunctionDeclaration> f, std::ostream& out);

    SpvId writeFunction(FunctionDefinition& f, std::ostream& out);

    void writeGlobalVars(VarDeclaration& v, std::ostream& out);

    void writeVarDeclaration(VarDeclaration& decl, std::ostream& out);

    SpvId writeVariableReference(VariableReference& ref, std::ostream& out);

    std::unique_ptr<LValue> getLValue(Expression& value, std::ostream& out);

    SpvId writeExpression(Expression& expr, std::ostream& out);
    
    SpvId writeIntrinsicCall(FunctionCall& c, std::ostream& out);

    SpvId writeFunctionCall(FunctionCall& c, std::ostream& out);

    SpvId writeSpecialIntrinsic(FunctionCall& c, SpecialIntrinsic kind, std::ostream& out);

    SpvId writeConstantVector(Constructor& c);

    SpvId writeFloatConstructor(Constructor& c, std::ostream& out);

    SpvId writeIntConstructor(Constructor& c, std::ostream& out);
    
    SpvId writeMatrixConstructor(Constructor& c, std::ostream& out);

    SpvId writeVectorConstructor(Constructor& c, std::ostream& out);

    SpvId writeConstructor(Constructor& c, std::ostream& out);

    SpvId writeFieldAccess(FieldAccess& f, std::ostream& out);

    SpvId writeSwizzle(Swizzle& swizzle, std::ostream& out);

    SpvId writeBinaryOperation(const Type& resultType, const Type& operandType, SpvId lhs, 
                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt, 
                               SpvOp_ ifBool, std::ostream& out);

    SpvId writeBinaryOperation(BinaryExpression& expr, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt,
                               std::ostream& out);

    SpvId writeBinaryExpression(BinaryExpression& b, std::ostream& out);

    SpvId writeTernaryExpression(TernaryExpression& t, std::ostream& out);

    SpvId writeIndexExpression(IndexExpression& expr, std::ostream& out);

    SpvId writeLogicalAnd(BinaryExpression& b, std::ostream& out);

    SpvId writeLogicalOr(BinaryExpression& o, std::ostream& out);

    SpvId writePrefixExpression(PrefixExpression& p, std::ostream& out);

    SpvId writePostfixExpression(PostfixExpression& p, std::ostream& out);

    SpvId writeBoolLiteral(BoolLiteral& b);

    SpvId writeIntLiteral(IntLiteral& i);

    SpvId writeFloatLiteral(FloatLiteral& f);

    void writeStatement(Statement& s, std::ostream& out);

    void writeBlock(Block& b, std::ostream& out);

    void writeIfStatement(IfStatement& stmt, std::ostream& out);

    void writeForStatement(ForStatement& f, std::ostream& out);

    void writeReturnStatement(ReturnStatement& r, std::ostream& out);

    void writeCapabilities(std::ostream& out);

    void writeInstructions(Program& program, std::ostream& out);

    void writeOpCode(SpvOp_ opCode, int length, std::ostream& out);

    void writeWord(int32_t word, std::ostream& out);

    void writeString(const char* string, std::ostream& out);

    void writeLabel(SpvId id, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, const char* string, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, const char* string, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, const char* string,
                          std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, 
                          std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, int32_t word7, std::ostream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, int32_t word7, int32_t word8, 
                          std::ostream& out);

    uint64_t fCapabilities;
    SpvId fIdCount;
    SpvId fGLSLExtendedInstructions;
    typedef std::tuple<IntrinsicKind, int32_t, int32_t, int32_t, int32_t> Intrinsic;
    std::unordered_map<std::string, Intrinsic> fIntrinsicMap;
    std::unordered_map<std::shared_ptr<FunctionDeclaration>, SpvId> fFunctionMap;
    std::unordered_map<std::shared_ptr<Variable>, SpvId> fVariableMap;
    std::unordered_map<std::shared_ptr<Variable>, int32_t> fInterfaceBlockMap;
    std::unordered_map<std::string, SpvId> fTypeMap;
    std::stringstream fCapabilitiesBuffer;
    std::stringstream fGlobalInitializersBuffer;
    std::stringstream fConstantBuffer;
    std::stringstream fExternalFunctionsBuffer;
    std::stringstream fVariableBuffer;
    std::stringstream fNameBuffer;
    std::stringstream fDecorationBuffer;

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
