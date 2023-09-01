/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkEnumBitMask.h"
#include "src/core/SkChecksum.h"
#include "src/sksl/GLSL.std.450.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIntrinsicList.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLPool.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/SkSLUtil.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFieldSymbol.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLPoison.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLTransform.h"
#include "src/utils/SkBitSet.h"

#include <cstring>
#include <set>
#include <string>
#include <utility>

using namespace skia_private;

#define kLast_Capability SpvCapabilityMultiViewport

constexpr int DEVICE_FRAGCOORDS_BUILTIN = -1000;
constexpr int DEVICE_CLOCKWISE_BUILTIN  = -1001;

namespace SkSL {

// Equality and hash operators for Instructions.
bool SPIRVCodeGenerator::Instruction::operator==(const SPIRVCodeGenerator::Instruction& that) const {
    return fOp         == that.fOp &&
           fResultKind == that.fResultKind &&
           fWords      == that.fWords;
}

struct SPIRVCodeGenerator::Instruction::Hash {
    uint32_t operator()(const SPIRVCodeGenerator::Instruction& key) const {
        uint32_t hash = key.fResultKind;
        hash = SkChecksum::Hash32(&key.fOp, sizeof(key.fOp), hash);
        hash = SkChecksum::Hash32(key.fWords.data(), key.fWords.size() * sizeof(int32_t), hash);
        return hash;
    }
};

// This class is used to pass values and result placeholder slots to writeInstruction.
struct SPIRVCodeGenerator::Word {
    enum Kind {
        kNone,  // intended for use as a sentinel, not part of any Instruction
        kSpvId,
        kNumber,
        kDefaultPrecisionResult,
        kRelaxedPrecisionResult,
        kUniqueResult,
        kKeyedResult,
    };

    Word(SpvId id) : fValue(id), fKind(Kind::kSpvId) {}
    Word(int32_t val, Kind kind) : fValue(val), fKind(kind) {}

    static Word Number(int32_t val) {
        return Word{val, Kind::kNumber};
    }

    static Word Result(const Type& type) {
        return (type.hasPrecision() && !type.highPrecision()) ? RelaxedResult() : Result();
    }

    static Word RelaxedResult() {
        return Word{(int32_t)NA, kRelaxedPrecisionResult};
    }

    static Word UniqueResult() {
        return Word{(int32_t)NA, kUniqueResult};
    }

    static Word Result() {
        return Word{(int32_t)NA, kDefaultPrecisionResult};
    }

    // Unlike a Result (where the result ID is always deduplicated to its first instruction) or a
    // UniqueResult (which always produces a new instruction), a KeyedResult allows an instruction
    // to be deduplicated among those that share the same `key`.
    static Word KeyedResult(int32_t key) { return Word{key, Kind::kKeyedResult}; }

    bool isResult() const { return fKind >= Kind::kDefaultPrecisionResult; }

    int32_t fValue;
    Kind fKind;
};

// Skia's magic number is 31 and goes in the top 16 bits. We can use the lower bits to version the
// sksl generator if we want.
// https://github.com/KhronosGroup/SPIRV-Headers/blob/master/include/spirv/spir-v.xml#L84
static const int32_t SKSL_MAGIC  = 0x001F0000;

SPIRVCodeGenerator::Intrinsic SPIRVCodeGenerator::getIntrinsic(IntrinsicKind ik) const {

#define ALL_GLSL(x) Intrinsic{kGLSL_STD_450_IntrinsicOpcodeKind, GLSLstd450 ## x, \
                              GLSLstd450 ## x, GLSLstd450 ## x, GLSLstd450 ## x}
#define BY_TYPE_GLSL(ifFloat, ifInt, ifUInt) Intrinsic{kGLSL_STD_450_IntrinsicOpcodeKind, \
                                                       GLSLstd450 ## ifFloat,             \
                                                       GLSLstd450 ## ifInt,               \
                                                       GLSLstd450 ## ifUInt,              \
                                                       SpvOpUndef}
#define ALL_SPIRV(x) Intrinsic{kSPIRV_IntrinsicOpcodeKind, \
                               SpvOp ## x, SpvOp ## x, SpvOp ## x, SpvOp ## x}
#define BOOL_SPIRV(x) Intrinsic{kSPIRV_IntrinsicOpcodeKind, \
                                SpvOpUndef, SpvOpUndef, SpvOpUndef, SpvOp ## x}
#define FLOAT_SPIRV(x) Intrinsic{kSPIRV_IntrinsicOpcodeKind, \
                                 SpvOp ## x, SpvOpUndef, SpvOpUndef, SpvOpUndef}
#define SPECIAL(x) Intrinsic{kSpecial_IntrinsicOpcodeKind, k ## x ## _SpecialIntrinsic, \
                             k ## x ## _SpecialIntrinsic, k ## x ## _SpecialIntrinsic,  \
                             k ## x ## _SpecialIntrinsic}

    switch (ik) {
        case k_round_IntrinsicKind:          return ALL_GLSL(Round);
        case k_roundEven_IntrinsicKind:      return ALL_GLSL(RoundEven);
        case k_trunc_IntrinsicKind:          return ALL_GLSL(Trunc);
        case k_abs_IntrinsicKind:            return BY_TYPE_GLSL(FAbs, SAbs, SAbs);
        case k_sign_IntrinsicKind:           return BY_TYPE_GLSL(FSign, SSign, SSign);
        case k_floor_IntrinsicKind:          return ALL_GLSL(Floor);
        case k_ceil_IntrinsicKind:           return ALL_GLSL(Ceil);
        case k_fract_IntrinsicKind:          return ALL_GLSL(Fract);
        case k_radians_IntrinsicKind:        return ALL_GLSL(Radians);
        case k_degrees_IntrinsicKind:        return ALL_GLSL(Degrees);
        case k_sin_IntrinsicKind:            return ALL_GLSL(Sin);
        case k_cos_IntrinsicKind:            return ALL_GLSL(Cos);
        case k_tan_IntrinsicKind:            return ALL_GLSL(Tan);
        case k_asin_IntrinsicKind:           return ALL_GLSL(Asin);
        case k_acos_IntrinsicKind:           return ALL_GLSL(Acos);
        case k_atan_IntrinsicKind:           return SPECIAL(Atan);
        case k_sinh_IntrinsicKind:           return ALL_GLSL(Sinh);
        case k_cosh_IntrinsicKind:           return ALL_GLSL(Cosh);
        case k_tanh_IntrinsicKind:           return ALL_GLSL(Tanh);
        case k_asinh_IntrinsicKind:          return ALL_GLSL(Asinh);
        case k_acosh_IntrinsicKind:          return ALL_GLSL(Acosh);
        case k_atanh_IntrinsicKind:          return ALL_GLSL(Atanh);
        case k_pow_IntrinsicKind:            return ALL_GLSL(Pow);
        case k_exp_IntrinsicKind:            return ALL_GLSL(Exp);
        case k_log_IntrinsicKind:            return ALL_GLSL(Log);
        case k_exp2_IntrinsicKind:           return ALL_GLSL(Exp2);
        case k_log2_IntrinsicKind:           return ALL_GLSL(Log2);
        case k_sqrt_IntrinsicKind:           return ALL_GLSL(Sqrt);
        case k_inverse_IntrinsicKind:        return ALL_GLSL(MatrixInverse);
        case k_outerProduct_IntrinsicKind:   return ALL_SPIRV(OuterProduct);
        case k_transpose_IntrinsicKind:      return ALL_SPIRV(Transpose);
        case k_isinf_IntrinsicKind:          return ALL_SPIRV(IsInf);
        case k_isnan_IntrinsicKind:          return ALL_SPIRV(IsNan);
        case k_inversesqrt_IntrinsicKind:    return ALL_GLSL(InverseSqrt);
        case k_determinant_IntrinsicKind:    return ALL_GLSL(Determinant);
        case k_matrixCompMult_IntrinsicKind: return SPECIAL(MatrixCompMult);
        case k_matrixInverse_IntrinsicKind:  return ALL_GLSL(MatrixInverse);
        case k_mod_IntrinsicKind:            return SPECIAL(Mod);
        case k_modf_IntrinsicKind:           return ALL_GLSL(Modf);
        case k_min_IntrinsicKind:            return SPECIAL(Min);
        case k_max_IntrinsicKind:            return SPECIAL(Max);
        case k_clamp_IntrinsicKind:          return SPECIAL(Clamp);
        case k_saturate_IntrinsicKind:       return SPECIAL(Saturate);
        case k_dot_IntrinsicKind:            return FLOAT_SPIRV(Dot);
        case k_mix_IntrinsicKind:            return SPECIAL(Mix);
        case k_step_IntrinsicKind:           return SPECIAL(Step);
        case k_smoothstep_IntrinsicKind:     return SPECIAL(SmoothStep);
        case k_fma_IntrinsicKind:            return ALL_GLSL(Fma);
        case k_frexp_IntrinsicKind:          return ALL_GLSL(Frexp);
        case k_ldexp_IntrinsicKind:          return ALL_GLSL(Ldexp);

#define PACK(type) case k_pack##type##_IntrinsicKind:   return ALL_GLSL(Pack##type); \
                   case k_unpack##type##_IntrinsicKind: return ALL_GLSL(Unpack##type)
        PACK(Snorm4x8);
        PACK(Unorm4x8);
        PACK(Snorm2x16);
        PACK(Unorm2x16);
        PACK(Half2x16);
        PACK(Double2x32);
#undef PACK

        case k_length_IntrinsicKind:        return ALL_GLSL(Length);
        case k_distance_IntrinsicKind:      return ALL_GLSL(Distance);
        case k_cross_IntrinsicKind:         return ALL_GLSL(Cross);
        case k_normalize_IntrinsicKind:     return ALL_GLSL(Normalize);
        case k_faceforward_IntrinsicKind:   return ALL_GLSL(FaceForward);
        case k_reflect_IntrinsicKind:       return ALL_GLSL(Reflect);
        case k_refract_IntrinsicKind:       return ALL_GLSL(Refract);
        case k_bitCount_IntrinsicKind:      return ALL_SPIRV(BitCount);
        case k_findLSB_IntrinsicKind:       return ALL_GLSL(FindILsb);
        case k_findMSB_IntrinsicKind:       return BY_TYPE_GLSL(FindSMsb, FindSMsb, FindUMsb);
        case k_dFdx_IntrinsicKind:          return FLOAT_SPIRV(DPdx);
        case k_dFdy_IntrinsicKind:          return SPECIAL(DFdy);
        case k_fwidth_IntrinsicKind:        return FLOAT_SPIRV(Fwidth);

        case k_sample_IntrinsicKind:      return SPECIAL(Texture);
        case k_sampleGrad_IntrinsicKind:  return SPECIAL(TextureGrad);
        case k_sampleLod_IntrinsicKind:   return SPECIAL(TextureLod);
        case k_subpassLoad_IntrinsicKind: return SPECIAL(SubpassLoad);

        case k_textureRead_IntrinsicKind:  return SPECIAL(TextureRead);
        case k_textureWrite_IntrinsicKind:  return SPECIAL(TextureWrite);
        case k_textureWidth_IntrinsicKind:  return SPECIAL(TextureWidth);
        case k_textureHeight_IntrinsicKind:  return SPECIAL(TextureHeight);

        case k_floatBitsToInt_IntrinsicKind:  return ALL_SPIRV(Bitcast);
        case k_floatBitsToUint_IntrinsicKind: return ALL_SPIRV(Bitcast);
        case k_intBitsToFloat_IntrinsicKind:  return ALL_SPIRV(Bitcast);
        case k_uintBitsToFloat_IntrinsicKind: return ALL_SPIRV(Bitcast);

        case k_any_IntrinsicKind:   return BOOL_SPIRV(Any);
        case k_all_IntrinsicKind:   return BOOL_SPIRV(All);
        case k_not_IntrinsicKind:   return BOOL_SPIRV(LogicalNot);

        case k_equal_IntrinsicKind:
            return Intrinsic{kSPIRV_IntrinsicOpcodeKind,
                             SpvOpFOrdEqual,
                             SpvOpIEqual,
                             SpvOpIEqual,
                             SpvOpLogicalEqual};
        case k_notEqual_IntrinsicKind:
            return Intrinsic{kSPIRV_IntrinsicOpcodeKind,
                             SpvOpFUnordNotEqual,
                             SpvOpINotEqual,
                             SpvOpINotEqual,
                             SpvOpLogicalNotEqual};
        case k_lessThan_IntrinsicKind:
            return Intrinsic{kSPIRV_IntrinsicOpcodeKind,
                             SpvOpFOrdLessThan,
                             SpvOpSLessThan,
                             SpvOpULessThan,
                             SpvOpUndef};
        case k_lessThanEqual_IntrinsicKind:
            return Intrinsic{kSPIRV_IntrinsicOpcodeKind,
                             SpvOpFOrdLessThanEqual,
                             SpvOpSLessThanEqual,
                             SpvOpULessThanEqual,
                             SpvOpUndef};
        case k_greaterThan_IntrinsicKind:
            return Intrinsic{kSPIRV_IntrinsicOpcodeKind,
                             SpvOpFOrdGreaterThan,
                             SpvOpSGreaterThan,
                             SpvOpUGreaterThan,
                             SpvOpUndef};
        case k_greaterThanEqual_IntrinsicKind:
            return Intrinsic{kSPIRV_IntrinsicOpcodeKind,
                             SpvOpFOrdGreaterThanEqual,
                             SpvOpSGreaterThanEqual,
                             SpvOpUGreaterThanEqual,
                             SpvOpUndef};

        case k_atomicAdd_IntrinsicKind:   return SPECIAL(AtomicAdd);
        case k_atomicLoad_IntrinsicKind:  return SPECIAL(AtomicLoad);
        case k_atomicStore_IntrinsicKind: return SPECIAL(AtomicStore);

        case k_storageBarrier_IntrinsicKind:   return SPECIAL(StorageBarrier);
        case k_workgroupBarrier_IntrinsicKind: return SPECIAL(WorkgroupBarrier);
        default:
            return Intrinsic{kInvalid_IntrinsicOpcodeKind, 0, 0, 0, 0};
    }
}

void SPIRVCodeGenerator::writeWord(int32_t word, OutputStream& out) {
    out.write((const char*) &word, sizeof(word));
}

static bool is_float(const Type& type) {
    return (type.isScalar() || type.isVector() || type.isMatrix()) &&
           type.componentType().isFloat();
}

static bool is_signed(const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isSigned();
}

static bool is_unsigned(const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isUnsigned();
}

static bool is_bool(const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isBoolean();
}

template <typename T>
static T pick_by_type(const Type& type, T ifFloat, T ifInt, T ifUInt, T ifBool) {
    if (is_float(type)) {
        return ifFloat;
    }
    if (is_signed(type)) {
        return ifInt;
    }
    if (is_unsigned(type)) {
        return ifUInt;
    }
    if (is_bool(type)) {
        return ifBool;
    }
    SkDEBUGFAIL("unrecognized type");
    return ifFloat;
}

static bool is_out(ModifierFlags f) {
    return SkToBool(f & ModifierFlag::kOut);
}

static bool is_in(ModifierFlags f) {
    if (f & ModifierFlag::kIn) {
        return true;  // `in` and `inout` both count
    }
    // If neither in/out flag is set, the type is implicitly `in`.
    return !SkToBool(f & ModifierFlag::kOut);
}

static bool is_control_flow_op(SpvOp_ op) {
    switch (op) {
        case SpvOpReturn:
        case SpvOpReturnValue:
        case SpvOpKill:
        case SpvOpSwitch:
        case SpvOpBranch:
        case SpvOpBranchConditional:
            return true;
        default:
            return false;
    }
}

static bool is_globally_reachable_op(SpvOp_ op) {
    switch (op) {
        case SpvOpConstant:
        case SpvOpConstantTrue:
        case SpvOpConstantFalse:
        case SpvOpConstantComposite:
        case SpvOpTypeVoid:
        case SpvOpTypeInt:
        case SpvOpTypeFloat:
        case SpvOpTypeBool:
        case SpvOpTypeVector:
        case SpvOpTypeMatrix:
        case SpvOpTypeArray:
        case SpvOpTypePointer:
        case SpvOpTypeFunction:
        case SpvOpTypeRuntimeArray:
        case SpvOpTypeStruct:
        case SpvOpTypeImage:
        case SpvOpTypeSampledImage:
        case SpvOpTypeSampler:
        case SpvOpVariable:
        case SpvOpFunction:
        case SpvOpFunctionParameter:
        case SpvOpFunctionEnd:
        case SpvOpExecutionMode:
        case SpvOpMemoryModel:
        case SpvOpCapability:
        case SpvOpExtInstImport:
        case SpvOpEntryPoint:
        case SpvOpSource:
        case SpvOpSourceExtension:
        case SpvOpName:
        case SpvOpMemberName:
        case SpvOpDecorate:
        case SpvOpMemberDecorate:
            return true;
        default:
            return false;
    }
}

void SPIRVCodeGenerator::writeOpCode(SpvOp_ opCode, int length, OutputStream& out) {
    SkASSERT(opCode != SpvOpLoad || &out != &fConstantBuffer);
    SkASSERT(opCode != SpvOpUndef);
    bool foundDeadCode = false;
    if (is_control_flow_op(opCode)) {
        // This instruction causes us to leave the current block.
        foundDeadCode = (fCurrentBlock == 0);
        fCurrentBlock = 0;
    } else if (!is_globally_reachable_op(opCode)) {
        foundDeadCode = (fCurrentBlock == 0);
    }

    if (foundDeadCode) {
        // We just encountered dead code--an instruction that don't have an associated block.
        // Synthesize a label if this happens; this is necessary to satisfy the validator.
        this->writeLabel(this->nextId(nullptr), kBranchlessBlock, out);
    }

    this->writeWord((length << 16) | opCode, out);
}

void SPIRVCodeGenerator::writeLabel(SpvId label, StraightLineLabelType, OutputStream& out) {
    // The straight-line label type is not important; in any case, no caches are invalidated.
    SkASSERT(!fCurrentBlock);
    fCurrentBlock = label;
    this->writeInstruction(SpvOpLabel, label, out);
}

