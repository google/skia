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
#include <unordered_set>

#include "src/core/SkOpts.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"

namespace SkSL {

class BinaryExpression;
class Block;
class ConstructorCompound;
class ConstructorCompoundCast;
class ConstructorDiagonalMatrix;
class ConstructorMatrixResize;
class ConstructorScalarCast;
class ConstructorSplat;
class DoStatement;
class FieldAccess;
class ForStatement;
class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class FunctionPrototype;
class IfStatement;
struct IndexExpression;
class InterfaceBlock;
enum IntrinsicKind : int8_t;
class Literal;
class Operator;
class PostfixExpression;
class PrefixExpression;
class ReturnStatement;
class Setting;
class StructDefinition;
class SwitchStatement;
struct Swizzle;
class TernaryExpression;
class VarDeclaration;
class VariableReference;

struct SPIRVNumberConstant {
    bool operator==(const SPIRVNumberConstant& that) const {
        return fValueBits == that.fValueBits &&
               fKind      == that.fKind;
    }
    int32_t                fValueBits;
    SkSL::Type::NumberKind fKind;
};

struct SPIRVVectorConstant {
    bool operator==(const SPIRVVectorConstant& that) const {
        return fTypeId     == that.fTypeId &&
               fValueId[0] == that.fValueId[0] &&
               fValueId[1] == that.fValueId[1] &&
               fValueId[2] == that.fValueId[2] &&
               fValueId[3] == that.fValueId[3];
    }
    SpvId fTypeId;
    SpvId fValueId[4];
};

}  // namespace SkSL

namespace std {

template <>
struct hash<SkSL::SPIRVNumberConstant> {
    size_t operator()(const SkSL::SPIRVNumberConstant& key) const {
        return key.fValueBits ^ (int)key.fKind;
    }
};

template <>
struct hash<SkSL::SPIRVVectorConstant> {
    size_t operator()(const SkSL::SPIRVVectorConstant& key) const {
        return SkOpts::hash(&key, sizeof(key));
    }
};

}  // namespace std

namespace SkSL {

/**
 * Converts a Program into a SPIR-V binary.
 */
class SPIRVCodeGenerator : public CodeGenerator {
public:
    class LValue {
    public:
        virtual ~LValue() {}

        // returns a pointer to the lvalue, if possible. If the lvalue cannot be directly referenced
        // by a pointer (e.g. vector swizzles), returns -1.
        virtual SpvId getPointer() { return -1; }

        // Returns true if a valid pointer returned by getPointer represents a memory object
        // (see https://github.com/KhronosGroup/SPIRV-Tools/issues/2892). Has no meaning if
        // getPointer() returns -1.
        virtual bool isMemoryObjectPointer() const { return true; }

        // Applies a swizzle to the components of the LValue, if possible. This is used to create
        // LValues that are swizzes-of-swizzles. Non-swizzle LValues can just return false.
        virtual bool applySwizzle(const ComponentArray& components, const Type& newType) {
            return false;
        }

        virtual SpvId load(OutputStream& out) = 0;

        virtual void store(SpvId value, OutputStream& out) = 0;
    };

    SPIRVCodeGenerator(const Context* context,
                       const Program* program,
                       OutputStream* out)
            : INHERITED(context, program, out)
            , fDefaultLayout(MemoryLayout::k140_Standard)
            , fCapabilities(0)
            , fIdCount(1)
            , fSetupFragPosition(false)
            , fCurrentBlock(0)
            , fSynthetics(fContext, /*builtin=*/true) {
        this->setupIntrinsics();
    }

    bool generateCode() override;

private:
    enum IntrinsicOpcodeKind {
        kGLSL_STD_450_IntrinsicOpcodeKind,
        kSPIRV_IntrinsicOpcodeKind,
        kSpecial_IntrinsicOpcodeKind
    };

    enum SpecialIntrinsic {
        kAtan_SpecialIntrinsic,
        kClamp_SpecialIntrinsic,
        kMatrixCompMult_SpecialIntrinsic,
        kMax_SpecialIntrinsic,
        kMin_SpecialIntrinsic,
        kMix_SpecialIntrinsic,
        kMod_SpecialIntrinsic,
        kDFdy_SpecialIntrinsic,
        kSaturate_SpecialIntrinsic,
        kSampledImage_SpecialIntrinsic,
        kSmoothStep_SpecialIntrinsic,
        kStep_SpecialIntrinsic,
        kSubpassLoad_SpecialIntrinsic,
        kTexture_SpecialIntrinsic,
    };

