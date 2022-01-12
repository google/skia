/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/codegen/SkSLSPIRVCodeGenerator.h"

#include "src/sksl/GLSL.std.450.h"

#include "include/sksl/DSLCore.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLConstructorArrayCast.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#endif

#define kLast_Capability SpvCapabilityMultiViewport

constexpr int DEVICE_FRAGCOORDS_BUILTIN = -1000;
constexpr int DEVICE_CLOCKWISE_BUILTIN  = -1001;

namespace SkSL {

// Skia's magic number is 31 and goes in the top 16 bits. We can use the lower bits to version the
// sksl generator if we want.
// https://github.com/KhronosGroup/SPIRV-Headers/blob/master/include/spirv/spir-v.xml#L84
static const int32_t SKSL_MAGIC  = 0x001F0000;

void SPIRVCodeGenerator::setupIntrinsics() {
#define ALL_GLSL(x) std::make_tuple(kGLSL_STD_450_IntrinsicOpcodeKind, GLSLstd450 ## x, \
                                    GLSLstd450 ## x, GLSLstd450 ## x, GLSLstd450 ## x)
#define BY_TYPE_GLSL(ifFloat, ifInt, ifUInt) std::make_tuple(kGLSL_STD_450_IntrinsicOpcodeKind, \
                                                             GLSLstd450 ## ifFloat,             \
                                                             GLSLstd450 ## ifInt,               \
                                                             GLSLstd450 ## ifUInt,              \
                                                             SpvOpUndef)
#define ALL_SPIRV(x) std::make_tuple(kSPIRV_IntrinsicOpcodeKind, \
                                     SpvOp ## x, SpvOp ## x, SpvOp ## x, SpvOp ## x)
#define SPECIAL(x) std::make_tuple(kSpecial_IntrinsicOpcodeKind, k ## x ## _SpecialIntrinsic, \
                                   k ## x ## _SpecialIntrinsic, k ## x ## _SpecialIntrinsic,  \
                                   k ## x ## _SpecialIntrinsic)
    fIntrinsicMap[k_round_IntrinsicKind]         = ALL_GLSL(Round);
    fIntrinsicMap[k_roundEven_IntrinsicKind]     = ALL_GLSL(RoundEven);
    fIntrinsicMap[k_trunc_IntrinsicKind]         = ALL_GLSL(Trunc);
    fIntrinsicMap[k_abs_IntrinsicKind]           = BY_TYPE_GLSL(FAbs, SAbs, SAbs);
    fIntrinsicMap[k_sign_IntrinsicKind]          = BY_TYPE_GLSL(FSign, SSign, SSign);
    fIntrinsicMap[k_floor_IntrinsicKind]         = ALL_GLSL(Floor);
    fIntrinsicMap[k_ceil_IntrinsicKind]          = ALL_GLSL(Ceil);
    fIntrinsicMap[k_fract_IntrinsicKind]         = ALL_GLSL(Fract);
    fIntrinsicMap[k_radians_IntrinsicKind]       = ALL_GLSL(Radians);
    fIntrinsicMap[k_degrees_IntrinsicKind]       = ALL_GLSL(Degrees);
    fIntrinsicMap[k_sin_IntrinsicKind]           = ALL_GLSL(Sin);
    fIntrinsicMap[k_cos_IntrinsicKind]           = ALL_GLSL(Cos);
    fIntrinsicMap[k_tan_IntrinsicKind]           = ALL_GLSL(Tan);
    fIntrinsicMap[k_asin_IntrinsicKind]          = ALL_GLSL(Asin);
    fIntrinsicMap[k_acos_IntrinsicKind]          = ALL_GLSL(Acos);
    fIntrinsicMap[k_atan_IntrinsicKind]          = SPECIAL(Atan);
    fIntrinsicMap[k_sinh_IntrinsicKind]          = ALL_GLSL(Sinh);
    fIntrinsicMap[k_cosh_IntrinsicKind]          = ALL_GLSL(Cosh);
    fIntrinsicMap[k_tanh_IntrinsicKind]          = ALL_GLSL(Tanh);
    fIntrinsicMap[k_asinh_IntrinsicKind]         = ALL_GLSL(Asinh);
    fIntrinsicMap[k_acosh_IntrinsicKind]         = ALL_GLSL(Acosh);
    fIntrinsicMap[k_atanh_IntrinsicKind]         = ALL_GLSL(Atanh);
    fIntrinsicMap[k_pow_IntrinsicKind]           = ALL_GLSL(Pow);
    fIntrinsicMap[k_exp_IntrinsicKind]           = ALL_GLSL(Exp);
    fIntrinsicMap[k_log_IntrinsicKind]           = ALL_GLSL(Log);
    fIntrinsicMap[k_exp2_IntrinsicKind]          = ALL_GLSL(Exp2);
    fIntrinsicMap[k_log2_IntrinsicKind]          = ALL_GLSL(Log2);
    fIntrinsicMap[k_sqrt_IntrinsicKind]          = ALL_GLSL(Sqrt);
    fIntrinsicMap[k_inverse_IntrinsicKind]       = ALL_GLSL(MatrixInverse);
    fIntrinsicMap[k_outerProduct_IntrinsicKind]  = ALL_SPIRV(OuterProduct);
    fIntrinsicMap[k_transpose_IntrinsicKind]     = ALL_SPIRV(Transpose);
    fIntrinsicMap[k_isinf_IntrinsicKind]         = ALL_SPIRV(IsInf);
    fIntrinsicMap[k_isnan_IntrinsicKind]         = ALL_SPIRV(IsNan);
    fIntrinsicMap[k_inversesqrt_IntrinsicKind]   = ALL_GLSL(InverseSqrt);
    fIntrinsicMap[k_determinant_IntrinsicKind]   = ALL_GLSL(Determinant);
    fIntrinsicMap[k_matrixCompMult_IntrinsicKind] = SPECIAL(MatrixCompMult);
    fIntrinsicMap[k_matrixInverse_IntrinsicKind] = ALL_GLSL(MatrixInverse);
    fIntrinsicMap[k_mod_IntrinsicKind]           = SPECIAL(Mod);
    fIntrinsicMap[k_modf_IntrinsicKind]          = ALL_GLSL(Modf);
    fIntrinsicMap[k_min_IntrinsicKind]           = SPECIAL(Min);
    fIntrinsicMap[k_max_IntrinsicKind]           = SPECIAL(Max);
    fIntrinsicMap[k_clamp_IntrinsicKind]         = SPECIAL(Clamp);
    fIntrinsicMap[k_saturate_IntrinsicKind]      = SPECIAL(Saturate);
    fIntrinsicMap[k_dot_IntrinsicKind]           = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                      SpvOpDot, SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[k_mix_IntrinsicKind]           = SPECIAL(Mix);
    fIntrinsicMap[k_step_IntrinsicKind]          = SPECIAL(Step);
    fIntrinsicMap[k_smoothstep_IntrinsicKind]    = SPECIAL(SmoothStep);
    fIntrinsicMap[k_fma_IntrinsicKind]           = ALL_GLSL(Fma);
    fIntrinsicMap[k_frexp_IntrinsicKind]         = ALL_GLSL(Frexp);
    fIntrinsicMap[k_ldexp_IntrinsicKind]         = ALL_GLSL(Ldexp);

#define PACK(type) fIntrinsicMap[k_pack##type##_IntrinsicKind] = ALL_GLSL(Pack##type); \
                   fIntrinsicMap[k_unpack##type##_IntrinsicKind] = ALL_GLSL(Unpack##type)
    PACK(Snorm4x8);
    PACK(Unorm4x8);
    PACK(Snorm2x16);
    PACK(Unorm2x16);
    PACK(Half2x16);
    PACK(Double2x32);
#undef PACK
    fIntrinsicMap[k_length_IntrinsicKind]      = ALL_GLSL(Length);
    fIntrinsicMap[k_distance_IntrinsicKind]    = ALL_GLSL(Distance);
    fIntrinsicMap[k_cross_IntrinsicKind]       = ALL_GLSL(Cross);
    fIntrinsicMap[k_normalize_IntrinsicKind]   = ALL_GLSL(Normalize);
    fIntrinsicMap[k_faceforward_IntrinsicKind] = ALL_GLSL(FaceForward);
    fIntrinsicMap[k_reflect_IntrinsicKind]     = ALL_GLSL(Reflect);
    fIntrinsicMap[k_refract_IntrinsicKind]     = ALL_GLSL(Refract);
    fIntrinsicMap[k_bitCount_IntrinsicKind]    = ALL_SPIRV(BitCount);
    fIntrinsicMap[k_findLSB_IntrinsicKind]     = ALL_GLSL(FindILsb);
    fIntrinsicMap[k_findMSB_IntrinsicKind]     = BY_TYPE_GLSL(FindSMsb, FindSMsb, FindUMsb);
    fIntrinsicMap[k_dFdx_IntrinsicKind]        = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                 SpvOpDPdx, SpvOpUndef,
                                                                 SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[k_dFdy_IntrinsicKind]        = SPECIAL(DFdy);
    fIntrinsicMap[k_fwidth_IntrinsicKind]      = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                 SpvOpFwidth, SpvOpUndef,
                                                                 SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[k_makeSampler2D_IntrinsicKind] = SPECIAL(SampledImage);

    fIntrinsicMap[k_sample_IntrinsicKind]      = SPECIAL(Texture);
    fIntrinsicMap[k_subpassLoad_IntrinsicKind] = SPECIAL(SubpassLoad);

    fIntrinsicMap[k_floatBitsToInt_IntrinsicKind]  = ALL_SPIRV(Bitcast);
    fIntrinsicMap[k_floatBitsToUint_IntrinsicKind] = ALL_SPIRV(Bitcast);
    fIntrinsicMap[k_intBitsToFloat_IntrinsicKind]  = ALL_SPIRV(Bitcast);
    fIntrinsicMap[k_uintBitsToFloat_IntrinsicKind] = ALL_SPIRV(Bitcast);

    fIntrinsicMap[k_any_IntrinsicKind]        = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                SpvOpUndef, SpvOpUndef,
                                                                SpvOpUndef, SpvOpAny);
    fIntrinsicMap[k_all_IntrinsicKind]        = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                SpvOpUndef, SpvOpUndef,
                                                                SpvOpUndef, SpvOpAll);
    fIntrinsicMap[k_not_IntrinsicKind]        = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                SpvOpUndef, SpvOpUndef, SpvOpUndef,
                                                                SpvOpLogicalNot);
    fIntrinsicMap[k_equal_IntrinsicKind]      = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                SpvOpFOrdEqual, SpvOpIEqual,
                                                                SpvOpIEqual, SpvOpLogicalEqual);
    fIntrinsicMap[k_notEqual_IntrinsicKind]   = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                SpvOpFOrdNotEqual, SpvOpINotEqual,
                                                                SpvOpINotEqual,
                                                                SpvOpLogicalNotEqual);
    fIntrinsicMap[k_lessThan_IntrinsicKind]         = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                      SpvOpFOrdLessThan,
                                                                      SpvOpSLessThan,
                                                                      SpvOpULessThan,
                                                                      SpvOpUndef);
    fIntrinsicMap[k_lessThanEqual_IntrinsicKind]    = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                      SpvOpFOrdLessThanEqual,
                                                                      SpvOpSLessThanEqual,
                                                                      SpvOpULessThanEqual,
                                                                      SpvOpUndef);
    fIntrinsicMap[k_greaterThan_IntrinsicKind]      = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                      SpvOpFOrdGreaterThan,
                                                                      SpvOpSGreaterThan,
                                                                      SpvOpUGreaterThan,
                                                                      SpvOpUndef);
    fIntrinsicMap[k_greaterThanEqual_IntrinsicKind] = std::make_tuple(kSPIRV_IntrinsicOpcodeKind,
                                                                      SpvOpFOrdGreaterThanEqual,
                                                                      SpvOpSGreaterThanEqual,
                                                                      SpvOpUGreaterThanEqual,
                                                                      SpvOpUndef);
// interpolateAt* not yet supported...
}

void SPIRVCodeGenerator::writeWord(int32_t word, OutputStream& out) {
    out.write((const char*) &word, sizeof(word));
}

static bool is_float(const Context& context, const Type& type) {
    return (type.isScalar() || type.isVector() || type.isMatrix()) &&
           type.componentType().isFloat();
}

static bool is_signed(const Context& context, const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isSigned();
}

static bool is_unsigned(const Context& context, const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isUnsigned();
}

static bool is_bool(const Context& context, const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isBoolean();
}

static bool is_out(const Modifiers& m) {
    return (m.fFlags & Modifiers::kOut_Flag) != 0;
}

static bool is_in(const Modifiers& m) {
    switch (m.fFlags & (Modifiers::kOut_Flag | Modifiers::kIn_Flag)) {
        case Modifiers::kOut_Flag:                       // out
            return false;

        case 0:                                          // implicit in
        case Modifiers::kIn_Flag:                        // explicit in
        case Modifiers::kOut_Flag | Modifiers::kIn_Flag: // inout
            return true;

        default: SkUNREACHABLE;
    }
}