void SPIRVCodeGenerator::writeLabel(SpvId label, BranchingLabelType type,
                                    ConditionalOpCounts ops, OutputStream& out) {
    switch (type) {
        case kBranchIsBelow:
        case kBranchesOnBothSides:
            // With a backward or bidirectional branch, we haven't seen the code between the label
            // and the branch yet, so any stored value is potentially suspect. Without scanning
            // ahead to check, the only safe option is to ditch the store cache entirely.
            fStoreCache.reset();
            [[fallthrough]];

        case kBranchIsAbove:
            // With a forward branch, we can rely on stores that we had cached at the start of the
            // statement/expression, if they haven't been touched yet. Anything newer than that is
            // pruned.
            this->pruneConditionalOps(ops);
            break;
    }

    // Emit the label.
    this->writeLabel(label, kBranchlessBlock, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, OutputStream& out) {
    this->writeOpCode(opCode, 1, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, OutputStream& out) {
    this->writeOpCode(opCode, 2, out);
    this->writeWord(word1, out);
}

void SPIRVCodeGenerator::writeString(std::string_view s, OutputStream& out) {
    out.write(s.data(), s.length());
    switch (s.length() % 4) {
        case 1:
            out.write8(0);
            [[fallthrough]];
        case 2:
            out.write8(0);
            [[fallthrough]];
        case 3:
            out.write8(0);
            break;
        default:
            this->writeWord(0, out);
            break;
    }
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, std::string_view string,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 1 + (string.length() + 4) / 4, out);
    this->writeString(string, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, std::string_view string,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 2 + (string.length() + 4) / 4, out);
    this->writeWord(word1, out);
    this->writeString(string, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          std::string_view string, OutputStream& out) {
    this->writeOpCode(opCode, 3 + (string.length() + 4) / 4, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeString(string, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 3, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          int32_t word3, OutputStream& out) {
    this->writeOpCode(opCode, 4, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeWord(word3, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          int32_t word3, int32_t word4, OutputStream& out) {
    this->writeOpCode(opCode, 5, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeWord(word3, out);
    this->writeWord(word4, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          int32_t word3, int32_t word4, int32_t word5,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 6, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeWord(word3, out);
    this->writeWord(word4, out);
    this->writeWord(word5, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          int32_t word3, int32_t word4, int32_t word5,
                                          int32_t word6, OutputStream& out) {
    this->writeOpCode(opCode, 7, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeWord(word3, out);
    this->writeWord(word4, out);
    this->writeWord(word5, out);
    this->writeWord(word6, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          int32_t word3, int32_t word4, int32_t word5,
                                          int32_t word6, int32_t word7, OutputStream& out) {
    this->writeOpCode(opCode, 8, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeWord(word3, out);
    this->writeWord(word4, out);
    this->writeWord(word5, out);
    this->writeWord(word6, out);
    this->writeWord(word7, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          int32_t word3, int32_t word4, int32_t word5,
                                          int32_t word6, int32_t word7, int32_t word8,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 9, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeWord(word3, out);
    this->writeWord(word4, out);
    this->writeWord(word5, out);
    this->writeWord(word6, out);
    this->writeWord(word7, out);
    this->writeWord(word8, out);
}

SPIRVCodeGenerator::Instruction SPIRVCodeGenerator::BuildInstructionKey(
        SpvOp_ opCode, const TArray<Word>& words) {
    // Assemble a cache key for this instruction.
    Instruction key;
    key.fOp = opCode;
    key.fWords.resize(words.size());
    key.fResultKind = Word::Kind::kNone;

    for (int index = 0; index < words.size(); ++index) {
        const Word& word = words[index];
        key.fWords[index] = word.fValue;
        if (word.isResult()) {
            SkASSERT(key.fResultKind == Word::Kind::kNone);
            key.fResultKind = word.fKind;
        }
    }

    return key;
}

SpvId SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode,
                                           const TArray<Word>& words,
                                           OutputStream& out) {
    // writeOpLoad and writeOpStore have dedicated code.
    SkASSERT(opCode != SpvOpLoad);
    SkASSERT(opCode != SpvOpStore);

    // If this instruction exists in our op cache, return the cached SpvId.
    Instruction key = BuildInstructionKey(opCode, words);
    if (SpvId* cachedOp = fOpCache.find(key)) {
        return *cachedOp;
    }

    SpvId result = NA;
    Precision precision = Precision::kDefault;

    switch (key.fResultKind) {
        case Word::Kind::kUniqueResult:
            // The instruction returns a SpvId, but we do not want deduplication.
            result = this->nextId(Precision::kDefault);
            fSpvIdCache.set(result, key);
            break;

        case Word::Kind::kNone:
            // The instruction doesn't return a SpvId, but we can still cache and deduplicate it.
            fOpCache.set(key, result);
            break;

        case Word::Kind::kRelaxedPrecisionResult:
            precision = Precision::kRelaxed;
            [[fallthrough]];

        case Word::Kind::kKeyedResult:
            [[fallthrough]];

        case Word::Kind::kDefaultPrecisionResult:
            // Consume a new SpvId.
            result = this->nextId(precision);
            fOpCache.set(key, result);
            fSpvIdCache.set(result, key);

            // Globally-reachable ops are not subject to the whims of flow control.
            if (!is_globally_reachable_op(opCode)) {
                fReachableOps.push_back(result);
            }
            break;

        default:
            SkDEBUGFAIL("unexpected result kind");
            break;
    }

    // Write the requested instruction.
    this->writeOpCode(opCode, words.size() + 1, out);
    for (const Word& word : words) {
        if (word.isResult()) {
            SkASSERT(result != NA);
            this->writeWord(result, out);
        } else {
            this->writeWord(word.fValue, out);
        }
    }

    // Return the result.
    return result;
}

SpvId SPIRVCodeGenerator::writeOpLoad(SpvId type,
                                      Precision precision,
                                      SpvId pointer,
                                      OutputStream& out) {
    // Look for this pointer in our load-cache.
    if (SpvId* cachedOp = fStoreCache.find(pointer)) {
        return *cachedOp;
    }

    // Write the requested OpLoad instruction.
    SpvId result = this->nextId(precision);
    this->writeInstruction(SpvOpLoad, type, result, pointer, out);
    return result;
}

void SPIRVCodeGenerator::writeOpStore(SpvStorageClass_ storageClass,
                                      SpvId pointer,
                                      SpvId value,
                                      OutputStream& out) {
    // Write the uncached SpvOpStore directly.
    this->writeInstruction(SpvOpStore, pointer, value, out);

    if (storageClass == SpvStorageClassFunction) {
        // Insert a pointer-to-SpvId mapping into the load cache. A writeOpLoad to this pointer will
        // return the cached value as-is.
        fStoreCache.set(pointer, value);
        fStoreOps.push_back(pointer);
    }
}

SpvId SPIRVCodeGenerator::writeOpConstantTrue(const Type& type) {
    return this->writeInstruction(SpvOpConstantTrue,
                                  Words{this->getType(type), Word::Result()},
                                  fConstantBuffer);
}

SpvId SPIRVCodeGenerator::writeOpConstantFalse(const Type& type) {
    return this->writeInstruction(SpvOpConstantFalse,
                                  Words{this->getType(type), Word::Result()},
                                  fConstantBuffer);
}

SpvId SPIRVCodeGenerator::writeOpConstant(const Type& type, int32_t valueBits) {
    return this->writeInstruction(
            SpvOpConstant,
            Words{this->getType(type), Word::Result(), Word::Number(valueBits)},
            fConstantBuffer);
}

SpvId SPIRVCodeGenerator::writeOpConstantComposite(const Type& type,
                                                   const TArray<SpvId>& values) {
    SkASSERT(values.size() == (type.isStruct() ? (int)type.fields().size() : type.columns()));

    Words words;
    words.push_back(this->getType(type));
    words.push_back(Word::Result());
    for (SpvId value : values) {
        words.push_back(value);
    }
    return this->writeInstruction(SpvOpConstantComposite, words, fConstantBuffer);
}

bool SPIRVCodeGenerator::toConstants(SpvId value, TArray<SpvId>* constants) {
    Instruction* instr = fSpvIdCache.find(value);
    if (!instr) {
        return false;
    }
    switch (instr->fOp) {
        case SpvOpConstant:
        case SpvOpConstantTrue:
        case SpvOpConstantFalse:
            constants->push_back(value);
            return true;

        case SpvOpConstantComposite: // OpConstantComposite ResultType ResultID Constituents...
            // Start at word 2 to skip past ResultType and ResultID.
            for (int i = 2; i < instr->fWords.size(); ++i) {
                if (!this->toConstants(instr->fWords[i], constants)) {
                    return false;
                }
            }
            return true;

        default:
            return false;
    }
}

bool SPIRVCodeGenerator::toConstants(SkSpan<const SpvId> values, TArray<SpvId>* constants) {
    for (SpvId value : values) {
        if (!this->toConstants(value, constants)) {
            return false;
        }
    }
    return true;
}

SpvId SPIRVCodeGenerator::writeOpCompositeConstruct(const Type& type,
                                                    const TArray<SpvId>& values,
                                                    OutputStream& out) {
    // If this is a vector composed entirely of literals, write a constant-composite instead.
    if (type.isVector()) {
        STArray<4, SpvId> constants;
        if (this->toConstants(SkSpan(values), &constants)) {
            // Create a vector from literals.
            return this->writeOpConstantComposite(type, constants);
        }
    }

    // If this is a matrix composed entirely of literals, constant-composite them instead.
    if (type.isMatrix()) {
        STArray<16, SpvId> constants;
        if (this->toConstants(SkSpan(values), &constants)) {
            // Create each matrix column.
            SkASSERT(type.isMatrix());
            const Type& vecType = type.componentType().toCompound(fContext,
                                                                  /*columns=*/type.rows(),
                                                                  /*rows=*/1);
            STArray<4, SpvId> columnIDs;
            for (int index=0; index < type.columns(); ++index) {
                STArray<4, SpvId> columnConstants(&constants[index * type.rows()],
                                                    type.rows());
                columnIDs.push_back(this->writeOpConstantComposite(vecType, columnConstants));
            }
            // Compose the matrix from its columns.
            return this->writeOpConstantComposite(type, columnIDs);
        }
    }

    Words words;
    words.push_back(this->getType(type));
    words.push_back(Word::Result(type));
    for (SpvId value : values) {
        words.push_back(value);
    }

    return this->writeInstruction(SpvOpCompositeConstruct, words, out);
}

SPIRVCodeGenerator::Instruction* SPIRVCodeGenerator::resultTypeForInstruction(
        const Instruction& instr) {
    // This list should contain every op that we cache that has a result and result-type.
    // (If one is missing, we will not find some optimization opportunities.)
    // Generally, the result type of an op is in the 0th word, but I'm not sure if this is
    // universally true, so it's configurable on a per-op basis.
    int resultTypeWord;
    switch (instr.fOp) {
        case SpvOpConstant:
        case SpvOpConstantTrue:
        case SpvOpConstantFalse:
        case SpvOpConstantComposite:
        case SpvOpCompositeConstruct:
        case SpvOpCompositeExtract:
        case SpvOpLoad:
            resultTypeWord = 0;
            break;

        default:
            return nullptr;
    }

    Instruction* typeInstr = fSpvIdCache.find(instr.fWords[resultTypeWord]);
    SkASSERT(typeInstr);
    return typeInstr;
}

int SPIRVCodeGenerator::numComponentsForVecInstruction(const Instruction& instr) {
    // If an instruction is in the op cache, its type should be as well.
    Instruction* typeInstr = this->resultTypeForInstruction(instr);
    SkASSERT(typeInstr);
    SkASSERT(typeInstr->fOp == SpvOpTypeVector || typeInstr->fOp == SpvOpTypeFloat ||
             typeInstr->fOp == SpvOpTypeInt || typeInstr->fOp == SpvOpTypeBool);

    // For vectors, extract their column count. Scalars have one component by definition.
    //   SpvOpTypeVector ResultID ComponentType NumComponents
    return (typeInstr->fOp == SpvOpTypeVector) ? typeInstr->fWords[2]
                                               : 1;
}

SpvId SPIRVCodeGenerator::toComponent(SpvId id, int component) {
    Instruction* instr = fSpvIdCache.find(id);
    if (!instr) {
        return NA;
    }
    if (instr->fOp == SpvOpConstantComposite) {
        // SpvOpConstantComposite ResultType ResultID [components...]
        // Add 2 to the component index to skip past ResultType and ResultID.
        return instr->fWords[2 + component];
    }
    if (instr->fOp == SpvOpCompositeConstruct) {
        // SpvOpCompositeConstruct ResultType ResultID [components...]
        // Vectors have special rules; check to see if we are composing a vector.
        Instruction* composedType = fSpvIdCache.find(instr->fWords[0]);
        SkASSERT(composedType);

        // When composing a non-vector, each instruction word maps 1:1 to the component index.
        // We can just extract out the associated component directly.
        if (composedType->fOp != SpvOpTypeVector) {
            return instr->fWords[2 + component];
        }

        // When composing a vector, components can be either scalars or vectors.
        // This means we need to check the op type on each component. (+2 to skip ResultType/Result)
        for (int index = 2; index < instr->fWords.size(); ++index) {
            int32_t currentWord = instr->fWords[index];

            // Retrieve the sub-instruction pointed to by OpCompositeConstruct.
            Instruction* subinstr = fSpvIdCache.find(currentWord);
            if (!subinstr) {
                return NA;
            }
            // If this subinstruction contains the component we're looking for...
            int numComponents = this->numComponentsForVecInstruction(*subinstr);
            if (component < numComponents) {
                if (numComponents == 1) {
                    // ... it's a scalar. Return it.
                    SkASSERT(component == 0);
                    return currentWord;
                } else {
                    // ... it's a vector. Recurse into it.
                    return this->toComponent(currentWord, component);
                }
            }
            // This sub-instruction doesn't contain our component. Keep walking forward.
            component -= numComponents;
        }
        SkDEBUGFAIL("component index goes past the end of this composite value");
        return NA;
    }
    return NA;
}

SpvId SPIRVCodeGenerator::writeOpCompositeExtract(const Type& type,
                                                  SpvId base,
                                                  int component,
                                                  OutputStream& out) {
    // If the base op is a composite, we can extract from it directly.
    SpvId result = this->toComponent(base, component);
    if (result != NA) {
        return result;
    }
    return this->writeInstruction(
            SpvOpCompositeExtract,
            {this->getType(type), Word::Result(type), base, Word::Number(component)},
            out);
}

SpvId SPIRVCodeGenerator::writeOpCompositeExtract(const Type& type,
                                                  SpvId base,
                                                  int componentA,
                                                  int componentB,
                                                  OutputStream& out) {
    // If the base op is a composite, we can extract from it directly.
    SpvId result = this->toComponent(base, componentA);
    if (result != NA) {
        return this->writeOpCompositeExtract(type, result, componentB, out);
    }
    return this->writeInstruction(SpvOpCompositeExtract,
                                  {this->getType(type),
                                   Word::Result(type),
                                   base,
                                   Word::Number(componentA),
                                   Word::Number(componentB)},
                                  out);
}

void SPIRVCodeGenerator::writeCapabilities(OutputStream& out) {
    for (uint64_t i = 0, bit = 1; i <= kLast_Capability; i++, bit <<= 1) {
        if (fCapabilities & bit) {
            this->writeInstruction(SpvOpCapability, (SpvId) i, out);
        }
    }
    this->writeInstruction(SpvOpCapability, SpvCapabilityShader, out);
}

SpvId SPIRVCodeGenerator::nextId(const Type* type) {
    return this->nextId(type && type->hasPrecision() && !type->highPrecision()
                ? Precision::kRelaxed
                : Precision::kDefault);
}

SpvId SPIRVCodeGenerator::nextId(Precision precision) {
    if (precision == Precision::kRelaxed && !fProgram.fConfig->fSettings.fForceHighPrecision) {
        this->writeInstruction(SpvOpDecorate, fIdCount, SpvDecorationRelaxedPrecision,
                               fDecorationBuffer);
    }
    return fIdCount++;
}

SpvId SPIRVCodeGenerator::writeStruct(const Type& type, const MemoryLayout& memoryLayout) {
    // If we've already written out this struct, return its existing SpvId.
    if (SpvId* cachedStructId = fStructMap.find(&type)) {
        return *cachedStructId;
    }

    // Write all of the field types first, so we don't inadvertently write them while we're in the
    // middle of writing the struct instruction.
    Words words;
    words.push_back(Word::UniqueResult());
    for (const auto& f : type.fields()) {
        words.push_back(this->getType(*f.fType, memoryLayout));
    }
    SpvId resultId = this->writeInstruction(SpvOpTypeStruct, words, fConstantBuffer);
    this->writeInstruction(SpvOpName, resultId, type.name(), fNameBuffer);
    fStructMap.set(&type, resultId);

    size_t offset = 0;
    for (int32_t i = 0; i < (int32_t) type.fields().size(); i++) {
        const Field& field = type.fields()[i];
        if (!memoryLayout.isSupported(*field.fType)) {
            fContext.fErrors->error(type.fPosition, "type '" + field.fType->displayName() +
                                                    "' is not permitted here");
            return resultId;
        }
        size_t size = memoryLayout.size(*field.fType);
        size_t alignment = memoryLayout.alignment(*field.fType);
        const Layout& fieldLayout = field.fLayout;
        if (fieldLayout.fOffset >= 0) {
            if (fieldLayout.fOffset < (int) offset) {
                fContext.fErrors->error(field.fPosition, "offset of field '" +
                        std::string(field.fName) + "' must be at least " + std::to_string(offset));
            }
            if (fieldLayout.fOffset % alignment) {
                fContext.fErrors->error(field.fPosition,
                                        "offset of field '" + std::string(field.fName) +
                                        "' must be a multiple of " + std::to_string(alignment));
            }
            offset = fieldLayout.fOffset;
        } else {
            size_t mod = offset % alignment;
            if (mod) {
                offset += alignment - mod;
            }
        }
        this->writeInstruction(SpvOpMemberName, resultId, i, field.fName, fNameBuffer);
        this->writeFieldLayout(fieldLayout, resultId, i);
        if (field.fLayout.fBuiltin < 0) {
            this->writeInstruction(SpvOpMemberDecorate, resultId, (SpvId) i, SpvDecorationOffset,
                                   (SpvId) offset, fDecorationBuffer);
        }
        if (field.fType->isMatrix()) {
            this->writeInstruction(SpvOpMemberDecorate, resultId, i, SpvDecorationColMajor,
                                   fDecorationBuffer);
            this->writeInstruction(SpvOpMemberDecorate, resultId, i, SpvDecorationMatrixStride,
                                   (SpvId) memoryLayout.stride(*field.fType),
                                   fDecorationBuffer);
        }
        if (!field.fType->highPrecision()) {
            this->writeInstruction(SpvOpMemberDecorate, resultId, (SpvId) i,
                                   SpvDecorationRelaxedPrecision, fDecorationBuffer);
        }
        offset += size;
        if ((field.fType->isArray() || field.fType->isStruct()) && offset % alignment != 0) {
            offset += alignment - offset % alignment;
        }
    }

    return resultId;
}

SpvId SPIRVCodeGenerator::getType(const Type& type) {
    return this->getType(type, fDefaultLayout);
}

SpvId SPIRVCodeGenerator::getType(const Type& rawType, const MemoryLayout& layout) {
    const Type* type = &rawType;

    switch (type->typeKind()) {
        case Type::TypeKind::kVoid: {
            return this->writeInstruction(SpvOpTypeVoid, Words{Word::Result()}, fConstantBuffer);
        }
        case Type::TypeKind::kScalar:
        case Type::TypeKind::kLiteral: {
            if (type->isBoolean()) {
                return this->writeInstruction(SpvOpTypeBool, {Word::Result()}, fConstantBuffer);
            }
            if (type->isSigned()) {
                return this->writeInstruction(
                        SpvOpTypeInt,
                        Words{Word::Result(), Word::Number(32), Word::Number(1)},
                        fConstantBuffer);
            }
            if (type->isUnsigned()) {
                return this->writeInstruction(
                        SpvOpTypeInt,
                        Words{Word::Result(), Word::Number(32), Word::Number(0)},
                        fConstantBuffer);
            }
            if (type->isFloat()) {
                return this->writeInstruction(
                        SpvOpTypeFloat,
                        Words{Word::Result(), Word::Number(32)},
                        fConstantBuffer);
            }
            SkDEBUGFAILF("unrecognized scalar type '%s'", type->description().c_str());
            return (SpvId)-1;
        }
        case Type::TypeKind::kVector: {
            SpvId scalarTypeId = this->getType(type->componentType(), layout);
            return this->writeInstruction(
                    SpvOpTypeVector,
                    Words{Word::Result(), scalarTypeId, Word::Number(type->columns())},
                    fConstantBuffer);
        }
        case Type::TypeKind::kMatrix: {
            SpvId vectorTypeId = this->getType(IndexExpression::IndexType(fContext, *type), layout);
            return this->writeInstruction(
                    SpvOpTypeMatrix,
                    Words{Word::Result(), vectorTypeId, Word::Number(type->columns())},
                    fConstantBuffer);
        }
        case Type::TypeKind::kArray: {
            if (!layout.isSupported(*type)) {
                fContext.fErrors->error(type->fPosition, "type '" + type->displayName() +
                                                         "' is not permitted here");
                return NA;
            }
            size_t stride = layout.stride(*type);
            SpvId typeId = this->getType(type->componentType(), layout);
            SpvId result = NA;
            if (type->isUnsizedArray()) {
                result = this->writeInstruction(SpvOpTypeRuntimeArray,
                                                Words{Word::KeyedResult(stride), typeId},
                                                fConstantBuffer);
            } else {
                SpvId countId = this->writeLiteral(type->columns(), *fContext.fTypes.fInt);
                result = this->writeInstruction(SpvOpTypeArray,
                                                Words{Word::KeyedResult(stride), typeId, countId},
                                                fConstantBuffer);
            }
            this->writeInstruction(SpvOpDecorate,
                                   {result, SpvDecorationArrayStride, Word::Number(stride)},
                                   fDecorationBuffer);
            return result;
        }
        case Type::TypeKind::kStruct: {
            return this->writeStruct(*type, layout);
        }
        case Type::TypeKind::kSeparateSampler: {
            return this->writeInstruction(SpvOpTypeSampler, Words{Word::Result()}, fConstantBuffer);
        }
        case Type::TypeKind::kSampler: {
            if (SpvDimBuffer == type->dimensions()) {
                fCapabilities |= 1ULL << SpvCapabilitySampledBuffer;
            }
            SpvId imageTypeId = this->getType(type->textureType(), layout);
            return this->writeInstruction(SpvOpTypeSampledImage,
                                          Words{Word::Result(), imageTypeId},
                                          fConstantBuffer);
        }
        case Type::TypeKind::kTexture: {
            SpvId floatTypeId = this->getType(*fContext.fTypes.fFloat, layout);
            int sampled = (type->textureAccess() == Type::TextureAccess::kSample) ? 1 : 2;

            // TODO(skia:293670098) SkSL doesn't provide a way to specify a pixel format. Until
            // then, access an unsampled storage texture as rgba8 and pretend the underlying
            // resource has a compatible format.
            SpvImageFormat format = (sampled == 2 && type->dimensions() != SpvDimSubpassData)
                                            ? SpvImageFormatRgba8
                                            : SpvImageFormatUnknown;
            return this->writeInstruction(SpvOpTypeImage,
                                          Words{Word::Result(),
                                                floatTypeId,
                                                Word::Number(type->dimensions()),
                                                Word::Number(type->isDepth()),
                                                Word::Number(type->isArrayedTexture()),
                                                Word::Number(type->isMultisampled()),
                                                Word::Number(sampled),
                                                format},
                                          fConstantBuffer);
        }
        case Type::TypeKind::kAtomic: {
            // SkSL currently only supports the atomicUint type.
            SkASSERT(type->matches(*fContext.fTypes.fAtomicUInt));
            // SPIR-V doesn't have atomic types. Rather, it allows atomic operations on primitive
            // types. The SPIR-V type of an SkSL atomic is simply the underlying type.
            return this->writeInstruction(SpvOpTypeInt,
                                          Words{Word::Result(), Word::Number(32), Word::Number(0)},
                                          fConstantBuffer);
        }
        default: {
            SkDEBUGFAILF("invalid type: %s", type->description().c_str());
            return NA;
        }
    }
}

SpvId SPIRVCodeGenerator::getFunctionType(const FunctionDeclaration& function) {
    Words words;
    words.push_back(Word::Result());
    words.push_back(this->getType(function.returnType()));
    for (const Variable* parameter : function.parameters()) {
        if (fUseTextureSamplerPairs && parameter->type().isSampler()) {
            words.push_back(this->getFunctionParameterType(parameter->type().textureType()));
            words.push_back(this->getFunctionParameterType(*fContext.fTypes.fSampler));
        } else {
            words.push_back(this->getFunctionParameterType(parameter->type()));
        }
    }
    return this->writeInstruction(SpvOpTypeFunction, words, fConstantBuffer);
}

SpvId SPIRVCodeGenerator::getFunctionParameterType(const Type& parameterType) {
    // glslang treats all function arguments as pointers whether they need to be or
    // not. I was initially puzzled by this until I ran bizarre failures with certain
    // patterns of function calls and control constructs, as exemplified by this minimal
    // failure case:
    //
    // void sphere(float x) {
    // }
    //
    // void map() {
    //     sphere(1.0);
    // }
    //
    // void main() {
    //     for (int i = 0; i < 1; i++) {
    //         map();
    //     }
    // }
    //
    // As of this writing, compiling this in the "obvious" way (with sphere taking a float)
    // crashes. Making it take a float* and storing the argument in a temporary variable,
    // as glslang does, fixes it.
    //
    // The consensus among shader compiler authors seems to be that GPU driver generally don't
    // handle value-based parameters consistently. It is highly likely that they fit their
    // implementations to conform to glslang. We take care to do so ourselves.
    //
    // Our implementation first stores every parameter value into a function storage-class pointer
    // before calling a function. The exception is for opaque handle types (samplers and textures)
    // which must be stored in a pointer with UniformConstant storage-class. This prevents
    // unnecessary temporaries (becuase opaque handles are always rooted in a pointer variable),
    // matches glslang's behavior, and translates into WGSL more easily when targeting Dawn.
    SpvStorageClass_ storageClass;
    if (parameterType.typeKind() == Type::TypeKind::kSampler ||
        parameterType.typeKind() == Type::TypeKind::kSeparateSampler ||
        parameterType.typeKind() == Type::TypeKind::kTexture) {
        storageClass = SpvStorageClassUniformConstant;
    } else {
        storageClass = SpvStorageClassFunction;
    }
    return this->getPointerType(parameterType, storageClass);
}

SpvId SPIRVCodeGenerator::getPointerType(const Type& type, SpvStorageClass_ storageClass) {
    return this->getPointerType(
            type, this->memoryLayoutForStorageClass(storageClass), storageClass);
}

SpvId SPIRVCodeGenerator::getPointerType(const Type& type, const MemoryLayout& layout,
                                         SpvStorageClass_ storageClass) {
    return this->writeInstruction(
            SpvOpTypePointer,
            Words{Word::Result(), Word::Number(storageClass), this->getType(type, layout)},
            fConstantBuffer);
}

SpvId SPIRVCodeGenerator::writeExpression(const Expression& expr, OutputStream& out) {
    switch (expr.kind()) {
        case Expression::Kind::kBinary:
            return this->writeBinaryExpression(expr.as<BinaryExpression>(), out);
        case Expression::Kind::kConstructorArrayCast:
            return this->writeExpression(*expr.as<ConstructorArrayCast>().argument(), out);
        case Expression::Kind::kConstructorArray:
        case Expression::Kind::kConstructorStruct:
            return this->writeCompositeConstructor(expr.asAnyConstructor(), out);
        case Expression::Kind::kConstructorDiagonalMatrix:
            return this->writeConstructorDiagonalMatrix(expr.as<ConstructorDiagonalMatrix>(), out);
        case Expression::Kind::kConstructorMatrixResize:
            return this->writeConstructorMatrixResize(expr.as<ConstructorMatrixResize>(), out);
        case Expression::Kind::kConstructorScalarCast:
            return this->writeConstructorScalarCast(expr.as<ConstructorScalarCast>(), out);
        case Expression::Kind::kConstructorSplat:
            return this->writeConstructorSplat(expr.as<ConstructorSplat>(), out);
        case Expression::Kind::kConstructorCompound:
            return this->writeConstructorCompound(expr.as<ConstructorCompound>(), out);
        case Expression::Kind::kConstructorCompoundCast:
            return this->writeConstructorCompoundCast(expr.as<ConstructorCompoundCast>(), out);
        case Expression::Kind::kEmpty:
            return NA;
        case Expression::Kind::kFieldAccess:
            return this->writeFieldAccess(expr.as<FieldAccess>(), out);
        case Expression::Kind::kFunctionCall:
            return this->writeFunctionCall(expr.as<FunctionCall>(), out);
        case Expression::Kind::kLiteral:
            return this->writeLiteral(expr.as<Literal>());
        case Expression::Kind::kPrefix:
            return this->writePrefixExpression(expr.as<PrefixExpression>(), out);
        case Expression::Kind::kPostfix:
            return this->writePostfixExpression(expr.as<PostfixExpression>(), out);
        case Expression::Kind::kSwizzle:
            return this->writeSwizzle(expr.as<Swizzle>(), out);
        case Expression::Kind::kVariableReference:
            return this->writeVariableReference(expr.as<VariableReference>(), out);
        case Expression::Kind::kTernary:
            return this->writeTernaryExpression(expr.as<TernaryExpression>(), out);
        case Expression::Kind::kIndex:
            return this->writeIndexExpression(expr.as<IndexExpression>(), out);
        case Expression::Kind::kSetting:
            return this->writeExpression(*expr.as<Setting>().toLiteral(fContext), out);
        default:
            SkDEBUGFAILF("unsupported expression: %s", expr.description().c_str());
            break;
    }
    return NA;
}

SpvId SPIRVCodeGenerator::writeIntrinsicCall(const FunctionCall& c, OutputStream& out) {
    const FunctionDeclaration& function = c.function();
    Intrinsic intrinsic = this->getIntrinsic(function.intrinsicKind());
    if (intrinsic.opKind == kInvalid_IntrinsicOpcodeKind) {
        fContext.fErrors->error(c.fPosition, "unsupported intrinsic '" + function.description() +
                "'");
        return NA;
    }
    const ExpressionArray& arguments = c.arguments();
    int32_t intrinsicId = intrinsic.floatOp;
    if (arguments.size() > 0) {
        const Type& type = arguments[0]->type();
        if (intrinsic.opKind == kSpecial_IntrinsicOpcodeKind) {
            // Keep the default float op.
        } else {
            intrinsicId = pick_by_type(type, intrinsic.floatOp, intrinsic.signedOp,
                                       intrinsic.unsignedOp, intrinsic.boolOp);
        }
    }
    switch (intrinsic.opKind) {
        case kGLSL_STD_450_IntrinsicOpcodeKind: {
            SpvId result = this->nextId(&c.type());
            TArray<SpvId> argumentIds;
            argumentIds.reserve_exact(arguments.size());
            std::vector<TempVar> tempVars;
            for (int i = 0; i < arguments.size(); i++) {
                argumentIds.push_back(this->writeFunctionCallArgument(c, i, &tempVars, out));
            }
            this->writeOpCode(SpvOpExtInst, 5 + (int32_t) argumentIds.size(), out);
            this->writeWord(this->getType(c.type()), out);
            this->writeWord(result, out);
            this->writeWord(fGLSLExtendedInstructions, out);
            this->writeWord(intrinsicId, out);
            for (SpvId id : argumentIds) {
                this->writeWord(id, out);
            }
            this->copyBackTempVars(tempVars, out);
            return result;
        }
        case kSPIRV_IntrinsicOpcodeKind: {
            // GLSL supports dot(float, float), but SPIR-V does not. Convert it to FMul
            if (intrinsicId == SpvOpDot && arguments[0]->type().isScalar()) {
                intrinsicId = SpvOpFMul;
            }
            SpvId result = this->nextId(&c.type());
            TArray<SpvId> argumentIds;
            argumentIds.reserve_exact(arguments.size());
            std::vector<TempVar> tempVars;
            for (int i = 0; i < arguments.size(); i++) {
                argumentIds.push_back(this->writeFunctionCallArgument(c, i, &tempVars, out));
            }
            if (!c.type().isVoid()) {
                this->writeOpCode((SpvOp_) intrinsicId, 3 + (int32_t) arguments.size(), out);
                this->writeWord(this->getType(c.type()), out);
                this->writeWord(result, out);
            } else {
                this->writeOpCode((SpvOp_) intrinsicId, 1 + (int32_t) arguments.size(), out);
            }
            for (SpvId id : argumentIds) {
                this->writeWord(id, out);
            }
            this->copyBackTempVars(tempVars, out);
            return result;
        }
        case kSpecial_IntrinsicOpcodeKind:
            return this->writeSpecialIntrinsic(c, (SpecialIntrinsic) intrinsicId, out);
        default:
            fContext.fErrors->error(c.fPosition, "unsupported intrinsic '" +
                    function.description() + "'");
            return NA;
    }
}

SpvId SPIRVCodeGenerator::vectorize(const Expression& arg, int vectorSize, OutputStream& out) {
    SkASSERT(vectorSize >= 1 && vectorSize <= 4);
    const Type& argType = arg.type();
    if (argType.isScalar() && vectorSize > 1) {
        ConstructorSplat splat{arg.fPosition,
                               argType.toCompound(fContext, vectorSize, /*rows=*/1),
                               arg.clone()};
        return this->writeConstructorSplat(splat, out);
    }

    SkASSERT(vectorSize == argType.columns());
    return this->writeExpression(arg, out);
}

TArray<SpvId> SPIRVCodeGenerator::vectorize(const ExpressionArray& args, OutputStream& out) {
    int vectorSize = 1;
    for (const auto& a : args) {
        if (a->type().isVector()) {
            if (vectorSize > 1) {
                SkASSERT(a->type().columns() == vectorSize);
            } else {
                vectorSize = a->type().columns();
            }
        }
    }
    TArray<SpvId> result;
    result.reserve_exact(args.size());
    for (const auto& arg : args) {
        result.push_back(this->vectorize(*arg, vectorSize, out));
    }
    return result;
}

void SPIRVCodeGenerator::writeGLSLExtendedInstruction(const Type& type, SpvId id, SpvId floatInst,
                                                      SpvId signedInst, SpvId unsignedInst,
                                                      const TArray<SpvId>& args,
                                                      OutputStream& out) {
    this->writeOpCode(SpvOpExtInst, 5 + args.size(), out);
    this->writeWord(this->getType(type), out);
    this->writeWord(id, out);
    this->writeWord(fGLSLExtendedInstructions, out);
    this->writeWord(pick_by_type(type, floatInst, signedInst, unsignedInst, NA), out);
    for (SpvId a : args) {
        this->writeWord(a, out);
    }
}

SpvId SPIRVCodeGenerator::writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind,
                                                OutputStream& out) {
    const ExpressionArray& arguments = c.arguments();
    const Type& callType = c.type();
    SpvId result = this->nextId(nullptr);
    switch (kind) {
        case kAtan_SpecialIntrinsic: {
            STArray<2, SpvId> argumentIds;
            for (const std::unique_ptr<Expression>& arg : arguments) {
                argumentIds.push_back(this->writeExpression(*arg, out));
            }
            this->writeOpCode(SpvOpExtInst, 5 + (int32_t) argumentIds.size(), out);
            this->writeWord(this->getType(callType), out);
            this->writeWord(result, out);
            this->writeWord(fGLSLExtendedInstructions, out);
            this->writeWord(argumentIds.size() == 2 ? GLSLstd450Atan2 : GLSLstd450Atan, out);
            for (SpvId id : argumentIds) {
                this->writeWord(id, out);
            }
            break;
        }
        case kSampledImage_SpecialIntrinsic: {
            SkASSERT(arguments.size() == 2);
            SpvId img = this->writeExpression(*arguments[0], out);
            SpvId sampler = this->writeExpression(*arguments[1], out);
            this->writeInstruction(SpvOpSampledImage,
                                   this->getType(callType),
                                   result,
                                   img,
                                   sampler,
                                   out);
            break;
        }
        case kSubpassLoad_SpecialIntrinsic: {
            SpvId img = this->writeExpression(*arguments[0], out);
            ExpressionArray args;
            args.reserve_exact(2);
            args.push_back(Literal::MakeInt(fContext, Position(), /*value=*/0));
            args.push_back(Literal::MakeInt(fContext, Position(), /*value=*/0));
            ConstructorCompound ctor(Position(), *fContext.fTypes.fInt2, std::move(args));
            SpvId coords = this->writeExpression(ctor, out);
            if (arguments.size() == 1) {
                this->writeInstruction(SpvOpImageRead,
                                       this->getType(callType),
                                       result,
                                       img,
                                       coords,
                                       out);
            } else {
                SkASSERT(arguments.size() == 2);
                SpvId sample = this->writeExpression(*arguments[1], out);
                this->writeInstruction(SpvOpImageRead,
                                       this->getType(callType),
                                       result,
                                       img,
                                       coords,
                                       SpvImageOperandsSampleMask,
                                       sample,
                                       out);
            }
            break;
        }
        case kTexture_SpecialIntrinsic: {
            SpvOp_ op = SpvOpImageSampleImplicitLod;
            const Type& arg1Type = arguments[1]->type();
            switch (arguments[0]->type().dimensions()) {
                case SpvDim1D:
                    if (arg1Type.matches(*fContext.fTypes.fFloat2)) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type.matches(*fContext.fTypes.fFloat));
                    }
                    break;
                case SpvDim2D:
                    if (arg1Type.matches(*fContext.fTypes.fFloat3)) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type.matches(*fContext.fTypes.fFloat2));
                    }
                    break;
                case SpvDim3D:
                    if (arg1Type.matches(*fContext.fTypes.fFloat4)) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type.matches(*fContext.fTypes.fFloat3));
                    }
                    break;
                case SpvDimCube:   // fall through
                case SpvDimRect:   // fall through
                case SpvDimBuffer: // fall through
                case SpvDimSubpassData:
                    break;
            }
            SpvId type = this->getType(callType);
            SpvId sampler = this->writeExpression(*arguments[0], out);
            SpvId uv = this->writeExpression(*arguments[1], out);
            if (arguments.size() == 3) {
                this->writeInstruction(op, type, result, sampler, uv,
                                       SpvImageOperandsBiasMask,
                                       this->writeExpression(*arguments[2], out),
                                       out);
            } else {
                SkASSERT(arguments.size() == 2);
                if (fProgram.fConfig->fSettings.fSharpenTextures) {
                    SpvId lodBias = this->writeLiteral(kSharpenTexturesBias,
                                                       *fContext.fTypes.fFloat);
                    this->writeInstruction(op, type, result, sampler, uv,
                                           SpvImageOperandsBiasMask, lodBias, out);
                } else {
                    this->writeInstruction(op, type, result, sampler, uv,
                                           out);
                }
            }
            break;
        }
        case kTextureGrad_SpecialIntrinsic: {
            SpvOp_ op = SpvOpImageSampleExplicitLod;
            SkASSERT(arguments.size() == 4);
            SkASSERT(arguments[0]->type().dimensions() == SpvDim2D);
            SkASSERT(arguments[1]->type().matches(*fContext.fTypes.fFloat2));
            SkASSERT(arguments[2]->type().matches(*fContext.fTypes.fFloat2));
            SkASSERT(arguments[3]->type().matches(*fContext.fTypes.fFloat2));
            SpvId type = this->getType(callType);
            SpvId sampler = this->writeExpression(*arguments[0], out);
            SpvId uv = this->writeExpression(*arguments[1], out);
            SpvId dPdx = this->writeExpression(*arguments[2], out);
            SpvId dPdy = this->writeExpression(*arguments[3], out);
            this->writeInstruction(op, type, result, sampler, uv, SpvImageOperandsGradMask,
                                   dPdx, dPdy, out);
            break;
        }
        case kTextureLod_SpecialIntrinsic: {
            SpvOp_ op = SpvOpImageSampleExplicitLod;
            SkASSERT(arguments.size() == 3);
            SkASSERT(arguments[0]->type().dimensions() == SpvDim2D);
            SkASSERT(arguments[2]->type().matches(*fContext.fTypes.fFloat));
            const Type& arg1Type = arguments[1]->type();
            if (arg1Type.matches(*fContext.fTypes.fFloat3)) {
                op = SpvOpImageSampleProjExplicitLod;
            } else {
                SkASSERT(arg1Type.matches(*fContext.fTypes.fFloat2));
            }
            SpvId type = this->getType(callType);
            SpvId sampler = this->writeExpression(*arguments[0], out);
            SpvId uv = this->writeExpression(*arguments[1], out);
            this->writeInstruction(op, type, result, sampler, uv,
                                   SpvImageOperandsLodMask,
                                   this->writeExpression(*arguments[2], out),
                                   out);
            break;
        }
        case kTextureRead_SpecialIntrinsic: {
            SkASSERT(arguments[0]->type().dimensions() == SpvDim2D);
            SkASSERT(arguments[1]->type().matches(*fContext.fTypes.fUInt2));

            SpvId type = this->getType(callType);
            SpvId image = this->writeExpression(*arguments[0], out);
            SpvId coord = this->writeExpression(*arguments[1], out);

            const Type& arg0Type = arguments[0]->type();
            SkASSERT(arg0Type.typeKind() == Type::TypeKind::kTexture);

            switch (arg0Type.textureAccess()) {
                case Type::TextureAccess::kSample:
                    this->writeInstruction(SpvOpImageFetch, type, result, image, coord,
                                           SpvImageOperandsLodMask,
                                           this->writeOpConstant(*fContext.fTypes.fInt, 0),
                                           out);
                    break;
                case Type::TextureAccess::kRead:
                case Type::TextureAccess::kReadWrite:
                    this->writeInstruction(SpvOpImageRead, type, result, image, coord, out);
                    break;
                case Type::TextureAccess::kWrite:
                default:
                    SkDEBUGFAIL("'textureRead' called on writeonly texture type");
                    break;
            }

            break;
        }
        case kTextureWrite_SpecialIntrinsic: {
            SkASSERT(arguments[0]->type().dimensions() == SpvDim2D);
            SkASSERT(arguments[1]->type().matches(*fContext.fTypes.fUInt2));
            SkASSERT(arguments[2]->type().matches(*fContext.fTypes.fHalf4));

            SpvId image = this->writeExpression(*arguments[0], out);
            SpvId coord = this->writeExpression(*arguments[1], out);
            SpvId texel = this->writeExpression(*arguments[2], out);

            this->writeInstruction(SpvOpImageWrite, image, coord, texel, out);
            break;
        }
        case kTextureWidth_SpecialIntrinsic:
        case kTextureHeight_SpecialIntrinsic: {
            SkASSERT(arguments[0]->type().dimensions() == SpvDim2D);
            fCapabilities |= 1ULL << SpvCapabilityImageQuery;

            SpvId dimsType = this->getType(*fContext.fTypes.fUInt2);
            SpvId dims = this->nextId(nullptr);
            SpvId image = this->writeExpression(*arguments[0], out);
            this->writeInstruction(SpvOpImageQuerySize, dimsType, dims, image, out);

            SpvId type = this->getType(callType);
            int32_t index = (kind == kTextureWidth_SpecialIntrinsic) ? 0 : 1;
            this->writeInstruction(SpvOpCompositeExtract, type, result, dims, index, out);
            break;
        }
        case kMod_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            const Type& operandType = arguments[0]->type();
            SpvOp_ op = pick_by_type(operandType, SpvOpFMod, SpvOpSMod, SpvOpUMod, SpvOpUndef);
            SkASSERT(op != SpvOpUndef);
            this->writeOpCode(op, 5, out);
            this->writeWord(this->getType(operandType), out);
            this->writeWord(result, out);
            this->writeWord(args[0], out);
            this->writeWord(args[1], out);
            break;
        }
        case kDFdy_SpecialIntrinsic: {
            SpvId fn = this->writeExpression(*arguments[0], out);
            this->writeOpCode(SpvOpDPdy, 4, out);
            this->writeWord(this->getType(callType), out);
            this->writeWord(result, out);
            this->writeWord(fn, out);
            if (!fProgram.fConfig->fSettings.fForceNoRTFlip) {
                this->addRTFlipUniform(c.fPosition);
                ComponentArray componentArray;
                for (int index = 0; index < callType.columns(); ++index) {
                    componentArray.push_back(SwizzleComponent::Y);
                }
                SpvId rtFlipY = this->writeSwizzle(*this->identifier(SKSL_RTFLIP_NAME),
                                                   componentArray, out);
                SpvId flipped = this->nextId(&callType);
                this->writeInstruction(SpvOpFMul, this->getType(callType), flipped, result,
                                       rtFlipY, out);
                result = flipped;
            }
            break;
        }
        case kClamp_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 3);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FClamp, GLSLstd450SClamp,
                                               GLSLstd450UClamp, args, out);
            break;
        }
        case kMax_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FMax, GLSLstd450SMax,
                                               GLSLstd450UMax, args, out);
            break;
        }
        case kMin_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FMin, GLSLstd450SMin,
                                               GLSLstd450UMin, args, out);
            break;
        }
        case kMix_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 3);
            if (arguments[2]->type().componentType().isBoolean()) {
                // Use OpSelect to implement Boolean mix().
                SpvId falseId     = this->writeExpression(*arguments[0], out);
                SpvId trueId      = this->writeExpression(*arguments[1], out);
                SpvId conditionId = this->writeExpression(*arguments[2], out);
                this->writeInstruction(SpvOpSelect, this->getType(arguments[0]->type()), result,
                                       conditionId, trueId, falseId, out);
            } else {
                this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FMix, SpvOpUndef,
                                                   SpvOpUndef, args, out);
            }
            break;
        }
        case kSaturate_SpecialIntrinsic: {
            SkASSERT(arguments.size() == 1);
            ExpressionArray finalArgs;
            finalArgs.reserve_exact(3);
            finalArgs.push_back(arguments[0]->clone());
            finalArgs.push_back(Literal::MakeFloat(fContext, Position(), /*value=*/0));
            finalArgs.push_back(Literal::MakeFloat(fContext, Position(), /*value=*/1));
            TArray<SpvId> spvArgs = this->vectorize(finalArgs, out);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FClamp, GLSLstd450SClamp,
                                               GLSLstd450UClamp, spvArgs, out);
            break;
        }
        case kSmoothStep_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 3);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450SmoothStep, SpvOpUndef,
                                               SpvOpUndef, args, out);
            break;
        }
        case kStep_SpecialIntrinsic: {
            TArray<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450Step, SpvOpUndef,
                                               SpvOpUndef, args, out);
            break;
        }
        case kMatrixCompMult_SpecialIntrinsic: {
            SkASSERT(arguments.size() == 2);
            SpvId lhs = this->writeExpression(*arguments[0], out);
            SpvId rhs = this->writeExpression(*arguments[1], out);
            result = this->writeComponentwiseMatrixBinary(callType, lhs, rhs, SpvOpFMul, out);
            break;
        }
        case kAtomicAdd_SpecialIntrinsic:
        case kAtomicLoad_SpecialIntrinsic:
        case kAtomicStore_SpecialIntrinsic:
            result = this->writeAtomicIntrinsic(c, kind, result, out);
            break;
        case kStorageBarrier_SpecialIntrinsic:
        case kWorkgroupBarrier_SpecialIntrinsic: {
            // Both barrier types operate in the workgroup execution and memory scope and differ
            // only in memory semantics. storageBarrier() is not a device-scope barrier.
            SpvId scopeId =
                    this->writeOpConstant(*fContext.fTypes.fUInt, (int32_t)SpvScopeWorkgroup);
            int32_t memSemMask = (kind == kStorageBarrier_SpecialIntrinsic)
                                         ? SpvMemorySemanticsAcquireReleaseMask |
                                                   SpvMemorySemanticsUniformMemoryMask
                                         : SpvMemorySemanticsAcquireReleaseMask |
                                                   SpvMemorySemanticsWorkgroupMemoryMask;
            SpvId memorySemanticsId = this->writeOpConstant(*fContext.fTypes.fUInt, memSemMask);
            this->writeInstruction(SpvOpControlBarrier,
                                   scopeId,  // execution scope
                                   scopeId,  // memory scope
                                   memorySemanticsId,
                                   out);
            break;
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeAtomicIntrinsic(const FunctionCall& c,
                                               SpecialIntrinsic kind,
                                               SpvId resultId,
                                               OutputStream& out) {
    const ExpressionArray& arguments = c.arguments();
    SkASSERT(!arguments.empty());

    std::unique_ptr<LValue> atomicPtr = this->getLValue(*arguments[0], out);
    SpvId atomicPtrId = atomicPtr->getPointer();
    if (atomicPtrId == NA) {
        SkDEBUGFAILF("atomic intrinsic expected a pointer argument: %s",
                     arguments[0]->description().c_str());
        return NA;
    }

    SpvId memoryScopeId = NA;
    {
        // In SkSL, the atomicUint type can only be declared as a workgroup variable or SSBO block
        // member. The two memory scopes that these map to are "workgroup" and "device",
        // respectively.
        SpvScope memoryScope;
        switch (atomicPtr->storageClass()) {
            case SpvStorageClassUniform:
                // We encode storage buffers in the uniform address space (with the BufferBlock
                // decorator).
                memoryScope = SpvScopeDevice;
                break;
            case SpvStorageClassWorkgroup:
                memoryScope = SpvScopeWorkgroup;
                break;
            default:
                SkDEBUGFAILF("atomic argument has invalid storage class: %d",
                             atomicPtr->storageClass());
                return NA;
        }
        memoryScopeId = this->writeOpConstant(*fContext.fTypes.fUInt, (int32_t)memoryScope);
    }

    SpvId relaxedMemoryOrderId =
            this->writeOpConstant(*fContext.fTypes.fUInt, (int32_t)SpvMemorySemanticsMaskNone);

    switch (kind) {
        case kAtomicAdd_SpecialIntrinsic:
            SkASSERT(arguments.size() == 2);
            this->writeInstruction(SpvOpAtomicIAdd,
                                   this->getType(c.type()),
                                   resultId,
                                   atomicPtrId,
                                   memoryScopeId,
                                   relaxedMemoryOrderId,
                                   this->writeExpression(*arguments[1], out),
                                   out);
            break;
        case kAtomicLoad_SpecialIntrinsic:
            SkASSERT(arguments.size() == 1);
            this->writeInstruction(SpvOpAtomicLoad,
                                   this->getType(c.type()),
                                   resultId,
                                   atomicPtrId,
                                   memoryScopeId,
                                   relaxedMemoryOrderId,
                                   out);
            break;
        case kAtomicStore_SpecialIntrinsic:
            SkASSERT(arguments.size() == 2);
            this->writeInstruction(SpvOpAtomicStore,
                                   atomicPtrId,
                                   memoryScopeId,
                                   relaxedMemoryOrderId,
                                   this->writeExpression(*arguments[1], out),
                                   out);
            break;
        default:
            SkUNREACHABLE;
    }

    return resultId;
}

SpvId SPIRVCodeGenerator::writeFunctionCallArgument(const FunctionCall& call,
                                                    int argIndex,
                                                    std::vector<TempVar>* tempVars,
                                                    OutputStream& out,
                                                    SpvId* outSynthesizedSamplerId) {
    const FunctionDeclaration& funcDecl = call.function();
    const Expression& arg = *call.arguments()[argIndex];
    ModifierFlags paramFlags = funcDecl.parameters()[argIndex]->modifierFlags();

    // ID of temporary variable that we will use to hold this argument, or 0 if it is being
    // passed directly
    SpvId tmpVar;
    // if we need a temporary var to store this argument, this is the value to store in the var
    SpvId tmpValueId = NA;

    if (is_out(paramFlags)) {
        std::unique_ptr<LValue> lv = this->getLValue(arg, out);
        // We handle out params with a temp var that we copy back to the original variable at the
        // end of the call. GLSL guarantees that the original variable will be unchanged until the
        // end of the call, and also that out params are written back to their original variables in
        // a specific order (left-to-right), so it's unsafe to pass a pointer to the original value.
        if (is_in(paramFlags)) {
            tmpValueId = lv->load(out);
        }
        tmpVar = this->nextId(&arg.type());
        tempVars->push_back(TempVar{tmpVar, &arg.type(), std::move(lv)});
    } else if (funcDecl.isIntrinsic()) {
        // Unlike user function calls, non-out intrinsic arguments don't need pointer parameters.
        return this->writeExpression(arg, out);
    } else if (arg.is<VariableReference>() &&
               (arg.type().typeKind() == Type::TypeKind::kSampler ||
                arg.type().typeKind() == Type::TypeKind::kSeparateSampler ||
                arg.type().typeKind() == Type::TypeKind::kTexture)) {
        // Opaque handle (sampler/texture) arguments are always declared as pointers but never
        // stored in intermediates when calling user-defined functions.
        //
        // The case for intrinsics (which take opaque arguments by value) is handled above just like
        // regular pointers.
        //
        // See getFunctionParameterType for further explanation.
        const Variable* var = arg.as<VariableReference>().variable();

        // In Dawn-mode the texture and sampler arguments are forwarded to the helper function.
        if (fUseTextureSamplerPairs && var->type().isSampler()) {
            if (const auto* p = fSynthesizedSamplerMap.find(var)) {
                SkASSERT(outSynthesizedSamplerId);

                SpvId* img = fVariableMap.find((*p)->fTexture.get());
                SpvId* sampler = fVariableMap.find((*p)->fSampler.get());
                SkASSERT(img);
                SkASSERT(sampler);

                *outSynthesizedSamplerId = *sampler;
                return *img;
            }
            SkDEBUGFAIL("sampler missing from fSynthesizedSamplerMap");
        }

        SpvId* entry = fVariableMap.find(var);
        SkASSERTF(entry, "%s", arg.description().c_str());
        return *entry;
    } else {
        // We always use pointer parameters when calling user functions.
        // See getFunctionParameterType for further explanation.
        tmpValueId = this->writeExpression(arg, out);
        tmpVar = this->nextId(nullptr);
    }
    this->writeInstruction(SpvOpVariable,
                           this->getPointerType(arg.type(), SpvStorageClassFunction),
                           tmpVar,
                           SpvStorageClassFunction,
                           fVariableBuffer);
    if (tmpValueId != NA) {
        this->writeOpStore(SpvStorageClassFunction, tmpVar, tmpValueId, out);
    }
    return tmpVar;
}

void SPIRVCodeGenerator::copyBackTempVars(const std::vector<TempVar>& tempVars, OutputStream& out) {
    for (const TempVar& tempVar : tempVars) {
        SpvId load = this->nextId(tempVar.type);
        this->writeInstruction(SpvOpLoad, this->getType(*tempVar.type), load, tempVar.spvId, out);
        tempVar.lvalue->store(load, out);
    }
}

SpvId SPIRVCodeGenerator::writeFunctionCall(const FunctionCall& c, OutputStream& out) {
    const FunctionDeclaration& function = c.function();
    if (function.isIntrinsic() && !function.definition()) {
        return this->writeIntrinsicCall(c, out);
    }
    const ExpressionArray& arguments = c.arguments();
    SpvId* entry = fFunctionMap.find(&function);
    if (!entry) {
        fContext.fErrors->error(c.fPosition, "function '" + function.description() +
                "' is not defined");
        return NA;
    }
    // Temp variables are used to write back out-parameters after the function call is complete.
    std::vector<TempVar> tempVars;
    TArray<SpvId> argumentIds;
    argumentIds.reserve_exact(arguments.size());
    for (int i = 0; i < arguments.size(); i++) {
        SpvId samplerId = NA;
        argumentIds.push_back(this->writeFunctionCallArgument(c, i, &tempVars, out, &samplerId));
        if (samplerId != NA) {
            argumentIds.push_back(samplerId);
        }
    }
    SpvId result = this->nextId(nullptr);
    this->writeOpCode(SpvOpFunctionCall, 4 + (int32_t)argumentIds.size(), out);
    this->writeWord(this->getType(c.type()), out);
    this->writeWord(result, out);
    this->writeWord(*entry, out);
    for (SpvId id : argumentIds) {
        this->writeWord(id, out);
    }
    // Now that the call is complete, we copy temp out-variables back to their real lvalues.
    this->copyBackTempVars(tempVars, out);
    return result;
}

SpvId SPIRVCodeGenerator::castScalarToType(SpvId inputExprId,
                                           const Type& inputType,
                                           const Type& outputType,
                                           OutputStream& out) {
    if (outputType.isFloat()) {
        return this->castScalarToFloat(inputExprId, inputType, outputType, out);
    }
    if (outputType.isSigned()) {
        return this->castScalarToSignedInt(inputExprId, inputType, outputType, out);
    }
    if (outputType.isUnsigned()) {
        return this->castScalarToUnsignedInt(inputExprId, inputType, outputType, out);
    }
    if (outputType.isBoolean()) {
        return this->castScalarToBoolean(inputExprId, inputType, outputType, out);
    }

    fContext.fErrors->error(Position(), "unsupported cast: " + inputType.description() + " to " +
            outputType.description());
    return inputExprId;
}

SpvId SPIRVCodeGenerator::writeFloatConstructor(const AnyConstructor& c, OutputStream& out) {
    SkASSERT(c.argumentSpan().size() == 1);
    SkASSERT(c.type().isFloat());
    const Expression& ctorExpr = *c.argumentSpan().front();
    SpvId expressionId = this->writeExpression(ctorExpr, out);
    return this->castScalarToFloat(expressionId, ctorExpr.type(), c.type(), out);
}

SpvId SPIRVCodeGenerator::castScalarToFloat(SpvId inputId, const Type& inputType,
                                            const Type& outputType, OutputStream& out) {
    // Casting a float to float is a no-op.
    if (inputType.isFloat()) {
        return inputId;
    }

    // Given the input type, generate the appropriate instruction to cast to float.
    SpvId result = this->nextId(&outputType);
    if (inputType.isBoolean()) {
        // Use OpSelect to convert the boolean argument to a literal 1.0 or 0.0.
        const SpvId oneID = this->writeLiteral(1.0, *fContext.fTypes.fFloat);
        const SpvId zeroID = this->writeLiteral(0.0, *fContext.fTypes.fFloat);
        this->writeInstruction(SpvOpSelect, this->getType(outputType), result,
                               inputId, oneID, zeroID, out);
    } else if (inputType.isSigned()) {
        this->writeInstruction(SpvOpConvertSToF, this->getType(outputType), result, inputId, out);
    } else if (inputType.isUnsigned()) {
        this->writeInstruction(SpvOpConvertUToF, this->getType(outputType), result, inputId, out);
    } else {
        SkDEBUGFAILF("unsupported type for float typecast: %s", inputType.description().c_str());
        return NA;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeIntConstructor(const AnyConstructor& c, OutputStream& out) {
    SkASSERT(c.argumentSpan().size() == 1);
    SkASSERT(c.type().isSigned());
    const Expression& ctorExpr = *c.argumentSpan().front();
    SpvId expressionId = this->writeExpression(ctorExpr, out);
    return this->castScalarToSignedInt(expressionId, ctorExpr.type(), c.type(), out);
}

SpvId SPIRVCodeGenerator::castScalarToSignedInt(SpvId inputId, const Type& inputType,
                                                const Type& outputType, OutputStream& out) {
    // Casting a signed int to signed int is a no-op.
    if (inputType.isSigned()) {
        return inputId;
    }

    // Given the input type, generate the appropriate instruction to cast to signed int.
    SpvId result = this->nextId(&outputType);
    if (inputType.isBoolean()) {
        // Use OpSelect to convert the boolean argument to a literal 1 or 0.
        const SpvId oneID = this->writeLiteral(1.0, *fContext.fTypes.fInt);
        const SpvId zeroID = this->writeLiteral(0.0, *fContext.fTypes.fInt);
        this->writeInstruction(SpvOpSelect, this->getType(outputType), result,
                               inputId, oneID, zeroID, out);
    } else if (inputType.isFloat()) {
        this->writeInstruction(SpvOpConvertFToS, this->getType(outputType), result, inputId, out);
    } else if (inputType.isUnsigned()) {
        this->writeInstruction(SpvOpBitcast, this->getType(outputType), result, inputId, out);
    } else {
        SkDEBUGFAILF("unsupported type for signed int typecast: %s",
                     inputType.description().c_str());
        return NA;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeUIntConstructor(const AnyConstructor& c, OutputStream& out) {
    SkASSERT(c.argumentSpan().size() == 1);
    SkASSERT(c.type().isUnsigned());
    const Expression& ctorExpr = *c.argumentSpan().front();
    SpvId expressionId = this->writeExpression(ctorExpr, out);
    return this->castScalarToUnsignedInt(expressionId, ctorExpr.type(), c.type(), out);
}

SpvId SPIRVCodeGenerator::castScalarToUnsignedInt(SpvId inputId, const Type& inputType,
                                                  const Type& outputType, OutputStream& out) {
    // Casting an unsigned int to unsigned int is a no-op.
    if (inputType.isUnsigned()) {
        return inputId;
    }

    // Given the input type, generate the appropriate instruction to cast to unsigned int.
    SpvId result = this->nextId(&outputType);
    if (inputType.isBoolean()) {
        // Use OpSelect to convert the boolean argument to a literal 1u or 0u.
        const SpvId oneID = this->writeLiteral(1.0, *fContext.fTypes.fUInt);
        const SpvId zeroID = this->writeLiteral(0.0, *fContext.fTypes.fUInt);
        this->writeInstruction(SpvOpSelect, this->getType(outputType), result,
                               inputId, oneID, zeroID, out);
    } else if (inputType.isFloat()) {
        this->writeInstruction(SpvOpConvertFToU, this->getType(outputType), result, inputId, out);
    } else if (inputType.isSigned()) {
        this->writeInstruction(SpvOpBitcast, this->getType(outputType), result, inputId, out);
    } else {
        SkDEBUGFAILF("unsupported type for unsigned int typecast: %s",
                     inputType.description().c_str());
        return NA;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeBooleanConstructor(const AnyConstructor& c, OutputStream& out) {
    SkASSERT(c.argumentSpan().size() == 1);
    SkASSERT(c.type().isBoolean());
    const Expression& ctorExpr = *c.argumentSpan().front();
    SpvId expressionId = this->writeExpression(ctorExpr, out);
    return this->castScalarToBoolean(expressionId, ctorExpr.type(), c.type(), out);
}

SpvId SPIRVCodeGenerator::castScalarToBoolean(SpvId inputId, const Type& inputType,
                                              const Type& outputType, OutputStream& out) {
    // Casting a bool to bool is a no-op.
    if (inputType.isBoolean()) {
        return inputId;
    }

    // Given the input type, generate the appropriate instruction to cast to bool.
    SpvId result = this->nextId(nullptr);
    if (inputType.isSigned()) {
        // Synthesize a boolean result by comparing the input against a signed zero literal.
        const SpvId zeroID = this->writeLiteral(0.0, *fContext.fTypes.fInt);
        this->writeInstruction(SpvOpINotEqual, this->getType(outputType), result,
                               inputId, zeroID, out);
    } else if (inputType.isUnsigned()) {
        // Synthesize a boolean result by comparing the input against an unsigned zero literal.
        const SpvId zeroID = this->writeLiteral(0.0, *fContext.fTypes.fUInt);
        this->writeInstruction(SpvOpINotEqual, this->getType(outputType), result,
                               inputId, zeroID, out);
    } else if (inputType.isFloat()) {
        // Synthesize a boolean result by comparing the input against a floating-point zero literal.
        const SpvId zeroID = this->writeLiteral(0.0, *fContext.fTypes.fFloat);
        this->writeInstruction(SpvOpFUnordNotEqual, this->getType(outputType), result,
                               inputId, zeroID, out);
    } else {
        SkDEBUGFAILF("unsupported type for boolean typecast: %s", inputType.description().c_str());
        return NA;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeMatrixCopy(SpvId src, const Type& srcType, const Type& dstType,
                                          OutputStream& out) {
    SkASSERT(srcType.isMatrix());
    SkASSERT(dstType.isMatrix());
    SkASSERT(srcType.componentType().matches(dstType.componentType()));
    const Type& srcColumnType = srcType.componentType().toCompound(fContext, srcType.rows(), 1);
    const Type& dstColumnType = dstType.componentType().toCompound(fContext, dstType.rows(), 1);
    SkASSERT(dstType.componentType().isFloat());
    SpvId dstColumnTypeId = this->getType(dstColumnType);
    const SpvId zeroId = this->writeLiteral(0.0, dstType.componentType());
    const SpvId oneId = this->writeLiteral(1.0, dstType.componentType());

    STArray<4, SpvId> columns;
    for (int i = 0; i < dstType.columns(); i++) {
        if (i < srcType.columns()) {
            // we're still inside the src matrix, copy the column
            SpvId srcColumn = this->writeOpCompositeExtract(srcColumnType, src, i, out);
            SpvId dstColumn;
            if (srcType.rows() == dstType.rows()) {
                // columns are equal size, don't need to do anything
                dstColumn = srcColumn;
            }
            else if (dstType.rows() > srcType.rows()) {
                // dst column is bigger, need to zero-pad it
                STArray<4, SpvId> values;
                values.push_back(srcColumn);
                for (int j = srcType.rows(); j < dstType.rows(); ++j) {
                    values.push_back((i == j) ? oneId : zeroId);
                }
                dstColumn = this->writeOpCompositeConstruct(dstColumnType, values, out);
            }
            else {
                // dst column is smaller, need to swizzle the src column
                dstColumn = this->nextId(&dstType);
                this->writeOpCode(SpvOpVectorShuffle, 5 + dstType.rows(), out);
                this->writeWord(dstColumnTypeId, out);
                this->writeWord(dstColumn, out);
                this->writeWord(srcColumn, out);
                this->writeWord(srcColumn, out);
                for (int j = 0; j < dstType.rows(); j++) {
                    this->writeWord(j, out);
                }
            }
            columns.push_back(dstColumn);
        } else {
            // we're past the end of the src matrix, need to synthesize an identity-matrix column
            STArray<4, SpvId> values;
            for (int j = 0; j < dstType.rows(); ++j) {
                values.push_back((i == j) ? oneId : zeroId);
            }
            columns.push_back(this->writeOpCompositeConstruct(dstColumnType, values, out));
        }
    }

    return this->writeOpCompositeConstruct(dstType, columns, out);
}

void SPIRVCodeGenerator::addColumnEntry(const Type& columnType,
                                        TArray<SpvId>* currentColumn,
                                        TArray<SpvId>* columnIds,
                                        int rows,
                                        SpvId entry,
                                        OutputStream& out) {
    SkASSERT(currentColumn->size() < rows);
    currentColumn->push_back(entry);
    if (currentColumn->size() == rows) {
        // Synthesize this column into a vector.
        SpvId columnId = this->writeOpCompositeConstruct(columnType, *currentColumn, out);
        columnIds->push_back(columnId);
        currentColumn->clear();
    }
}

SpvId SPIRVCodeGenerator::writeMatrixConstructor(const ConstructorCompound& c, OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(!c.arguments().empty());
    const Type& arg0Type = c.arguments()[0]->type();
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    STArray<16, SpvId> arguments;
    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
        arguments.push_back(this->writeExpression(*arg, out));
    }

    if (arguments.size() == 1 && arg0Type.isVector()) {
        // Special-case handling of float4 -> mat2x2.
        SkASSERT(type.rows() == 2 && type.columns() == 2);
        SkASSERT(arg0Type.columns() == 4);
        SpvId v[4];
        for (int i = 0; i < 4; ++i) {
            v[i] = this->writeOpCompositeExtract(type.componentType(), arguments[0], i, out);
        }
        const Type& vecType = type.componentType().toCompound(fContext, /*columns=*/2, /*rows=*/1);
        SpvId v0v1 = this->writeOpCompositeConstruct(vecType, {v[0], v[1]}, out);
        SpvId v2v3 = this->writeOpCompositeConstruct(vecType, {v[2], v[3]}, out);
        return this->writeOpCompositeConstruct(type, {v0v1, v2v3}, out);
    }

    int rows = type.rows();
    const Type& columnType = type.componentType().toCompound(fContext,
                                                             /*columns=*/rows, /*rows=*/1);
    // SpvIds of completed columns of the matrix.
    STArray<4, SpvId> columnIds;
    // SpvIds of scalars we have written to the current column so far.
    STArray<4, SpvId> currentColumn;
    for (int i = 0; i < arguments.size(); i++) {
        const Type& argType = c.arguments()[i]->type();
        if (currentColumn.empty() && argType.isVector() && argType.columns() == rows) {
            // This vector is a complete matrix column by itself and can be used as-is.
            columnIds.push_back(arguments[i]);
        } else if (argType.columns() == 1) {
            // This argument is a lone scalar and can be added to the current column as-is.
            this->addColumnEntry(columnType, &currentColumn, &columnIds, rows, arguments[i], out);
        } else {
            // This argument needs to be decomposed into its constituent scalars.
            for (int j = 0; j < argType.columns(); ++j) {
                SpvId swizzle = this->writeOpCompositeExtract(argType.componentType(),
                                                              arguments[i], j, out);
                this->addColumnEntry(columnType, &currentColumn, &columnIds, rows, swizzle, out);
            }
        }
    }
    SkASSERT(columnIds.size() == type.columns());
    return this->writeOpCompositeConstruct(type, columnIds, out);
}

SpvId SPIRVCodeGenerator::writeConstructorCompound(const ConstructorCompound& c,
                                                   OutputStream& out) {
    return c.type().isMatrix() ? this->writeMatrixConstructor(c, out)
                               : this->writeVectorConstructor(c, out);
}

SpvId SPIRVCodeGenerator::writeVectorConstructor(const ConstructorCompound& c, OutputStream& out) {
    const Type& type = c.type();
    const Type& componentType = type.componentType();
    SkASSERT(type.isVector());

    STArray<4, SpvId> arguments;
    for (int i = 0; i < c.arguments().size(); i++) {
        const Type& argType = c.arguments()[i]->type();
        SkASSERT(componentType.numberKind() == argType.componentType().numberKind());

        SpvId arg = this->writeExpression(*c.arguments()[i], out);
        if (argType.isMatrix()) {
            // CompositeConstruct cannot take a 2x2 matrix as an input, so we need to extract out
            // each scalar separately.
            SkASSERT(argType.rows() == 2);
            SkASSERT(argType.columns() == 2);
            for (int j = 0; j < 4; ++j) {
                arguments.push_back(this->writeOpCompositeExtract(componentType, arg,
                                                                  j / 2, j % 2, out));
            }
        } else if (argType.isVector()) {
            // There's a bug in the Intel Vulkan driver where OpCompositeConstruct doesn't handle
            // vector arguments at all, so we always extract each vector component and pass them
            // into OpCompositeConstruct individually.
            for (int j = 0; j < argType.columns(); j++) {
                arguments.push_back(this->writeOpCompositeExtract(componentType, arg, j, out));
            }
        } else {
            arguments.push_back(arg);
        }
    }

    return this->writeOpCompositeConstruct(type, arguments, out);
}

SpvId SPIRVCodeGenerator::writeConstructorSplat(const ConstructorSplat& c, OutputStream& out) {
    // Write the splat argument.
    SpvId argument = this->writeExpression(*c.argument(), out);

    // Generate a OpCompositeConstruct which repeats the argument N times.
    STArray<4, SpvId> values;
    values.push_back_n(/*n=*/c.type().columns(), /*t=*/argument);
    return this->writeOpCompositeConstruct(c.type(), values, out);
}

SpvId SPIRVCodeGenerator::writeCompositeConstructor(const AnyConstructor& c, OutputStream& out) {
    SkASSERT(c.type().isArray() || c.type().isStruct());
    auto ctorArgs = c.argumentSpan();

    STArray<4, SpvId> arguments;
    for (const std::unique_ptr<Expression>& arg : ctorArgs) {
        arguments.push_back(this->writeExpression(*arg, out));
    }

    return this->writeOpCompositeConstruct(c.type(), arguments, out);
}

SpvId SPIRVCodeGenerator::writeConstructorScalarCast(const ConstructorScalarCast& c,
                                                     OutputStream& out) {
    const Type& type = c.type();
    if (type.componentType().numberKind() == c.argument()->type().componentType().numberKind()) {
        return this->writeExpression(*c.argument(), out);
    }

    const Expression& ctorExpr = *c.argument();
    SpvId expressionId = this->writeExpression(ctorExpr, out);
    return this->castScalarToType(expressionId, ctorExpr.type(), type, out);
}

SpvId SPIRVCodeGenerator::writeConstructorCompoundCast(const ConstructorCompoundCast& c,
                                                       OutputStream& out) {
    const Type& ctorType = c.type();
    const Type& argType = c.argument()->type();
    SkASSERT(ctorType.isVector() || ctorType.isMatrix());

    // Write the composite that we are casting. If the actual type matches, we are done.
    SpvId compositeId = this->writeExpression(*c.argument(), out);
    if (ctorType.componentType().numberKind() == argType.componentType().numberKind()) {
        return compositeId;
    }

    // writeMatrixCopy can cast matrices to a different type.
    if (ctorType.isMatrix()) {
        return this->writeMatrixCopy(compositeId, argType, ctorType, out);
    }

    // SPIR-V doesn't support vector(vector-of-different-type) directly, so we need to extract the
    // components and convert each one manually.
    const Type& srcType = argType.componentType();
    const Type& dstType = ctorType.componentType();

    STArray<4, SpvId> arguments;
    for (int index = 0; index < argType.columns(); ++index) {
        SpvId componentId = this->writeOpCompositeExtract(srcType, compositeId, index, out);
        arguments.push_back(this->castScalarToType(componentId, srcType, dstType, out));
    }

    return this->writeOpCompositeConstruct(ctorType, arguments, out);
}

SpvId SPIRVCodeGenerator::writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                                         OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(c.argument()->type().isScalar());

    // Write out the scalar argument.
    SpvId diagonal = this->writeExpression(*c.argument(), out);

    // Build the diagonal matrix.
    SpvId zeroId = this->writeLiteral(0.0, *fContext.fTypes.fFloat);

    const Type& vecType = type.componentType().toCompound(fContext,
                                                          /*columns=*/type.rows(),
                                                          /*rows=*/1);
    STArray<4, SpvId> columnIds;
    STArray<4, SpvId> arguments;
    arguments.resize(type.rows());
    for (int column = 0; column < type.columns(); column++) {
        for (int row = 0; row < type.rows(); row++) {
            arguments[row] = (row == column) ? diagonal : zeroId;
        }
        columnIds.push_back(this->writeOpCompositeConstruct(vecType, arguments, out));
    }
    return this->writeOpCompositeConstruct(type, columnIds, out);
}

SpvId SPIRVCodeGenerator::writeConstructorMatrixResize(const ConstructorMatrixResize& c,
                                                       OutputStream& out) {
    // Write the input matrix.
    SpvId argument = this->writeExpression(*c.argument(), out);

    // Use matrix-copy to resize the input matrix to its new size.
    return this->writeMatrixCopy(argument, c.argument()->type(), c.type(), out);
}

static SpvStorageClass_ get_storage_class_for_global_variable(
        const Variable& var, SpvStorageClass_ fallbackStorageClass) {
    SkASSERT(var.storage() == Variable::Storage::kGlobal);

    if (var.type().typeKind() == Type::TypeKind::kSampler ||
        var.type().typeKind() == Type::TypeKind::kSeparateSampler ||
        var.type().typeKind() == Type::TypeKind::kTexture) {
        return SpvStorageClassUniformConstant;
    }

    const Layout& layout = var.layout();
    ModifierFlags flags = var.modifierFlags();
    if (flags & ModifierFlag::kIn) {
        SkASSERT(!(layout.fFlags & LayoutFlag::kPushConstant));
        return SpvStorageClassInput;
    }
    if (flags & ModifierFlag::kOut) {
        SkASSERT(!(layout.fFlags & LayoutFlag::kPushConstant));
        return SpvStorageClassOutput;
    }
    if (flags.isUniform()) {
        if (layout.fFlags & LayoutFlag::kPushConstant) {
            return SpvStorageClassPushConstant;
        }
        return SpvStorageClassUniform;
    }
    if (flags.isBuffer()) {
        // Note: In SPIR-V 1.3, a storage buffer can be declared with the "StorageBuffer"
        // storage class and the "Block" decoration and the <1.3 approach we use here ("Uniform"
        // storage class and the "BufferBlock" decoration) is deprecated. Since we target SPIR-V
        // 1.0, we have to use the deprecated approach which is well supported in Vulkan and
        // addresses SkSL use cases (notably SkSL currently doesn't support pointer features that
        // would benefit from SPV_KHR_variable_pointers capabilities).
        return SpvStorageClassUniform;
    }
    if (flags.isWorkgroup()) {
        return SpvStorageClassWorkgroup;
    }
    return fallbackStorageClass;
}

static SpvStorageClass_ get_storage_class(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kVariableReference: {
            const Variable& var = *expr.as<VariableReference>().variable();
            if (var.storage() != Variable::Storage::kGlobal) {
                return SpvStorageClassFunction;
            }
            return get_storage_class_for_global_variable(var, SpvStorageClassPrivate);
        }
        case Expression::Kind::kFieldAccess:
            return get_storage_class(*expr.as<FieldAccess>().base());
        case Expression::Kind::kIndex:
            return get_storage_class(*expr.as<IndexExpression>().base());
        default:
            return SpvStorageClassFunction;
    }
}

TArray<SpvId> SPIRVCodeGenerator::getAccessChain(const Expression& expr, OutputStream& out) {
    switch (expr.kind()) {
        case Expression::Kind::kIndex: {
            const IndexExpression& indexExpr = expr.as<IndexExpression>();
            if (indexExpr.base()->is<Swizzle>()) {
                // Access chains don't directly support dynamically indexing into a swizzle, but we
                // can rewrite them into a supported form.
                return this->getAccessChain(*Transform::RewriteIndexedSwizzle(fContext, indexExpr),
                                            out);
            }
            // All other index-expressions can be represented as typical access chains.
            TArray<SpvId> chain = this->getAccessChain(*indexExpr.base(), out);
            chain.push_back(this->writeExpression(*indexExpr.index(), out));
            return chain;
        }
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& fieldExpr = expr.as<FieldAccess>();
            TArray<SpvId> chain = this->getAccessChain(*fieldExpr.base(), out);
            chain.push_back(this->writeLiteral(fieldExpr.fieldIndex(), *fContext.fTypes.fInt));
            return chain;
        }
        default: {
            SpvId id = this->getLValue(expr, out)->getPointer();
            SkASSERT(id != NA);
            return TArray<SpvId>{id};
        }
    }
    SkUNREACHABLE;
}

class PointerLValue : public SPIRVCodeGenerator::LValue {
public:
    PointerLValue(SPIRVCodeGenerator& gen, SpvId pointer, bool isMemoryObject, SpvId type,
                  SPIRVCodeGenerator::Precision precision, SpvStorageClass_ storageClass)
    : fGen(gen)
    , fPointer(pointer)
    , fIsMemoryObject(isMemoryObject)
    , fType(type)
    , fPrecision(precision)
    , fStorageClass(storageClass) {}

    SpvId getPointer() override {
        return fPointer;
    }

    bool isMemoryObjectPointer() const override {
        return fIsMemoryObject;
    }

    SpvStorageClass storageClass() const override {
        return fStorageClass;
    }

    SpvId load(OutputStream& out) override {
        return fGen.writeOpLoad(fType, fPrecision, fPointer, out);
    }

    void store(SpvId value, OutputStream& out) override {
        if (!fIsMemoryObject) {
            // We are going to write into an access chain; this could represent one component of a
            // vector, or one element of an array. This has the potential to invalidate other,
            // *unknown* elements of our store cache. (e.g. if the store cache holds `%50 = myVec4`,
            // and we store `%60 = myVec4.z`, this invalidates the cached value for %50.) To avoid
            // relying on stale data, reset the store cache entirely when this happens.
            fGen.fStoreCache.reset();
        }

        fGen.writeOpStore(fStorageClass, fPointer, value, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fPointer;
    const bool fIsMemoryObject;
    const SpvId fType;
    const SPIRVCodeGenerator::Precision fPrecision;
    const SpvStorageClass_ fStorageClass;
};

class SwizzleLValue : public SPIRVCodeGenerator::LValue {
public:
    SwizzleLValue(SPIRVCodeGenerator& gen, SpvId vecPointer, const ComponentArray& components,
                  const Type& baseType, const Type& swizzleType, SpvStorageClass_ storageClass)
    : fGen(gen)
    , fVecPointer(vecPointer)
    , fComponents(components)
    , fBaseType(&baseType)
    , fSwizzleType(&swizzleType)
    , fStorageClass(storageClass) {}

    bool applySwizzle(const ComponentArray& components, const Type& newType) override {
        ComponentArray updatedSwizzle;
        for (int8_t component : components) {
            if (component < 0 || component >= fComponents.size()) {
                SkDEBUGFAILF("swizzle accessed nonexistent component %d", (int)component);
                return false;
            }
            updatedSwizzle.push_back(fComponents[component]);
        }
        fComponents = updatedSwizzle;
        fSwizzleType = &newType;
        return true;
    }

    SpvStorageClass storageClass() const override {
        return fStorageClass;
    }

    SpvId load(OutputStream& out) override {
        SpvId base = fGen.nextId(fBaseType);
        fGen.writeInstruction(SpvOpLoad, fGen.getType(*fBaseType), base, fVecPointer, out);
        SpvId result = fGen.nextId(fBaseType);
        fGen.writeOpCode(SpvOpVectorShuffle, 5 + (int32_t) fComponents.size(), out);
        fGen.writeWord(fGen.getType(*fSwizzleType), out);
        fGen.writeWord(result, out);
        fGen.writeWord(base, out);
        fGen.writeWord(base, out);
        for (int component : fComponents) {
            fGen.writeWord(component, out);
        }
        return result;
    }

    void store(SpvId value, OutputStream& out) override {
        // use OpVectorShuffle to mix and match the vector components. We effectively create
        // a virtual vector out of the concatenation of the left and right vectors, and then
        // select components from this virtual vector to make the result vector. For
        // instance, given:
        // float3L = ...;
        // float3R = ...;
        // L.xz = R.xy;
        // we end up with the virtual vector (L.x, L.y, L.z, R.x, R.y, R.z). Then we want
        // our result vector to look like (R.x, L.y, R.y), so we need to select indices
        // (3, 1, 4).
        SpvId base = fGen.nextId(fBaseType);
        fGen.writeInstruction(SpvOpLoad, fGen.getType(*fBaseType), base, fVecPointer, out);
        SpvId shuffle = fGen.nextId(fBaseType);
        fGen.writeOpCode(SpvOpVectorShuffle, 5 + fBaseType->columns(), out);
        fGen.writeWord(fGen.getType(*fBaseType), out);
        fGen.writeWord(shuffle, out);
        fGen.writeWord(base, out);
        fGen.writeWord(value, out);
        for (int i = 0; i < fBaseType->columns(); i++) {
            // current offset into the virtual vector, defaults to pulling the unmodified
            // value from the left side
            int offset = i;
            // check to see if we are writing this component
            for (int j = 0; j < fComponents.size(); j++) {
                if (fComponents[j] == i) {
                    // we're writing to this component, so adjust the offset to pull from
                    // the correct component of the right side instead of preserving the
                    // value from the left
                    offset = (int) (j + fBaseType->columns());
                    break;
                }
            }
            fGen.writeWord(offset, out);
        }
        fGen.writeOpStore(fStorageClass, fVecPointer, shuffle, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fVecPointer;
    ComponentArray fComponents;
    const Type* fBaseType;
    const Type* fSwizzleType;
    const SpvStorageClass_ fStorageClass;
};

int SPIRVCodeGenerator::findUniformFieldIndex(const Variable& var) const {
    int* fieldIndex = fTopLevelUniformMap.find(&var);
    return fieldIndex ? *fieldIndex : -1;
}

std::unique_ptr<SPIRVCodeGenerator::LValue> SPIRVCodeGenerator::getLValue(const Expression& expr,
                                                                          OutputStream& out) {
    const Type& type = expr.type();
    Precision precision = type.highPrecision() ? Precision::kDefault : Precision::kRelaxed;
    switch (expr.kind()) {
        case Expression::Kind::kVariableReference: {
            const Variable& var = *expr.as<VariableReference>().variable();
            int uniformIdx = this->findUniformFieldIndex(var);
            if (uniformIdx >= 0) {
                SpvId memberId = this->nextId(nullptr);
                SpvId typeId = this->getPointerType(type, SpvStorageClassUniform);
                SpvId uniformIdxId = this->writeLiteral((double)uniformIdx, *fContext.fTypes.fInt);
                this->writeInstruction(SpvOpAccessChain, typeId, memberId, fUniformBufferId,
                                       uniformIdxId, out);
                return std::make_unique<PointerLValue>(
                        *this,
                        memberId,
                        /*isMemoryObjectPointer=*/true,
                        this->getType(type, this->memoryLayoutForVariable(var)),
                        precision,
                        SpvStorageClassUniform);
            }
            SpvId typeId = this->getType(type, this->memoryLayoutForVariable(var));
            SpvId* entry = fVariableMap.find(&var);
            SkASSERTF(entry, "%s", expr.description().c_str());
            return std::make_unique<PointerLValue>(*this, *entry,
                                                   /*isMemoryObjectPointer=*/true,
                                                   typeId, precision, get_storage_class(expr));
        }
        case Expression::Kind::kIndex: // fall through
        case Expression::Kind::kFieldAccess: {
            TArray<SpvId> chain = this->getAccessChain(expr, out);
            SpvId member = this->nextId(nullptr);
            SpvStorageClass_ storageClass = get_storage_class(expr);
            this->writeOpCode(SpvOpAccessChain, (SpvId) (3 + chain.size()), out);
            this->writeWord(this->getPointerType(type, storageClass), out);
            this->writeWord(member, out);
            for (SpvId idx : chain) {
                this->writeWord(idx, out);
            }
            return std::make_unique<PointerLValue>(
                    *this,
                    member,
                    /*isMemoryObjectPointer=*/false,
                    this->getType(type, this->memoryLayoutForStorageClass(storageClass)),
                    precision,
                    storageClass);
        }
        case Expression::Kind::kSwizzle: {
            const Swizzle& swizzle = expr.as<Swizzle>();
            std::unique_ptr<LValue> lvalue = this->getLValue(*swizzle.base(), out);
            if (lvalue->applySwizzle(swizzle.components(), type)) {
                return lvalue;
            }
            SpvId base = lvalue->getPointer();
            if (base == NA) {
                fContext.fErrors->error(swizzle.fPosition,
                        "unable to retrieve lvalue from swizzle");
            }
            SpvStorageClass_ storageClass = get_storage_class(*swizzle.base());
            if (swizzle.components().size() == 1) {
                SpvId member = this->nextId(nullptr);
                SpvId typeId = this->getPointerType(type, storageClass);
                SpvId indexId = this->writeLiteral(swizzle.components()[0], *fContext.fTypes.fInt);
                this->writeInstruction(SpvOpAccessChain, typeId, member, base, indexId, out);
                return std::make_unique<PointerLValue>(*this, member,
                                                       /*isMemoryObjectPointer=*/false,
                                                       this->getType(type),
                                                       precision, storageClass);
            } else {
                return std::make_unique<SwizzleLValue>(*this, base, swizzle.components(),
                                                       swizzle.base()->type(), type, storageClass);
            }
        }
        default: {
            // expr isn't actually an lvalue, create a placeholder variable for it. This case
            // happens due to the need to store values in temporary variables during function
            // calls (see comments in getFunctionParameterType); erroneous uses of rvalues as
            // lvalues should have been caught before code generation.
            //
            // This is with the exception of opaque handle types (textures/samplers) which are
            // always defined as UniformConstant pointers and don't need to be explicitly stored
            // into a temporary (which is handled explicitly in writeFunctionCallArgument).
            SpvId result = this->nextId(nullptr);
            SpvId pointerType = this->getPointerType(type, SpvStorageClassFunction);
            this->writeInstruction(SpvOpVariable, pointerType, result, SpvStorageClassFunction,
                                   fVariableBuffer);
            this->writeOpStore(SpvStorageClassFunction, result, this->writeExpression(expr, out),
                               out);
            return std::make_unique<PointerLValue>(*this, result, /*isMemoryObjectPointer=*/true,
                                                   this->getType(type), precision,
                                                   SpvStorageClassFunction);
        }
    }
}

std::unique_ptr<Expression> SPIRVCodeGenerator::identifier(std::string_view name) {
    std::unique_ptr<Expression> expr = ThreadContext::Compiler().convertIdentifier(Position(),name);
    return expr ? std::move(expr)
                : Poison::Make(Position(), fContext);
}

SpvId SPIRVCodeGenerator::writeVariableReference(const VariableReference& ref, OutputStream& out) {
    const Variable* variable = ref.variable();
    switch (variable->layout().fBuiltin) {
        case DEVICE_FRAGCOORDS_BUILTIN: {
            // Down below, we rewrite raw references to sk_FragCoord with expressions that reference
            // DEVICE_FRAGCOORDS_BUILTIN. This is a fake variable that means we need to directly
            // access the fragcoord; do so now.
            return this->getLValue(*this->identifier("sk_FragCoord"), out)->load(out);
        }
        case DEVICE_CLOCKWISE_BUILTIN: {
            // Down below, we rewrite raw references to sk_Clockwise with expressions that reference
            // DEVICE_CLOCKWISE_BUILTIN. This is a fake variable that means we need to directly
            // access front facing; do so now.
            return this->getLValue(*this->identifier("sk_Clockwise"), out)->load(out);
        }
        case SK_SECONDARYFRAGCOLOR_BUILTIN: {
            // sk_SecondaryFragColor corresponds to gl_SecondaryFragColorEXT, which isn't supposed
            // to appear in a SPIR-V program (it's only valid in ES2). Report an error.
            fContext.fErrors->error(ref.fPosition,
                                    "sk_SecondaryFragColor is not allowed in SPIR-V");
            return NA;
        }
        case SK_FRAGCOORD_BUILTIN: {
            if (fProgram.fConfig->fSettings.fForceNoRTFlip) {
                return this->getLValue(*this->identifier("sk_FragCoord"), out)->load(out);
            }

            // Handle inserting use of uniform to flip y when referencing sk_FragCoord.
            this->addRTFlipUniform(ref.fPosition);
            // Use sk_RTAdjust to compute the flipped coordinate
            // Use a uniform to flip the Y coordinate. The new expression will be written in
            // terms of $device_FragCoords, which is a fake variable that means "access the
            // underlying fragcoords directly without flipping it".
            static constexpr char DEVICE_COORDS_NAME[] = "$device_FragCoords";
            if (!fProgram.fSymbols->find(DEVICE_COORDS_NAME)) {
                AutoAttachPoolToThread attach(fProgram.fPool.get());
                Layout layout;
                layout.fBuiltin = DEVICE_FRAGCOORDS_BUILTIN;
                auto coordsVar = Variable::Make(/*pos=*/Position(),
                                                /*modifiersPosition=*/Position(),
                                                layout,
                                                ModifierFlag::kNone,
                                                fContext.fTypes.fFloat4.get(),
                                                DEVICE_COORDS_NAME,
                                                /*mangledName=*/"",
                                                /*builtin=*/true,
                                                Variable::Storage::kGlobal);
                fSPIRVBonusVariables.add(coordsVar.get());
                fProgram.fSymbols->add(std::move(coordsVar));
            }
            std::unique_ptr<Expression> deviceCoord = this->identifier(DEVICE_COORDS_NAME);
            std::unique_ptr<Expression> rtFlip = this->identifier(SKSL_RTFLIP_NAME);
            SpvId rtFlipX = this->writeSwizzle(*rtFlip, {SwizzleComponent::X}, out);
            SpvId rtFlipY = this->writeSwizzle(*rtFlip, {SwizzleComponent::Y}, out);
            SpvId deviceCoordX  = this->writeSwizzle(*deviceCoord, {SwizzleComponent::X}, out);
            SpvId deviceCoordY  = this->writeSwizzle(*deviceCoord, {SwizzleComponent::Y}, out);
            SpvId deviceCoordZW = this->writeSwizzle(*deviceCoord, {SwizzleComponent::Z,
                                                                    SwizzleComponent::W}, out);
            // Compute `flippedY = u_RTFlip.y * $device_FragCoords.y`.
            SpvId flippedY = this->writeBinaryExpression(
                                     *fContext.fTypes.fFloat, rtFlipY, OperatorKind::STAR,
                                     *fContext.fTypes.fFloat, deviceCoordY,
                                     *fContext.fTypes.fFloat, out);

            // Compute `flippedY = u_RTFlip.x + flippedY`.
            flippedY = this->writeBinaryExpression(
                               *fContext.fTypes.fFloat, rtFlipX, OperatorKind::PLUS,
                               *fContext.fTypes.fFloat, flippedY,
                               *fContext.fTypes.fFloat, out);

            // Return `float4(deviceCoord.x, flippedY, deviceCoord.zw)`.
            return this->writeOpCompositeConstruct(*fContext.fTypes.fFloat4,
                                                   {deviceCoordX, flippedY, deviceCoordZW},
                                                   out);
        }
        case SK_CLOCKWISE_BUILTIN: {
            if (fProgram.fConfig->fSettings.fForceNoRTFlip) {
                return this->getLValue(*this->identifier("sk_Clockwise"), out)->load(out);
            }

            // Apply RTFlip to sk_Clockwise.
            this->addRTFlipUniform(ref.fPosition);
            // Use a uniform to flip the Y coordinate. The new expression will be written in
            // terms of $device_Clockwise, which is a fake variable that means "access the
            // underlying FrontFacing directly".
            static constexpr char DEVICE_CLOCKWISE_NAME[] = "$device_Clockwise";
            if (!fProgram.fSymbols->find(DEVICE_CLOCKWISE_NAME)) {
                AutoAttachPoolToThread attach(fProgram.fPool.get());
                Layout layout;
                layout.fBuiltin = DEVICE_CLOCKWISE_BUILTIN;
                auto clockwiseVar = Variable::Make(/*pos=*/Position(),
                                                   /*modifiersPosition=*/Position(),
                                                   layout,
                                                   ModifierFlag::kNone,
                                                   fContext.fTypes.fBool.get(),
                                                   DEVICE_CLOCKWISE_NAME,
                                                   /*mangledName=*/"",
                                                   /*builtin=*/true,
                                                   Variable::Storage::kGlobal);
                fSPIRVBonusVariables.add(clockwiseVar.get());
                fProgram.fSymbols->add(std::move(clockwiseVar));
            }
            // FrontFacing in Vulkan is defined in terms of a top-down render target. In Skia,
            // we use the default convention of "counter-clockwise face is front".

            // Compute `positiveRTFlip = (rtFlip.y > 0)`.
            std::unique_ptr<Expression> rtFlip = this->identifier(SKSL_RTFLIP_NAME);
            SpvId rtFlipY = this->writeSwizzle(*rtFlip, {SwizzleComponent::Y}, out);
            SpvId zero = this->writeLiteral(0.0, *fContext.fTypes.fFloat);
            SpvId positiveRTFlip = this->writeBinaryExpression(
                                           *fContext.fTypes.fFloat, rtFlipY, OperatorKind::GT,
                                           *fContext.fTypes.fFloat, zero,
                                           *fContext.fTypes.fBool, out);

            // Compute `positiveRTFlip ^^ $device_Clockwise`.
            std::unique_ptr<Expression> deviceClockwise = this->identifier(DEVICE_CLOCKWISE_NAME);
            SpvId deviceClockwiseID = this->writeExpression(*deviceClockwise, out);
            return this->writeBinaryExpression(
                           *fContext.fTypes.fBool, positiveRTFlip, OperatorKind::LOGICALXOR,
                           *fContext.fTypes.fBool, deviceClockwiseID,
                           *fContext.fTypes.fBool, out);
        }
        default: {
            // Constant-propagate variables that have a known compile-time value.
            if (const Expression* expr = ConstantFolder::GetConstantValueOrNull(ref)) {
                return this->writeExpression(*expr, out);
            }

            // A reference to a sampler variable at global scope with synthesized texture/sampler
            // backing should construct a function-scope combined image-sampler from the synthesized
            // constituents. This is the case in which a sample intrinsic was invoked.
            //
            // Variable references to opaque handles (texture/sampler) that appear as the argument
            // of a user-defined function call are explicitly handled in writeFunctionCallArgument.
            if (fUseTextureSamplerPairs && variable->type().isSampler()) {
                if (const auto* p = fSynthesizedSamplerMap.find(variable)) {
                    SpvId* imgPtr = fVariableMap.find((*p)->fTexture.get());
                    SpvId* samplerPtr = fVariableMap.find((*p)->fSampler.get());
                    SkASSERT(imgPtr);
                    SkASSERT(samplerPtr);

                    SpvId img = this->writeOpLoad(this->getType((*p)->fTexture->type()),
                                                  Precision::kDefault, *imgPtr, out);
                    SpvId sampler = this->writeOpLoad(this->getType((*p)->fSampler->type()),
                                                      Precision::kDefault,
                                                      *samplerPtr,
                                                      out);
                    SpvId result = this->nextId(nullptr);
                    this->writeInstruction(SpvOpSampledImage,
                                           this->getType(variable->type()),
                                           result,
                                           img,
                                           sampler,
                                           out);
                    return result;
                }
                SkDEBUGFAIL("sampler missing from fSynthesizedSamplerMap");
            }
            return this->getLValue(ref, out)->load(out);
        }
    }
}

SpvId SPIRVCodeGenerator::writeIndexExpression(const IndexExpression& expr, OutputStream& out) {
    if (expr.base()->type().isVector()) {
        SpvId base = this->writeExpression(*expr.base(), out);
        SpvId index = this->writeExpression(*expr.index(), out);
        SpvId result = this->nextId(nullptr);
        this->writeInstruction(SpvOpVectorExtractDynamic, this->getType(expr.type()), result, base,
                               index, out);
        return result;
    }
    return getLValue(expr, out)->load(out);
}

SpvId SPIRVCodeGenerator::writeFieldAccess(const FieldAccess& f, OutputStream& out) {
    return getLValue(f, out)->load(out);
}

SpvId SPIRVCodeGenerator::writeSwizzle(const Expression& baseExpr,
                                       const ComponentArray& components,
                                       OutputStream& out) {
    size_t count = components.size();
    const Type& type = baseExpr.type().componentType().toCompound(fContext, count, /*rows=*/1);
    SpvId base = this->writeExpression(baseExpr, out);
    if (count == 1) {
        return this->writeOpCompositeExtract(type, base, components[0], out);
    }

    SpvId result = this->nextId(&type);
    this->writeOpCode(SpvOpVectorShuffle, 5 + (int32_t) count, out);
    this->writeWord(this->getType(type), out);
    this->writeWord(result, out);
    this->writeWord(base, out);
    this->writeWord(base, out);
    for (int component : components) {
        this->writeWord(component, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeSwizzle(const Swizzle& swizzle, OutputStream& out) {
    return this->writeSwizzle(*swizzle.base(), swizzle.components(), out);
}

SpvId SPIRVCodeGenerator::writeBinaryOperation(const Type& resultType,
                                               const Type& operandType, SpvId lhs,
                                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt,
                                               SpvOp_ ifUInt, SpvOp_ ifBool, OutputStream& out) {
    SpvId result = this->nextId(&resultType);
    SpvOp_ op = pick_by_type(operandType, ifFloat, ifInt, ifUInt, ifBool);
    if (op == SpvOpUndef) {
        fContext.fErrors->error(operandType.fPosition,
                "unsupported operand for binary expression: " + operandType.description());
        return NA;
    }
    this->writeInstruction(op, this->getType(resultType), result, lhs, rhs, out);
    return result;
}

SpvId SPIRVCodeGenerator::foldToBool(SpvId id, const Type& operandType, SpvOp op,
                                     OutputStream& out) {
    if (operandType.isVector()) {
        SpvId result = this->nextId(nullptr);
        this->writeInstruction(op, this->getType(*fContext.fTypes.fBool), result, id, out);
        return result;
    }
    return id;
}

SpvId SPIRVCodeGenerator::writeMatrixComparison(const Type& operandType, SpvId lhs, SpvId rhs,
                                                SpvOp_ floatOperator, SpvOp_ intOperator,
                                                SpvOp_ vectorMergeOperator, SpvOp_ mergeOperator,
                                                OutputStream& out) {
    SpvOp_ compareOp = is_float(operandType) ? floatOperator : intOperator;
    SkASSERT(operandType.isMatrix());
    const Type& columnType = operandType.componentType().toCompound(fContext,
                                                                    operandType.rows(),
                                                                    1);
    SpvId bvecType = this->getType(fContext.fTypes.fBool->toCompound(fContext,
                                                                     operandType.rows(),
                                                                     1));
    SpvId boolType = this->getType(*fContext.fTypes.fBool);
    SpvId result = 0;
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId columnL = this->writeOpCompositeExtract(columnType, lhs, i, out);
        SpvId columnR = this->writeOpCompositeExtract(columnType, rhs, i, out);
        SpvId compare = this->nextId(&operandType);
        this->writeInstruction(compareOp, bvecType, compare, columnL, columnR, out);
        SpvId merge = this->nextId(nullptr);
        this->writeInstruction(vectorMergeOperator, boolType, merge, compare, out);
        if (result != 0) {
            SpvId next = this->nextId(nullptr);
            this->writeInstruction(mergeOperator, boolType, next, result, merge, out);
            result = next;
        } else {
            result = merge;
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeComponentwiseMatrixUnary(const Type& operandType,
                                                        SpvId operand,
                                                        SpvOp_ op,
                                                        OutputStream& out) {
    SkASSERT(operandType.isMatrix());
    const Type& columnType = operandType.componentType().toCompound(fContext,
                                                                    /*columns=*/operandType.rows(),
                                                                    /*rows=*/1);
    SpvId columnTypeId = this->getType(columnType);

    STArray<4, SpvId> columns;
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId srcColumn = this->writeOpCompositeExtract(columnType, operand, i, out);
        SpvId dstColumn = this->nextId(&operandType);
        this->writeInstruction(op, columnTypeId, dstColumn, srcColumn, out);
        columns.push_back(dstColumn);
    }

    return this->writeOpCompositeConstruct(operandType, columns, out);
}

SpvId SPIRVCodeGenerator::writeComponentwiseMatrixBinary(const Type& operandType, SpvId lhs,
                                                         SpvId rhs, SpvOp_ op, OutputStream& out) {
    SkASSERT(operandType.isMatrix());
    const Type& columnType = operandType.componentType().toCompound(fContext,
                                                                    /*columns=*/operandType.rows(),
                                                                    /*rows=*/1);
    SpvId columnTypeId = this->getType(columnType);

    STArray<4, SpvId> columns;
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId columnL = this->writeOpCompositeExtract(columnType, lhs, i, out);
        SpvId columnR = this->writeOpCompositeExtract(columnType, rhs, i, out);
        columns.push_back(this->nextId(&operandType));
        this->writeInstruction(op, columnTypeId, columns[i], columnL, columnR, out);
    }
    return this->writeOpCompositeConstruct(operandType, columns, out);
}

SpvId SPIRVCodeGenerator::writeReciprocal(const Type& type, SpvId value, OutputStream& out) {
    SkASSERT(type.isFloat());
    SpvId one = this->writeLiteral(1.0, type);
    SpvId reciprocal = this->nextId(&type);
    this->writeInstruction(SpvOpFDiv, this->getType(type), reciprocal, one, value, out);
    return reciprocal;
}

SpvId SPIRVCodeGenerator::writeScalarToMatrixSplat(const Type& matrixType,
                                                   SpvId scalarId,
                                                   OutputStream& out) {
    // Splat the scalar into a vector.
    const Type& vectorType = matrixType.componentType().toCompound(fContext,
                                                                   /*columns=*/matrixType.rows(),
                                                                   /*rows=*/1);
    STArray<4, SpvId> vecArguments;
    vecArguments.push_back_n(/*n=*/matrixType.rows(), /*t=*/scalarId);
    SpvId vectorId = this->writeOpCompositeConstruct(vectorType, vecArguments, out);

    // Splat the vector into a matrix.
    STArray<4, SpvId> matArguments;
    matArguments.push_back_n(/*n=*/matrixType.columns(), /*t=*/vectorId);
    return this->writeOpCompositeConstruct(matrixType, matArguments, out);
}

static bool types_match(const Type& a, const Type& b) {
    if (a.matches(b)) {
        return true;
    }
    return (a.typeKind() == b.typeKind()) &&
           (a.isScalar() || a.isVector() || a.isMatrix()) &&
           (a.columns() == b.columns() && a.rows() == b.rows()) &&
           a.componentType().numberKind() == b.componentType().numberKind();
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const Type& leftType, SpvId lhs, Operator op,
                                                const Type& rightType, SpvId rhs,
                                                const Type& resultType, OutputStream& out) {
    // The comma operator ignores the type of the left-hand side entirely.
    if (op.kind() == Operator::Kind::COMMA) {
        return rhs;
    }
    // overall type we are operating on: float2, int, uint4...
    const Type* operandType;
    if (types_match(leftType, rightType)) {
        operandType = &leftType;
    } else {
        // IR allows mismatched types in expressions (e.g. float2 * float), but they need special
        // handling in SPIR-V
        if (leftType.isVector() && rightType.isNumber()) {
            if (resultType.componentType().isFloat()) {
                switch (op.kind()) {
                    case Operator::Kind::SLASH: {
                        rhs = this->writeReciprocal(rightType, rhs, out);
                        [[fallthrough]];
                    }
                    case Operator::Kind::STAR: {
                        SpvId result = this->nextId(&resultType);
                        this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                               result, lhs, rhs, out);
                        return result;
                    }
                    default:
                        break;
                }
            }
            // Vectorize the right-hand side.
            STArray<4, SpvId> arguments;
            arguments.push_back_n(/*n=*/leftType.columns(), /*t=*/rhs);
            rhs = this->writeOpCompositeConstruct(leftType, arguments, out);
            operandType = &leftType;
        } else if (rightType.isVector() && leftType.isNumber()) {
            if (resultType.componentType().isFloat()) {
                if (op.kind() == Operator::Kind::STAR) {
                    SpvId result = this->nextId(&resultType);
                    this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                           result, rhs, lhs, out);
                    return result;
                }
            }
            // Vectorize the left-hand side.
            STArray<4, SpvId> arguments;
            arguments.push_back_n(/*n=*/rightType.columns(), /*t=*/lhs);
            lhs = this->writeOpCompositeConstruct(rightType, arguments, out);
            operandType = &rightType;
        } else if (leftType.isMatrix()) {
            if (op.kind() == Operator::Kind::STAR) {
                // Matrix-times-vector and matrix-times-scalar have dedicated ops in SPIR-V.
                SpvOp_ spvop;
                if (rightType.isMatrix()) {
                    spvop = SpvOpMatrixTimesMatrix;
                } else if (rightType.isVector()) {
                    spvop = SpvOpMatrixTimesVector;
                } else {
                    SkASSERT(rightType.isScalar());
                    spvop = SpvOpMatrixTimesScalar;
                }
                SpvId result = this->nextId(&resultType);
                this->writeInstruction(spvop, this->getType(resultType), result, lhs, rhs, out);
                return result;
            } else {
                // Matrix-op-vector is not supported in GLSL/SkSL for non-multiplication ops; we
                // expect to have a scalar here.
                SkASSERT(rightType.isScalar());

                // Splat rhs across an entire matrix so we can reuse the matrix-op-matrix path.
                SpvId rhsMatrix = this->writeScalarToMatrixSplat(leftType, rhs, out);

                // Perform this operation as matrix-op-matrix.
                return this->writeBinaryExpression(leftType, lhs, op, leftType, rhsMatrix,
                                                   resultType, out);
            }
        } else if (rightType.isMatrix()) {
            if (op.kind() == Operator::Kind::STAR) {
                // Matrix-times-vector and matrix-times-scalar have dedicated ops in SPIR-V.
                SpvId result = this->nextId(&resultType);
                if (leftType.isVector()) {
                    this->writeInstruction(SpvOpVectorTimesMatrix, this->getType(resultType),
                                           result, lhs, rhs, out);
                } else {
                    SkASSERT(leftType.isScalar());
                    this->writeInstruction(SpvOpMatrixTimesScalar, this->getType(resultType),
                                           result, rhs, lhs, out);
                }
                return result;
            } else {
                // Vector-op-matrix is not supported in GLSL/SkSL for non-multiplication ops; we
                // expect to have a scalar here.
                SkASSERT(leftType.isScalar());

                // Splat lhs across an entire matrix so we can reuse the matrix-op-matrix path.
                SpvId lhsMatrix = this->writeScalarToMatrixSplat(rightType, lhs, out);

                // Perform this operation as matrix-op-matrix.
                return this->writeBinaryExpression(rightType, lhsMatrix, op, rightType, rhs,
                                                   resultType, out);
            }
        } else {
            fContext.fErrors->error(leftType.fPosition, "unsupported mixed-type expression");
            return NA;
        }
    }

    switch (op.kind()) {
        case Operator::Kind::EQEQ: {
            if (operandType->isMatrix()) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFOrdEqual,
                                                   SpvOpIEqual, SpvOpAll, SpvOpLogicalAnd, out);
            }
            if (operandType->isStruct()) {
                return this->writeStructComparison(*operandType, lhs, op, rhs, out);
            }
            if (operandType->isArray()) {
                return this->writeArrayComparison(*operandType, lhs, op, rhs, out);
            }
            SkASSERT(resultType.isBoolean());
            const Type* tmpType;
            if (operandType->isVector()) {
                tmpType = &fContext.fTypes.fBool->toCompound(fContext,
                                                             operandType->columns(),
                                                             operandType->rows());
            } else {
                tmpType = &resultType;
            }
            if (lhs == rhs) {
                // This ignores the effects of NaN.
                return this->writeOpConstantTrue(*fContext.fTypes.fBool);
            }
            return this->foldToBool(this->writeBinaryOperation(*tmpType, *operandType, lhs, rhs,
                                                               SpvOpFOrdEqual, SpvOpIEqual,
                                                               SpvOpIEqual, SpvOpLogicalEqual, out),
                                    *operandType, SpvOpAll, out);
        }
        case Operator::Kind::NEQ:
            if (operandType->isMatrix()) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFUnordNotEqual,
                                                   SpvOpINotEqual, SpvOpAny, SpvOpLogicalOr, out);
            }
            if (operandType->isStruct()) {
                return this->writeStructComparison(*operandType, lhs, op, rhs, out);
            }
            if (operandType->isArray()) {
                return this->writeArrayComparison(*operandType, lhs, op, rhs, out);
            }
            [[fallthrough]];
        case Operator::Kind::LOGICALXOR:
            SkASSERT(resultType.isBoolean());
            const Type* tmpType;
            if (operandType->isVector()) {
                tmpType = &fContext.fTypes.fBool->toCompound(fContext,
                                                             operandType->columns(),
                                                             operandType->rows());
            } else {
                tmpType = &resultType;
            }
            if (lhs == rhs) {
                // This ignores the effects of NaN.
                return this->writeOpConstantFalse(*fContext.fTypes.fBool);
            }
            return this->foldToBool(this->writeBinaryOperation(*tmpType, *operandType, lhs, rhs,
                                                               SpvOpFUnordNotEqual, SpvOpINotEqual,
                                                               SpvOpINotEqual, SpvOpLogicalNotEqual,
                                                               out),
                                    *operandType, SpvOpAny, out);
        case Operator::Kind::GT:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThan, SpvOpSGreaterThan,
                                              SpvOpUGreaterThan, SpvOpUndef, out);
        case Operator::Kind::LT:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFOrdLessThan,
                                              SpvOpSLessThan, SpvOpULessThan, SpvOpUndef, out);
        case Operator::Kind::GTEQ:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThanEqual, SpvOpSGreaterThanEqual,
                                              SpvOpUGreaterThanEqual, SpvOpUndef, out);
        case Operator::Kind::LTEQ:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdLessThanEqual, SpvOpSLessThanEqual,
                                              SpvOpULessThanEqual, SpvOpUndef, out);
        case Operator::Kind::PLUS:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType.matches(rightType));
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs, SpvOpFAdd, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                              SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
        case Operator::Kind::MINUS:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType.matches(rightType));
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs, SpvOpFSub, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFSub,
                                              SpvOpISub, SpvOpISub, SpvOpUndef, out);
        case Operator::Kind::STAR:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                // matrix multiply
                SpvId result = this->nextId(&resultType);
                this->writeInstruction(SpvOpMatrixTimesMatrix, this->getType(resultType), result,
                                       lhs, rhs, out);
                return result;
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMul,
                                              SpvOpIMul, SpvOpIMul, SpvOpUndef, out);
        case Operator::Kind::SLASH:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType.matches(rightType));
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs, SpvOpFDiv, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFDiv,
                                              SpvOpSDiv, SpvOpUDiv, SpvOpUndef, out);
        case Operator::Kind::PERCENT:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMod,
                                              SpvOpSMod, SpvOpUMod, SpvOpUndef, out);
        case Operator::Kind::SHL:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpShiftLeftLogical, SpvOpShiftLeftLogical,
                                              SpvOpUndef, out);
        case Operator::Kind::SHR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpShiftRightArithmetic, SpvOpShiftRightLogical,
                                              SpvOpUndef, out);
        case Operator::Kind::BITWISEAND:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseAnd, SpvOpBitwiseAnd, SpvOpUndef, out);
        case Operator::Kind::BITWISEOR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseOr, SpvOpBitwiseOr, SpvOpUndef, out);
        case Operator::Kind::BITWISEXOR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseXor, SpvOpBitwiseXor, SpvOpUndef, out);
        default:
            fContext.fErrors->error(Position(), "unsupported token");
            return NA;
    }
}