    enum class Precision {
        kDefault,
        kRelaxed,
    };

    struct TempVar {
        SpvId spvId;
        const Type* type;
        std::unique_ptr<SPIRVCodeGenerator::LValue> lvalue;
    };

    void setupIntrinsics();

    /**
     * Pass in the type to automatically add a RelaxedPrecision decoration for the id when
     * appropriate, or null to never add one.
     */
    SpvId nextId(const Type* type);

    SpvId nextId(Precision precision);

    const Type& getActualType(const Type& type);

    SpvId getType(const Type& type);

    SpvId getType(const Type& type, const MemoryLayout& layout);

    SpvId getImageType(const Type& type);

    SpvId getFunctionType(const FunctionDeclaration& function);

    SpvId getPointerType(const Type& type, SpvStorageClass_ storageClass);

    SpvId getPointerType(const Type& type, const MemoryLayout& layout,
                         SpvStorageClass_ storageClass);

    std::vector<SpvId> getAccessChain(const Expression& expr, OutputStream& out);

    void writeLayout(const Layout& layout, SpvId target);

    void writeLayout(const Layout& layout, SpvId target, int member);

    void writeStruct(const Type& type, const MemoryLayout& layout, SpvId resultId);

    void writeProgramElement(const ProgramElement& pe, OutputStream& out);

    SpvId writeInterfaceBlock(const InterfaceBlock& intf, bool appendRTFlip = true);

    SpvId writeFunctionStart(const FunctionDeclaration& f, OutputStream& out);

    SpvId writeFunctionDeclaration(const FunctionDeclaration& f, OutputStream& out);

    SpvId writeFunction(const FunctionDefinition& f, OutputStream& out);

    void writeGlobalVar(ProgramKind kind, const VarDeclaration& v);

    void writeVarDeclaration(const VarDeclaration& var, OutputStream& out);

    SpvId writeVariableReference(const VariableReference& ref, OutputStream& out);

    int findUniformFieldIndex(const Variable& var) const;

    std::unique_ptr<LValue> getLValue(const Expression& value, OutputStream& out);

    SpvId writeExpression(const Expression& expr, OutputStream& out);

    SpvId writeIntrinsicCall(const FunctionCall& c, OutputStream& out);

    SpvId writeFunctionCallArgument(const Expression& arg,
                                    const Modifiers& paramModifiers,
                                    std::vector<TempVar>* tempVars,
                                    OutputStream& out);

    void copyBackTempVars(const std::vector<TempVar>& tempVars, OutputStream& out);

    SpvId writeFunctionCall(const FunctionCall& c, OutputStream& out);


    void writeGLSLExtendedInstruction(const Type& type, SpvId id, SpvId floatInst,
                                      SpvId signedInst, SpvId unsignedInst,
                                      const std::vector<SpvId>& args, OutputStream& out);

    /**
     * Promotes an expression to a vector. If the expression is already a vector with vectorSize
     * columns, returns it unmodified. If the expression is a scalar, either promotes it to a
     * vector (if vectorSize > 1) or returns it unmodified (if vectorSize == 1). Asserts if the
     * expression is already a vector and it does not have vectorSize columns.
     */
    SpvId vectorize(const Expression& expr, int vectorSize, OutputStream& out);

    /**
     * Given a list of potentially mixed scalars and vectors, promotes the scalars to match the
     * size of the vectors and returns the ids of the written expressions. e.g. given (float, vec2),
     * returns (vec2(float), vec2). It is an error to use mismatched vector sizes, e.g. (float,
     * vec2, vec3).
     */
    std::vector<SpvId> vectorize(const ExpressionArray& args, OutputStream& out);

    SpvId writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind, OutputStream& out);

    SpvId writeConstantVector(const AnyConstructor& c);

    SpvId writeScalarToMatrixSplat(const Type& matrixType, SpvId scalarId, OutputStream& out);

