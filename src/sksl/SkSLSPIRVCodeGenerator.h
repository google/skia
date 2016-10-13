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

        virtual SpvId load(std::ostream& out) = 0;

        virtual void store(SpvId value, std::ostream& out) = 0;
    };

    SPIRVCodeGenerator(const Context* context)
    : fContext(*context)
    , fCapabilities(1 << SpvCapabilityShader)
    , fIdCount(1)
    , fBoolTrue(0)
    , fBoolFalse(0)
    , fCurrentBlock(0) {
        this->setupIntrinsics();
    }

    void generateCode(const Program& program, std::ostream& out) override;

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

    SpvId getFunctionType(const FunctionDeclaration& function);

    SpvId getPointerType(const Type& type, SpvStorageClass_ storageClass);

    std::vector<SpvId> getAccessChain(const Expression& expr, std::ostream& out);

    void writeLayout(const Layout& layout, SpvId target);

    void writeLayout(const Layout& layout, SpvId target, int member);

    void writeStruct(const Type& type, SpvId resultId);

    void writeProgramElement(const ProgramElement& pe, std::ostream& out);

    SpvId writeInterfaceBlock(const InterfaceBlock& intf);

    SpvId writeFunctionStart(const FunctionDeclaration& f, std::ostream& out);
    
    SpvId writeFunctionDeclaration(const FunctionDeclaration& f, std::ostream& out);

    SpvId writeFunction(const FunctionDefinition& f, std::ostream& out);

    void writeGlobalVars(Program::Kind kind, const VarDeclarations& v, std::ostream& out);

    void writeVarDeclarations(const VarDeclarations& decl, std::ostream& out);

    SpvId writeVariableReference(const VariableReference& ref, std::ostream& out);

    std::unique_ptr<LValue> getLValue(const Expression& value, std::ostream& out);

    SpvId writeExpression(const Expression& expr, std::ostream& out);
    
    SpvId writeIntrinsicCall(const FunctionCall& c, std::ostream& out);

    SpvId writeFunctionCall(const FunctionCall& c, std::ostream& out);

    SpvId writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind, std::ostream& out);

    SpvId writeConstantVector(const Constructor& c);

    SpvId writeFloatConstructor(const Constructor& c, std::ostream& out);

    SpvId writeIntConstructor(const Constructor& c, std::ostream& out);
    
    SpvId writeMatrixConstructor(const Constructor& c, std::ostream& out);

    SpvId writeVectorConstructor(const Constructor& c, std::ostream& out);

    SpvId writeConstructor(const Constructor& c, std::ostream& out);

    SpvId writeFieldAccess(const FieldAccess& f, std::ostream& out);

    SpvId writeSwizzle(const Swizzle& swizzle, std::ostream& out);

    SpvId writeBinaryOperation(const Type& resultType, const Type& operandType, SpvId lhs, 
                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt, 
                               SpvOp_ ifBool, std::ostream& out);

    SpvId writeBinaryOperation(const BinaryExpression& expr, SpvOp_ ifFloat, SpvOp_ ifInt, 
                               SpvOp_ ifUInt, std::ostream& out);

    SpvId writeBinaryExpression(const BinaryExpression& b, std::ostream& out);

    SpvId writeTernaryExpression(const TernaryExpression& t, std::ostream& out);

    SpvId writeIndexExpression(const IndexExpression& expr, std::ostream& out);

    SpvId writeLogicalAnd(const BinaryExpression& b, std::ostream& out);

    SpvId writeLogicalOr(const BinaryExpression& o, std::ostream& out);

    SpvId writePrefixExpression(const PrefixExpression& p, std::ostream& out);

    SpvId writePostfixExpression(const PostfixExpression& p, std::ostream& out);

    SpvId writeBoolLiteral(const BoolLiteral& b);

    SpvId writeIntLiteral(const IntLiteral& i);

    SpvId writeFloatLiteral(const FloatLiteral& f);

    void writeStatement(const Statement& s, std::ostream& out);

    void writeBlock(const Block& b, std::ostream& out);

    void writeIfStatement(const IfStatement& stmt, std::ostream& out);

    void writeForStatement(const ForStatement& f, std::ostream& out);

    void writeReturnStatement(const ReturnStatement& r, std::ostream& out);

    void writeCapabilities(std::ostream& out);

    void writeInstructions(const Program& program, std::ostream& out);

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

    const Context& fContext;

    uint64_t fCapabilities;
    SpvId fIdCount;
    SpvId fGLSLExtendedInstructions;
    typedef std::tuple<IntrinsicKind, int32_t, int32_t, int32_t, int32_t> Intrinsic;
    std::unordered_map<std::string, Intrinsic> fIntrinsicMap;
    std::unordered_map<const FunctionDeclaration*, SpvId> fFunctionMap;
    std::unordered_map<const Variable*, SpvId> fVariableMap;
    std::unordered_map<const Variable*, int32_t> fInterfaceBlockMap;
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