void SPIRVCodeGenerator::writeOpCode(SpvOp_ opCode, int length, OutputStream& out) {
    SkASSERT(opCode != SpvOpLoad || &out != &fConstantBuffer);
    SkASSERT(opCode != SpvOpUndef);
    switch (opCode) {
        case SpvOpReturn:      // fall through
        case SpvOpReturnValue: // fall through
        case SpvOpKill:        // fall through
        case SpvOpSwitch:      // fall through
        case SpvOpBranch:      // fall through
        case SpvOpBranchConditional:
            if (fCurrentBlock == 0) {
                // We just encountered dead code--instructions that don't have an associated block.
                // Synthesize a label if this happens; this is necessary to satisfy the validator.
                this->writeLabel(this->nextId(nullptr), out);
            }
            fCurrentBlock = 0;
            break;
        case SpvOpConstant:          // fall through
        case SpvOpConstantTrue:      // fall through
        case SpvOpConstantFalse:     // fall through
        case SpvOpConstantComposite: // fall through
        case SpvOpTypeVoid:          // fall through
        case SpvOpTypeInt:           // fall through
        case SpvOpTypeFloat:         // fall through
        case SpvOpTypeBool:          // fall through
        case SpvOpTypeVector:        // fall through
        case SpvOpTypeMatrix:        // fall through
        case SpvOpTypeArray:         // fall through
        case SpvOpTypePointer:       // fall through
        case SpvOpTypeFunction:      // fall through
        case SpvOpTypeRuntimeArray:  // fall through
        case SpvOpTypeStruct:        // fall through
        case SpvOpTypeImage:         // fall through
        case SpvOpTypeSampledImage:  // fall through
        case SpvOpTypeSampler:       // fall through
        case SpvOpVariable:          // fall through
        case SpvOpFunction:          // fall through
        case SpvOpFunctionParameter: // fall through
        case SpvOpFunctionEnd:       // fall through
        case SpvOpExecutionMode:     // fall through
        case SpvOpMemoryModel:       // fall through
        case SpvOpCapability:        // fall through
        case SpvOpExtInstImport:     // fall through
        case SpvOpEntryPoint:        // fall through
        case SpvOpSource:            // fall through
        case SpvOpSourceExtension:   // fall through
        case SpvOpName:              // fall through
        case SpvOpMemberName:        // fall through
        case SpvOpDecorate:          // fall through
        case SpvOpMemberDecorate:
            break;
        default:
            // We may find ourselves with dead code--instructions that don't have an associated
            // block. This should be a rare event, but if it happens, synthesize a label; this is
            // necessary to satisfy the validator.
            if (fCurrentBlock == 0) {
                this->writeLabel(this->nextId(nullptr), out);
            }
            break;
    }
    this->writeWord((length << 16) | opCode, out);
}