    SpvId writeFloatConstructor(const AnyConstructor& c, OutputStream& out);

    SpvId castScalarToFloat(SpvId inputId, const Type& inputType, const Type& outputType,
                            OutputStream& out);

    SpvId writeIntConstructor(const AnyConstructor& c, OutputStream& out);

    SpvId castScalarToSignedInt(SpvId inputId, const Type& inputType, const Type& outputType,
                                OutputStream& out);

    SpvId writeUIntConstructor(const AnyConstructor& c, OutputStream& out);

    SpvId castScalarToUnsignedInt(SpvId inputId, const Type& inputType, const Type& outputType,
                                  OutputStream& out);

    SpvId writeBooleanConstructor(const AnyConstructor& c, OutputStream& out);

    SpvId castScalarToBoolean(SpvId inputId, const Type& inputType, const Type& outputType,
                              OutputStream& out);

    SpvId castScalarToType(SpvId inputExprId, const Type& inputType, const Type& outputType,
                           OutputStream& out);

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
    SpvId writeMatrixCopy(SpvId src, const Type& srcType, const Type& dstType, OutputStream& out);

    void addColumnEntry(const Type& columnType, std::vector<SpvId>* currentColumn,
                        std::vector<SpvId>* columnIds, int rows, SpvId entry, OutputStream& out);

    SpvId writeConstructorCompound(const ConstructorCompound& c, OutputStream& out);

    SpvId writeMatrixConstructor(const ConstructorCompound& c, OutputStream& out);

    SpvId writeVectorConstructor(const ConstructorCompound& c, OutputStream& out);

    SpvId writeCompositeConstructor(const AnyConstructor& c, OutputStream& out);