SpvId SPIRVCodeGenerator::writeArrayComparison(const Type& arrayType, SpvId lhs, Operator op,
                                               SpvId rhs, OutputStream& out) {
    // The inputs must be arrays, and the op must be == or !=.
    SkASSERT(op.kind() == Operator::Kind::EQEQ || op.kind() == Operator::Kind::NEQ);
    SkASSERT(arrayType.isArray());
    const Type& componentType = arrayType.componentType();
    const int arraySize = arrayType.columns();
    SkASSERT(arraySize > 0);

    // Synthesize equality checks for each item in the array.
    const Type& boolType = *fContext.fTypes.fBool;
    SpvId allComparisons = NA;
    for (int index = 0; index < arraySize; ++index) {
        // Get the left and right item in the array.
        SpvId itemL = this->writeOpCompositeExtract(componentType, lhs, index, out);
        SpvId itemR = this->writeOpCompositeExtract(componentType, rhs, index, out);
        // Use `writeBinaryExpression` with the requested == or != operator on these items.
        SpvId comparison = this->writeBinaryExpression(componentType, itemL, op,
                                                       componentType, itemR, boolType, out);
        // Merge this comparison result with all the other comparisons we've done.
        allComparisons = this->mergeComparisons(comparison, allComparisons, op, out);
    }
    return allComparisons;
}

