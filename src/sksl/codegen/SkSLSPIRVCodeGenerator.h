/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLMemoryLayout.h"
#include "src/sksl/SkSLStringStream.h"
#include "src/sksl/codegen/SkSLCodeGenerator.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/spirv.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace SkSL {

class AnyConstructor;
class BinaryExpression;
class Block;
class ConstructorCompound;
class ConstructorCompoundCast;
class ConstructorDiagonalMatrix;
class ConstructorMatrixResize;
class ConstructorScalarCast;
class ConstructorSplat;
class Context;
class DoStatement;
class Expression;
class FieldAccess;
class ForStatement;
class FunctionCall;
class IfStatement;
class IndexExpression;
struct Layout;
class Literal;
class Operator;
class OutputStream;
class Position;
class PostfixExpression;
class PrefixExpression;
class ProgramElement;
class ReturnStatement;
class Statement;
class SwitchStatement;
class Swizzle;
class TernaryExpression;
class VarDeclaration;
class VariableReference;
enum class ProgramKind : int8_t;
enum IntrinsicKind : int8_t;
struct Program;

/**
 * Converts a Program into a SPIR-V binary.
 */
class SPIRVCodeGenerator : public CodeGenerator {
public:
    // We reserve an impossible SpvId as a sentinel. (NA meaning none, n/a, etc.)
    static constexpr SpvId NA = (SpvId)-1;

    class LValue {
    public:
        virtual ~LValue() {}

        // returns a pointer to the lvalue, if possible. If the lvalue cannot be directly referenced
        // by a pointer (e.g. vector swizzles), returns NA.
        virtual SpvId getPointer() { return NA; }

        // Returns true if a valid pointer returned by getPointer represents a memory object
        // (see https://github.com/KhronosGroup/SPIRV-Tools/issues/2892). Has no meaning if
        // getPointer() returns NA.
        virtual bool isMemoryObjectPointer() const { return true; }

        // Applies a swizzle to the components of the LValue, if possible. This is used to create
        // LValues that are swizzes-of-swizzles. Non-swizzle LValues can just return false.
        virtual bool applySwizzle(const ComponentArray& components, const Type& newType) {
            return false;
        }

        // Returns the storage class of the lvalue.
        virtual SpvStorageClass storageClass() const = 0;

        virtual SpvId load(OutputStream& out) = 0;

        virtual void store(SpvId value, OutputStream& out) = 0;
    };

    SPIRVCodeGenerator(const Context* context, const Program* program, OutputStream* out)
            : INHERITED(context, program, out) {}

    bool generateCode() override;

private:
    enum IntrinsicOpcodeKind {
        kGLSL_STD_450_IntrinsicOpcodeKind,
        kSPIRV_IntrinsicOpcodeKind,
        kSpecial_IntrinsicOpcodeKind,
        kInvalid_IntrinsicOpcodeKind,
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
        kTextureGrad_SpecialIntrinsic,
        kTextureLod_SpecialIntrinsic,
        kTextureRead_SpecialIntrinsic,
        kTextureWrite_SpecialIntrinsic,
        kTextureWidth_SpecialIntrinsic,
        kTextureHeight_SpecialIntrinsic,
        kAtomicAdd_SpecialIntrinsic,
        kAtomicLoad_SpecialIntrinsic,
        kAtomicStore_SpecialIntrinsic,
        kStorageBarrier_SpecialIntrinsic,
        kWorkgroupBarrier_SpecialIntrinsic,
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

    /**
     * Pass in the type to automatically add a RelaxedPrecision decoration for the id when
     * appropriate, or null to never add one.
     */
    SpvId nextId(const Type* type);

    SpvId nextId(Precision precision);

    SpvId getType(const Type& type);

    SpvId getType(const Type& type, const Layout& typeLayout, const MemoryLayout& memoryLayout);

    SpvId getFunctionType(const FunctionDeclaration& function);

    SpvId getFunctionParameterType(const Type& parameterType, const Layout& parameterLayout);

    SpvId getPointerType(const Type& type, SpvStorageClass_ storageClass);

    SpvId getPointerType(const Type& type,
                         const Layout& typeLayout,
                         const MemoryLayout& memoryLayout,
                         SpvStorageClass_ storageClass);

    skia_private::TArray<SpvId> getAccessChain(const Expression& expr, OutputStream& out);

    void writeLayout(const Layout& layout, SpvId target, Position pos);

    void writeFieldLayout(const Layout& layout, SpvId target, int member);

    SpvId writeStruct(const Type& type, const MemoryLayout& memoryLayout);

    void writeProgramElement(const ProgramElement& pe, OutputStream& out);