    SpvId writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c, OutputStream& out);

    SpvId writeConstructorMatrixResize(const ConstructorMatrixResize& c, OutputStream& out);

    SpvId writeConstructorScalarCast(const ConstructorScalarCast& c, OutputStream& out);

    SpvId writeConstructorSplat(const ConstructorSplat& c, OutputStream& out);

    SpvId writeConstructorCompoundCast(const ConstructorCompoundCast& c, OutputStream& out);

    SpvId writeComposite(const std::vector<SpvId>& arguments, const Type& type, OutputStream& out);

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

    SpvId writeStructComparison(const Type& structType, SpvId lhs, Operator op, SpvId rhs,
                                OutputStream& out);

    SpvId writeArrayComparison(const Type& structType, SpvId lhs, Operator op, SpvId rhs,
                               OutputStream& out);

    // Used by writeStructComparison and writeArrayComparison to logically combine field-by-field
    // comparisons into an overall comparison result.
    // - `a.x == b.x` merged with `a.y == b.y` generates `(a.x == b.x) && (a.y == b.y)`
    // - `a.x != b.x` merged with `a.y != b.y` generates `(a.x != b.x) || (a.y != b.y)`
    SpvId mergeComparisons(SpvId comparison, SpvId allComparisons, Operator op, OutputStream& out);

    SpvId writeComponentwiseMatrixBinary(const Type& operandType, SpvId lhs, SpvId rhs,
                                         SpvOp_ op, OutputStream& out);

    SpvId writeBinaryOperation(const Type& resultType, const Type& operandType, SpvId lhs,
                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt,
                               SpvOp_ ifBool, OutputStream& out);

    SpvId writeBinaryOperation(const BinaryExpression& expr, SpvOp_ ifFloat, SpvOp_ ifInt,
                               SpvOp_ ifUInt, OutputStream& out);

    SpvId writeReciprocal(const Type& type, SpvId value, OutputStream& out);

    SpvId writeBinaryExpression(const Type& leftType, SpvId lhs, Operator op,
                                const Type& rightType, SpvId rhs, const Type& resultType,
                                OutputStream& out);

    SpvId writeBinaryExpression(const BinaryExpression& b, OutputStream& out);

    SpvId writeTernaryExpression(const TernaryExpression& t, OutputStream& out);

    SpvId writeIndexExpression(const IndexExpression& expr, OutputStream& out);

    SpvId writeLogicalAnd(const Expression& left, const Expression& right, OutputStream& out);

    SpvId writeLogicalOr(const Expression& left, const Expression& right, OutputStream& out);

    SpvId writePrefixExpression(const PrefixExpression& p, OutputStream& out);

    SpvId writePostfixExpression(const PostfixExpression& p, OutputStream& out);

    SpvId writeLiteral(const Literal& f);

    void writeStatement(const Statement& s, OutputStream& out);

    void writeBlock(const Block& b, OutputStream& out);

    void writeIfStatement(const IfStatement& stmt, OutputStream& out);

    void writeForStatement(const ForStatement& f, OutputStream& out);

    void writeDoStatement(const DoStatement& d, OutputStream& out);

    void writeSwitchStatement(const SwitchStatement& s, OutputStream& out);

    void writeReturnStatement(const ReturnStatement& r, OutputStream& out);

    void writeCapabilities(OutputStream& out);

    void writeInstructions(const Program& program, OutputStream& out);

    void writeOpCode(SpvOp_ opCode, int length, OutputStream& out);

    void writeWord(int32_t word, OutputStream& out);

    void writeString(skstd::string_view s, OutputStream& out);

    void writeLabel(SpvId id, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, skstd::string_view string, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, skstd::string_view string,
                          OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, skstd::string_view string,
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

    bool isDead(const Variable& var) const;

    MemoryLayout memoryLayoutForVariable(const Variable&) const;

    struct EntrypointAdapter {
        std::unique_ptr<FunctionDefinition> entrypointDef;
        std::unique_ptr<FunctionDeclaration> entrypointDecl;
        Layout fLayout;
        Modifiers fModifiers;
    };

    EntrypointAdapter writeEntrypointAdapter(const FunctionDeclaration& main);

    struct UniformBuffer {
        std::unique_ptr<InterfaceBlock> fInterfaceBlock;
        std::unique_ptr<Variable> fInnerVariable;
        std::unique_ptr<Type> fStruct;
    };

    void writeUniformBuffer(std::shared_ptr<SymbolTable> topLevelSymbolTable);

    void addRTFlipUniform(int line);

    const MemoryLayout fDefaultLayout;

    uint64_t fCapabilities;
    SpvId fIdCount;
    SpvId fGLSLExtendedInstructions;
    typedef std::tuple<IntrinsicOpcodeKind, int32_t, int32_t, int32_t, int32_t> Intrinsic;
    std::unordered_map<IntrinsicKind, Intrinsic> fIntrinsicMap;
    std::unordered_map<const FunctionDeclaration*, SpvId> fFunctionMap;
    std::unordered_map<const Variable*, SpvId> fVariableMap;
    std::unordered_map<const Variable*, int32_t> fInterfaceBlockMap;
    std::unordered_map<String, SpvId> fImageTypeMap;
    std::unordered_map<String, SpvId> fTypeMap;
    StringStream fCapabilitiesBuffer;
    StringStream fGlobalInitializersBuffer;
    StringStream fConstantBuffer;
    StringStream fExtraGlobalsBuffer;
    StringStream fVariableBuffer;
    StringStream fNameBuffer;
    StringStream fDecorationBuffer;

    std::unordered_map<SPIRVNumberConstant, SpvId> fNumberConstants;
    std::unordered_map<SPIRVVectorConstant, SpvId> fVectorConstants;
    bool fSetupFragPosition;
    // label of the current block, or 0 if we are not in a block
    SpvId fCurrentBlock;
    std::stack<SpvId> fBreakTarget;
    std::stack<SpvId> fContinueTarget;
    bool fWroteRTFlip = false;
    // holds variables synthesized during output, for lifetime purposes
    SymbolTable fSynthetics;
    int fSkInCount = 1;
    // Holds a list of uniforms that were declared as globals at the top-level instead of in an
    // interface block.
    UniformBuffer fUniformBuffer;
    std::vector<const VarDeclaration*> fTopLevelUniforms;
    std::unordered_map<const Variable*, int> fTopLevelUniformMap; //<var, UniformBuffer field index>
    std::unordered_set<const Variable*> fSPIRVBonusVariables;
    SpvId fUniformBufferId = -1;

    friend class PointerLValue;
    friend class SwizzleLValue;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