SpvId SPIRVCodeGenerator::writeStructComparison(const Type& structType, SpvId lhs, Operator op,
                                                SpvId rhs, OutputStream& out) {
    // The inputs must be structs containing fields, and the op must be == or !=.
    SkASSERT(op.kind() == Operator::Kind::EQEQ || op.kind() == Operator::Kind::NEQ);
    SkASSERT(structType.isStruct());
    SkSpan<const Field> fields = structType.fields();
    SkASSERT(!fields.empty());

    // Synthesize equality checks for each field in the struct.
    const Type& boolType = *fContext.fTypes.fBool;
    SpvId allComparisons = NA;
    for (int index = 0; index < (int)fields.size(); ++index) {
        // Get the left and right versions of this field.
        const Type& fieldType = *fields[index].fType;

        SpvId fieldL = this->writeOpCompositeExtract(fieldType, lhs, index, out);
        SpvId fieldR = this->writeOpCompositeExtract(fieldType, rhs, index, out);
        // Use `writeBinaryExpression` with the requested == or != operator on these fields.
        SpvId comparison = this->writeBinaryExpression(fieldType, fieldL, op, fieldType, fieldR,
                                                       boolType, out);
        // Merge this comparison result with all the other comparisons we've done.
        allComparisons = this->mergeComparisons(comparison, allComparisons, op, out);
    }
    return allComparisons;
}