    SpvId writeInterfaceBlock(const InterfaceBlock& intf, bool appendRTFlip = true);

    SpvId writeFunctionStart(const FunctionDeclaration& f, OutputStream& out);

    SpvId writeFunctionDeclaration(const FunctionDeclaration& f, OutputStream& out);

    SpvId writeFunction(const FunctionDefinition& f, OutputStream& out);

    bool writeGlobalVarDeclaration(ProgramKind kind, const VarDeclaration& v);

    SpvId writeGlobalVar(ProgramKind kind, SpvStorageClass_, const Variable& v);

    void writeVarDeclaration(const VarDeclaration& var, OutputStream& out);

    SpvId writeVariableReference(const VariableReference& ref, OutputStream& out);

    int findUniformFieldIndex(const Variable& var) const;

    std::unique_ptr<LValue> getLValue(const Expression& value, OutputStream& out);

    SpvId writeExpression(const Expression& expr, OutputStream& out);

    SpvId writeIntrinsicCall(const FunctionCall& c, OutputStream& out);

    SpvId writeFunctionCallArgument(const FunctionCall& call,
                                    int argIndex,
                                    std::vector<TempVar>* tempVars,
                                    OutputStream& out,
                                    SpvId* outSynthesizedSamplerId = nullptr);

    void copyBackTempVars(const std::vector<TempVar>& tempVars, OutputStream& out);

    SpvId writeFunctionCall(const FunctionCall& c, OutputStream& out);


    void writeGLSLExtendedInstruction(const Type& type, SpvId id, SpvId floatInst,
                                      SpvId signedInst, SpvId unsignedInst,
                                      const skia_private::TArray<SpvId>& args, OutputStream& out);

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
    skia_private::TArray<SpvId> vectorize(const ExpressionArray& args, OutputStream& out);

    SpvId writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind, OutputStream& out);
    SpvId writeAtomicIntrinsic(const FunctionCall& c,
                               SpecialIntrinsic kind,
                               SpvId resultId,
                               OutputStream& out);

    SpvId writeScalarToMatrixSplat(const Type& matrixType, SpvId scalarId, OutputStream& out);

    SpvId castScalarToFloat(SpvId inputId, const Type& inputType, const Type& outputType,
                            OutputStream& out);

    SpvId castScalarToSignedInt(SpvId inputId, const Type& inputType, const Type& outputType,
                                OutputStream& out);

    SpvId castScalarToUnsignedInt(SpvId inputId, const Type& inputType, const Type& outputType,
                                  OutputStream& out);

    SpvId castScalarToBoolean(SpvId inputId, const Type& inputType, const Type& outputType,
                              OutputStream& out);

    SpvId castScalarToType(SpvId inputExprId, const Type& inputType, const Type& outputType,
                           OutputStream& out);

    /**
     * Writes a potentially-different-sized copy of a matrix. Entries which do not exist in the
     * source matrix are filled with zero; entries which do not exist in the destination matrix are
     * ignored.
     */
    SpvId writeMatrixCopy(SpvId src, const Type& srcType, const Type& dstType, OutputStream& out);

    void addColumnEntry(const Type& columnType,
                        skia_private::TArray<SpvId>* currentColumn,
                        skia_private::TArray<SpvId>* columnIds,
                        int rows, SpvId entry, OutputStream& out);

    SpvId writeConstructorCompound(const ConstructorCompound& c, OutputStream& out);

    SpvId writeMatrixConstructor(const ConstructorCompound& c, OutputStream& out);

    SpvId writeVectorConstructor(const ConstructorCompound& c, OutputStream& out);

    SpvId writeCompositeConstructor(const AnyConstructor& c, OutputStream& out);