void SPIRVCodeGenerator::writeLabel(SpvId label, OutputStream& out) {
    SkASSERT(!fCurrentBlock);
    fCurrentBlock = label;
    this->writeInstruction(SpvOpLabel, label, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, OutputStream& out) {
    this->writeOpCode(opCode, 1, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, OutputStream& out) {
    this->writeOpCode(opCode, 2, out);
    this->writeWord(word1, out);
}

void SPIRVCodeGenerator::writeString(skstd::string_view s, OutputStream& out) {
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

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, skstd::string_view string,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 1 + (string.length() + 4) / 4, out);
    this->writeString(string, out);
}


void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, skstd::string_view string,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 2 + (string.length() + 4) / 4, out);
    this->writeWord(word1, out);
    this->writeString(string, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          skstd::string_view string, OutputStream& out) {
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

void SPIRVCodeGenerator::writeStruct(const Type& type, const MemoryLayout& memoryLayout,
                                     SpvId resultId) {
    this->writeInstruction(SpvOpName, resultId, String(type.name()).c_str(), fNameBuffer);
    // go ahead and write all of the field types, so we don't inadvertently write them while we're
    // in the middle of writing the struct instruction
    std::vector<SpvId> types;
    for (const auto& f : type.fields()) {
        types.push_back(this->getType(*f.fType, memoryLayout));
    }
    this->writeOpCode(SpvOpTypeStruct, 2 + (int32_t) types.size(), fConstantBuffer);
    this->writeWord(resultId, fConstantBuffer);
    for (SpvId id : types) {
        this->writeWord(id, fConstantBuffer);
    }
    size_t offset = 0;
    for (int32_t i = 0; i < (int32_t) type.fields().size(); i++) {
        const Type::Field& field = type.fields()[i];
        if (!MemoryLayout::LayoutIsSupported(*field.fType)) {
            fContext.fErrors->error(type.fLine, "type '" + field.fType->name() +
                                    "' is not permitted here");
            return;
        }
        size_t size = memoryLayout.size(*field.fType);
        size_t alignment = memoryLayout.alignment(*field.fType);
        const Layout& fieldLayout = field.fModifiers.fLayout;
        if (fieldLayout.fOffset >= 0) {
            if (fieldLayout.fOffset < (int) offset) {
                fContext.fErrors->error(type.fLine,
                                        "offset of field '" + field.fName + "' must be at "
                                        "least " + to_string((int) offset));
            }
            if (fieldLayout.fOffset % alignment) {
                fContext.fErrors->error(type.fLine,
                                        "offset of field '" + field.fName + "' must be a multiple"
                                        " of " + to_string((int) alignment));
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
        if (field.fModifiers.fLayout.fBuiltin < 0) {
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
}

const Type& SPIRVCodeGenerator::getActualType(const Type& type) {
    if (type.isFloat()) {
        return *fContext.fTypes.fFloat;
    }
    if (type.isSigned()) {
        return *fContext.fTypes.fInt;
    }
    if (type.isUnsigned()) {
        return *fContext.fTypes.fUInt;
    }
    if (type.isMatrix() || type.isVector()) {
        if (type.componentType().matches(*fContext.fTypes.fHalf)) {
            return fContext.fTypes.fFloat->toCompound(fContext, type.columns(), type.rows());
        }
        if (type.componentType().matches(*fContext.fTypes.fShort)) {
            return fContext.fTypes.fInt->toCompound(fContext, type.columns(), type.rows());
        }
        if (type.componentType().matches(*fContext.fTypes.fUShort)) {
            return fContext.fTypes.fUInt->toCompound(fContext, type.columns(), type.rows());
        }
    }
    return type;
}

SpvId SPIRVCodeGenerator::getType(const Type& type) {
    return this->getType(type, fDefaultLayout);
}

SpvId SPIRVCodeGenerator::getType(const Type& rawType, const MemoryLayout& layout) {
    const Type* type;
    std::unique_ptr<Type> arrayType;
    String arrayName;

    if (rawType.isArray()) {
        // For arrays, we need to synthesize a temporary Array type using the "actual" component
        // type. That is, if `short[10]` is passed in, we need to synthesize a `int[10]` Type.
        // Otherwise, we can end up with two different SpvIds for the same array type.
        const Type& component = this->getActualType(rawType.componentType());
        arrayName = component.getArrayName(rawType.columns());
        arrayType = Type::MakeArrayType(arrayName, component, rawType.columns());
        type = arrayType.get();
    } else {
        // For non-array types, we can simply look up the "actual" type and use it.
        type = &this->getActualType(rawType);
    }

    String key(type->name());
    if (type->isStruct() || type->isArray()) {
        key += to_string((int)layout.fStd);
#ifdef SK_DEBUG
        SkASSERT(layout.fStd == MemoryLayout::Standard::k140_Standard ||
                 layout.fStd == MemoryLayout::Standard::k430_Standard);
        MemoryLayout::Standard otherStd = layout.fStd == MemoryLayout::Standard::k140_Standard
                                                  ? MemoryLayout::Standard::k430_Standard
                                                  : MemoryLayout::Standard::k140_Standard;
        String otherKey = type->name() + to_string((int)otherStd);
        SkASSERT(fTypeMap.find(otherKey) == fTypeMap.end());
#endif
    }
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId(nullptr);
        switch (type->typeKind()) {
            case Type::TypeKind::kScalar:
                if (type->isBoolean()) {
                    this->writeInstruction(SpvOpTypeBool, result, fConstantBuffer);
                } else if (type->isSigned()) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 1, fConstantBuffer);
                } else if (type->isUnsigned()) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 0, fConstantBuffer);
                } else if (type->isFloat()) {
                    this->writeInstruction(SpvOpTypeFloat, result, 32, fConstantBuffer);
                } else {
                    SkDEBUGFAILF("unrecognized scalar type '%s'", type->description().c_str());
                }
                break;
            case Type::TypeKind::kVector:
                this->writeInstruction(SpvOpTypeVector, result,
                                       this->getType(type->componentType(), layout),
                                       type->columns(), fConstantBuffer);
                break;
            case Type::TypeKind::kMatrix:
                this->writeInstruction(
                        SpvOpTypeMatrix,
                        result,
                        this->getType(IndexExpression::IndexType(fContext, *type), layout),
                        type->columns(),
                        fConstantBuffer);
                break;
            case Type::TypeKind::kStruct:
                this->writeStruct(*type, layout, result);
                break;
            case Type::TypeKind::kArray: {
                if (!MemoryLayout::LayoutIsSupported(*type)) {
                    fContext.fErrors->error(type->fLine,
                                            "type '" + type->name() + "' is not permitted here");
                    return this->nextId(nullptr);
                }
                if (type->columns() > 0) {
                    SpvId typeId = this->getType(type->componentType(), layout);
                    SpvId countId = this->writeLiteral(type->columns(), *fContext.fTypes.fInt);
                    this->writeInstruction(SpvOpTypeArray, result, typeId, countId,
                                           fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(*type),
                                           fDecorationBuffer);
                } else {
                    // We shouldn't have any runtime-sized arrays right now
                    fContext.fErrors->error(type->fLine,
                                            "runtime-sized arrays are not supported in SPIR-V");
                    this->writeInstruction(SpvOpTypeRuntimeArray, result,
                                           this->getType(type->componentType(), layout),
                                           fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(*type),
                                           fDecorationBuffer);
                }
                break;
            }
            case Type::TypeKind::kSampler: {
                SpvId image = result;
                if (SpvDimSubpassData != type->dimensions()) {
                    image = this->getType(type->textureType(), layout);
                }
                if (SpvDimBuffer == type->dimensions()) {
                    fCapabilities |= (((uint64_t) 1) << SpvCapabilitySampledBuffer);
                }
                if (SpvDimSubpassData != type->dimensions()) {
                    this->writeInstruction(SpvOpTypeSampledImage, result, image, fConstantBuffer);
                }
                break;
            }
            case Type::TypeKind::kSeparateSampler: {
                this->writeInstruction(SpvOpTypeSampler, result, fConstantBuffer);
                break;
            }
            case Type::TypeKind::kTexture: {
                this->writeInstruction(SpvOpTypeImage, result,
                                       this->getType(*fContext.fTypes.fFloat, layout),
                                       type->dimensions(), type->isDepth(),
                                       type->isArrayedTexture(), type->isMultisampled(),
                                       type->isSampled() ? 1 : 2, SpvImageFormatUnknown,
                                       fConstantBuffer);
                fImageTypeMap[key] = result;
                break;
            }
            default:
                if (type->isVoid()) {
                    this->writeInstruction(SpvOpTypeVoid, result, fConstantBuffer);
                } else {
                    SkDEBUGFAILF("invalid type: %s", type->description().c_str());
                }
                break;
        }
        fTypeMap[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::getImageType(const Type& type) {
    SkASSERT(type.typeKind() == Type::TypeKind::kSampler);
    this->getType(type);
    String key = type.name() + to_string((int) fDefaultLayout.fStd);
    SkASSERT(fImageTypeMap.find(key) != fImageTypeMap.end());
    return fImageTypeMap[key];
}

SpvId SPIRVCodeGenerator::getFunctionType(const FunctionDeclaration& function) {
    String key = to_string(this->getType(function.returnType())) + "(";
    String separator;
    const std::vector<const Variable*>& parameters = function.parameters();
    for (size_t i = 0; i < parameters.size(); i++) {
        key += separator;
        separator = ", ";
        key += to_string(this->getType(parameters[i]->type()));
    }
    key += ")";
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId(nullptr);
        int32_t length = 3 + (int32_t) parameters.size();
        SpvId returnType = this->getType(function.returnType());
        std::vector<SpvId> parameterTypes;
        for (size_t i = 0; i < parameters.size(); i++) {
            // glslang seems to treat all function arguments as pointers whether they need to be or
            // not. I  was initially puzzled by this until I ran bizarre failures with certain
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
            // as glslang does, fixes it. It's entirely possible I simply missed whichever part of
            // the spec makes this make sense.
            parameterTypes.push_back(this->getPointerType(parameters[i]->type(),
                                                          SpvStorageClassFunction));
        }
        this->writeOpCode(SpvOpTypeFunction, length, fConstantBuffer);
        this->writeWord(result, fConstantBuffer);
        this->writeWord(returnType, fConstantBuffer);
        for (SpvId id : parameterTypes) {
            this->writeWord(id, fConstantBuffer);
        }
        fTypeMap[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::getPointerType(const Type& type, SpvStorageClass_ storageClass) {
    return this->getPointerType(type, fDefaultLayout, storageClass);
}

SpvId SPIRVCodeGenerator::getPointerType(const Type& rawType, const MemoryLayout& layout,
                                         SpvStorageClass_ storageClass) {
    const Type& type = this->getActualType(rawType);
    String key = type.displayName() + "*" + to_string(layout.fStd) + to_string(storageClass);
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId(nullptr);
        this->writeInstruction(SpvOpTypePointer, result, storageClass,
                               this->getType(type), fConstantBuffer);
        fTypeMap[key] = result;
        return result;
    }
    return entry->second;
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
        default:
            SkDEBUGFAILF("unsupported expression: %s", expr.description().c_str());
            break;
    }
    return -1;
}

SpvId SPIRVCodeGenerator::writeIntrinsicCall(const FunctionCall& c, OutputStream& out) {
    const FunctionDeclaration& function = c.function();
    auto intrinsic = fIntrinsicMap.find(function.intrinsicKind());
    if (intrinsic == fIntrinsicMap.end()) {
        fContext.fErrors->error(c.fLine, "unsupported intrinsic '" + function.description() + "'");
        return -1;
    }
    int32_t intrinsicId;
    const ExpressionArray& arguments = c.arguments();
    if (arguments.size() > 0) {
        const Type& type = arguments[0]->type();
        if (std::get<0>(intrinsic->second) == kSpecial_IntrinsicOpcodeKind ||
            is_float(fContext, type)) {
            intrinsicId = std::get<1>(intrinsic->second);
        } else if (is_signed(fContext, type)) {
            intrinsicId = std::get<2>(intrinsic->second);
        } else if (is_unsigned(fContext, type)) {
            intrinsicId = std::get<3>(intrinsic->second);
        } else if (is_bool(fContext, type)) {
            intrinsicId = std::get<4>(intrinsic->second);
        } else {
            intrinsicId = std::get<1>(intrinsic->second);
        }
    } else {
        intrinsicId = std::get<1>(intrinsic->second);
    }
    switch (std::get<0>(intrinsic->second)) {
        case kGLSL_STD_450_IntrinsicOpcodeKind: {
            SpvId result = this->nextId(&c.type());
            std::vector<SpvId> argumentIds;
            std::vector<TempVar> tempVars;
            argumentIds.reserve(arguments.size());
            for (size_t i = 0; i < arguments.size(); i++) {
                if (is_out(function.parameters()[i]->modifiers())) {
                    argumentIds.push_back(
                            this->writeFunctionCallArgument(*arguments[i],
                                                            function.parameters()[i]->modifiers(),
                                                            &tempVars,
                                                            out));
                } else {
                    argumentIds.push_back(this->writeExpression(*arguments[i], out));
                }
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
            std::vector<SpvId> argumentIds;
            std::vector<TempVar> tempVars;
            argumentIds.reserve(arguments.size());
            for (size_t i = 0; i < arguments.size(); i++) {
                if (is_out(function.parameters()[i]->modifiers())) {
                    argumentIds.push_back(
                            this->writeFunctionCallArgument(*arguments[i],
                                                            function.parameters()[i]->modifiers(),
                                                            &tempVars,
                                                            out));
                } else {
                    argumentIds.push_back(this->writeExpression(*arguments[i], out));
                }
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
            fContext.fErrors->error(c.fLine, "unsupported intrinsic '" + function.description() +
                                             "'");
            return -1;
    }
}

SpvId SPIRVCodeGenerator::vectorize(const Expression& arg, int vectorSize, OutputStream& out) {
    SkASSERT(vectorSize >= 1 && vectorSize <= 4);
    const Type& argType = arg.type();
    SpvId raw = this->writeExpression(arg, out);
    if (argType.isScalar()) {
        if (vectorSize == 1) {
            return raw;
        }
        SpvId vector = this->nextId(&argType);
        this->writeOpCode(SpvOpCompositeConstruct, 3 + vectorSize, out);
        this->writeWord(this->getType(argType.toCompound(fContext, vectorSize, 1)), out);
        this->writeWord(vector, out);
        for (int i = 0; i < vectorSize; i++) {
            this->writeWord(raw, out);
        }
        return vector;
    } else {
        SkASSERT(vectorSize == argType.columns());
        return raw;
    }
}

std::vector<SpvId> SPIRVCodeGenerator::vectorize(const ExpressionArray& args, OutputStream& out) {
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
    std::vector<SpvId> result;
    result.reserve(args.size());
    for (const auto& arg : args) {
        result.push_back(this->vectorize(*arg, vectorSize, out));
    }
    return result;
}

void SPIRVCodeGenerator::writeGLSLExtendedInstruction(const Type& type, SpvId id, SpvId floatInst,
                                                      SpvId signedInst, SpvId unsignedInst,
                                                      const std::vector<SpvId>& args,
                                                      OutputStream& out) {
    this->writeOpCode(SpvOpExtInst, 5 + args.size(), out);
    this->writeWord(this->getType(type), out);
    this->writeWord(id, out);
    this->writeWord(fGLSLExtendedInstructions, out);

    if (is_float(fContext, type)) {
        this->writeWord(floatInst, out);
    } else if (is_signed(fContext, type)) {
        this->writeWord(signedInst, out);
    } else if (is_unsigned(fContext, type)) {
        this->writeWord(unsignedInst, out);
    } else {
        SkASSERT(false);
    }
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
            std::vector<SpvId> argumentIds;
            argumentIds.reserve(arguments.size());
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
            args.reserve_back(2);
            args.push_back(Literal::MakeInt(fContext, /*line=*/-1, /*value=*/0));
            args.push_back(Literal::MakeInt(fContext, /*line=*/-1, /*value=*/0));
            ConstructorCompound ctor(/*line=*/-1, *fContext.fTypes.fInt2, std::move(args));
            SpvId coords = this->writeConstantVector(ctor);
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
                    SpvId lodBias = this->writeLiteral(-0.5, *fContext.fTypes.fFloat);
                    this->writeInstruction(op, type, result, sampler, uv,
                                           SpvImageOperandsBiasMask, lodBias, out);
                } else {
                    this->writeInstruction(op, type, result, sampler, uv,
                                           out);
                }
            }
            break;
        }
        case kMod_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            const Type& operandType = arguments[0]->type();
            SpvOp_ op;
            if (is_float(fContext, operandType)) {
                op = SpvOpFMod;
            } else if (is_signed(fContext, operandType)) {
                op = SpvOpSMod;
            } else if (is_unsigned(fContext, operandType)) {
                op = SpvOpUMod;
            } else {
                SkASSERT(false);
                return 0;
            }
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
            this->addRTFlipUniform(c.fLine);
            using namespace dsl;
            DSLExpression rtFlip(ThreadContext::Compiler().convertIdentifier(/*line=*/-1,
                    SKSL_RTFLIP_NAME));
            SpvId rtFlipY = this->vectorize(*rtFlip.y().release(), callType.columns(), out);
            SpvId flipped = this->nextId(&callType);
            this->writeInstruction(SpvOpFMul, this->getType(callType), flipped, result, rtFlipY,
                                   out);
            result = flipped;
            break;
        }
        case kClamp_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 3);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FClamp, GLSLstd450SClamp,
                                               GLSLstd450UClamp, args, out);
            break;
        }
        case kMax_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FMax, GLSLstd450SMax,
                                               GLSLstd450UMax, args, out);
            break;
        }
        case kMin_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FMin, GLSLstd450SMin,
                                               GLSLstd450UMin, args, out);
            break;
        }
        case kMix_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
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
            finalArgs.reserve_back(3);
            finalArgs.push_back(arguments[0]->clone());
            finalArgs.push_back(Literal::MakeFloat(fContext, /*line=*/-1, /*value=*/0));
            finalArgs.push_back(Literal::MakeFloat(fContext, /*line=*/-1, /*value=*/1));
            std::vector<SpvId> spvArgs = this->vectorize(finalArgs, out);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FClamp, GLSLstd450SClamp,
                                               GLSLstd450UClamp, spvArgs, out);
            break;
        }
        case kSmoothStep_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
            SkASSERT(args.size() == 3);
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450SmoothStep, SpvOpUndef,
                                               SpvOpUndef, args, out);
            break;
        }
        case kStep_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(arguments, out);
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
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFunctionCallArgument(const Expression& arg,
                                                    const Modifiers& paramModifiers,
                                                    std::vector<TempVar>* tempVars,
                                                    OutputStream& out) {
    // ID of temporary variable that we will use to hold this argument, or 0 if it is being
    // passed directly
    SpvId tmpVar;
    // if we need a temporary var to store this argument, this is the value to store in the var
    SpvId tmpValueId = -1;

    if (is_out(paramModifiers)) {
        std::unique_ptr<LValue> lv = this->getLValue(arg, out);
        SpvId ptr = lv->getPointer();
        if (ptr != (SpvId) -1 && lv->isMemoryObjectPointer()) {
            return ptr;
        }

        // lvalue cannot simply be read and written via a pointer (e.g. it's a swizzle). We need to
        // to use a temp variable.
        if (is_in(paramModifiers)) {
            tmpValueId = lv->load(out);
        }
        tmpVar = this->nextId(&arg.type());
        tempVars->push_back(TempVar{tmpVar, &arg.type(), std::move(lv)});
    } else {
        // See getFunctionType for an explanation of why we're always using pointer parameters.
        tmpValueId = this->writeExpression(arg, out);
        tmpVar = this->nextId(nullptr);
    }
    this->writeInstruction(SpvOpVariable,
                           this->getPointerType(arg.type(), SpvStorageClassFunction),
                           tmpVar,
                           SpvStorageClassFunction,
                           fVariableBuffer);
    if (tmpValueId != (SpvId)-1) {
        this->writeInstruction(SpvOpStore, tmpVar, tmpValueId, out);
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
    const auto& entry = fFunctionMap.find(&function);
    if (entry == fFunctionMap.end()) {
        fContext.fErrors->error(c.fLine, "function '" + function.description() +
                                         "' is not defined");
        return -1;
    }
    // Temp variables are used to write back out-parameters after the function call is complete.
    std::vector<TempVar> tempVars;
    std::vector<SpvId> argumentIds;
    argumentIds.reserve(arguments.size());
    for (size_t i = 0; i < arguments.size(); i++) {
        argumentIds.push_back(this->writeFunctionCallArgument(*arguments[i],
                                                              function.parameters()[i]->modifiers(),
                                                              &tempVars,
                                                              out));
    }
    SpvId result = this->nextId(nullptr);
    this->writeOpCode(SpvOpFunctionCall, 4 + (int32_t) arguments.size(), out);
    this->writeWord(this->getType(c.type()), out);
    this->writeWord(result, out);
    this->writeWord(entry->second, out);
    for (SpvId id : argumentIds) {
        this->writeWord(id, out);
    }
    // Now that the call is complete, we copy temp out-variables back to their real lvalues.
    this->copyBackTempVars(tempVars, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeConstantVector(const AnyConstructor& c) {
    const Type& type = c.type();
    SkASSERT(type.isVector() && c.isCompileTimeConstant());

    // Get each of the constructor components as SPIR-V constants.
    SPIRVVectorConstant key{this->getType(type),
                            /*fValueId=*/{SpvId(-1), SpvId(-1), SpvId(-1), SpvId(-1)}};

    const Type& scalarType = type.componentType();
    for (int n = 0; n < type.columns(); n++) {
        skstd::optional<double> slotVal = c.getConstantValue(n);
        if (!slotVal.has_value()) {
            SkDEBUGFAILF("writeConstantVector: %s not actually constant", c.description().c_str());
            return (SpvId)-1;
        }
        key.fValueId[n] = this->writeLiteral(*slotVal, scalarType);
    }

    // Check to see if we've already synthesized this vector constant.
    auto [iter, newlyCreated] = fVectorConstants.insert({key, (SpvId)-1});
    if (newlyCreated) {
        // Emit an OpConstantComposite instruction for this constant.
        SpvId result = this->nextId(&type);
        this->writeOpCode(SpvOpConstantComposite, 3 + type.columns(), fConstantBuffer);
        this->writeWord(key.fTypeId, fConstantBuffer);
        this->writeWord(result, fConstantBuffer);
        for (int i = 0; i < type.columns(); i++) {
            this->writeWord(key.fValueId[i], fConstantBuffer);
        }
        iter->second = result;
    }
    return iter->second;
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

    fContext.fErrors->error(-1, "unsupported cast: " + inputType.description() +
                                " to " + outputType.description());
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
        return (SpvId)-1;
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
        return (SpvId)-1;
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
        return (SpvId)-1;
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
        return (SpvId)-1;
    }
    return result;
}

void SPIRVCodeGenerator::writeUniformScaleMatrix(SpvId id, SpvId diagonal, const Type& type,
                                                 OutputStream& out) {
    SpvId zeroId = this->writeLiteral(0.0, *fContext.fTypes.fFloat);
    std::vector<SpvId> columnIds;
    columnIds.reserve(type.columns());
    for (int column = 0; column < type.columns(); column++) {
        this->writeOpCode(SpvOpCompositeConstruct, 3 + type.rows(),
                          out);
        this->writeWord(this->getType(type.componentType().toCompound(
                                fContext, /*columns=*/type.rows(), /*rows=*/1)),
                        out);
        SpvId columnId = this->nextId(&type);
        this->writeWord(columnId, out);
        columnIds.push_back(columnId);
        for (int row = 0; row < type.rows(); row++) {
            this->writeWord(row == column ? diagonal : zeroId, out);
        }
    }
    this->writeOpCode(SpvOpCompositeConstruct, 3 + type.columns(),
                      out);
    this->writeWord(this->getType(type), out);
    this->writeWord(id, out);
    for (SpvId columnId : columnIds) {
        this->writeWord(columnId, out);
    }
}

SpvId SPIRVCodeGenerator::writeMatrixCopy(SpvId src, const Type& srcType, const Type& dstType,
                                          OutputStream& out) {
    SkASSERT(srcType.isMatrix());
    SkASSERT(dstType.isMatrix());
    SkASSERT(srcType.componentType().matches(dstType.componentType()));
    SpvId id = this->nextId(&dstType);
    SpvId srcColumnType = this->getType(srcType.componentType().toCompound(fContext,
                                                                           srcType.rows(),
                                                                           1));
    SpvId dstColumnType = this->getType(dstType.componentType().toCompound(fContext,
                                                                           dstType.rows(),
                                                                           1));
    SkASSERT(dstType.componentType().isFloat());
    const SpvId zeroId = this->writeLiteral(0.0, dstType.componentType());
    const SpvId oneId = this->writeLiteral(1.0, dstType.componentType());

    SpvId columns[4];
    for (int i = 0; i < dstType.columns(); i++) {
        if (i < srcType.columns()) {
            // we're still inside the src matrix, copy the column
            SpvId srcColumn = this->nextId(&dstType);
            this->writeInstruction(SpvOpCompositeExtract, srcColumnType, srcColumn, src, i, out);
            SpvId dstColumn;
            if (srcType.rows() == dstType.rows()) {
                // columns are equal size, don't need to do anything
                dstColumn = srcColumn;
            }
            else if (dstType.rows() > srcType.rows()) {
                // dst column is bigger, need to zero-pad it
                dstColumn = this->nextId(&dstType);
                int delta = dstType.rows() - srcType.rows();
                this->writeOpCode(SpvOpCompositeConstruct, 4 + delta, out);
                this->writeWord(dstColumnType, out);
                this->writeWord(dstColumn, out);
                this->writeWord(srcColumn, out);
                for (int j = srcType.rows(); j < dstType.rows(); ++j) {
                    this->writeWord((i == j) ? oneId : zeroId, out);
                }
            }
            else {
                // dst column is smaller, need to swizzle the src column
                dstColumn = this->nextId(&dstType);
                this->writeOpCode(SpvOpVectorShuffle, 5 + dstType.rows(), out);
                this->writeWord(dstColumnType, out);
                this->writeWord(dstColumn, out);
                this->writeWord(srcColumn, out);
                this->writeWord(srcColumn, out);
                for (int j = 0; j < dstType.rows(); j++) {
                    this->writeWord(j, out);
                }
            }
            columns[i] = dstColumn;
        } else {
            // we're past the end of the src matrix, need to synthesize an identity-matrix column
            SpvId identityColumn = this->nextId(&dstType);
            this->writeOpCode(SpvOpCompositeConstruct, 3 + dstType.rows(), out);
            this->writeWord(dstColumnType, out);
            this->writeWord(identityColumn, out);
            for (int j = 0; j < dstType.rows(); ++j) {
                this->writeWord((i == j) ? oneId : zeroId, out);
            }
            columns[i] = identityColumn;
        }
    }
    this->writeOpCode(SpvOpCompositeConstruct, 3 + dstType.columns(), out);
    this->writeWord(this->getType(dstType), out);
    this->writeWord(id, out);
    for (int i = 0; i < dstType.columns(); i++) {
        this->writeWord(columns[i], out);
    }
    return id;
}

void SPIRVCodeGenerator::addColumnEntry(const Type& columnType,
                                        std::vector<SpvId>* currentColumn,
                                        std::vector<SpvId>* columnIds,
                                        int rows,
                                        SpvId entry,
                                        OutputStream& out) {
    SkASSERT((int)currentColumn->size() < rows);
    currentColumn->push_back(entry);
    if ((int)currentColumn->size() == rows) {
        // Synthesize this column into a vector.
        SpvId columnId = this->writeComposite(*currentColumn, columnType, out);
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
    std::vector<SpvId> arguments;
    arguments.reserve(c.arguments().size());
    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
        arguments.push_back(this->writeExpression(*arg, out));
    }

    if (arguments.size() == 1 && arg0Type.isVector()) {
        // Special-case handling of float4 -> mat2x2.
        SkASSERT(type.rows() == 2 && type.columns() == 2);
        SkASSERT(arg0Type.columns() == 4);
        SpvId componentType = this->getType(type.componentType());
        SpvId v[4];
        for (int i = 0; i < 4; ++i) {
            v[i] = this->nextId(&type);
            this->writeInstruction(SpvOpCompositeExtract, componentType, v[i], arguments[0], i,
                                   out);
        }
        const Type& vecType = type.componentType().toCompound(fContext, /*columns=*/2, /*rows=*/1);
        SpvId v0v1 = this->writeComposite({v[0], v[1]}, vecType, out);
        SpvId v2v3 = this->writeComposite({v[2], v[3]}, vecType, out);
        return this->writeComposite({v0v1, v2v3}, type, out);
    }

    int rows = type.rows();
    const Type& columnType = type.componentType().toCompound(fContext,
                                                             /*columns=*/rows, /*rows=*/1);
    // SpvIds of completed columns of the matrix.
    std::vector<SpvId> columnIds;
    // SpvIds of scalars we have written to the current column so far.
    std::vector<SpvId> currentColumn;
    for (size_t i = 0; i < arguments.size(); i++) {
        const Type& argType = c.arguments()[i]->type();
        if (currentColumn.empty() && argType.isVector() && argType.columns() == rows) {
            // This vector is a complete matrix column by itself and can be used as-is.
            columnIds.push_back(arguments[i]);
        } else if (argType.columns() == 1) {
            // This argument is a lone scalar and can be added to the current column as-is.
            this->addColumnEntry(columnType, &currentColumn, &columnIds, rows, arguments[i], out);
        } else {
            // This argument needs to be decomposed into its constituent scalars.
            SpvId componentType = this->getType(argType.componentType());
            for (int j = 0; j < argType.columns(); ++j) {
                SpvId swizzle = this->nextId(&argType);
                this->writeInstruction(SpvOpCompositeExtract, componentType, swizzle,
                                       arguments[i], j, out);
                this->addColumnEntry(columnType, &currentColumn, &columnIds, rows, swizzle, out);
            }
        }
    }
    SkASSERT(columnIds.size() == (size_t) type.columns());
    return this->writeComposite(columnIds, type, out);
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

    if (c.isCompileTimeConstant()) {
        return this->writeConstantVector(c);
    }

    std::vector<SpvId> arguments;
    arguments.reserve(c.arguments().size());
    for (size_t i = 0; i < c.arguments().size(); i++) {
        const Type& argType = c.arguments()[i]->type();
        SkASSERT(componentType.matches(argType.componentType()));

        SpvId arg = this->writeExpression(*c.arguments()[i], out);
        if (argType.isMatrix()) {
            // CompositeConstruct cannot take a 2x2 matrix as an input, so we need to extract out
            // each scalar separately.
            SkASSERT(argType.rows() == 2);
            SkASSERT(argType.columns() == 2);
            for (int j = 0; j < 4; ++j) {
                SpvId componentId = this->nextId(&componentType);
                this->writeInstruction(SpvOpCompositeExtract, this->getType(componentType),
                                       componentId, arg, j / 2, j % 2, out);
                arguments.push_back(componentId);
            }
        } else if (argType.isVector()) {
            // There's a bug in the Intel Vulkan driver where OpCompositeConstruct doesn't handle
            // vector arguments at all, so we always extract each vector component and pass them
            // into OpCompositeConstruct individually.
            for (int j = 0; j < argType.columns(); j++) {
                SpvId componentId = this->nextId(&componentType);
                this->writeInstruction(SpvOpCompositeExtract, this->getType(componentType),
                                       componentId, arg, j, out);
                arguments.push_back(componentId);
            }
        } else {
            arguments.push_back(arg);
        }
    }

    return this->writeComposite(arguments, type, out);
}

SpvId SPIRVCodeGenerator::writeComposite(const std::vector<SpvId>& arguments,
                                         const Type& type,
                                         OutputStream& out) {
    SkASSERT(arguments.size() == (type.isStruct() ? type.fields().size() : (size_t)type.columns()));

    SpvId result = this->nextId(&type);
    this->writeOpCode(SpvOpCompositeConstruct, 3 + (int32_t) arguments.size(), out);
    this->writeWord(this->getType(type), out);
    this->writeWord(result, out);
    for (SpvId id : arguments) {
        this->writeWord(id, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstructorSplat(const ConstructorSplat& c, OutputStream& out) {
    // Use writeConstantVector to deduplicate constant splats.
    if (c.isCompileTimeConstant()) {
        return this->writeConstantVector(c);
    }

    // Write the splat argument.
    SpvId argument = this->writeExpression(*c.argument(), out);

    // Generate a OpCompositeConstruct which repeats the argument N times.
    std::vector<SpvId> arguments(/*count*/ c.type().columns(), /*value*/ argument);
    return this->writeComposite(arguments, c.type(), out);
}


SpvId SPIRVCodeGenerator::writeCompositeConstructor(const AnyConstructor& c, OutputStream& out) {
    SkASSERT(c.type().isArray() || c.type().isStruct());
    auto ctorArgs = c.argumentSpan();

    std::vector<SpvId> arguments;
    arguments.reserve(ctorArgs.size());
    for (const std::unique_ptr<Expression>& arg : ctorArgs) {
        arguments.push_back(this->writeExpression(*arg, out));
    }

    return this->writeComposite(arguments, c.type(), out);
}

SpvId SPIRVCodeGenerator::writeConstructorScalarCast(const ConstructorScalarCast& c,
                                                     OutputStream& out) {
    const Type& type = c.type();
    if (this->getActualType(type).matches(this->getActualType(c.argument()->type()))) {
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
    if (this->getActualType(ctorType).matches(this->getActualType(argType))) {
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

    std::vector<SpvId> arguments;
    arguments.reserve(argType.columns());
    for (int index = 0; index < argType.columns(); ++index) {
        SpvId componentId = this->nextId(&srcType);
        this->writeInstruction(SpvOpCompositeExtract, this->getType(srcType), componentId,
                               compositeId, index, out);
        arguments.push_back(this->castScalarToType(componentId, srcType, dstType, out));
    }

    return this->writeComposite(arguments, ctorType, out);
}

SpvId SPIRVCodeGenerator::writeConstructorDiagonalMatrix(const ConstructorDiagonalMatrix& c,
                                                         OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(c.argument()->type().isScalar());

    // Write out the scalar argument.
    SpvId argument = this->writeExpression(*c.argument(), out);

    // Build the diagonal matrix.
    SpvId result = this->nextId(&type);
    this->writeUniformScaleMatrix(result, argument, type, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeConstructorMatrixResize(const ConstructorMatrixResize& c,
                                                       OutputStream& out) {
    // Write the input matrix.
    SpvId argument = this->writeExpression(*c.argument(), out);

    // Use matrix-copy to resize the input matrix to its new size.
    return this->writeMatrixCopy(argument, c.argument()->type(), c.type(), out);
}

static SpvStorageClass_ get_storage_class(const Variable& var,
                                          SpvStorageClass_ fallbackStorageClass) {
    const Modifiers& modifiers = var.modifiers();
    if (modifiers.fFlags & Modifiers::kIn_Flag) {
        SkASSERT(!(modifiers.fLayout.fFlags & Layout::kPushConstant_Flag));
        return SpvStorageClassInput;
    }
    if (modifiers.fFlags & Modifiers::kOut_Flag) {
        SkASSERT(!(modifiers.fLayout.fFlags & Layout::kPushConstant_Flag));
        return SpvStorageClassOutput;
    }
    if (modifiers.fFlags & Modifiers::kUniform_Flag) {
        if (modifiers.fLayout.fFlags & Layout::kPushConstant_Flag) {
            return SpvStorageClassPushConstant;
        }
        if (var.type().typeKind() == Type::TypeKind::kSampler ||
            var.type().typeKind() == Type::TypeKind::kSeparateSampler ||
            var.type().typeKind() == Type::TypeKind::kTexture) {
            return SpvStorageClassUniformConstant;
        }
        return SpvStorageClassUniform;
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
            return get_storage_class(var, SpvStorageClassPrivate);
        }
        case Expression::Kind::kFieldAccess:
            return get_storage_class(*expr.as<FieldAccess>().base());
        case Expression::Kind::kIndex:
            return get_storage_class(*expr.as<IndexExpression>().base());
        default:
            return SpvStorageClassFunction;
    }
}

std::vector<SpvId> SPIRVCodeGenerator::getAccessChain(const Expression& expr, OutputStream& out) {
    std::vector<SpvId> chain;
    switch (expr.kind()) {
        case Expression::Kind::kIndex: {
            const IndexExpression& indexExpr = expr.as<IndexExpression>();
            chain = this->getAccessChain(*indexExpr.base(), out);
            chain.push_back(this->writeExpression(*indexExpr.index(), out));
            break;
        }
        case Expression::Kind::kFieldAccess: {
            const FieldAccess& fieldExpr = expr.as<FieldAccess>();
            chain = this->getAccessChain(*fieldExpr.base(), out);
            chain.push_back(this->writeLiteral(fieldExpr.fieldIndex(), *fContext.fTypes.fInt));
            break;
        }
        default: {
            SpvId id = this->getLValue(expr, out)->getPointer();
            SkASSERT(id != (SpvId) -1);
            chain.push_back(id);
            break;
        }
    }
    return chain;
}

class PointerLValue : public SPIRVCodeGenerator::LValue {
public:
    PointerLValue(SPIRVCodeGenerator& gen, SpvId pointer, bool isMemoryObject, SpvId type,
                  SPIRVCodeGenerator::Precision precision)
    : fGen(gen)
    , fPointer(pointer)
    , fIsMemoryObject(isMemoryObject)
    , fType(type)
    , fPrecision(precision) {}

    SpvId getPointer() override {
        return fPointer;
    }

    bool isMemoryObjectPointer() const override {
        return fIsMemoryObject;
    }

    SpvId load(OutputStream& out) override {
        SpvId result = fGen.nextId(fPrecision);
        fGen.writeInstruction(SpvOpLoad, fType, result, fPointer, out);
        return result;
    }

    void store(SpvId value, OutputStream& out) override {
        fGen.writeInstruction(SpvOpStore, fPointer, value, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fPointer;
    const bool fIsMemoryObject;
    const SpvId fType;
    const SPIRVCodeGenerator::Precision fPrecision;
};

class SwizzleLValue : public SPIRVCodeGenerator::LValue {
public:
    SwizzleLValue(SPIRVCodeGenerator& gen, SpvId vecPointer, const ComponentArray& components,
                  const Type& baseType, const Type& swizzleType)
    : fGen(gen)
    , fVecPointer(vecPointer)
    , fComponents(components)
    , fBaseType(&baseType)
    , fSwizzleType(&swizzleType) {}

    bool applySwizzle(const ComponentArray& components, const Type& newType) override {
        ComponentArray updatedSwizzle;
        for (int8_t component : components) {
            if (component < 0 || component >= fComponents.count()) {
                SkDEBUGFAILF("swizzle accessed nonexistent component %d", (int)component);
                return false;
            }
            updatedSwizzle.push_back(fComponents[component]);
        }
        fComponents = updatedSwizzle;
        fSwizzleType = &newType;
        return true;
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
            for (size_t j = 0; j < fComponents.size(); j++) {
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
        fGen.writeInstruction(SpvOpStore, fVecPointer, shuffle, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fVecPointer;
    ComponentArray fComponents;
    const Type* fBaseType;
    const Type* fSwizzleType;
};

int SPIRVCodeGenerator::findUniformFieldIndex(const Variable& var) const {
    auto iter = fTopLevelUniformMap.find(&var);
    return (iter != fTopLevelUniformMap.end()) ? iter->second : -1;
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
                return std::make_unique<PointerLValue>(*this, memberId,
                                                       /*isMemoryObjectPointer=*/true,
                                                       this->getType(type), precision);
            }
            SpvId typeId = this->getType(type, this->memoryLayoutForVariable(var));
            auto entry = fVariableMap.find(&var);
            SkASSERTF(entry != fVariableMap.end(), "%s", expr.description().c_str());
            return std::make_unique<PointerLValue>(*this, entry->second,
                                                   /*isMemoryObjectPointer=*/true,
                                                   typeId, precision);
        }
        case Expression::Kind::kIndex: // fall through
        case Expression::Kind::kFieldAccess: {
            std::vector<SpvId> chain = this->getAccessChain(expr, out);
            SpvId member = this->nextId(nullptr);
            this->writeOpCode(SpvOpAccessChain, (SpvId) (3 + chain.size()), out);
            this->writeWord(this->getPointerType(type, get_storage_class(expr)), out);
            this->writeWord(member, out);
            for (SpvId idx : chain) {
                this->writeWord(idx, out);
            }
            return std::make_unique<PointerLValue>(*this, member, /*isMemoryObjectPointer=*/false,
                                                   this->getType(type), precision);
        }
        case Expression::Kind::kSwizzle: {
            const Swizzle& swizzle = expr.as<Swizzle>();
            std::unique_ptr<LValue> lvalue = this->getLValue(*swizzle.base(), out);
            if (lvalue->applySwizzle(swizzle.components(), type)) {
                return lvalue;
            }
            SpvId base = lvalue->getPointer();
            if (base == (SpvId) -1) {
                fContext.fErrors->error(swizzle.fLine, "unable to retrieve lvalue from swizzle");
            }
            if (swizzle.components().size() == 1) {
                SpvId member = this->nextId(nullptr);
                SpvId typeId = this->getPointerType(type, get_storage_class(*swizzle.base()));
                SpvId indexId = this->writeLiteral(swizzle.components()[0], *fContext.fTypes.fInt);
                this->writeInstruction(SpvOpAccessChain, typeId, member, base, indexId, out);
                return std::make_unique<PointerLValue>(*this,
                                                       member,
                                                       /*isMemoryObjectPointer=*/false,
                                                       this->getType(type),
                                                       precision);
            } else {
                return std::make_unique<SwizzleLValue>(*this, base, swizzle.components(),
                                                       swizzle.base()->type(), type);
            }
        }
        default: {
            // expr isn't actually an lvalue, create a placeholder variable for it. This case
            // happens due to the need to store values in temporary variables during function
            // calls (see comments in getFunctionType); erroneous uses of rvalues as lvalues
            // should have been caught before code generation
            SpvId result = this->nextId(nullptr);
            SpvId pointerType = this->getPointerType(type, SpvStorageClassFunction);
            this->writeInstruction(SpvOpVariable, pointerType, result, SpvStorageClassFunction,
                                   fVariableBuffer);
            this->writeInstruction(SpvOpStore, result, this->writeExpression(expr, out), out);
            return std::make_unique<PointerLValue>(*this, result, /*isMemoryObjectPointer=*/true,
                                                   this->getType(type), precision);
        }
    }
}

SpvId SPIRVCodeGenerator::writeVariableReference(const VariableReference& ref, OutputStream& out) {
    const Variable* variable = ref.variable();
    if (variable->modifiers().fLayout.fBuiltin == DEVICE_FRAGCOORDS_BUILTIN) {
        // Down below, we rewrite raw references to sk_FragCoord with expressions that reference
        // DEVICE_FRAGCOORDS_BUILTIN. This is a fake variable that means we need to directly access
        // the fragcoord; do so now.
        dsl::DSLGlobalVar fragCoord("sk_FragCoord");
        return this->getLValue(*dsl::DSLExpression(fragCoord).release(), out)->load(out);
    }
    if (variable->modifiers().fLayout.fBuiltin == DEVICE_CLOCKWISE_BUILTIN) {
        // Down below, we rewrite raw references to sk_Clockwise with expressions that reference
        // DEVICE_CLOCKWISE_BUILTIN. This is a fake variable that means we need to directly
        // access front facing; do so now.
        dsl::DSLGlobalVar clockwise("sk_Clockwise");
        return this->getLValue(*dsl::DSLExpression(clockwise).release(), out)->load(out);
    }

    // Handle inserting use of uniform to flip y when referencing sk_FragCoord.
    if (variable->modifiers().fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
        this->addRTFlipUniform(ref.fLine);
        // Use sk_RTAdjust to compute the flipped coordinate
        using namespace dsl;
        const char* DEVICE_COORDS_NAME = "__device_FragCoords";
        SymbolTable& symbols = *ThreadContext::SymbolTable();
        // Use a uniform to flip the Y coordinate. The new expression will be written in
        // terms of __device_FragCoords, which is a fake variable that means "access the
        // underlying fragcoords directly without flipping it".
        DSLExpression rtFlip(ThreadContext::Compiler().convertIdentifier(/*line=*/-1,
                SKSL_RTFLIP_NAME));
        if (!symbols[DEVICE_COORDS_NAME]) {
            AutoAttachPoolToThread attach(fProgram.fPool.get());
            Modifiers modifiers;
            modifiers.fLayout.fBuiltin = DEVICE_FRAGCOORDS_BUILTIN;
            auto coordsVar = std::make_unique<Variable>(/*line=*/-1,
                                                        fContext.fModifiersPool->add(modifiers),
                                                        DEVICE_COORDS_NAME,
                                                        fContext.fTypes.fFloat4.get(),
                                                        true,
                                                        Variable::Storage::kGlobal);
            fSPIRVBonusVariables.insert(coordsVar.get());
            symbols.add(std::move(coordsVar));
        }
        DSLGlobalVar deviceCoord(DEVICE_COORDS_NAME);
        std::unique_ptr<Expression> rtFlipSkSLExpr = rtFlip.release();
        DSLExpression x = DSLExpression(rtFlipSkSLExpr->clone()).x();
        DSLExpression y = DSLExpression(std::move(rtFlipSkSLExpr)).y();
        return this->writeExpression(*dsl::Float4(deviceCoord.x(),
                                                  std::move(x) + std::move(y) * deviceCoord.y(),
                                                  deviceCoord.z(),
                                                  deviceCoord.w()).release(),
                                     out);
    }

    // Handle flipping sk_Clockwise.
    if (variable->modifiers().fLayout.fBuiltin == SK_CLOCKWISE_BUILTIN) {
        this->addRTFlipUniform(ref.fLine);
        using namespace dsl;
        const char* DEVICE_CLOCKWISE_NAME = "__device_Clockwise";
        SymbolTable& symbols = *ThreadContext::SymbolTable();
        // Use a uniform to flip the Y coordinate. The new expression will be written in
        // terms of __device_Clockwise, which is a fake variable that means "access the
        // underlying FrontFacing directly".
        DSLExpression rtFlip(ThreadContext::Compiler().convertIdentifier(/*line=*/-1,
                SKSL_RTFLIP_NAME));
        if (!symbols[DEVICE_CLOCKWISE_NAME]) {
            AutoAttachPoolToThread attach(fProgram.fPool.get());
            Modifiers modifiers;
            modifiers.fLayout.fBuiltin = DEVICE_CLOCKWISE_BUILTIN;
            auto clockwiseVar = std::make_unique<Variable>(/*line=*/-1,
                                                           fContext.fModifiersPool->add(modifiers),
                                                           DEVICE_CLOCKWISE_NAME,
                                                           fContext.fTypes.fBool.get(),
                                                           true,
                                                           Variable::Storage::kGlobal);
            fSPIRVBonusVariables.insert(clockwiseVar.get());
            symbols.add(std::move(clockwiseVar));
        }
        DSLGlobalVar deviceClockwise(DEVICE_CLOCKWISE_NAME);
        // FrontFacing in Vulkan is defined in terms of a top-down render target. In skia,
        // we use the default convention of "counter-clockwise face is front".
        return this->writeExpression(*dsl::Bool(Select(rtFlip.y() > 0,
                                                       !deviceClockwise,
                                                       deviceClockwise)).release(),
                                     out);
    }

    return this->getLValue(ref, out)->load(out);
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

SpvId SPIRVCodeGenerator::writeSwizzle(const Swizzle& swizzle, OutputStream& out) {
    SpvId base = this->writeExpression(*swizzle.base(), out);
    SpvId result = this->nextId(&swizzle.type());
    size_t count = swizzle.components().size();
    if (count == 1) {
        this->writeInstruction(SpvOpCompositeExtract, this->getType(swizzle.type()), result, base,
                               swizzle.components()[0], out);
    } else {
        this->writeOpCode(SpvOpVectorShuffle, 5 + (int32_t) count, out);
        this->writeWord(this->getType(swizzle.type()), out);
        this->writeWord(result, out);
        this->writeWord(base, out);
        this->writeWord(base, out);
        for (int component : swizzle.components()) {
            this->writeWord(component, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeBinaryOperation(const Type& resultType,
                                               const Type& operandType, SpvId lhs,
                                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt,
                                               SpvOp_ ifUInt, SpvOp_ ifBool, OutputStream& out) {
    SpvId result = this->nextId(&resultType);
    if (is_float(fContext, operandType)) {
        this->writeInstruction(ifFloat, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_signed(fContext, operandType)) {
        this->writeInstruction(ifInt, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_unsigned(fContext, operandType)) {
        this->writeInstruction(ifUInt, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_bool(fContext, operandType)) {
        this->writeInstruction(ifBool, this->getType(resultType), result, lhs, rhs, out);
    } else {
        fContext.fErrors->error(operandType.fLine,
                "unsupported operand for binary expression: " + operandType.description());
    }
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
    SpvOp_ compareOp = is_float(fContext, operandType) ? floatOperator : intOperator;
    SkASSERT(operandType.isMatrix());
    SpvId columnType = this->getType(operandType.componentType().toCompound(fContext,
                                                                            operandType.rows(),
                                                                            1));
    SpvId bvecType = this->getType(fContext.fTypes.fBool->toCompound(fContext,
                                                                    operandType.rows(),
                                                                    1));
    SpvId boolType = this->getType(*fContext.fTypes.fBool);
    SpvId result = 0;
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId columnL = this->nextId(&operandType);
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnL, lhs, i, out);
        SpvId columnR = this->nextId(&operandType);
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnR, rhs, i, out);
        SpvId compare = this->nextId(&operandType);
        this->writeInstruction(compareOp, bvecType, compare, columnL, columnR, out);
        SpvId merge = this->nextId(nullptr);
        this->writeInstruction(vectorMergeOperator, boolType, merge, compare, out);
        if (result != 0) {
            SpvId next = this->nextId(nullptr);
            this->writeInstruction(mergeOperator, boolType, next, result, merge, out);
            result = next;
        }
        else {
            result = merge;
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeComponentwiseMatrixBinary(const Type& operandType, SpvId lhs,
                                                         SpvId rhs, SpvOp_ op, OutputStream& out) {
    SkASSERT(operandType.isMatrix());
    SpvId columnType = this->getType(operandType.componentType().toCompound(fContext,
                                                                            operandType.rows(),
                                                                            1));
    std::vector<SpvId> columns;
    columns.reserve(operandType.columns());
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId columnL = this->nextId(&operandType);
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnL, lhs, i, out);
        SpvId columnR = this->nextId(&operandType);
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnR, rhs, i, out);
        columns.push_back(this->nextId(&operandType));
        this->writeInstruction(op, columnType, columns[i], columnL, columnR, out);
    }
    return this->writeComposite(columns, operandType, out);
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
    std::vector<SpvId> vecArguments(/*count*/ matrixType.rows(), /*value*/ scalarId);
    SpvId vectorId = this->writeComposite(vecArguments, vectorType, out);

    // Splat the vector into a matrix.
    std::vector<SpvId> matArguments(/*count*/ matrixType.columns(), /*value*/ vectorId);
    return this->writeComposite(matArguments, matrixType, out);
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const Type& leftType, SpvId lhs, Operator op,
                                                const Type& rightType, SpvId rhs,
                                                const Type& resultType, OutputStream& out) {
    // The comma operator ignores the type of the left-hand side entirely.
    if (op.kind() == Token::Kind::TK_COMMA) {
        return rhs;
    }
    // overall type we are operating on: float2, int, uint4...
    const Type* operandType;
    // IR allows mismatched types in expressions (e.g. float2 * float), but they need special
    // handling in SPIR-V
    if (!this->getActualType(leftType).matches(this->getActualType(rightType))) {
        if (leftType.isVector() && rightType.isNumber()) {
            if (resultType.componentType().isFloat()) {
                switch (op.kind()) {
                    case Token::Kind::TK_SLASH: {
                        rhs = this->writeReciprocal(rightType, rhs, out);
                        [[fallthrough]];
                    }
                    case Token::Kind::TK_STAR: {
                        SpvId result = this->nextId(&resultType);
                        this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                               result, lhs, rhs, out);
                        return result;
                    }
                    default:
                        break;
                }
            }
            // promote number to vector
            const Type& vecType = leftType;
            SpvId vec = this->nextId(&vecType);
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vecType.columns(), out);
            this->writeWord(this->getType(vecType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < vecType.columns(); i++) {
                this->writeWord(rhs, out);
            }
            rhs = vec;
            operandType = &leftType;
        } else if (rightType.isVector() && leftType.isNumber()) {
            if (resultType.componentType().isFloat()) {
                if (op.kind() == Token::Kind::TK_STAR) {
                    SpvId result = this->nextId(&resultType);
                    this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                           result, rhs, lhs, out);
                    return result;
                }
            }
            // promote number to vector
            const Type& vecType = rightType;
            SpvId vec = this->nextId(&vecType);
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vecType.columns(), out);
            this->writeWord(this->getType(vecType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < vecType.columns(); i++) {
                this->writeWord(lhs, out);
            }
            lhs = vec;
            operandType = &rightType;
        } else if (leftType.isMatrix()) {
            if (op.kind() == Token::Kind::TK_STAR) {
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
            if (op.kind() == Token::Kind::TK_STAR) {
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
            fContext.fErrors->error(leftType.fLine, "unsupported mixed-type expression");
            return -1;
        }
    } else {
        operandType = &this->getActualType(leftType);
        SkASSERT(operandType->matches(this->getActualType(rightType)));
    }
    switch (op.kind()) {
        case Token::Kind::TK_EQEQ: {
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
            return this->foldToBool(this->writeBinaryOperation(*tmpType, *operandType, lhs, rhs,
                                                               SpvOpFOrdEqual, SpvOpIEqual,
                                                               SpvOpIEqual, SpvOpLogicalEqual, out),
                                    *operandType, SpvOpAll, out);
        }
        case Token::Kind::TK_NEQ:
            if (operandType->isMatrix()) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFOrdNotEqual,
                                                   SpvOpINotEqual, SpvOpAny, SpvOpLogicalOr, out);
            }
            if (operandType->isStruct()) {
                return this->writeStructComparison(*operandType, lhs, op, rhs, out);
            }
            if (operandType->isArray()) {
                return this->writeArrayComparison(*operandType, lhs, op, rhs, out);
            }
            [[fallthrough]];
        case Token::Kind::TK_LOGICALXOR:
            SkASSERT(resultType.isBoolean());
            const Type* tmpType;
            if (operandType->isVector()) {
                tmpType = &fContext.fTypes.fBool->toCompound(fContext,
                                                             operandType->columns(),
                                                             operandType->rows());
            } else {
                tmpType = &resultType;
            }
            return this->foldToBool(this->writeBinaryOperation(*tmpType, *operandType, lhs, rhs,
                                                               SpvOpFOrdNotEqual, SpvOpINotEqual,
                                                               SpvOpINotEqual, SpvOpLogicalNotEqual,
                                                               out),
                                    *operandType, SpvOpAny, out);
        case Token::Kind::TK_GT:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThan, SpvOpSGreaterThan,
                                              SpvOpUGreaterThan, SpvOpUndef, out);
        case Token::Kind::TK_LT:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFOrdLessThan,
                                              SpvOpSLessThan, SpvOpULessThan, SpvOpUndef, out);
        case Token::Kind::TK_GTEQ:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThanEqual, SpvOpSGreaterThanEqual,
                                              SpvOpUGreaterThanEqual, SpvOpUndef, out);
        case Token::Kind::TK_LTEQ:
            SkASSERT(resultType.isBoolean());
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdLessThanEqual, SpvOpSLessThanEqual,
                                              SpvOpULessThanEqual, SpvOpUndef, out);
        case Token::Kind::TK_PLUS:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType.matches(rightType));
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs, SpvOpFAdd, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                              SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
        case Token::Kind::TK_MINUS:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType.matches(rightType));
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs, SpvOpFSub, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFSub,
                                              SpvOpISub, SpvOpISub, SpvOpUndef, out);
        case Token::Kind::TK_STAR:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                // matrix multiply
                SpvId result = this->nextId(&resultType);
                this->writeInstruction(SpvOpMatrixTimesMatrix, this->getType(resultType), result,
                                       lhs, rhs, out);
                return result;
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMul,
                                              SpvOpIMul, SpvOpIMul, SpvOpUndef, out);
        case Token::Kind::TK_SLASH:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType.matches(rightType));
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs, SpvOpFDiv, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFDiv,
                                              SpvOpSDiv, SpvOpUDiv, SpvOpUndef, out);
        case Token::Kind::TK_PERCENT:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMod,
                                              SpvOpSMod, SpvOpUMod, SpvOpUndef, out);
        case Token::Kind::TK_SHL:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpShiftLeftLogical, SpvOpShiftLeftLogical,
                                              SpvOpUndef, out);
        case Token::Kind::TK_SHR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpShiftRightArithmetic, SpvOpShiftRightLogical,
                                              SpvOpUndef, out);
        case Token::Kind::TK_BITWISEAND:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseAnd, SpvOpBitwiseAnd, SpvOpUndef, out);
        case Token::Kind::TK_BITWISEOR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseOr, SpvOpBitwiseOr, SpvOpUndef, out);
        case Token::Kind::TK_BITWISEXOR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseXor, SpvOpBitwiseXor, SpvOpUndef, out);
        default:
            fContext.fErrors->error(0, "unsupported token");
            return -1;
    }
}