SpvId SPIRVCodeGenerator::mergeComparisons(SpvId comparison, SpvId allComparisons, Operator op,
                                           OutputStream& out) {
    // If this is the first entry, we don't need to merge comparison results with anything.
    if (allComparisons == NA) {
        return comparison;
    }
    // Use LogicalAnd or LogicalOr to combine the comparison with all the other comparisons.
    const Type& boolType = *fContext.fTypes.fBool;
    SpvId boolTypeId = this->getType(boolType);
    SpvId logicalOp = this->nextId(&boolType);
    switch (op.kind()) {
        case Operator::Kind::EQEQ:
            this->writeInstruction(SpvOpLogicalAnd, boolTypeId, logicalOp,
                                   comparison, allComparisons, out);
            break;
        case Operator::Kind::NEQ:
            this->writeInstruction(SpvOpLogicalOr, boolTypeId, logicalOp,
                                   comparison, allComparisons, out);
            break;
        default:
            SkDEBUGFAILF("mergeComparisons only supports == and !=, not %s", op.operatorName());
            return NA;
    }
    return logicalOp;
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const BinaryExpression& b, OutputStream& out) {
    const Expression* left = b.left().get();
    const Expression* right = b.right().get();
    Operator op = b.getOperator();

    switch (op.kind()) {
        case Operator::Kind::EQ: {
            // Handles assignment.
            SpvId rhs = this->writeExpression(*right, out);
            this->getLValue(*left, out)->store(rhs, out);
            return rhs;
        }
        case Operator::Kind::LOGICALAND:
            // Handles short-circuiting; we don't necessarily evaluate both LHS and RHS.
            return this->writeLogicalAnd(*b.left(), *b.right(), out);

        case Operator::Kind::LOGICALOR:
            // Handles short-circuiting; we don't necessarily evaluate both LHS and RHS.
            return this->writeLogicalOr(*b.left(), *b.right(), out);

        default:
            break;
    }

    std::unique_ptr<LValue> lvalue;
    SpvId lhs;
    if (op.isAssignment()) {
        lvalue = this->getLValue(*left, out);
        lhs = lvalue->load(out);
    } else {
        lvalue = nullptr;
        lhs = this->writeExpression(*left, out);
    }

    SpvId rhs = this->writeExpression(*right, out);
    SpvId result = this->writeBinaryExpression(left->type(), lhs, op.removeAssignment(),
                                               right->type(), rhs, b.type(), out);
    if (lvalue) {
        lvalue->store(result, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalAnd(const Expression& left, const Expression& right,
                                          OutputStream& out) {
    SpvId falseConstant = this->writeLiteral(0.0, *fContext.fTypes.fBool);
    SpvId lhs = this->writeExpression(left, out);

    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    SpvId rhsLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, rhsLabel, end, out);
    this->writeLabel(rhsLabel, kBranchIsOnPreviousLine, out);
    SpvId rhs = this->writeExpression(right, out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    SpvId result = this->nextId(nullptr);
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fTypes.fBool), result, falseConstant,
                           lhsBlock, rhs, rhsBlock, out);

    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalOr(const Expression& left, const Expression& right,
                                         OutputStream& out) {
    SpvId trueConstant = this->writeLiteral(1.0, *fContext.fTypes.fBool);
    SpvId lhs = this->writeExpression(left, out);

    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    SpvId rhsLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, end, rhsLabel, out);
    this->writeLabel(rhsLabel, kBranchIsOnPreviousLine, out);
    SpvId rhs = this->writeExpression(right, out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    SpvId result = this->nextId(nullptr);
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fTypes.fBool), result, trueConstant,
                           lhsBlock, rhs, rhsBlock, out);

    return result;
}

