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

#include "SkSLCodeGenerator.h"
#include "SkSLMemoryLayout.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLBoolLiteral.h"
#include "ir/SkSLConstructor.h"
#include "ir/SkSLDoStatement.h"
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
#include "ir/SkSLSwitchStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLVarDeclarations.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLVariableReference.h"
#include "ir/SkSLWhileStatement.h"
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

        virtual SpvId load(OutputStream& out) = 0;

        virtual void store(SpvId value, OutputStream& out) = 0;
    };

    SPIRVCodeGenerator(const Context* context, const Program* program, ErrorReporter* errors,
                       OutputStream* out)
    : INHERITED(program, errors, out)
    , fContext(*context)
    , fDefaultLayout(MemoryLayout::k140_Standard)
    , fCapabilities(0)
    , fIdCount(1)
    , fBoolTrue(0)
    , fBoolFalse(0)
    , fSetupFragPosition(false)
    , fCurrentBlock(0)
    , fSynthetics(nullptr, errors) {
        this->setupIntrinsics();
    }

    bool generateCode() override;

private:
    enum IntrinsicKind {
        kGLSL_STD_450_IntrinsicKind,
        kSPIRV_IntrinsicKind,
        kSpecial_IntrinsicKind
    };

    enum SpecialIntrinsic {
        kAtan_SpecialIntrinsic,
        kClamp_SpecialIntrinsic,
        kMax_SpecialIntrinsic,
        kMin_SpecialIntrinsic,
        kMix_SpecialIntrinsic,
        kMod_SpecialIntrinsic,
        kDFdy_SpecialIntrinsic,
        kSaturate_SpecialIntrinsic,
        kSubpassLoad_SpecialIntrinsic,
        kTexture_SpecialIntrinsic,
    };

    enum class Precision {
        kLow,
        kHigh,
    };

    void setupIntrinsics();

    SpvId nextId();

    Type getActualType(const Type& type);

    SpvId getType(const Type& type);

    SpvId getType(const Type& type, const MemoryLayout& layout);

    SpvId getImageType(const Type& type);

    SpvId getFunctionType(const FunctionDeclaration& function);

    SpvId getPointerType(const Type& type, SpvStorageClass_ storageClass);

    SpvId getPointerType(const Type& type, const MemoryLayout& layout,
                         SpvStorageClass_ storageClass);

    void writePrecisionModifier(Precision precision, SpvId id);

    void writePrecisionModifier(const Type& type, SpvId id);

    std::vector<SpvId> getAccessChain(const Expression& expr, OutputStream& out);

    void writeLayout(const Layout& layout, SpvId target);

    void writeLayout(const Layout& layout, SpvId target, int member);

    void writeStruct(const Type& type, const MemoryLayout& layout, SpvId resultId);

    void writeProgramElement(const ProgramElement& pe, OutputStream& out);

    SpvId writeInterfaceBlock(const InterfaceBlock& intf);

    SpvId writeFunctionStart(const FunctionDeclaration& f, OutputStream& out);

    SpvId writeFunctionDeclaration(const FunctionDeclaration& f, OutputStream& out);

    SpvId writeFunction(const FunctionDefinition& f, OutputStream& out);

    void writeGlobalVars(Program::Kind kind, const VarDeclarations& v, OutputStream& out);

    void writeVarDeclarations(const VarDeclarations& decl, OutputStream& out);

    SpvId writeVariableReference(const VariableReference& ref, OutputStream& out);

    std::unique_ptr<LValue> getLValue(const Expression& value, OutputStream& out);

    SpvId writeExpression(const Expression& expr, OutputStream& out);

    SpvId writeIntrinsicCall(const FunctionCall& c, OutputStream& out);

    SpvId writeFunctionCall(const FunctionCall& c, OutputStream& out);


    void writeGLSLExtendedInstruction(const Type& type, SpvId id, SpvId floatInst,
                                      SpvId signedInst, SpvId unsignedInst,
                                      const std::vector<SpvId>& args, OutputStream& out);

    /**
     * Given a list of potentially mixed scalars and vectors, promotes the scalars to match the
     * size of the vectors and returns the ids of the written expressions. e.g. given (float, vec2),
     * returns (vec2(float), vec2). It is an error to use mismatched vector sizes, e.g. (float,
     * vec2, vec3).
     */
    std::vector<SpvId> vectorize(const std::vector<std::unique_ptr<Expression>>& args,
                                 OutputStream& out);

    SpvId writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind, OutputStream& out);

    SpvId writeConstantVector(const Constructor& c);

    SpvId writeFloatConstructor(const Constructor& c, OutputStream& out);

    SpvId writeIntConstructor(const Constructor& c, OutputStream& out);

    SpvId writeUIntConstructor(const Constructor& c, OutputStream& out);

    /**
     * Writes a matrix with the diagonal entries all equal to the provided expression, and all other
     * entries equal to zero.
     */
    void writeUniformScaleMatrix(SpvId id, SpvId diagonal, const Type& type, OutputStream& out);

    /**
     * Writes a potentially-different-sized copy of a matrix. Entries which do not exist in the
     * source matrix are filled with zero; entries which do not exist in the destination matrix are
     * ignored.
     */
    void writeMatrixCopy(SpvId id, SpvId src, const Type& srcType, const Type& dstType,
                         OutputStream& out);

    SpvId writeMatrixConstructor(const Constructor& c, OutputStream& out);

    SpvId writeVectorConstructor(const Constructor& c, OutputStream& out);

    SpvId writeArrayConstructor(const Constructor& c, OutputStream& out);

    SpvId writeConstructor(const Constructor& c, OutputStream& out);

    SpvId writeFieldAccess(const FieldAccess& f, OutputStream& out);

    SpvId writeSwizzle(const Swizzle& swizzle, OutputStream& out);

    /**
     * Folds the potentially-vector result of a logical operation down to a single bool. If
     * operandType is a vector type, assumes that the intermediate result in id is a bvec of the
     * same dimensions, and applys all() to it to fold it down to a single bool value. Otherwise,
     * returns the original id value.
     */
    SpvId foldToBool(SpvId id, const Type& operandType, SpvOp op, OutputStream& out);

    SpvId writeMatrixComparison(const Type& operandType, SpvId lhs, SpvId rhs, SpvOp_ floatOperator,
                                SpvOp_ intOperator, SpvOp_ vectorMergeOperator,
                                SpvOp_ mergeOperator, OutputStream& out);

    SpvId writeComponentwiseMatrixBinary(const Type& operandType, SpvId lhs, SpvId rhs,
                                         SpvOp_ floatOperator, SpvOp_ intOperator,
                                         OutputStream& out);

    SpvId writeBinaryOperation(const Type& resultType, const Type& operandType, SpvId lhs,
                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt,
                               SpvOp_ ifBool, OutputStream& out);

    SpvId writeBinaryOperation(const BinaryExpression& expr, SpvOp_ ifFloat, SpvOp_ ifInt,
                               SpvOp_ ifUInt, OutputStream& out);

    SpvId writeBinaryExpression(const BinaryExpression& b, OutputStream& out);

    SpvId writeTernaryExpression(const TernaryExpression& t, OutputStream& out);

    SpvId writeIndexExpression(const IndexExpression& expr, OutputStream& out);

    SpvId writeLogicalAnd(const BinaryExpression& b, OutputStream& out);

    SpvId writeLogicalOr(const BinaryExpression& o, OutputStream& out);

    SpvId writePrefixExpression(const PrefixExpression& p, OutputStream& out);

    SpvId writePostfixExpression(const PostfixExpression& p, OutputStream& out);

    SpvId writeBoolLiteral(const BoolLiteral& b);

    SpvId writeIntLiteral(const IntLiteral& i);

    SpvId writeFloatLiteral(const FloatLiteral& f);

    void writeStatement(const Statement& s, OutputStream& out);

    void writeBlock(const Block& b, OutputStream& out);

    void writeIfStatement(const IfStatement& stmt, OutputStream& out);

    void writeForStatement(const ForStatement& f, OutputStream& out);

    void writeWhileStatement(const WhileStatement& w, OutputStream& out);

    void writeDoStatement(const DoStatement& d, OutputStream& out);

    void writeSwitchStatement(const SwitchStatement& s, OutputStream& out);

    void writeReturnStatement(const ReturnStatement& r, OutputStream& out);

    void writeCapabilities(OutputStream& out);

    void writeInstructions(const Program& program, OutputStream& out);

    void writeOpCode(SpvOp_ opCode, int length, OutputStream& out);

    void writeWord(int32_t word, OutputStream& out);

    void writeString(const char* string, size_t length, OutputStream& out);

    void writeLabel(SpvId id, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, StringFragment string, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, StringFragment string, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, StringFragment string,
                          OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3,
                          OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, int32_t word7, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, int32_t word3, int32_t word4,
                          int32_t word5, int32_t word6, int32_t word7, int32_t word8,
                          OutputStream& out);

    void writeGeometryShaderExecutionMode(SpvId entryPoint, OutputStream& out);

    const Context& fContext;
    const MemoryLayout fDefaultLayout;

    uint64_t fCapabilities;
    SpvId fIdCount;
    SpvId fGLSLExtendedInstructions;
    typedef std::tuple<IntrinsicKind, int32_t, int32_t, int32_t, int32_t> Intrinsic;
    std::unordered_map<String, Intrinsic> fIntrinsicMap;
    std::unordered_map<const FunctionDeclaration*, SpvId> fFunctionMap;
    std::unordered_map<const Variable*, SpvId> fVariableMap;
    std::unordered_map<const Variable*, int32_t> fInterfaceBlockMap;
    std::unordered_map<String, SpvId> fImageTypeMap;
    std::unordered_map<String, SpvId> fTypeMap;
    StringStream fCapabilitiesBuffer;
    StringStream fGlobalInitializersBuffer;
    StringStream fConstantBuffer;
    StringStream fExtraGlobalsBuffer;
    StringStream fExternalFunctionsBuffer;
    StringStream fVariableBuffer;
    StringStream fNameBuffer;
    StringStream fDecorationBuffer;

    SpvId fBoolTrue;
    SpvId fBoolFalse;
    std::unordered_map<int64_t, SpvId> fIntConstants;
    std::unordered_map<uint64_t, SpvId> fUIntConstants;
    std::unordered_map<float, SpvId> fFloatConstants;
    std::unordered_map<double, SpvId> fDoubleConstants;
    // The constant float2(0, 1), used in swizzling
    SpvId fConstantZeroOneVector = 0;
    bool fSetupFragPosition;
    // label of the current block, or 0 if we are not in a block
    SpvId fCurrentBlock;
    std::stack<SpvId> fBreakTarget;
    std::stack<SpvId> fContinueTarget;
    SpvId fRTHeightStructId = (SpvId) -1;
    SpvId fRTHeightFieldIndex = (SpvId) -1;
    // holds variables synthesized during output, for lifetime purposes
    SymbolTable fSynthetics;
    int fSkInCount = 1;

    friend class PointerLValue;
    friend class SwizzleLValue;

    typedef CodeGenerator INHERITED;
};

}

#endif