SpvId SPIRVCodeGenerator::writeArrayComparison(const Type& arrayType, SpvId lhs, Operator op,
                                               SpvId rhs, OutputStream& out) {
    // The inputs must be arrays, and the op must be == or !=.
    SkASSERT(op.kind() == Token::Kind::TK_EQEQ || op.kind() == Token::Kind::TK_NEQ);
    SkASSERT(arrayType.isArray());
    const Type& componentType = arrayType.componentType();
    const SpvId componentTypeId = this->getType(componentType);
    const int arraySize = arrayType.columns();
    SkASSERT(arraySize > 0);

    // Synthesize equality checks for each item in the array.
    const Type& boolType = *fContext.fTypes.fBool;
    SpvId allComparisons = (SpvId)-1;
    for (int index = 0; index < arraySize; ++index) {
        // Get the left and right item in the array.
        SpvId itemL = this->nextId(&componentType);
        this->writeInstruction(SpvOpCompositeExtract, componentTypeId, itemL, lhs, index, out);
        SpvId itemR = this->nextId(&componentType);
        this->writeInstruction(SpvOpCompositeExtract, componentTypeId, itemR, rhs, index, out);
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
    SkASSERT(op.kind() == Token::Kind::TK_EQEQ || op.kind() == Token::Kind::TK_NEQ);
    SkASSERT(structType.isStruct());
    const std::vector<Type::Field>& fields = structType.fields();
    SkASSERT(!fields.empty());

    // Synthesize equality checks for each field in the struct.
    const Type& boolType = *fContext.fTypes.fBool;
    SpvId allComparisons = (SpvId)-1;
    for (int index = 0; index < (int)fields.size(); ++index) {
        // Get the left and right versions of this field.
        const Type& fieldType = *fields[index].fType;
        const SpvId fieldTypeId = this->getType(fieldType);

        SpvId fieldL = this->nextId(&fieldType);
        this->writeInstruction(SpvOpCompositeExtract, fieldTypeId, fieldL, lhs, index, out);
        SpvId fieldR = this->nextId(&fieldType);
        this->writeInstruction(SpvOpCompositeExtract, fieldTypeId, fieldR, rhs, index, out);
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
    if (allComparisons == (SpvId)-1) {
        return comparison;
    }
    // Use LogicalAnd or LogicalOr to combine the comparison with all the other comparisons.
    const Type& boolType = *fContext.fTypes.fBool;
    SpvId boolTypeId = this->getType(boolType);
    SpvId logicalOp = this->nextId(&boolType);
    switch (op.kind()) {
        case Token::Kind::TK_EQEQ:
            this->writeInstruction(SpvOpLogicalAnd, boolTypeId, logicalOp,
                                   comparison, allComparisons, out);
            break;
        case Token::Kind::TK_NEQ:
            this->writeInstruction(SpvOpLogicalOr, boolTypeId, logicalOp,
                                   comparison, allComparisons, out);
            break;
        default:
            SkDEBUGFAILF("mergeComparisons only supports == and !=, not %s", op.operatorName());
            return (SpvId)-1;
    }
    return logicalOp;
}

static float division_by_literal_value(Operator op, const Expression& right) {
    // If this is a division by a literal value, returns that literal value. Otherwise, returns 0.
    if (op.kind() == Token::Kind::TK_SLASH && right.isFloatLiteral()) {
        float rhsValue = right.as<Literal>().floatValue();
        if (std::isfinite(rhsValue)) {
            return rhsValue;
        }
    }
    return 0.0f;
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const BinaryExpression& b, OutputStream& out) {
    const Expression* left = b.left().get();
    const Expression* right = b.right().get();
    Operator op = b.getOperator();

    switch (op.kind()) {
        case Token::Kind::TK_EQ: {
            // Handles assignment.
            SpvId rhs = this->writeExpression(*right, out);
            this->getLValue(*left, out)->store(rhs, out);
            return rhs;
        }
        case Token::Kind::TK_LOGICALAND:
            // Handles short-circuiting; we don't necessarily evaluate both LHS and RHS.
            return this->writeLogicalAnd(*b.left(), *b.right(), out);

        case Token::Kind::TK_LOGICALOR:
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

    SpvId rhs;
    float rhsValue = division_by_literal_value(op, *right);
    if (rhsValue != 0.0f) {
        // Rewrite floating-point division by a literal into multiplication by the reciprocal.
        // This converts `expr / 2` into `expr * 0.5`
        // This improves codegen, especially for certain types of divides (e.g. vector/scalar).
        op = Operator(Token::Kind::TK_STAR);
        rhs = this->writeLiteral(1.0 / rhsValue, right->type());
    } else {
        // Write the right-hand side expression normally.
        rhs = this->writeExpression(*right, out);
    }

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
    SpvId rhsLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, rhsLabel, end, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(right, out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId(nullptr);
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fTypes.fBool), result, falseConstant,
                           lhsBlock, rhs, rhsBlock, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalOr(const Expression& left, const Expression& right,
                                         OutputStream& out) {
    SpvId trueConstant = this->writeLiteral(1.0, *fContext.fTypes.fBool);
    SpvId lhs = this->writeExpression(left, out);
    SpvId rhsLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, end, rhsLabel, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(right, out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId(nullptr);
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fTypes.fBool), result, trueConstant,
                           lhsBlock, rhs, rhsBlock, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeTernaryExpression(const TernaryExpression& t, OutputStream& out) {
    const Type& type = t.type();
    SpvId test = this->writeExpression(*t.test(), out);
    if (t.ifTrue()->type().columns() == 1 &&
        t.ifTrue()->isCompileTimeConstant() &&
        t.ifFalse()->isCompileTimeConstant()) {
        // both true and false are constants, can just use OpSelect
        SpvId result = this->nextId(nullptr);
        SpvId trueId = this->writeExpression(*t.ifTrue(), out);
        SpvId falseId = this->writeExpression(*t.ifFalse(), out);
        this->writeInstruction(SpvOpSelect, this->getType(type), result, test, trueId, falseId,
                               out);
        return result;
    }
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
    this->writeLabel(trueLabel, out);
    this->writeInstruction(SpvOpStore, var, this->writeExpression(*t.ifTrue(), out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(falseLabel, out);
    this->writeInstruction(SpvOpStore, var, this->writeExpression(*t.ifFalse(), out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId(&type);
    this->writeInstruction(SpvOpLoad, this->getType(type), result, var, out);
    return result;
}

SpvId SPIRVCodeGenerator::writePrefixExpression(const PrefixExpression& p, OutputStream& out) {
    const Type& type = p.type();
    if (p.getOperator().kind() == Token::Kind::TK_MINUS) {
        SpvId result = this->nextId(&type);
        SpvId typeId = this->getType(type);
        SpvId expr = this->writeExpression(*p.operand(), out);
        if (is_float(fContext, type)) {
            this->writeInstruction(SpvOpFNegate, typeId, result, expr, out);
        } else if (is_signed(fContext, type) || is_unsigned(fContext, type)) {
            this->writeInstruction(SpvOpSNegate, typeId, result, expr, out);
        } else {
            SkDEBUGFAILF("unsupported prefix expression %s", p.description().c_str());
        }
        return result;
    }
    switch (p.getOperator().kind()) {
        case Token::Kind::TK_PLUS:
            return this->writeExpression(*p.operand(), out);
        case Token::Kind::TK_PLUSPLUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
            SpvId one = this->writeLiteral(1.0, type);
            SpvId result = this->writeBinaryOperation(type, type, lv->load(out), one,
                                                      SpvOpFAdd, SpvOpIAdd, SpvOpIAdd, SpvOpUndef,
                                                      out);
            lv->store(result, out);
            return result;
        }
        case Token::Kind::TK_MINUSMINUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
            SpvId one = this->writeLiteral(1.0, type);
            SpvId result = this->writeBinaryOperation(type, type, lv->load(out), one, SpvOpFSub,
                                                      SpvOpISub, SpvOpISub, SpvOpUndef, out);
            lv->store(result, out);
            return result;
        }
        case Token::Kind::TK_LOGICALNOT: {
            SkASSERT(p.operand()->type().isBoolean());
            SpvId result = this->nextId(nullptr);
            this->writeInstruction(SpvOpLogicalNot, this->getType(type), result,
                                   this->writeExpression(*p.operand(), out), out);
            return result;
        }
        case Token::Kind::TK_BITWISENOT: {
            SpvId result = this->nextId(nullptr);
            this->writeInstruction(SpvOpNot, this->getType(type), result,
                                   this->writeExpression(*p.operand(), out), out);
            return result;
        }
        default:
            SkDEBUGFAILF("unsupported prefix expression: %s", p.description().c_str());
            return -1;
    }
}

SpvId SPIRVCodeGenerator::writePostfixExpression(const PostfixExpression& p, OutputStream& out) {
    const Type& type = p.type();
    std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
    SpvId result = lv->load(out);
    SpvId one = this->writeLiteral(1.0, type);
    switch (p.getOperator().kind()) {
        case Token::Kind::TK_PLUSPLUS: {
            SpvId temp = this->writeBinaryOperation(type, type, result, one, SpvOpFAdd,
                                                    SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
            lv->store(temp, out);
            return result;
        }
        case Token::Kind::TK_MINUSMINUS: {
            SpvId temp = this->writeBinaryOperation(type, type, result, one, SpvOpFSub,
                                                    SpvOpISub, SpvOpISub, SpvOpUndef, out);
            lv->store(temp, out);
            return result;
        }
        default:
            SkDEBUGFAILF("unsupported postfix expression %s", p.description().c_str());
            return -1;
    }
}

SpvId SPIRVCodeGenerator::writeLiteral(const Literal& l) {
    return this->writeLiteral(l.value(), l.type());
}

SpvId SPIRVCodeGenerator::writeLiteral(double value, const Type& type) {
    int32_t valueBits;
    if (type.isFloat()) {
        float fValue = value;
        memcpy(&valueBits, &fValue, sizeof(valueBits));
    } else {
        SKSL_INT iValue = value;
        valueBits = iValue;
    }

    SPIRVNumberConstant key{valueBits, type.numberKind()};
    auto [iter, newlyCreated] = fNumberConstants.insert({key, (SpvId)-1});
    if (newlyCreated) {
        SpvId result = this->nextId(nullptr);
        iter->second = result;

        if (type.isBoolean()) {
            this->writeInstruction(valueBits ? SpvOpConstantTrue : SpvOpConstantFalse,
                                   this->getType(type), result, fConstantBuffer);
        } else {
            this->writeInstruction(SpvOpConstant, this->getType(type), result,
                                   (SpvId)valueBits, fConstantBuffer);
        }
    }

    return iter->second;
}

SpvId SPIRVCodeGenerator::writeFunctionStart(const FunctionDeclaration& f, OutputStream& out) {
    SpvId result = fFunctionMap[&f];
    SpvId returnTypeId = this->getType(f.returnType());
    SpvId functionTypeId = this->getFunctionType(f);
    this->writeInstruction(SpvOpFunction, returnTypeId, result,
                           SpvFunctionControlMaskNone, functionTypeId, out);
    String mangledName = f.mangledName();
    this->writeInstruction(SpvOpName,
                           result,
                           skstd::string_view(mangledName.c_str(), mangledName.size()),
                           fNameBuffer);
    for (const Variable* parameter : f.parameters()) {
        SpvId id = this->nextId(nullptr);
        fVariableMap[parameter] = id;
        SpvId type = this->getPointerType(parameter->type(), SpvStorageClassFunction);
        this->writeInstruction(SpvOpFunctionParameter, type, id, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFunction(const FunctionDefinition& f, OutputStream& out) {
    fVariableBuffer.reset();
    SpvId result = this->writeFunctionStart(f.declaration(), out);
    fCurrentBlock = 0;
    this->writeLabel(this->nextId(nullptr), out);
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
    return result;
}

void SPIRVCodeGenerator::writeLayout(const Layout& layout, SpvId target, int line) {
    bool isPushConstant = (layout.fFlags & Layout::kPushConstant_Flag);
    if (layout.fLocation >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationLocation, layout.fLocation,
                               fDecorationBuffer);
    }
    if (layout.fBinding >= 0) {
        if (isPushConstant) {
            fContext.fErrors->error(line, "Can't apply 'binding' to push constants");
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
            fContext.fErrors->error(line, "Can't apply 'set' to push constants");
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

MemoryLayout SPIRVCodeGenerator::memoryLayoutForVariable(const Variable& v) const {
    bool pushConstant = ((v.modifiers().fLayout.fFlags & Layout::kPushConstant_Flag) != 0);
    return pushConstant ? MemoryLayout(MemoryLayout::k430_Standard) : fDefaultLayout;
}

SpvId SPIRVCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf, bool appendRTFlip) {
    MemoryLayout memoryLayout = this->memoryLayoutForVariable(intf.variable());
    SpvId result = this->nextId(nullptr);
    const Variable& intfVar = intf.variable();
    const Type& type = intfVar.type();
    if (!MemoryLayout::LayoutIsSupported(type)) {
        fContext.fErrors->error(type.fLine, "type '" + type.name() + "' is not permitted here");
        return this->nextId(nullptr);
    }
    SpvStorageClass_ storageClass = get_storage_class(intf.variable(), SpvStorageClassFunction);
    if (fProgram.fInputs.fUseFlipRTUniform && appendRTFlip && type.isStruct()) {
        // We can only have one interface block (because we use push_constant and that is limited
        // to one per program), so we need to append rtflip to this one rather than synthesize an
        // entirely new block when the variable is referenced. And we can't modify the existing
        // block, so we instead create a modified copy of it and write that.
        std::vector<Type::Field> fields = type.fields();
        fields.emplace_back(Modifiers(Layout(/*flags=*/0,
                                             /*location=*/-1,
                                             fProgram.fConfig->fSettings.fRTFlipOffset,
                                             /*binding=*/-1,
                                             /*index=*/-1,
                                             /*set=*/-1,
                                             /*builtin=*/-1,
                                             /*inputAttachmentIndex=*/-1),
                                      /*flags=*/0),
                            SKSL_RTFLIP_NAME,
                            fContext.fTypes.fFloat2.get());
        {
            AutoAttachPoolToThread attach(fProgram.fPool.get());
            const Type* rtFlipStructType =
                    fProgram.fSymbols->takeOwnershipOfSymbol(Type::MakeStructType(
                            type.fLine, type.name(), std::move(fields), /*interfaceBlock=*/true));
            const Variable* modifiedVar = fProgram.fSymbols->takeOwnershipOfSymbol(
                    std::make_unique<Variable>(intfVar.fLine,
                                               &intfVar.modifiers(),
                                               intfVar.name(),
                                               rtFlipStructType,
                                               intfVar.isBuiltin(),
                                               intfVar.storage()));
            fSPIRVBonusVariables.insert(modifiedVar);
            InterfaceBlock modifiedCopy(intf.fLine,
                                        *modifiedVar,
                                        intf.typeName(),
                                        intf.instanceName(),
                                        intf.arraySize(),
                                        intf.typeOwner());
            result = this->writeInterfaceBlock(modifiedCopy, false);
            fProgram.fSymbols->add(std::make_unique<Field>(
                    /*line=*/-1, modifiedVar, rtFlipStructType->fields().size() - 1));
        }
        fVariableMap[&intfVar] = result;
        fWroteRTFlip = true;
        return result;
    }
    const Modifiers& intfModifiers = intfVar.modifiers();
    SpvId typeId = this->getType(type, memoryLayout);
    if (intfModifiers.fLayout.fBuiltin == -1) {
        this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBlock, fDecorationBuffer);
    }
    SpvId ptrType = this->nextId(nullptr);
    this->writeInstruction(SpvOpTypePointer, ptrType, storageClass, typeId, fConstantBuffer);
    this->writeInstruction(SpvOpVariable, ptrType, result, storageClass, fConstantBuffer);
    Layout layout = intfModifiers.fLayout;
    if (storageClass == SpvStorageClassUniform && layout.fSet < 0) {
        layout.fSet = fProgram.fConfig->fSettings.fDefaultUniformSet;
    }
    this->writeLayout(layout, result, intfVar.fLine);
    fVariableMap[&intfVar] = result;
    return result;
}

bool SPIRVCodeGenerator::isDead(const Variable& var) const {
    // During SPIR-V code generation, we synthesize some extra bonus variables that don't actually
    // exist in the Program at all and aren't tracked by the ProgramUsage. They aren't dead, though.
    if (fSPIRVBonusVariables.count(&var)) {
        return false;
    }
    ProgramUsage::VariableCounts counts = fProgram.usage()->get(var);
    if (counts.fRead || counts.fWrite) {
        return false;
    }
    // It's not entirely clear what the rules are for eliding interface variables. Generally, it
    // causes problems to elide them, even when they're dead.
    return !(var.modifiers().fFlags &
             (Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag));
}

void SPIRVCodeGenerator::writeGlobalVar(ProgramKind kind, const VarDeclaration& varDecl) {
    const Variable& var = varDecl.var();
    if (var.modifiers().fLayout.fBuiltin == SK_FRAGCOLOR_BUILTIN &&
        kind != ProgramKind::kFragment) {
        SkASSERT(!fProgram.fConfig->fSettings.fFragColorIsInOut);
        return;
    }
    if (var.modifiers().fLayout.fBuiltin == SK_SECONDARYFRAGCOLOR_BUILTIN) {
        return;
    }
    if (this->isDead(var)) {
        return;
    }
    SpvStorageClass_ storageClass = get_storage_class(var, SpvStorageClassPrivate);
    if (storageClass == SpvStorageClassUniform) {
        // Top-level uniforms are emitted in writeUniformBuffer.
        fTopLevelUniforms.push_back(&varDecl);
        return;
    }
    const Type& type = var.type();
    Layout layout = var.modifiers().fLayout;
    if (layout.fSet < 0 && storageClass == SpvStorageClassUniformConstant) {
        layout.fSet = fProgram.fConfig->fSettings.fDefaultUniformSet;
    }
    SpvId id = this->nextId(&type);
    fVariableMap[&var] = id;
    SpvId typeId = this->getPointerType(type, storageClass);
    this->writeInstruction(SpvOpVariable, typeId, id, storageClass, fConstantBuffer);
    this->writeInstruction(SpvOpName, id, var.name(), fNameBuffer);
    if (varDecl.value()) {
        SkASSERT(!fCurrentBlock);
        fCurrentBlock = -1;
        SpvId value = this->writeExpression(*varDecl.value(), fGlobalInitializersBuffer);
        this->writeInstruction(SpvOpStore, id, value, fGlobalInitializersBuffer);
        fCurrentBlock = 0;
    }
    this->writeLayout(layout, id, var.fLine);
    if (var.modifiers().fFlags & Modifiers::kFlat_Flag) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationFlat, fDecorationBuffer);
    }
    if (var.modifiers().fFlags & Modifiers::kNoPerspective_Flag) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationNoPerspective,
                                fDecorationBuffer);
    }
}

void SPIRVCodeGenerator::writeVarDeclaration(const VarDeclaration& varDecl, OutputStream& out) {
    const Variable& var = varDecl.var();
    SpvId id = this->nextId(&var.type());
    fVariableMap[&var] = id;
    SpvId type = this->getPointerType(var.type(), SpvStorageClassFunction);
    this->writeInstruction(SpvOpVariable, type, id, SpvStorageClassFunction, fVariableBuffer);
    this->writeInstruction(SpvOpName, id, var.name(), fNameBuffer);
    if (varDecl.value()) {
        SpvId value = this->writeExpression(*varDecl.value(), out);
        this->writeInstruction(SpvOpStore, id, value, out);
    }
}

void SPIRVCodeGenerator::writeStatement(const Statement& s, OutputStream& out) {
    switch (s.kind()) {
        case Statement::Kind::kInlineMarker:
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
            this->writeInstruction(SpvOpBranch, fBreakTarget.top(), out);
            break;
        case Statement::Kind::kContinue:
            this->writeInstruction(SpvOpBranch, fContinueTarget.top(), out);
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

void SPIRVCodeGenerator::writeIfStatement(const IfStatement& stmt, OutputStream& out) {
    SpvId test = this->writeExpression(*stmt.test(), out);
    SpvId ifTrue = this->nextId(nullptr);
    SpvId ifFalse = this->nextId(nullptr);
    if (stmt.ifFalse()) {
        SpvId end = this->nextId(nullptr);
        this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
        this->writeInstruction(SpvOpBranchConditional, test, ifTrue, ifFalse, out);
        this->writeLabel(ifTrue, out);
        this->writeStatement(*stmt.ifTrue(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, end, out);
        }
        this->writeLabel(ifFalse, out);
        this->writeStatement(*stmt.ifFalse(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, end, out);
        }
        this->writeLabel(end, out);
    } else {
        this->writeInstruction(SpvOpSelectionMerge, ifFalse, SpvSelectionControlMaskNone, out);
        this->writeInstruction(SpvOpBranchConditional, test, ifTrue, ifFalse, out);
        this->writeLabel(ifTrue, out);
        this->writeStatement(*stmt.ifTrue(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, ifFalse, out);
        }
        this->writeLabel(ifFalse, out);
    }
}

void SPIRVCodeGenerator::writeForStatement(const ForStatement& f, OutputStream& out) {
    if (f.initializer()) {
        this->writeStatement(*f.initializer(), out);
    }
    SpvId header = this->nextId(nullptr);
    SpvId start = this->nextId(nullptr);
    SpvId body = this->nextId(nullptr);
    SpvId next = this->nextId(nullptr);
    fContinueTarget.push(next);
    SpvId end = this->nextId(nullptr);
    fBreakTarget.push(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, out);
    this->writeInstruction(SpvOpLoopMerge, end, next, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, out);
    if (f.test()) {
        SpvId test = this->writeExpression(*f.test(), out);
        this->writeInstruction(SpvOpBranchConditional, test, body, end, out);
    } else {
        this->writeInstruction(SpvOpBranch, body, out);
    }
    this->writeLabel(body, out);
    this->writeStatement(*f.statement(), out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
    }
    this->writeLabel(next, out);
    if (f.next()) {
        this->writeExpression(*f.next(), out);
    }
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(end, out);
    fBreakTarget.pop();
    fContinueTarget.pop();
}

void SPIRVCodeGenerator::writeDoStatement(const DoStatement& d, OutputStream& out) {
    SpvId header = this->nextId(nullptr);
    SpvId start = this->nextId(nullptr);
    SpvId next = this->nextId(nullptr);
    SpvId continueTarget = this->nextId(nullptr);
    fContinueTarget.push(continueTarget);
    SpvId end = this->nextId(nullptr);
    fBreakTarget.push(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, out);
    this->writeInstruction(SpvOpLoopMerge, end, continueTarget, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, out);
    this->writeStatement(*d.statement(), out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
    }
    this->writeLabel(next, out);
    this->writeInstruction(SpvOpBranch, continueTarget, out);
    this->writeLabel(continueTarget, out);
    SpvId test = this->writeExpression(*d.test(), out);
    this->writeInstruction(SpvOpBranchConditional, test, header, end, out);
    this->writeLabel(end, out);
    fBreakTarget.pop();
    fContinueTarget.pop();
}

void SPIRVCodeGenerator::writeSwitchStatement(const SwitchStatement& s, OutputStream& out) {
    SpvId value = this->writeExpression(*s.value(), out);
    std::vector<SpvId> labels;
    SpvId end = this->nextId(nullptr);
    SpvId defaultLabel = end;
    fBreakTarget.push(end);
    int size = 3;
    auto& cases = s.cases();
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
    labels.push_back(end);
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeOpCode(SpvOpSwitch, size, out);
    this->writeWord(value, out);
    this->writeWord(defaultLabel, out);
    for (size_t i = 0; i < cases.size(); ++i) {
        const SwitchCase& c = cases[i]->as<SwitchCase>();
        if (c.isDefault()) {
            continue;
        }
        this->writeWord(c.value(), out);
        this->writeWord(labels[i], out);
    }
    for (size_t i = 0; i < cases.size(); ++i) {
        const SwitchCase& c = cases[i]->as<SwitchCase>();
        this->writeLabel(labels[i], out);
        this->writeStatement(*c.statement(), out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, labels[i + 1], out);
        }
    }
    this->writeLabel(end, out);
    fBreakTarget.pop();
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
    const Symbol* skFragColorSymbol = (*symbolTable)["sk_FragColor"];
    SkASSERT(skFragColorSymbol);
    const Variable& skFragColorVar = skFragColorSymbol->as<Variable>();
    auto skFragColorRef = std::make_unique<VariableReference>(/*line=*/-1, &skFragColorVar,
                                                              VariableReference::RefKind::kWrite);
    // Synthesize a call to the `main()` function.
    if (!main.returnType().matches(skFragColorRef->type())) {
        fContext.fErrors->error(main.fLine, "SPIR-V does not support returning '" +
                                            main.returnType().description() + "' from main()");
        return {};
    }
    ExpressionArray args;
    if (main.parameters().size() == 1) {
        if (!main.parameters()[0]->type().matches(*fContext.fTypes.fFloat2)) {
            fContext.fErrors->error(main.fLine,
                    "SPIR-V does not support parameter of type '" +
                    main.parameters()[0]->type().description() + "' to main()");
            return {};
        }
        args.push_back(dsl::Float2(0).release());
    }
    auto callMainFn = std::make_unique<FunctionCall>(/*line=*/-1, &main.returnType(), &main,
                                                     std::move(args));

    // Synthesize `skFragColor = main()` as a BinaryExpression.
    auto assignmentStmt = std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
            /*line=*/-1,
            std::move(skFragColorRef),
            Token::Kind::TK_EQ,
            std::move(callMainFn),
            &main.returnType()));

    // Function bodies are always wrapped in a Block.
    StatementArray entrypointStmts;
    entrypointStmts.push_back(std::move(assignmentStmt));
    auto entrypointBlock = Block::Make(/*line=*/-1, std::move(entrypointStmts),
                                       symbolTable, /*isScope=*/true);
    // Declare an entrypoint function.
    EntrypointAdapter adapter;
    adapter.fLayout = {};
    adapter.fModifiers = Modifiers{adapter.fLayout, Modifiers::kHasSideEffects_Flag};
    adapter.entrypointDecl =
            std::make_unique<FunctionDeclaration>(/*line=*/-1,
                                                  &adapter.fModifiers,
                                                  "_entrypoint",
                                                  /*parameters=*/std::vector<const Variable*>{},
                                                  /*returnType=*/fContext.fTypes.fVoid.get(),
                                                  /*builtin=*/false);
    // Define it.
    adapter.entrypointDef = FunctionDefinition::Convert(fContext,
                                                        /*line=*/-1,
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
    std::vector<Type::Field> fields;
    fields.reserve(fTopLevelUniforms.size());
    fTopLevelUniformMap.reserve(fTopLevelUniforms.size());
    for (const VarDeclaration* topLevelUniform : fTopLevelUniforms) {
        const Variable* var = &topLevelUniform->var();
        fTopLevelUniformMap[var] = (int)fields.size();
        fields.emplace_back(var->modifiers(), var->name(), &var->type());
    }
    fUniformBuffer.fStruct = Type::MakeStructType(
            /*line=*/-1, kUniformBufferName, std::move(fields), /*interfaceBlock=*/true);

    // Create a global variable to contain this struct.
    Layout layout;
    layout.fBinding = fProgram.fConfig->fSettings.fDefaultUniformBinding;
    layout.fSet     = fProgram.fConfig->fSettings.fDefaultUniformSet;
    Modifiers modifiers{layout, Modifiers::kUniform_Flag};

    fUniformBuffer.fInnerVariable = std::make_unique<Variable>(
            /*line=*/-1, fProgram.fModifiers->add(modifiers), kUniformBufferName,
            fUniformBuffer.fStruct.get(), /*builtin=*/false, Variable::Storage::kGlobal);

    // Create an interface block object for this global variable.
    fUniformBuffer.fInterfaceBlock = std::make_unique<InterfaceBlock>(
            /*offset=*/-1, *fUniformBuffer.fInnerVariable, kUniformBufferName,
            kUniformBufferName, /*arraySize=*/0, topLevelSymbolTable);

    // Generate an interface block and hold onto its ID.
    fUniformBufferId = this->writeInterfaceBlock(*fUniformBuffer.fInterfaceBlock);
}

void SPIRVCodeGenerator::addRTFlipUniform(int line) {
    if (fWroteRTFlip) {
        return;
    }
    // Flip variable hasn't been written yet. This means we don't have an existing
    // interface block, so we're free to just synthesize one.
    fWroteRTFlip = true;
    std::vector<Type::Field> fields;
    if (fProgram.fConfig->fSettings.fRTFlipOffset < 0) {
        fContext.fErrors->error(line, "RTFlipOffset is negative");
    }
    fields.emplace_back(Modifiers(Layout(/*flags=*/0,
                                         /*location=*/-1,
                                         fProgram.fConfig->fSettings.fRTFlipOffset,
                                         /*binding=*/-1,
                                         /*index=*/-1,
                                         /*set=*/-1,
                                         /*builtin=*/-1,
                                         /*inputAttachmentIndex=*/-1),
                                  /*flags=*/0),
                        SKSL_RTFLIP_NAME,
                        fContext.fTypes.fFloat2.get());
    skstd::string_view name = "sksl_synthetic_uniforms";
    const Type* intfStruct = fSynthetics.takeOwnershipOfSymbol(
            Type::MakeStructType(/*line=*/-1, name, fields, /*interfaceBlock=*/true));
    bool usePushConstants = fProgram.fConfig->fSettings.fUsePushConstants;
    int binding = -1, set = -1;
    if (!usePushConstants) {
        binding = fProgram.fConfig->fSettings.fRTFlipBinding;
        if (binding == -1) {
            fContext.fErrors->error(line, "layout(binding=...) is required in SPIR-V");
        }
        set = fProgram.fConfig->fSettings.fRTFlipSet;
        if (set == -1) {
            fContext.fErrors->error(line, "layout(set=...) is required in SPIR-V");
        }
    }
    int flags = usePushConstants ? Layout::Flag::kPushConstant_Flag : 0;
    const Modifiers* modsPtr;
    {
        AutoAttachPoolToThread attach(fProgram.fPool.get());
        Modifiers modifiers(Layout(flags,
                                   /*location=*/-1,
                                   /*offset=*/-1,
                                   binding,
                                   /*index=*/-1,
                                   set,
                                   /*builtin=*/-1,
                                   /*inputAttachmentIndex=*/-1),
                            Modifiers::kUniform_Flag);
        modsPtr = fProgram.fModifiers->add(modifiers);
    }
    const Variable* intfVar = fSynthetics.takeOwnershipOfSymbol(
            std::make_unique<Variable>(/*line=*/-1,
                                       modsPtr,
                                       name,
                                       intfStruct,
                                       /*builtin=*/false,
                                       Variable::Storage::kGlobal));
    fSPIRVBonusVariables.insert(intfVar);
    {
        AutoAttachPoolToThread attach(fProgram.fPool.get());
        fProgram.fSymbols->add(std::make_unique<Field>(/*line=*/-1, intfVar, /*field=*/0));
    }
    InterfaceBlock intf(/*line=*/-1,
                        *intfVar,
                        name,
                        /*instanceName=*/"",
                        /*arraySize=*/0,
                        std::make_shared<SymbolTable>(fContext, /*builtin=*/false));

    this->writeInterfaceBlock(intf, false);
}

void SPIRVCodeGenerator::writeInstructions(const Program& program, OutputStream& out) {
    fGLSLExtendedInstructions = this->nextId(nullptr);
    StringStream body;
    // Assign SpvIds to functions.
    const FunctionDeclaration* main = nullptr;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<FunctionDefinition>()) {
            const FunctionDefinition& funcDef = e->as<FunctionDefinition>();
            const FunctionDeclaration& funcDecl = funcDef.declaration();
            fFunctionMap[&funcDecl] = this->nextId(nullptr);
            if (funcDecl.isMain()) {
                main = &funcDecl;
            }
        }
    }
    // Make sure we have a main() function.
    if (!main) {
        fContext.fErrors->error(/*line=*/-1, "program does not contain a main() function");
        return;
    }
    // Emit interface blocks.
    std::set<SpvId> interfaceVars;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<InterfaceBlock>()) {
            const InterfaceBlock& intf = e->as<InterfaceBlock>();
            SpvId id = this->writeInterfaceBlock(intf);

            const Modifiers& modifiers = intf.variable().modifiers();
            if ((modifiers.fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) &&
                modifiers.fLayout.fBuiltin == -1 && !this->isDead(intf.variable())) {
                interfaceVars.insert(id);
            }
        }
    }
    // Emit global variable declarations.
    for (const ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            this->writeGlobalVar(program.fConfig->fKind,
                                 e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>());
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
            fFunctionMap[adapter.entrypointDecl.get()] = this->nextId(nullptr);
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
    for (auto entry : fVariableMap) {
        const Variable* var = entry.first;
        if (var->storage() == Variable::Storage::kGlobal &&
            (var->modifiers().fFlags & (Modifiers::kIn_Flag | Modifiers::kOut_Flag)) &&
            !this->isDead(*var)) {
            interfaceVars.insert(entry.second);
        }
    }
    this->writeCapabilities(out);
    this->writeInstruction(SpvOpExtInstImport, fGLSLExtendedInstructions, "GLSL.std.450", out);
    this->writeInstruction(SpvOpMemoryModel, SpvAddressingModelLogical, SpvMemoryModelGLSL450, out);
    this->writeOpCode(SpvOpEntryPoint, (SpvId) (3 + (main->name().length() + 4) / 4) +
                      (int32_t) interfaceVars.size(), out);
    switch (program.fConfig->fKind) {
        case ProgramKind::kVertex:
            this->writeWord(SpvExecutionModelVertex, out);
            break;
        case ProgramKind::kFragment:
            this->writeWord(SpvExecutionModelFragment, out);
            break;
        default:
            SK_ABORT("cannot write this kind of program to SPIR-V\n");
    }
    SpvId entryPoint = fFunctionMap[main];
    this->writeWord(entryPoint, out);
    this->writeString(main->name(), out);
    for (int var : interfaceVars) {
        this->writeWord(var, out);
    }
    if (program.fConfig->fKind == ProgramKind::kFragment) {
        this->writeInstruction(SpvOpExecutionMode,
                               fFunctionMap[main],
                               SpvExecutionModeOriginUpperLeft,
                               out);
    }
    for (const ProgramElement* e : program.elements()) {
        if (e->is<Extension>()) {
            this->writeInstruction(SpvOpSourceExtension, e->as<Extension>().name(), out);
        }
    }

    write_stringstream(fExtraGlobalsBuffer, out);
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
    fContext.fErrors->reportPendingErrors(PositionInfo());
    return fContext.fErrors->errorCount() == 0;
}

}  // namespace SkSL