SpvId SPIRVCodeGenerator::writeTernaryExpression(const TernaryExpression& t, OutputStream& out) {
    const Type& type = t.type();
    SpvId test = this->writeExpression(*t.test(), out);
    if (t.ifTrue()->type().columns() == 1 &&
        Analysis::IsCompileTimeConstant(*t.ifTrue()) &&
        Analysis::IsCompileTimeConstant(*t.ifFalse())) {
        // both true and false are constants, can just use OpSelect
        SpvId result = this->nextId(nullptr);
        SpvId trueId = this->writeExpression(*t.ifTrue(), out);
        SpvId falseId = this->writeExpression(*t.ifFalse(), out);
        this->writeInstruction(SpvOpSelect, this->getType(type), result, test, trueId, falseId,
                               out);
        return result;
    }

    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    // was originally using OpPhi to choose the result, but for some reason that is crashing on
    // Adreno. Switched to storing the result in a temp variable as glslang does.
    SpvId var = this->nextId(nullptr);
    this->writeInstruction(SpvOpVariable, this->getPointerType(type, SpvStorageClassFunction),
                           var, SpvStorageClassFunction, fVariableBuffer);
    SpvId trueLabel = this->nextId(nullptr);
    SpvId falseLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, test, trueLabel, falseLabel, out);
    this->writeLabel(trueLabel, kBranchIsOnPreviousLine, out);
    this->writeOpStore(SpvStorageClassFunction, var, this->writeExpression(*t.ifTrue(), out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(falseLabel, kBranchIsAbove, conditionalOps, out);
    this->writeOpStore(SpvStorageClassFunction, var, this->writeExpression(*t.ifFalse(), out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    SpvId result = this->nextId(&type);
    this->writeInstruction(SpvOpLoad, this->getType(type), result, var, out);

    return result;
}

SpvId SPIRVCodeGenerator::writePrefixExpression(const PrefixExpression& p, OutputStream& out) {
    const Type& type = p.type();
    if (p.getOperator().kind() == Operator::Kind::MINUS) {
        SpvOp_ negateOp = pick_by_type(type, SpvOpFNegate, SpvOpSNegate, SpvOpSNegate, SpvOpUndef);
        SkASSERT(negateOp != SpvOpUndef);
        SpvId expr = this->writeExpression(*p.operand(), out);
        if (type.isMatrix()) {
            return this->writeComponentwiseMatrixUnary(type, expr, negateOp, out);
        }
        SpvId result = this->nextId(&type);
        SpvId typeId = this->getType(type);
        this->writeInstruction(negateOp, typeId, result, expr, out);
        return result;
    }
    switch (p.getOperator().kind()) {
        case Operator::Kind::PLUS:
            return this->writeExpression(*p.operand(), out);
        case Operator::Kind::PLUSPLUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
            SpvId one = this->writeLiteral(1.0, type);
            SpvId result = this->writeBinaryOperation(type, type, lv->load(out), one,
                                                      SpvOpFAdd, SpvOpIAdd, SpvOpIAdd, SpvOpUndef,
                                                      out);
            lv->store(result, out);
            return result;
        }
        case Operator::Kind::MINUSMINUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
            SpvId one = this->writeLiteral(1.0, type);
            SpvId result = this->writeBinaryOperation(type, type, lv->load(out), one, SpvOpFSub,
                                                      SpvOpISub, SpvOpISub, SpvOpUndef, out);
            lv->store(result, out);
            return result;
        }
        case Operator::Kind::LOGICALNOT: {
            SkASSERT(p.operand()->type().isBoolean());
            SpvId result = this->nextId(nullptr);
            this->writeInstruction(SpvOpLogicalNot, this->getType(type), result,
                                   this->writeExpression(*p.operand(), out), out);
            return result;
        }
        case Operator::Kind::BITWISENOT: {
            SpvId result = this->nextId(nullptr);
            this->writeInstruction(SpvOpNot, this->getType(type), result,
                                   this->writeExpression(*p.operand(), out), out);
            return result;
        }
        default:
            SkDEBUGFAILF("unsupported prefix expression: %s",
                         p.description(OperatorPrecedence::kExpression).c_str());
            return NA;
    }
}

SpvId SPIRVCodeGenerator::writePostfixExpression(const PostfixExpression& p, OutputStream& out) {
    const Type& type = p.type();
    std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
    SpvId result = lv->load(out);
    SpvId one = this->writeLiteral(1.0, type);
    switch (p.getOperator().kind()) {
        case Operator::Kind::PLUSPLUS: {
            SpvId temp = this->writeBinaryOperation(type, type, result, one, SpvOpFAdd,
                                                    SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
            lv->store(temp, out);
            return result;
        }
        case Operator::Kind::MINUSMINUS: {
            SpvId temp = this->writeBinaryOperation(type, type, result, one, SpvOpFSub,
                                                    SpvOpISub, SpvOpISub, SpvOpUndef, out);
            lv->store(temp, out);
            return result;
        }
        default:
            SkDEBUGFAILF("unsupported postfix expression %s",
                         p.description(OperatorPrecedence::kExpression).c_str());
            return NA;
    }
}

SpvId SPIRVCodeGenerator::writeLiteral(const Literal& l) {
    return this->writeLiteral(l.value(), l.type());
}

SpvId SPIRVCodeGenerator::writeLiteral(double value, const Type& type) {
    switch (type.numberKind()) {
        case Type::NumberKind::kFloat: {
            float floatVal = value;
            int32_t valueBits;
            memcpy(&valueBits, &floatVal, sizeof(valueBits));
            return this->writeOpConstant(type, valueBits);
        }
        case Type::NumberKind::kBoolean: {
            return value ? this->writeOpConstantTrue(type)
                         : this->writeOpConstantFalse(type);
        }
        default: {
            return this->writeOpConstant(type, (SKSL_INT)value);
        }
    }
}

SpvId SPIRVCodeGenerator::writeFunctionStart(const FunctionDeclaration& f, OutputStream& out) {
    SpvId result = fFunctionMap[&f];
    SpvId returnTypeId = this->getType(f.returnType());
    SpvId functionTypeId = this->getFunctionType(f);
    this->writeInstruction(SpvOpFunction, returnTypeId, result,
                           SpvFunctionControlMaskNone, functionTypeId, out);
    std::string mangledName = f.mangledName();
    this->writeInstruction(SpvOpName,
                           result,
                           std::string_view(mangledName.c_str(), mangledName.size()),
                           fNameBuffer);
    for (const Variable* parameter : f.parameters()) {
        if (fUseTextureSamplerPairs && parameter->type().isSampler()) {
            auto [texture, sampler] = this->synthesizeTextureAndSampler(*parameter);

            SpvId textureId = this->nextId(nullptr);
            SpvId samplerId = this->nextId(nullptr);
            fVariableMap.set(texture, textureId);
            fVariableMap.set(sampler, samplerId);

            SpvId textureType = this->getFunctionParameterType(texture->type());
            SpvId samplerType = this->getFunctionParameterType(sampler->type());

            this->writeInstruction(SpvOpFunctionParameter, textureType, textureId, out);
            this->writeInstruction(SpvOpFunctionParameter, samplerType, samplerId, out);
        } else {
            SpvId id = this->nextId(nullptr);
            fVariableMap.set(parameter, id);

            SpvId type = this->getFunctionParameterType(parameter->type());
            this->writeInstruction(SpvOpFunctionParameter, type, id, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFunction(const FunctionDefinition& f, OutputStream& out) {
    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    fVariableBuffer.reset();
    SpvId result = this->writeFunctionStart(f.declaration(), out);
    fCurrentBlock = 0;
    this->writeLabel(this->nextId(nullptr), kBranchlessBlock, out);
    StringStream bodyBuffer;
    this->writeBlock(f.body()->as<Block>(), bodyBuffer);
    write_stringstream(fVariableBuffer, out);
    if (f.declaration().isMain()) {
        write_stringstream(fGlobalInitializersBuffer, out);
    }
    write_stringstream(bodyBuffer, out);
    if (fCurrentBlock) {
        if (f.declaration().returnType().isVoid()) {
            this->writeInstruction(SpvOpReturn, out);
        } else {
            this->writeInstruction(SpvOpUnreachable, out);
        }
    }
    this->writeInstruction(SpvOpFunctionEnd, out);
    this->pruneConditionalOps(conditionalOps);
    return result;
}

void SPIRVCodeGenerator::writeLayout(const Layout& layout, SpvId target, Position pos) {
    bool isPushConstant = SkToBool(layout.fFlags & LayoutFlag::kPushConstant);
    if (layout.fLocation >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationLocation, layout.fLocation,
                               fDecorationBuffer);
    }
    if (layout.fBinding >= 0) {
        if (isPushConstant) {
            fContext.fErrors->error(pos, "Can't apply 'binding' to push constants");
        } else {
            this->writeInstruction(SpvOpDecorate, target, SpvDecorationBinding, layout.fBinding,
                                   fDecorationBuffer);
        }
    }
    if (layout.fIndex >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationIndex, layout.fIndex,
                               fDecorationBuffer);
    }
    if (layout.fSet >= 0) {
        if (isPushConstant) {
            fContext.fErrors->error(pos, "Can't apply 'set' to push constants");
        } else {
            this->writeInstruction(SpvOpDecorate, target, SpvDecorationDescriptorSet, layout.fSet,
                                   fDecorationBuffer);
        }
    }
    if (layout.fInputAttachmentIndex >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationInputAttachmentIndex,
                               layout.fInputAttachmentIndex, fDecorationBuffer);
        fCapabilities |= (((uint64_t) 1) << SpvCapabilityInputAttachment);
    }
    if (layout.fBuiltin >= 0 && layout.fBuiltin != SK_FRAGCOLOR_BUILTIN) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationBuiltIn, layout.fBuiltin,
                               fDecorationBuffer);
    }
}

void SPIRVCodeGenerator::writeFieldLayout(const Layout& layout, SpvId target, int member) {
    // 'binding' and 'set' can not be applied to struct members
    SkASSERT(layout.fBinding == -1);
    SkASSERT(layout.fSet == -1);
    if (layout.fLocation >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationLocation,
                               layout.fLocation, fDecorationBuffer);
    }
    if (layout.fIndex >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationIndex,
                               layout.fIndex, fDecorationBuffer);
    }
    if (layout.fInputAttachmentIndex >= 0) {
        this->writeInstruction(SpvOpDecorate, target, member, SpvDecorationInputAttachmentIndex,
                               layout.fInputAttachmentIndex, fDecorationBuffer);
    }
    if (layout.fBuiltin >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationBuiltIn,
                               layout.fBuiltin, fDecorationBuffer);
    }
}