    SpvId writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c, OutputStream& out);

    SpvId writeConstructorMatrixResize(const ConstructorMatrixResize& c, OutputStream& out);

    SpvId writeConstructorScalarCast(const ConstructorScalarCast& c, OutputStream& out);

    SpvId writeConstructorSplat(const ConstructorSplat& c, OutputStream& out);

    SpvId writeConstructorCompoundCast(const ConstructorCompoundCast& c, OutputStream& out);

    SpvId writeFieldAccess(const FieldAccess& f, OutputStream& out);

    SpvId writeSwizzle(const Expression& baseExpr,
                       const ComponentArray& components,
                       OutputStream& out);

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

    SpvId writeComponentwiseMatrixUnary(const Type& operandType,
                                        SpvId operand,
                                        SpvOp_ op,
                                        OutputStream& out);

    SpvId writeComponentwiseMatrixBinary(const Type& operandType, SpvId lhs, SpvId rhs,
                                         SpvOp_ op, OutputStream& out);

    SpvId writeBinaryOperation(const Type& resultType, const Type& operandType, SpvId lhs,
                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt, SpvOp_ ifUInt,
                               SpvOp_ ifBool, OutputStream& out);

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

    SpvId writeLiteral(double value, const Type& type);

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

    void writeString(std::string_view s, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, std::string_view string, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, std::string_view string,
                          OutputStream& out);

    void writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2, std::string_view string,
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

    // This form of writeInstruction can deduplicate redundant ops.
    struct Word;
    // 8 Words is enough for nearly all instructions (except variable-length instructions like
    // OpAccessChain or OpConstantComposite).
    using Words = skia_private::STArray<8, Word, true>;
    SpvId writeInstruction(
            SpvOp_ opCode, const skia_private::TArray<Word, true>& words, OutputStream& out);

    struct Instruction {
        SpvId fOp;
        int32_t fResultKind;
        skia_private::STArray<8, int32_t>  fWords;

        bool operator==(const Instruction& that) const;
        struct Hash;
    };

    static Instruction BuildInstructionKey(SpvOp_ opCode,
                                           const skia_private::TArray<Word, true>& words);

    // The writeOpXxxxx calls will simplify and deduplicate ops where possible.
    SpvId writeOpConstantTrue(const Type& type);
    SpvId writeOpConstantFalse(const Type& type);
    SpvId writeOpConstant(const Type& type, int32_t valueBits);
    SpvId writeOpConstantComposite(const Type& type, const skia_private::TArray<SpvId>& values);
    SpvId writeOpCompositeConstruct(const Type& type, const skia_private::TArray<SpvId>&,
                                    OutputStream& out);
    SpvId writeOpCompositeExtract(const Type& type, SpvId base, int component, OutputStream& out);
    SpvId writeOpCompositeExtract(const Type& type, SpvId base, int componentA, int componentB,
                                  OutputStream& out);
    SpvId writeOpLoad(SpvId type, Precision precision, SpvId pointer, OutputStream& out);
    void writeOpStore(SpvStorageClass_ storageClass, SpvId pointer, SpvId value, OutputStream& out);

    // Converts the provided SpvId(s) into an array of scalar OpConstants, if it can be done.
    bool toConstants(SpvId value, skia_private::TArray<SpvId>* constants);
    bool toConstants(SkSpan<const SpvId> values, skia_private::TArray<SpvId>* constants);

    // Extracts the requested component SpvId from a composite instruction, if it can be done.
    Instruction* resultTypeForInstruction(const Instruction& instr);
    int numComponentsForVecInstruction(const Instruction& instr);
    SpvId toComponent(SpvId id, int component);

    struct ConditionalOpCounts {
        int numReachableOps;
        int numStoreOps;
    };
    ConditionalOpCounts getConditionalOpCounts();
    void pruneConditionalOps(ConditionalOpCounts ops);

    enum StraightLineLabelType {
        // Use "BranchlessBlock" for blocks which are never explicitly branched-to at all. This
        // happens at the start of a function, or when we find unreachable code.
        kBranchlessBlock,

        // Use "BranchIsOnPreviousLine" when writing a label that comes immediately after its
        // associated branch. Example usage:
        // - SPIR-V does not implicitly fall through from one block to the next, so you may need to
        //   use an OpBranch to explicitly jump to the next block, even when they are adjacent in
        //   the code.
        // - The block immediately following an OpBranchConditional or OpSwitch.
        kBranchIsOnPreviousLine,
    };

    enum BranchingLabelType {
        // Use "BranchIsAbove" for labels which are referenced by OpBranch or OpBranchConditional
        // ops that are above the label in the code--i.e., the branch skips forward in the code.
        kBranchIsAbove,

        // Use "BranchIsBelow" for labels which are referenced by OpBranch or OpBranchConditional
        // ops below the label in the code--i.e., the branch jumps backward in the code.
        kBranchIsBelow,

        // Use "BranchesOnBothSides" for labels which have branches coming from both directions.
        kBranchesOnBothSides,
    };
    void writeLabel(SpvId label, StraightLineLabelType type, OutputStream& out);
    void writeLabel(SpvId label, BranchingLabelType type, ConditionalOpCounts ops,
                    OutputStream& out);

    MemoryLayout memoryLayoutForStorageClass(SpvStorageClass_ storageClass);
    MemoryLayout memoryLayoutForVariable(const Variable&) const;

    struct EntrypointAdapter {
        std::unique_ptr<FunctionDefinition> entrypointDef;
        std::unique_ptr<FunctionDeclaration> entrypointDecl;
    };

    EntrypointAdapter writeEntrypointAdapter(const FunctionDeclaration& main);

    struct UniformBuffer {
        std::unique_ptr<InterfaceBlock> fInterfaceBlock;
        std::unique_ptr<Variable> fInnerVariable;
        std::unique_ptr<Type> fStruct;
    };

    void writeUniformBuffer(std::shared_ptr<SymbolTable> topLevelSymbolTable);

    void addRTFlipUniform(Position pos);

    std::unique_ptr<Expression> identifier(std::string_view name);

    std::tuple<const Variable*, const Variable*> synthesizeTextureAndSampler(
            const Variable& combinedSampler);

    const MemoryLayout fDefaultMemoryLayout{MemoryLayout::Standard::k140};

    uint64_t fCapabilities = 0;
    SpvId fIdCount = 1;
    SpvId fGLSLExtendedInstructions;
    struct Intrinsic {
        IntrinsicOpcodeKind opKind;
        int32_t floatOp;
        int32_t signedOp;
        int32_t unsignedOp;
        int32_t boolOp;
    };
    Intrinsic getIntrinsic(IntrinsicKind) const;
    skia_private::THashMap<const FunctionDeclaration*, SpvId> fFunctionMap;
    skia_private::THashMap<const Variable*, SpvId> fVariableMap;
    skia_private::THashMap<const Type*, SpvId> fStructMap;
    StringStream fGlobalInitializersBuffer;
    StringStream fConstantBuffer;
    StringStream fVariableBuffer;
    StringStream fNameBuffer;
    StringStream fDecorationBuffer;

    // Mapping from combined sampler declarations to synthesized texture/sampler variables.
    // This is used when the sampler is declared as `layout(webgpu)` or `layout(direct3d)`.
    bool fUseTextureSamplerPairs = false;
    struct SynthesizedTextureSamplerPair {
        // The names of the synthesized variables. The Variable objects themselves store string
        // views referencing these strings. It is important for the std::string instances to have a
        // fixed memory location after the string views get created, which is why
        // `fSynthesizedSamplerMap` stores unique_ptr instead of values.
        std::string fTextureName;
        std::string fSamplerName;
        std::unique_ptr<Variable> fTexture;
        std::unique_ptr<Variable> fSampler;
    };
    skia_private::THashMap<const Variable*, std::unique_ptr<SynthesizedTextureSamplerPair>>
            fSynthesizedSamplerMap;

    // These caches map SpvIds to Instructions, and vice-versa. This enables us to deduplicate code
    // (by detecting an Instruction we've already issued and reusing the SpvId), and to introspect
    // and simplify code we've already emitted  (by taking a SpvId from an Instruction and following
    // it back to its source).

    // A map of instruction -> SpvId:
    skia_private::THashMap<Instruction, SpvId, Instruction::Hash> fOpCache;
    // A map of SpvId -> instruction:
    skia_private::THashMap<SpvId, Instruction> fSpvIdCache;
    // A map of SpvId -> value SpvId:
    skia_private::THashMap<SpvId, SpvId> fStoreCache;

    // "Reachable" ops are instructions which can safely be accessed from the current block.
    // For instance, if our SPIR-V contains `%3 = OpFAdd %1 %2`, we would be able to access and
    // reuse that computation on following lines. However, if that Add operation occurred inside an
    // `if` block, then its SpvId becomes inaccessible once we complete the if statement (since
    // depending on the if condition, we may or may not have actually done that computation). The
    // same logic applies to other control-flow blocks as well. Once an instruction becomes
    // unreachable, we remove it from both op-caches.
    skia_private::TArray<SpvId> fReachableOps;

    // The "store-ops" list contains a running list of all the pointers in the store cache. If a
    // store occurs inside of a conditional block, once that block exits, we no longer know what is
    // stored in that particular SpvId. At that point, we must remove any associated entry from the
    // store cache.
    skia_private::TArray<SpvId> fStoreOps;

    // label of the current block, or 0 if we are not in a block
    SpvId fCurrentBlock = 0;
    skia_private::TArray<SpvId> fBreakTarget;
    skia_private::TArray<SpvId> fContinueTarget;
    bool fWroteRTFlip = false;
    // holds variables synthesized during output, for lifetime purposes
    SymbolTable fSynthetics{/*builtin=*/true};
    // Holds a list of uniforms that were declared as globals at the top-level instead of in an
    // interface block.
    UniformBuffer fUniformBuffer;
    std::vector<const VarDeclaration*> fTopLevelUniforms;
    skia_private::THashMap<const Variable*, int>
            fTopLevelUniformMap;  // <var, UniformBuffer field index>
    SpvId fUniformBufferId = NA;

    friend class PointerLValue;
    friend class SwizzleLValue;

    using INHERITED = CodeGenerator;
};

}  // namespace SkSL

#endif