MemoryLayout SPIRVCodeGenerator::memoryLayoutForStorageClass(SpvStorageClass_ storageClass) {
    return storageClass == SpvStorageClassPushConstant ? MemoryLayout(MemoryLayout::Standard::k430)
                                                       : fDefaultLayout;
}

MemoryLayout SPIRVCodeGenerator::memoryLayoutForVariable(const Variable& v) const {
    bool pushConstant = SkToBool(v.layout().fFlags & LayoutFlag::kPushConstant);
    return pushConstant ? MemoryLayout(MemoryLayout::Standard::k430) : fDefaultLayout;
}

SpvId SPIRVCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf, bool appendRTFlip) {
    MemoryLayout memoryLayout = this->memoryLayoutForVariable(*intf.var());
    SpvId result = this->nextId(nullptr);
    const Variable& intfVar = *intf.var();
    const Type& type = intfVar.type();
    if (!memoryLayout.isSupported(type)) {
        fContext.fErrors->error(type.fPosition, "type '" + type.displayName() +
                                                "' is not permitted here");
        return this->nextId(nullptr);
    }
    SpvStorageClass_ storageClass =
            get_storage_class_for_global_variable(intfVar, SpvStorageClassFunction);
    if (fProgram.fInterface.fUseFlipRTUniform && appendRTFlip && type.isStruct()) {
        // We can only have one interface block (because we use push_constant and that is limited
        // to one per program), so we need to append rtflip to this one rather than synthesize an
        // entirely new block when the variable is referenced. And we can't modify the existing
        // block, so we instead create a modified copy of it and write that.
        SkSpan<const Field> fieldSpan = type.fields();
        TArray<Field> fields(fieldSpan.data(), fieldSpan.size());
        fields.emplace_back(Position(),
                            Layout(LayoutFlag::kNone,
                                   /*location=*/-1,
                                   fProgram.fConfig->fSettings.fRTFlipOffset,
                                   /*binding=*/-1,
                                   /*index=*/-1,
                                   /*set=*/-1,
                                   /*builtin=*/-1,
                                   /*inputAttachmentIndex=*/-1),
                            ModifierFlag::kNone,
                            SKSL_RTFLIP_NAME,
                            fContext.fTypes.fFloat2.get());
        {
            AutoAttachPoolToThread attach(fProgram.fPool.get());
            const Type* rtFlipStructType = fProgram.fSymbols->takeOwnershipOfSymbol(
                    Type::MakeStructType(fContext,
                                         type.fPosition,
                                         type.name(),
                                         std::move(fields),
                                         /*interfaceBlock=*/true));
            Variable* modifiedVar = fProgram.fSymbols->takeOwnershipOfSymbol(
                    Variable::Make(intfVar.fPosition,
                                   intfVar.modifiersPosition(),
                                   intfVar.layout(),
                                   intfVar.modifierFlags(),
                                   rtFlipStructType,
                                   intfVar.name(),
                                   /*mangledName=*/"",
                                   intfVar.isBuiltin(),
                                   intfVar.storage()));
            fSPIRVBonusVariables.add(modifiedVar);
            InterfaceBlock modifiedCopy(intf.fPosition, modifiedVar, intf.typeOwner());
            result = this->writeInterfaceBlock(modifiedCopy, /*appendRTFlip=*/false);
            fProgram.fSymbols->add(std::make_unique<FieldSymbol>(
                    Position(), modifiedVar, rtFlipStructType->fields().size() - 1));
        }
        fVariableMap.set(&intfVar, result);
        fWroteRTFlip = true;
        return result;
    }
    SpvId typeId = this->getType(type, memoryLayout);
    if (intfVar.layout().fBuiltin == -1) {
        // Note: In SPIR-V 1.3, a storage buffer can be declared with the "StorageBuffer"
        // storage class and the "Block" decoration and the <1.3 approach we use here ("Uniform"
        // storage class and the "BufferBlock" decoration) is deprecated. Since we target SPIR-V
        // 1.0, we have to use the deprecated approach which is well supported in Vulkan and
        // addresses SkSL use cases (notably SkSL currently doesn't support pointer features that
        // would benefit from SPV_KHR_variable_pointers capabilities).
        bool isStorageBuffer = intfVar.modifierFlags().isBuffer();
        this->writeInstruction(SpvOpDecorate,
                               typeId,
                               isStorageBuffer ? SpvDecorationBufferBlock : SpvDecorationBlock,
                               fDecorationBuffer);
    }
    SpvId ptrType = this->nextId(nullptr);
    this->writeInstruction(SpvOpTypePointer, ptrType, storageClass, typeId, fConstantBuffer);
    this->writeInstruction(SpvOpVariable, ptrType, result, storageClass, fConstantBuffer);
    Layout layout = intfVar.layout();
    if (storageClass == SpvStorageClassUniform && layout.fSet < 0) {
        layout.fSet = fProgram.fConfig->fSettings.fDefaultUniformSet;
    }
    this->writeLayout(layout, result, intfVar.fPosition);
    fVariableMap.set(&intfVar, result);
    return result;
}

bool SPIRVCodeGenerator::isDead(const Variable& var) const {
    // During SPIR-V code generation, we synthesize some extra bonus variables that don't actually
    // exist in the Program at all and aren't tracked by the ProgramUsage. They aren't dead, though.
    if (fSPIRVBonusVariables.contains(&var)) {
        return false;
    }
    ProgramUsage::VariableCounts counts = fProgram.usage()->get(var);
    if (counts.fRead || counts.fWrite) {
        return false;
    }
    // It's not entirely clear what the rules are for eliding interface variables. Generally, it
    // causes problems to elide them, even when they're dead.
    return !(var.modifierFlags() &
             (ModifierFlag::kIn | ModifierFlag::kOut | ModifierFlag::kUniform));
}

// This function determines whether to skip an OpVariable (of pointer type) declaration for
// compile-time constant scalars and vectors which we turn into OpConstant/OpConstantComposite and
// always reference by value.
//
// Accessing a matrix or array member with a dynamic index requires the use of OpAccessChain which
// requires a base operand of pointer type. However, a vector can always be accessed by value using
// OpVectorExtractDynamic (see writeIndexExpression).
//
// This is why we always emit an OpVariable for all non-scalar and non-vector types in case they get
// accessed via a dynamic index.
static bool is_vardecl_compile_time_constant(const VarDeclaration& varDecl) {
    return varDecl.var()->modifierFlags().isConst() &&
           (varDecl.var()->type().isScalar() || varDecl.var()->type().isVector()) &&
           (ConstantFolder::GetConstantValueOrNull(*varDecl.value()) ||
            Analysis::IsCompileTimeConstant(*varDecl.value()));
}

bool SPIRVCodeGenerator::writeGlobalVarDeclaration(ProgramKind kind,
                                                   const VarDeclaration& varDecl) {
    const Variable* var = varDecl.var();
    const LayoutFlags backendFlags = var->layout().fFlags & LayoutFlag::kAllBackends;
    const LayoutFlags kPermittedBackendFlags =
            LayoutFlag::kVulkan | LayoutFlag::kWebGPU | LayoutFlag::kDirect3D;
    if (backendFlags & ~kPermittedBackendFlags) {
        fContext.fErrors->error(var->fPosition, "incompatible backend flag in SPIR-V codegen");
        return false;
    }

    // If this global variable is a compile-time constant then we'll emit OpConstant or
    // OpConstantComposite later when the variable is referenced. Avoid declaring an OpVariable now.
    if (is_vardecl_compile_time_constant(varDecl)) {
        return true;
    }

    SpvStorageClass_ storageClass =
            get_storage_class_for_global_variable(*var, SpvStorageClassPrivate);
    if (storageClass == SpvStorageClassUniform) {
        // Top-level uniforms are emitted in writeUniformBuffer.
        fTopLevelUniforms.push_back(&varDecl);
        return true;
    }

    if (this->isDead(*var)) {
        return true;
    }

    if (fUseTextureSamplerPairs && var->type().isSampler()) {
        if (var->layout().fTexture == -1 || var->layout().fSampler == -1) {
            fContext.fErrors->error(var->fPosition, "selected backend requires separate texture "
                                                    "and sampler indices");
            return false;
        }
        SkASSERT(storageClass == SpvStorageClassUniformConstant);

        auto [texture, sampler] = this->synthesizeTextureAndSampler(*var);
        this->writeGlobalVar(kind, storageClass, *texture);
        this->writeGlobalVar(kind, storageClass, *sampler);

        return true;
    }

    SpvId id = this->writeGlobalVar(kind, storageClass, *var);
    if (id != NA && varDecl.value()) {
        SkASSERT(!fCurrentBlock);
        fCurrentBlock = NA;
        SpvId value = this->writeExpression(*varDecl.value(), fGlobalInitializersBuffer);
        this->writeOpStore(storageClass, id, value, fGlobalInitializersBuffer);
        fCurrentBlock = 0;
    }
    return true;
}

SpvId SPIRVCodeGenerator::writeGlobalVar(ProgramKind kind,
                                         SpvStorageClass_ storageClass,
                                         const Variable& var) {
    if (var.layout().fBuiltin == SK_FRAGCOLOR_BUILTIN &&
        !ProgramConfig::IsFragment(kind)) {
        SkASSERT(!fProgram.fConfig->fSettings.fFragColorIsInOut);
        return NA;
    }

    // Add this global to the variable map.
    const Type& type = var.type();
    SpvId id = this->nextId(&type);
    fVariableMap.set(&var, id);

    Layout layout = var.layout();
    if (layout.fSet < 0 && storageClass == SpvStorageClassUniformConstant) {
        layout.fSet = fProgram.fConfig->fSettings.fDefaultUniformSet;
    }

    SpvId typeId = this->getPointerType(type, storageClass);
    this->writeInstruction(SpvOpVariable, typeId, id, storageClass, fConstantBuffer);
    this->writeInstruction(SpvOpName, id, var.name(), fNameBuffer);
    this->writeLayout(layout, id, var.fPosition);
    if (var.modifierFlags() & ModifierFlag::kFlat) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationFlat, fDecorationBuffer);
    }
    if (var.modifierFlags() & ModifierFlag::kNoPerspective) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationNoPerspective,
                               fDecorationBuffer);
    }
    if (var.modifierFlags().isWriteOnly()) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationNonReadable, fDecorationBuffer);
    } else if (var.modifierFlags().isReadOnly()) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationNonWritable, fDecorationBuffer);
    }

    return id;
}

void SPIRVCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl, OutputStream& out) {
    // If this variable is a compile-time constant then we'll emit OpConstant or
    // OpConstantComposite later when the variable is referenced. Avoid declaring an OpVariable now.
    if (is_vardecl_compile_time_constant(varDecl)) {
        return;
    }

    const Variable* var = varDecl.var();
    SpvId id = this->nextId(&var->type());
    fVariableMap.set(var, id);
    SpvId type = this->getPointerType(var->type(), SpvStorageClassFunction);
    this->writeInstruction(SpvOpVariable, type, id, SpvStorageClassFunction, fVariableBuffer);
    this->writeInstruction(SpvOpName, id, var->name(), fNameBuffer);
    if (varDecl.value()) {
        SpvId value = this->writeExpression(*varDecl.value(), out);
        this->writeOpStore(SpvStorageClassFunction, id, value, out);
    }
}

void SPIRVCodeGenerator::writeStatement(const Statement& s, OutputStream& out) {
    switch (s.kind()) {
        case Statement::Kind::kNop:
            break;
        case Statement::Kind::kBlock:
            this->writeBlock(s.as<Block>(), out);
            break;
        case Statement::Kind::kExpression:
            this->writeExpression(*s.as<ExpressionStatement>().expression(), out);
            break;
        case Statement::Kind::kReturn:
            this->writeReturnStatement(s.as<ReturnStatement>(), out);
            break;
        case Statement::Kind::kVarDeclaration:
            this->writeVarDeclaration(s.as<VarDeclaration>(), out);
            break;
        case Statement::Kind::kIf:
            this->writeIfStatement(s.as<IfStatement>(), out);
            break;
        case Statement::Kind::kFor:
            this->writeForStatement(s.as<ForStatement>(), out);
            break;
        case Statement::Kind::kDo:
            this->writeDoStatement(s.as<DoStatement>(), out);
            break;
        case Statement::Kind::kSwitch:
            this->writeSwitchStatement(s.as<SwitchStatement>(), out);
            break;
        case Statement::Kind::kBreak:
            this->writeInstruction(SpvOpBranch, fBreakTarget.back(), out);
            break;
        case Statement::Kind::kContinue:
            this->writeInstruction(SpvOpBranch, fContinueTarget.back(), out);
            break;
        case Statement::Kind::kDiscard:
            this->writeInstruction(SpvOpKill, out);
            break;
        default:
            SkDEBUGFAILF("unsupported statement: %s", s.description().c_str());
            break;
    }
}

void SPIRVCodeGenerator::writeBlock(const Block& b, OutputStream& out) {
    for (const std::unique_ptr<Statement>& stmt : b.children()) {
        this->writeStatement(*stmt, out);
    }
}

SPIRVCodeGenerator::ConditionalOpCounts SPIRVCodeGenerator::getConditionalOpCounts() {
    return {fReachableOps.size(), fStoreOps.size()};
}

void SPIRVCodeGenerator::pruneConditionalOps(ConditionalOpCounts ops) {
    // Remove ops which are no longer reachable.
    while (fReachableOps.size() > ops.numReachableOps) {
        SpvId prunableSpvId = fReachableOps.back();
        const Instruction* prunableOp = fSpvIdCache.find(prunableSpvId);

        if (prunableOp) {
            fOpCache.remove(*prunableOp);
            fSpvIdCache.remove(prunableSpvId);
        } else {
            SkDEBUGFAIL("reachable-op list contains unrecognized SpvId");
        }

        fReachableOps.pop_back();
    }

    // Remove any cached stores that occurred during the conditional block.
    while (fStoreOps.size() > ops.numStoreOps) {
        if (fStoreCache.find(fStoreOps.back())) {
            fStoreCache.remove(fStoreOps.back());
        }
        fStoreOps.pop_back();
    }
}

void SPIRVCodeGenerator::writeIfStatement(const IfStatement& stmt, OutputStream& out) {
    SpvId test = this->writeExpression(*stmt.test(), out);
    SpvId ifTrue = this->nextId(nullptr);
    SpvId ifFalse = this->nextId(nullptr);

    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    if (stmt.ifFalse()) {
        SpvId end = this->nextId(nullptr);
        this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
        this->writeInstruction(SpvOpBranchConditional, test, ifTrue, ifFalse, out);
        this->writeLabel(ifTrue, kBranchIsOnPreviousLine, out);
        this->writeStatement(*stmt.ifTrue(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, end, out);
        }
        this->writeLabel(ifFalse, kBranchIsAbove, conditionalOps, out);
        this->writeStatement(*stmt.ifFalse(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, end, out);
        }
        this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    } else {
        this->writeInstruction(SpvOpSelectionMerge, ifFalse, SpvSelectionControlMaskNone, out);
        this->writeInstruction(SpvOpBranchConditional, test, ifTrue, ifFalse, out);
        this->writeLabel(ifTrue, kBranchIsOnPreviousLine, out);
        this->writeStatement(*stmt.ifTrue(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, ifFalse, out);
        }
        this->writeLabel(ifFalse, kBranchIsAbove, conditionalOps, out);
    }
}

void SPIRVCodeGenerator::writeForStatement(const ForStatement& f, OutputStream& out) {
    if (f.initializer()) {
        this->writeStatement(*f.initializer(), out);
    }

    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    // The store cache isn't trustworthy in the presence of branches; store caching only makes sense
    // in the context of linear straight-line execution. If we wanted to be more clever, we could
    // only invalidate store cache entries for variables affected by the loop body, but for now we
    // simply clear the entire cache whenever branching occurs.
    SpvId header = this->nextId(nullptr);
    SpvId start = this->nextId(nullptr);
    SpvId body = this->nextId(nullptr);
    SpvId next = this->nextId(nullptr);
    fContinueTarget.push_back(next);
    SpvId end = this->nextId(nullptr);
    fBreakTarget.push_back(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, kBranchIsBelow, conditionalOps, out);
    this->writeInstruction(SpvOpLoopMerge, end, next, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, kBranchIsOnPreviousLine, out);
    if (f.test()) {
        SpvId test = this->writeExpression(*f.test(), out);
        this->writeInstruction(SpvOpBranchConditional, test, body, end, out);
    } else {
        this->writeInstruction(SpvOpBranch, body, out);
    }
    this->writeLabel(body, kBranchIsOnPreviousLine, out);
    this->writeStatement(*f.statement(), out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
    }
    this->writeLabel(next, kBranchIsAbove, conditionalOps, out);
    if (f.next()) {
        this->writeExpression(*f.next(), out);
    }
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    fBreakTarget.pop_back();
    fContinueTarget.pop_back();
}

void SPIRVCodeGenerator::writeDoStatement(const DoStatement& d, OutputStream& out) {
    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    // The store cache isn't trustworthy in the presence of branches; store caching only makes sense
    // in the context of linear straight-line execution. If we wanted to be more clever, we could
    // only invalidate store cache entries for variables affected by the loop body, but for now we
    // simply clear the entire cache whenever branching occurs.
    SpvId header = this->nextId(nullptr);
    SpvId start = this->nextId(nullptr);
    SpvId next = this->nextId(nullptr);
    SpvId continueTarget = this->nextId(nullptr);
    fContinueTarget.push_back(continueTarget);
    SpvId end = this->nextId(nullptr);
    fBreakTarget.push_back(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, kBranchIsBelow, conditionalOps, out);
    this->writeInstruction(SpvOpLoopMerge, end, continueTarget, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, kBranchIsOnPreviousLine, out);
    this->writeStatement(*d.statement(), out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
        this->writeLabel(next, kBranchIsOnPreviousLine, out);
        this->writeInstruction(SpvOpBranch, continueTarget, out);
    }
    this->writeLabel(continueTarget, kBranchIsAbove, conditionalOps, out);
    SpvId test = this->writeExpression(*d.test(), out);
    this->writeInstruction(SpvOpBranchConditional, test, header, end, out);
    this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    fBreakTarget.pop_back();
    fContinueTarget.pop_back();
}

void SPIRVCodeGenerator::writeSwitchStatement(const SwitchStatement& s, OutputStream& out) {
    SpvId value = this->writeExpression(*s.value(), out);

    ConditionalOpCounts conditionalOps = this->getConditionalOpCounts();

    // The store cache isn't trustworthy in the presence of branches; store caching only makes sense
    // in the context of linear straight-line execution. If we wanted to be more clever, we could
    // only invalidate store cache entries for variables affected by the switch body, but for now we
    // simply clear the entire cache whenever branching occurs.
    TArray<SpvId> labels;
    SpvId end = this->nextId(nullptr);
    SpvId defaultLabel = end;
    fBreakTarget.push_back(end);
    int size = 3;
    const StatementArray& cases = s.cases();
    for (const std::unique_ptr<Statement>& stmt : cases) {
        const SwitchCase& c = stmt->as<SwitchCase>();
        SpvId label = this->nextId(nullptr);
        labels.push_back(label);
        if (!c.isDefault()) {
            size += 2;
        } else {
            defaultLabel = label;
        }
    }

    // We should have exactly one label for each case.
    SkASSERT(labels.size() == cases.size());

    // Collapse adjacent switch-cases into one; that is, reduce `case 1: case 2: case 3:` into a
    // single OpLabel. The Tint SPIR-V reader does not support switch-case fallthrough, but it
    // does support multiple switch-cases branching to the same label.
    SkBitSet caseIsCollapsed(cases.size());
    for (int index = cases.size() - 2; index >= 0; index--) {
        if (cases[index]->as<SwitchCase>().statement()->isEmpty()) {
            caseIsCollapsed.set(index);
            labels[index] = labels[index + 1];
        }
    }

    labels.push_back(end);

    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeOpCode(SpvOpSwitch, size, out);
    this->writeWord(value, out);
    this->writeWord(defaultLabel, out);
    for (int i = 0; i < cases.size(); ++i) {
        const SwitchCase& c = cases[i]->as<SwitchCase>();
        if (c.isDefault()) {
            continue;
        }
        this->writeWord(c.value(), out);
        this->writeWord(labels[i], out);
    }
    for (int i = 0; i < cases.size(); ++i) {
        if (caseIsCollapsed.test(i)) {
            continue;
        }
        const SwitchCase& c = cases[i]->as<SwitchCase>();
        if (i == 0) {
            this->writeLabel(labels[i], kBranchIsOnPreviousLine, out);
        } else {
            this->writeLabel(labels[i], kBranchIsAbove, conditionalOps, out);
        }
        this->writeStatement(*c.statement(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, labels[i + 1], out);
        }
    }
    this->writeLabel(end, kBranchIsAbove, conditionalOps, out);
    fBreakTarget.pop_back();
}

void SPIRVCodeGenerator::writeReturnStatement(const ReturnStatement& r, OutputStream& out) {
    if (r.expression()) {
        this->writeInstruction(SpvOpReturnValue, this->writeExpression(*r.expression(), out),
                               out);
    } else {
        this->writeInstruction(SpvOpReturn, out);
    }
}

// Given any function, returns the top-level symbol table (OUTSIDE of the function's scope).
static std::shared_ptr<SymbolTable> get_top_level_symbol_table(const FunctionDeclaration& anyFunc) {
    return anyFunc.definition()->body()->as<Block>().symbolTable()->fParent;
}

SPIRVCodeGenerator::EntrypointAdapter SPIRVCodeGenerator::writeEntrypointAdapter(
        const FunctionDeclaration& main) {
    // Our goal is to synthesize a tiny helper function which looks like this:
    //     void _entrypoint() { sk_FragColor = main(); }

    // Fish a symbol table out of main().
    std::shared_ptr<SymbolTable> symbolTable = get_top_level_symbol_table(main);

    // Get `sk_FragColor` as a writable reference.
    const Symbol* skFragColorSymbol = symbolTable->find("sk_FragColor");
    SkASSERT(skFragColorSymbol);
    const Variable& skFragColorVar = skFragColorSymbol->as<Variable>();
    auto skFragColorRef = std::make_unique<VariableReference>(Position(), &skFragColorVar,
                                                              VariableReference::RefKind::kWrite);
    // Synthesize a call to the `main()` function.
    if (!main.returnType().matches(skFragColorRef->type())) {
        fContext.fErrors->error(main.fPosition, "SPIR-V does not support returning '" +
                main.returnType().description() + "' from main()");
        return {};
    }
    ExpressionArray args;
    if (main.parameters().size() == 1) {
        if (!main.parameters()[0]->type().matches(*fContext.fTypes.fFloat2)) {
            fContext.fErrors->error(main.fPosition,
                    "SPIR-V does not support parameter of type '" +
                    main.parameters()[0]->type().description() + "' to main()");
            return {};
        }
        double kZero[2] = {0.0, 0.0};
        args.push_back(ConstructorCompound::MakeFromConstants(fContext, Position{},
                                                              *fContext.fTypes.fFloat2, kZero));
    }
    auto callMainFn = std::make_unique<FunctionCall>(Position(), &main.returnType(), &main,
                                                     std::move(args));

    // Synthesize `skFragColor = main()` as a BinaryExpression.
    auto assignmentStmt = std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
            Position(),
            std::move(skFragColorRef),
            Operator::Kind::EQ,
            std::move(callMainFn),
            &main.returnType()));

    // Function bodies are always wrapped in a Block.
    StatementArray entrypointStmts;
    entrypointStmts.push_back(std::move(assignmentStmt));
    auto entrypointBlock = Block::Make(Position(), std::move(entrypointStmts),
                                       Block::Kind::kBracedScope, symbolTable);
    // Declare an entrypoint function.
    EntrypointAdapter adapter;
    adapter.entrypointDecl =
            std::make_unique<FunctionDeclaration>(fContext,
                                                  Position(),
                                                  ModifierFlag::kNone,
                                                  "_entrypoint",
                                                  /*parameters=*/TArray<Variable*>{},
                                                  /*returnType=*/fContext.fTypes.fVoid.get(),
                                                  kNotIntrinsic);
    // Define it.
    adapter.entrypointDef = FunctionDefinition::Convert(fContext,
                                                        Position(),
                                                        *adapter.entrypointDecl,
                                                        std::move(entrypointBlock),
                                                        /*builtin=*/false);

    adapter.entrypointDecl->setDefinition(adapter.entrypointDef.get());
    return adapter;
}

void SPIRVCodeGenerator::writeUniformBuffer(std::shared_ptr<SymbolTable> topLevelSymbolTable) {
    SkASSERT(!fTopLevelUniforms.empty());
    static constexpr char kUniformBufferName[] = "_UniformBuffer";

    // Convert the list of top-level uniforms into a matching struct named _UniformBuffer, and build
    // a lookup table of variables to UniformBuffer field indices.
    TArray<Field> fields;
    fields.reserve_exact(fTopLevelUniforms.size());
    for (const VarDeclaration* topLevelUniform : fTopLevelUniforms) {
        const Variable* var = topLevelUniform->var();
        fTopLevelUniformMap.set(var, (int)fields.size());
        ModifierFlags flags = var->modifierFlags() & ~ModifierFlag::kUniform;
        fields.emplace_back(var->fPosition, var->layout(), flags, var->name(), &var->type());
    }
    fUniformBuffer.fStruct = Type::MakeStructType(fContext,
                                                  Position(),
                                                  kUniformBufferName,
                                                  std::move(fields),
                                                  /*interfaceBlock=*/true);

    // Create a global variable to contain this struct.
    Layout layout;
    layout.fBinding = fProgram.fConfig->fSettings.fDefaultUniformBinding;
    layout.fSet     = fProgram.fConfig->fSettings.fDefaultUniformSet;

    fUniformBuffer.fInnerVariable = Variable::Make(/*pos=*/Position(),
                                                   /*modifiersPosition=*/Position(),
                                                   layout,
                                                   ModifierFlag::kUniform,
                                                   fUniformBuffer.fStruct.get(),
                                                   kUniformBufferName,
                                                   /*mangledName=*/"",
                                                   /*builtin=*/false,
                                                   Variable::Storage::kGlobal);

    // Create an interface block object for this global variable.
    fUniformBuffer.fInterfaceBlock =
            std::make_unique<InterfaceBlock>(Position(),
                                             fUniformBuffer.fInnerVariable.get(),
                                             topLevelSymbolTable);

    // Generate an interface block and hold onto its ID.
    fUniformBufferId = this->writeInterfaceBlock(*fUniformBuffer.fInterfaceBlock);
}

void SPIRVCodeGenerator::addRTFlipUniform(Position pos) {
    SkASSERT(!fProgram.fConfig->fSettings.fForceNoRTFlip);

    if (fWroteRTFlip) {
        return;
    }
    // Flip variable hasn't been written yet. This means we don't have an existing
    // interface block, so we're free to just synthesize one.
    fWroteRTFlip = true;
    TArray<Field> fields;
    if (fProgram.fConfig->fSettings.fRTFlipOffset < 0) {
        fContext.fErrors->error(pos, "RTFlipOffset is negative");
    }
    fields.emplace_back(pos,
                        Layout(LayoutFlag::kNone,
                               /*location=*/-1,
                               fProgram.fConfig->fSettings.fRTFlipOffset,
                               /*binding=*/-1,
                               /*index=*/-1,
                               /*set=*/-1,
                               /*builtin=*/-1,
                               /*inputAttachmentIndex=*/-1),
                        ModifierFlag::kNone,
                        SKSL_RTFLIP_NAME,
                        fContext.fTypes.fFloat2.get());
    std::string_view name = "sksl_synthetic_uniforms";
    const Type* intfStruct = fSynthetics.takeOwnershipOfSymbol(Type::MakeStructType(
            fContext, Position(), name, std::move(fields), /*interfaceBlock=*/true));
    bool usePushConstants = fProgram.fConfig->fSettings.fUsePushConstants;
    int binding = -1, set = -1;
    if (!usePushConstants) {
        binding = fProgram.fConfig->fSettings.fRTFlipBinding;
        if (binding == -1) {
            fContext.fErrors->error(pos, "layout(binding=...) is required in SPIR-V");
        }
        set = fProgram.fConfig->fSettings.fRTFlipSet;
        if (set == -1) {
            fContext.fErrors->error(pos, "layout(set=...) is required in SPIR-V");
        }
    }
    Layout layout(/*flags=*/usePushConstants ? LayoutFlag::kPushConstant : LayoutFlag::kNone,
                  /*location=*/-1,
                  /*offset=*/-1,
                  binding,
                  /*index=*/-1,
                  set,
                  /*builtin=*/-1,
                  /*inputAttachmentIndex=*/-1);
    Variable* intfVar =
            fSynthetics.takeOwnershipOfSymbol(Variable::Make(/*pos=*/Position(),
                                                             /*modifiersPosition=*/Position(),
                                                             layout,
                                                             ModifierFlag::kUniform,
                                                             intfStruct,
                                                             name,
                                                             /*mangledName=*/"",
                                                             /*builtin=*/false,
                                                             Variable::Storage::kGlobal));
    fSPIRVBonusVariables.add(intfVar);
    {
        AutoAttachPoolToThread attach(fProgram.fPool.get());
        fProgram.fSymbols->add(std::make_unique<FieldSymbol>(Position(), intfVar, /*field=*/0));
    }
    InterfaceBlock intf(Position(), intfVar, std::make_shared<SymbolTable>(/*builtin=*/false));
    this->writeInterfaceBlock(intf, false);
}

std::tuple<const Variable*, const Variable*> SPIRVCodeGenerator::synthesizeTextureAndSampler(
        const Variable& combinedSampler) {
    SkASSERT(fUseTextureSamplerPairs);
    SkASSERT(combinedSampler.type().typeKind() == Type::TypeKind::kSampler);

    const Layout& layout = combinedSampler.layout();

    auto data = std::make_unique<SynthesizedTextureSamplerPair>();

    Layout texLayout = layout;
    texLayout.fBinding = layout.fTexture;
    data->fTextureName = std::string(combinedSampler.name()) + "_texture";

    auto texture = Variable::Make(/*pos=*/Position(),
                                  /*modifiersPosition=*/Position(),
                                  texLayout,
                                  combinedSampler.modifierFlags(),
                                  &combinedSampler.type().textureType(),
                                  data->fTextureName,
                                  /*mangledName=*/"",
                                  /*builtin=*/false,
                                  Variable::Storage::kGlobal);

    Layout samplerLayout = layout;
    samplerLayout.fBinding = layout.fSampler;
    data->fSamplerName = std::string(combinedSampler.name()) + "_sampler";

    auto sampler = Variable::Make(/*pos=*/Position(),
                                  /*modifiersPosition=*/Position(),
                                  samplerLayout,
                                  combinedSampler.modifierFlags(),
                                  fContext.fTypes.fSampler.get(),
                                  data->fSamplerName,
                                  /*mangledName=*/"",
                                  /*builtin=*/false,
                                  Variable::Storage::kGlobal);

    const Variable* t = texture.get();
    const Variable* s = sampler.get();
    data->fTexture = std::move(texture);
    data->fSampler = std::move(sampler);
    fSynthesizedSamplerMap.set(&combinedSampler, std::move(data));

    return {t, s};
}

void SPIRVCodeGenerator::writeInstructions(const Program& program, OutputStream& out) {
    fGLSLExtendedInstructions = this->nextId(nullptr);
    StringStream body;

    // Do an initial pass over the program elements to establish some baseline info.
    const FunctionDeclaration* main = nullptr;
    int localSizeX = 1, localSizeY = 1, localSizeZ = 1;
    Position combinedSamplerPos;
    Position separateSamplerPos;
    for (const ProgramElement* e : program.elements()) {
        switch (e->kind()) {
            case ProgramElement::Kind::kFunction: {
                // Assign SpvIds to functions.
                const FunctionDefinition& funcDef = e->as<FunctionDefinition>();
                const FunctionDeclaration& funcDecl = funcDef.declaration();
                fFunctionMap.set(&funcDecl, this->nextId(nullptr));
                if (funcDecl.isMain()) {
                    main = &funcDecl;
                }
                break;
            }
            case ProgramElement::Kind::kGlobalVar: {
                // Look for sampler variables and determine whether or not this program uses
                // combined samplers or separate samplers. The layout backend will be marked as
                // WebGPU for separate samplers, or Vulkan for combined samplers.
                const GlobalVarDeclaration& decl = e->as<GlobalVarDeclaration>();
                const Variable& var = *decl.varDeclaration().var();
                if (var.type().isSampler()) {
                    if (var.layout().fFlags & LayoutFlag::kVulkan) {
                        combinedSamplerPos = decl.position();
                    }
                    if (var.layout().fFlags & (LayoutFlag::kWebGPU | LayoutFlag::kDirect3D)) {
                        separateSamplerPos = decl.position();
                    }
                }
                break;
            }
            case ProgramElement::Kind::kModifiers: {
                // If this is a compute program, collect the local-size values. Dimensions that are
                // not present will be assigned a value of 1.
                if (ProgramConfig::IsCompute(program.fConfig->fKind)) {
                    const ModifiersDeclaration& modifiers = e->as<ModifiersDeclaration>();
                    if (modifiers.layout().fLocalSizeX >= 0) {
                        localSizeX = modifiers.layout().fLocalSizeX;
                    }
                    if (modifiers.layout().fLocalSizeY >= 0) {
                        localSizeY = modifiers.layout().fLocalSizeY;
                    }
                    if (modifiers.layout().fLocalSizeZ >= 0) {
                        localSizeZ = modifiers.layout().fLocalSizeZ;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    // Make sure we have a main() function.
    if (!main) {
        fContext.fErrors->error(Position(), "program does not contain a main() function");
        return;
    }
    // Make sure our program's sampler usage is consistent.
    if (combinedSamplerPos.valid() && separateSamplerPos.valid()) {
        fContext.fErrors->error(Position(), "programs cannot contain a mixture of sampler types");
        fContext.fErrors->error(combinedSamplerPos, "combined sampler found here:");
        fContext.fErrors->error(separateSamplerPos, "separate sampler found here:");
        return;
    }
    fUseTextureSamplerPairs = separateSamplerPos.valid();

    // Emit interface blocks.
    std::set<SpvId> interfaceVars;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<InterfaceBlock>()) {
            const InterfaceBlock& intf = e->as<InterfaceBlock>();
            SpvId id = this->writeInterfaceBlock(intf);

            if ((intf.var()->modifierFlags() & (ModifierFlag::kIn | ModifierFlag::kOut)) &&
                intf.var()->layout().fBuiltin == -1 &&
                !this->isDead(*intf.var())) {
                interfaceVars.insert(id);
            }
        }
    }
    // Emit global variable declarations.
    for (const ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            if (!this->writeGlobalVarDeclaration(program.fConfig->fKind,
                                                 e->as<GlobalVarDeclaration>().varDeclaration())) {
                return;
            }
        }
    }
    // Emit top-level uniforms into a dedicated uniform buffer.
    if (!fTopLevelUniforms.empty()) {
        this->writeUniformBuffer(get_top_level_symbol_table(*main));
    }
    // If main() returns a half4, synthesize a tiny entrypoint function which invokes the real
    // main() and stores the result into sk_FragColor.
    EntrypointAdapter adapter;
    if (main->returnType().matches(*fContext.fTypes.fHalf4)) {
        adapter = this->writeEntrypointAdapter(*main);
        if (adapter.entrypointDecl) {
            fFunctionMap.set(adapter.entrypointDecl.get(), this->nextId(nullptr));
            this->writeFunction(*adapter.entrypointDef, body);
            main = adapter.entrypointDecl.get();
        }
    }
    // Emit all the functions.
    for (const ProgramElement* e : program.elements()) {
        if (e->is<FunctionDefinition>()) {
            this->writeFunction(e->as<FunctionDefinition>(), body);
        }
    }
    // Add global in/out variables to the list of interface variables.
    for (const auto& [var, spvId] : fVariableMap) {
        if (var->storage() == Variable::Storage::kGlobal &&
            (var->modifierFlags() & (ModifierFlag::kIn | ModifierFlag::kOut)) &&
            !this->isDead(*var)) {
            interfaceVars.insert(spvId);
        }
    }
    this->writeCapabilities(out);
    this->writeInstruction(SpvOpExtInstImport, fGLSLExtendedInstructions, "GLSL.std.450", out);
    this->writeInstruction(SpvOpMemoryModel, SpvAddressingModelLogical, SpvMemoryModelGLSL450, out);
    this->writeOpCode(SpvOpEntryPoint, (SpvId) (3 + (main->name().length() + 4) / 4) +
                      (int32_t) interfaceVars.size(), out);
    if (ProgramConfig::IsVertex(program.fConfig->fKind)) {
        this->writeWord(SpvExecutionModelVertex, out);
    } else if (ProgramConfig::IsFragment(program.fConfig->fKind)) {
        this->writeWord(SpvExecutionModelFragment, out);
    } else if (ProgramConfig::IsCompute(program.fConfig->fKind)) {
        this->writeWord(SpvExecutionModelGLCompute, out);
    } else {
        SK_ABORT("cannot write this kind of program to SPIR-V\n");
    }
    SpvId entryPoint = fFunctionMap[main];
    this->writeWord(entryPoint, out);
    this->writeString(main->name(), out);
    for (int var : interfaceVars) {
        this->writeWord(var, out);
    }
    if (ProgramConfig::IsFragment(program.fConfig->fKind)) {
        this->writeInstruction(SpvOpExecutionMode,
                               fFunctionMap[main],
                               SpvExecutionModeOriginUpperLeft,
                               out);
    } else if (ProgramConfig::IsCompute(program.fConfig->fKind)) {
        this->writeInstruction(SpvOpExecutionMode,
                               fFunctionMap[main],
                               SpvExecutionModeLocalSize,
                               localSizeX, localSizeY, localSizeZ,
                               out);
    }
    for (const ProgramElement* e : program.elements()) {
        if (e->is<Extension>()) {
            this->writeInstruction(SpvOpSourceExtension, e->as<Extension>().name(), out);
        }
    }

    write_stringstream(fNameBuffer, out);
    write_stringstream(fDecorationBuffer, out);
    write_stringstream(fConstantBuffer, out);
    write_stringstream(body, out);
}

bool SPIRVCodeGenerator::generateCode() {
    SkASSERT(!fContext.fErrors->errorCount());
    this->writeWord(SpvMagicNumber, *fOut);
    this->writeWord(SpvVersion, *fOut);
    this->writeWord(SKSL_MAGIC, *fOut);
    StringStream buffer;
    this->writeInstructions(fProgram, buffer);
    this->writeWord(fIdCount, *fOut);
    this->writeWord(0, *fOut); // reserved, always zero
    write_stringstream(buffer, *fOut);
    return fContext.fErrors->errorCount() == 0;
}

}  // namespace SkSL
