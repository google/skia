/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLSPIRVCodeGenerator.h"

#include "GLSL.std.450.h"

#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLExtension.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLVariableReference.h"
#include "SkSLCompiler.h"

namespace SkSL {

#define SPIRV_DEBUG 0

static const int32_t SKSL_MAGIC  = 0x0; // FIXME: we should probably register a magic number

void SPIRVCodeGenerator::setupIntrinsics() {
#define ALL_GLSL(x) std::make_tuple(kGLSL_STD_450_IntrinsicKind, GLSLstd450 ## x, GLSLstd450 ## x, \
                                    GLSLstd450 ## x, GLSLstd450 ## x)
#define BY_TYPE_GLSL(ifFloat, ifInt, ifUInt) std::make_tuple(kGLSL_STD_450_IntrinsicKind, \
                                                             GLSLstd450 ## ifFloat, \
                                                             GLSLstd450 ## ifInt, \
                                                             GLSLstd450 ## ifUInt, \
                                                             SpvOpUndef)
#define SPECIAL(x) std::make_tuple(kSpecial_IntrinsicKind, k ## x ## _SpecialIntrinsic, \
                                   k ## x ## _SpecialIntrinsic, k ## x ## _SpecialIntrinsic, \
                                   k ## x ## _SpecialIntrinsic)
    fIntrinsicMap[String("round")]         = ALL_GLSL(Round);
    fIntrinsicMap[String("roundEven")]     = ALL_GLSL(RoundEven);
    fIntrinsicMap[String("trunc")]         = ALL_GLSL(Trunc);
    fIntrinsicMap[String("abs")]           = BY_TYPE_GLSL(FAbs, SAbs, SAbs);
    fIntrinsicMap[String("sign")]          = BY_TYPE_GLSL(FSign, SSign, SSign);
    fIntrinsicMap[String("floor")]         = ALL_GLSL(Floor);
    fIntrinsicMap[String("ceil")]          = ALL_GLSL(Ceil);
    fIntrinsicMap[String("fract")]         = ALL_GLSL(Fract);
    fIntrinsicMap[String("radians")]       = ALL_GLSL(Radians);
    fIntrinsicMap[String("degrees")]       = ALL_GLSL(Degrees);
    fIntrinsicMap[String("sin")]           = ALL_GLSL(Sin);
    fIntrinsicMap[String("cos")]           = ALL_GLSL(Cos);
    fIntrinsicMap[String("tan")]           = ALL_GLSL(Tan);
    fIntrinsicMap[String("asin")]          = ALL_GLSL(Asin);
    fIntrinsicMap[String("acos")]          = ALL_GLSL(Acos);
    fIntrinsicMap[String("atan")]          = SPECIAL(Atan);
    fIntrinsicMap[String("sinh")]          = ALL_GLSL(Sinh);
    fIntrinsicMap[String("cosh")]          = ALL_GLSL(Cosh);
    fIntrinsicMap[String("tanh")]          = ALL_GLSL(Tanh);
    fIntrinsicMap[String("asinh")]         = ALL_GLSL(Asinh);
    fIntrinsicMap[String("acosh")]         = ALL_GLSL(Acosh);
    fIntrinsicMap[String("atanh")]         = ALL_GLSL(Atanh);
    fIntrinsicMap[String("pow")]           = ALL_GLSL(Pow);
    fIntrinsicMap[String("exp")]           = ALL_GLSL(Exp);
    fIntrinsicMap[String("log")]           = ALL_GLSL(Log);
    fIntrinsicMap[String("exp2")]          = ALL_GLSL(Exp2);
    fIntrinsicMap[String("log2")]          = ALL_GLSL(Log2);
    fIntrinsicMap[String("sqrt")]          = ALL_GLSL(Sqrt);
    fIntrinsicMap[String("inversesqrt")]   = ALL_GLSL(InverseSqrt);
    fIntrinsicMap[String("determinant")]   = ALL_GLSL(Determinant);
    fIntrinsicMap[String("matrixInverse")] = ALL_GLSL(MatrixInverse);
    fIntrinsicMap[String("mod")]           = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpFMod,
                                                               SpvOpSMod, SpvOpUMod, SpvOpUndef);
    fIntrinsicMap[String("min")]           = BY_TYPE_GLSL(FMin, SMin, UMin);
    fIntrinsicMap[String("max")]           = BY_TYPE_GLSL(FMax, SMax, UMax);
    fIntrinsicMap[String("clamp")]         = BY_TYPE_GLSL(FClamp, SClamp, UClamp);
    fIntrinsicMap[String("dot")]           = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDot,
                                                               SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("mix")]           = ALL_GLSL(FMix);
    fIntrinsicMap[String("step")]          = ALL_GLSL(Step);
    fIntrinsicMap[String("smoothstep")]    = ALL_GLSL(SmoothStep);
    fIntrinsicMap[String("fma")]           = ALL_GLSL(Fma);
    fIntrinsicMap[String("frexp")]         = ALL_GLSL(Frexp);
    fIntrinsicMap[String("ldexp")]         = ALL_GLSL(Ldexp);

#define PACK(type) fIntrinsicMap[String("pack" #type)] = ALL_GLSL(Pack ## type); \
                   fIntrinsicMap[String("unpack" #type)] = ALL_GLSL(Unpack ## type)
    PACK(Snorm4x8);
    PACK(Unorm4x8);
    PACK(Snorm2x16);
    PACK(Unorm2x16);
    PACK(Half2x16);
    PACK(Double2x32);
    fIntrinsicMap[String("length")]      = ALL_GLSL(Length);
    fIntrinsicMap[String("distance")]    = ALL_GLSL(Distance);
    fIntrinsicMap[String("cross")]       = ALL_GLSL(Cross);
    fIntrinsicMap[String("normalize")]   = ALL_GLSL(Normalize);
    fIntrinsicMap[String("faceForward")] = ALL_GLSL(FaceForward);
    fIntrinsicMap[String("reflect")]     = ALL_GLSL(Reflect);
    fIntrinsicMap[String("refract")]     = ALL_GLSL(Refract);
    fIntrinsicMap[String("findLSB")]     = ALL_GLSL(FindILsb);
    fIntrinsicMap[String("findMSB")]     = BY_TYPE_GLSL(FindSMsb, FindSMsb, FindUMsb);
    fIntrinsicMap[String("dFdx")]        = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDPdx,
                                                             SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("dFdy")]        = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDPdy,
                                                             SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("dFdy")]        = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDPdy,
                                                             SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("texture")]     = SPECIAL(Texture);

    fIntrinsicMap[String("subpassLoad")] = SPECIAL(SubpassLoad);

    fIntrinsicMap[String("any")]              = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpUndef,
                                                                  SpvOpUndef, SpvOpUndef, SpvOpAny);
    fIntrinsicMap[String("all")]              = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpUndef,
                                                                  SpvOpUndef, SpvOpUndef, SpvOpAll);
    fIntrinsicMap[String("equal")]            = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                  SpvOpFOrdEqual, SpvOpIEqual,
                                                                  SpvOpIEqual, SpvOpLogicalEqual);
    fIntrinsicMap[String("notEqual")]         = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                  SpvOpFOrdNotEqual, SpvOpINotEqual,
                                                                  SpvOpINotEqual,
                                                                  SpvOpLogicalNotEqual);
    fIntrinsicMap[String("lessThan")]         = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                  SpvOpSLessThan, SpvOpULessThan,
                                                                  SpvOpFOrdLessThan, SpvOpUndef);
    fIntrinsicMap[String("lessThanEqual")]    = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                  SpvOpSLessThanEqual,
                                                                  SpvOpULessThanEqual,
                                                                  SpvOpFOrdLessThanEqual,
                                                                  SpvOpUndef);
    fIntrinsicMap[String("greaterThan")]      = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                  SpvOpSGreaterThan,
                                                                  SpvOpUGreaterThan,
                                                                  SpvOpFOrdGreaterThan,
                                                                  SpvOpUndef);
    fIntrinsicMap[String("greaterThanEqual")] = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                  SpvOpSGreaterThanEqual,
                                                                  SpvOpUGreaterThanEqual,
                                                                  SpvOpFOrdGreaterThanEqual,
                                                                  SpvOpUndef);
// interpolateAt* not yet supported...
}

void SPIRVCodeGenerator::writeWord(int32_t word, OutputStream& out) {
#if SPIRV_DEBUG
    out << "(" << word << ") ";
#else
    out.write((const char*) &word, sizeof(word));
#endif
}

static bool is_float(const Context& context, const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return is_float(context, type.componentType());
    }
    return type == *context.fFloat_Type || type == *context.fDouble_Type;
}

static bool is_signed(const Context& context, const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return is_signed(context, type.componentType());
    }
    return type == *context.fInt_Type;
}

static bool is_unsigned(const Context& context, const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return is_unsigned(context, type.componentType());
    }
    return type == *context.fUInt_Type;
}

static bool is_bool(const Context& context, const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return is_bool(context, type.componentType());
    }
    return type == *context.fBool_Type;
}

static bool is_out(const Variable& var) {
    return (var.fModifiers.fFlags & Modifiers::kOut_Flag) != 0;
}

#if SPIRV_DEBUG
static String opcode_text(SpvOp_ opCode) {
    switch (opCode) {
        case SpvOpNop:
            return String("Nop");
        case SpvOpUndef:
            return String("Undef");
        case SpvOpSourceContinued:
            return String("SourceContinued");
        case SpvOpSource:
            return String("Source");
        case SpvOpSourceExtension:
            return String("SourceExtension");
        case SpvOpName:
            return String("Name");
        case SpvOpMemberName:
            return String("MemberName");
        case SpvOpString:
            return String("String");
        case SpvOpLine:
            return String("Line");
        case SpvOpExtension:
            return String("Extension");
        case SpvOpExtInstImport:
            return String("ExtInstImport");
        case SpvOpExtInst:
            return String("ExtInst");
        case SpvOpMemoryModel:
            return String("MemoryModel");
        case SpvOpEntryPoint:
            return String("EntryPoint");
        case SpvOpExecutionMode:
            return String("ExecutionMode");
        case SpvOpCapability:
            return String("Capability");
        case SpvOpTypeVoid:
            return String("TypeVoid");
        case SpvOpTypeBool:
            return String("TypeBool");
        case SpvOpTypeInt:
            return String("TypeInt");
        case SpvOpTypeFloat:
            return String("TypeFloat");
        case SpvOpTypeVector:
            return String("TypeVector");
        case SpvOpTypeMatrix:
            return String("TypeMatrix");
        case SpvOpTypeImage:
            return String("TypeImage");
        case SpvOpTypeSampler:
            return String("TypeSampler");
        case SpvOpTypeSampledImage:
            return String("TypeSampledImage");
        case SpvOpTypeArray:
            return String("TypeArray");
        case SpvOpTypeRuntimeArray:
            return String("TypeRuntimeArray");
        case SpvOpTypeStruct:
            return String("TypeStruct");
        case SpvOpTypeOpaque:
            return String("TypeOpaque");
        case SpvOpTypePointer:
            return String("TypePointer");
        case SpvOpTypeFunction:
            return String("TypeFunction");
        case SpvOpTypeEvent:
            return String("TypeEvent");
        case SpvOpTypeDeviceEvent:
            return String("TypeDeviceEvent");
        case SpvOpTypeReserveId:
            return String("TypeReserveId");
        case SpvOpTypeQueue:
            return String("TypeQueue");
        case SpvOpTypePipe:
            return String("TypePipe");
        case SpvOpTypeForwardPointer:
            return String("TypeForwardPointer");
        case SpvOpConstantTrue:
            return String("ConstantTrue");
        case SpvOpConstantFalse:
            return String("ConstantFalse");
        case SpvOpConstant:
            return String("Constant");
        case SpvOpConstantComposite:
            return String("ConstantComposite");
        case SpvOpConstantSampler:
            return String("ConstantSampler");
        case SpvOpConstantNull:
            return String("ConstantNull");
        case SpvOpSpecConstantTrue:
            return String("SpecConstantTrue");
        case SpvOpSpecConstantFalse:
            return String("SpecConstantFalse");
        case SpvOpSpecConstant:
            return String("SpecConstant");
        case SpvOpSpecConstantComposite:
            return String("SpecConstantComposite");
        case SpvOpSpecConstantOp:
            return String("SpecConstantOp");
        case SpvOpFunction:
            return String("Function");
        case SpvOpFunctionParameter:
            return String("FunctionParameter");
        case SpvOpFunctionEnd:
            return String("FunctionEnd");
        case SpvOpFunctionCall:
            return String("FunctionCall");
        case SpvOpVariable:
            return String("Variable");
        case SpvOpImageTexelPointer:
            return String("ImageTexelPointer");
        case SpvOpLoad:
            return String("Load");
        case SpvOpStore:
            return String("Store");
        case SpvOpCopyMemory:
            return String("CopyMemory");
        case SpvOpCopyMemorySized:
            return String("CopyMemorySized");
        case SpvOpAccessChain:
            return String("AccessChain");
        case SpvOpInBoundsAccessChain:
            return String("InBoundsAccessChain");
        case SpvOpPtrAccessChain:
            return String("PtrAccessChain");
        case SpvOpArrayLength:
            return String("ArrayLength");
        case SpvOpGenericPtrMemSemantics:
            return String("GenericPtrMemSemantics");
        case SpvOpInBoundsPtrAccessChain:
            return String("InBoundsPtrAccessChain");
        case SpvOpDecorate:
            return String("Decorate");
        case SpvOpMemberDecorate:
            return String("MemberDecorate");
        case SpvOpDecorationGroup:
            return String("DecorationGroup");
        case SpvOpGroupDecorate:
            return String("GroupDecorate");
        case SpvOpGroupMemberDecorate:
            return String("GroupMemberDecorate");
        case SpvOpVectorExtractDynamic:
            return String("VectorExtractDynamic");
        case SpvOpVectorInsertDynamic:
            return String("VectorInsertDynamic");
        case SpvOpVectorShuffle:
            return String("VectorShuffle");
        case SpvOpCompositeConstruct:
            return String("CompositeConstruct");
        case SpvOpCompositeExtract:
            return String("CompositeExtract");
        case SpvOpCompositeInsert:
            return String("CompositeInsert");
        case SpvOpCopyObject:
            return String("CopyObject");
        case SpvOpTranspose:
            return String("Transpose");
        case SpvOpSampledImage:
            return String("SampledImage");
        case SpvOpImageSampleImplicitLod:
            return String("ImageSampleImplicitLod");
        case SpvOpImageSampleExplicitLod:
            return String("ImageSampleExplicitLod");
        case SpvOpImageSampleDrefImplicitLod:
            return String("ImageSampleDrefImplicitLod");
        case SpvOpImageSampleDrefExplicitLod:
            return String("ImageSampleDrefExplicitLod");
        case SpvOpImageSampleProjImplicitLod:
            return String("ImageSampleProjImplicitLod");
        case SpvOpImageSampleProjExplicitLod:
            return String("ImageSampleProjExplicitLod");
        case SpvOpImageSampleProjDrefImplicitLod:
            return String("ImageSampleProjDrefImplicitLod");
        case SpvOpImageSampleProjDrefExplicitLod:
            return String("ImageSampleProjDrefExplicitLod");
        case SpvOpImageFetch:
            return String("ImageFetch");
        case SpvOpImageGather:
            return String("ImageGather");
        case SpvOpImageDrefGather:
            return String("ImageDrefGather");
        case SpvOpImageRead:
            return String("ImageRead");
        case SpvOpImageWrite:
            return String("ImageWrite");
        case SpvOpImage:
            return String("Image");
        case SpvOpImageQueryFormat:
            return String("ImageQueryFormat");
        case SpvOpImageQueryOrder:
            return String("ImageQueryOrder");
        case SpvOpImageQuerySizeLod:
            return String("ImageQuerySizeLod");
        case SpvOpImageQuerySize:
            return String("ImageQuerySize");
        case SpvOpImageQueryLod:
            return String("ImageQueryLod");
        case SpvOpImageQueryLevels:
            return String("ImageQueryLevels");
        case SpvOpImageQuerySamples:
            return String("ImageQuerySamples");
        case SpvOpConvertFToU:
            return String("ConvertFToU");
        case SpvOpConvertFToS:
            return String("ConvertFToS");
        case SpvOpConvertSToF:
            return String("ConvertSToF");
        case SpvOpConvertUToF:
            return String("ConvertUToF");
        case SpvOpUConvert:
            return String("UConvert");
        case SpvOpSConvert:
            return String("SConvert");
        case SpvOpFConvert:
            return String("FConvert");
        case SpvOpQuantizeToF16:
            return String("QuantizeToF16");
        case SpvOpConvertPtrToU:
            return String("ConvertPtrToU");
        case SpvOpSatConvertSToU:
            return String("SatConvertSToU");
        case SpvOpSatConvertUToS:
            return String("SatConvertUToS");
        case SpvOpConvertUToPtr:
            return String("ConvertUToPtr");
        case SpvOpPtrCastToGeneric:
            return String("PtrCastToGeneric");
        case SpvOpGenericCastToPtr:
            return String("GenericCastToPtr");
        case SpvOpGenericCastToPtrExplicit:
            return String("GenericCastToPtrExplicit");
        case SpvOpBitcast:
            return String("Bitcast");
        case SpvOpSNegate:
            return String("SNegate");
        case SpvOpFNegate:
            return String("FNegate");
        case SpvOpIAdd:
            return String("IAdd");
        case SpvOpFAdd:
            return String("FAdd");
        case SpvOpISub:
            return String("ISub");
        case SpvOpFSub:
            return String("FSub");
        case SpvOpIMul:
            return String("IMul");
        case SpvOpFMul:
            return String("FMul");
        case SpvOpUDiv:
            return String("UDiv");
        case SpvOpSDiv:
            return String("SDiv");
        case SpvOpFDiv:
            return String("FDiv");
        case SpvOpUMod:
            return String("UMod");
        case SpvOpSRem:
            return String("SRem");
        case SpvOpSMod:
            return String("SMod");
        case SpvOpFRem:
            return String("FRem");
        case SpvOpFMod:
            return String("FMod");
        case SpvOpVectorTimesScalar:
            return String("VectorTimesScalar");
        case SpvOpMatrixTimesScalar:
            return String("MatrixTimesScalar");
        case SpvOpVectorTimesMatrix:
            return String("VectorTimesMatrix");
        case SpvOpMatrixTimesVector:
            return String("MatrixTimesVector");
        case SpvOpMatrixTimesMatrix:
            return String("MatrixTimesMatrix");
        case SpvOpOuterProduct:
            return String("OuterProduct");
        case SpvOpDot:
            return String("Dot");
        case SpvOpIAddCarry:
            return String("IAddCarry");
        case SpvOpISubBorrow:
            return String("ISubBorrow");
        case SpvOpUMulExtended:
            return String("UMulExtended");
        case SpvOpSMulExtended:
            return String("SMulExtended");
        case SpvOpAny:
            return String("Any");
        case SpvOpAll:
            return String("All");
        case SpvOpIsNan:
            return String("IsNan");
        case SpvOpIsInf:
            return String("IsInf");
        case SpvOpIsFinite:
            return String("IsFinite");
        case SpvOpIsNormal:
            return String("IsNormal");
        case SpvOpSignBitSet:
            return String("SignBitSet");
        case SpvOpLessOrGreater:
            return String("LessOrGreater");
        case SpvOpOrdered:
            return String("Ordered");
        case SpvOpUnordered:
            return String("Unordered");
        case SpvOpLogicalEqual:
            return String("LogicalEqual");
        case SpvOpLogicalNotEqual:
            return String("LogicalNotEqual");
        case SpvOpLogicalOr:
            return String("LogicalOr");
        case SpvOpLogicalAnd:
            return String("LogicalAnd");
        case SpvOpLogicalNot:
            return String("LogicalNot");
        case SpvOpSelect:
            return String("Select");
        case SpvOpIEqual:
            return String("IEqual");
        case SpvOpINotEqual:
            return String("INotEqual");
        case SpvOpUGreaterThan:
            return String("UGreaterThan");
        case SpvOpSGreaterThan:
            return String("SGreaterThan");
        case SpvOpUGreaterThanEqual:
            return String("UGreaterThanEqual");
        case SpvOpSGreaterThanEqual:
            return String("SGreaterThanEqual");
        case SpvOpULessThan:
            return String("ULessThan");
        case SpvOpSLessThan:
            return String("SLessThan");
        case SpvOpULessThanEqual:
            return String("ULessThanEqual");
        case SpvOpSLessThanEqual:
            return String("SLessThanEqual");
        case SpvOpFOrdEqual:
            return String("FOrdEqual");
        case SpvOpFUnordEqual:
            return String("FUnordEqual");
        case SpvOpFOrdNotEqual:
            return String("FOrdNotEqual");
        case SpvOpFUnordNotEqual:
            return String("FUnordNotEqual");
        case SpvOpFOrdLessThan:
            return String("FOrdLessThan");
        case SpvOpFUnordLessThan:
            return String("FUnordLessThan");
        case SpvOpFOrdGreaterThan:
            return String("FOrdGreaterThan");
        case SpvOpFUnordGreaterThan:
            return String("FUnordGreaterThan");
        case SpvOpFOrdLessThanEqual:
            return String("FOrdLessThanEqual");
        case SpvOpFUnordLessThanEqual:
            return String("FUnordLessThanEqual");
        case SpvOpFOrdGreaterThanEqual:
            return String("FOrdGreaterThanEqual");
        case SpvOpFUnordGreaterThanEqual:
            return String("FUnordGreaterThanEqual");
        case SpvOpShiftRightLogical:
            return String("ShiftRightLogical");
        case SpvOpShiftRightArithmetic:
            return String("ShiftRightArithmetic");
        case SpvOpShiftLeftLogical:
            return String("ShiftLeftLogical");
        case SpvOpBitwiseOr:
            return String("BitwiseOr");
        case SpvOpBitwiseXor:
            return String("BitwiseXor");
        case SpvOpBitwiseAnd:
            return String("BitwiseAnd");
        case SpvOpNot:
            return String("Not");
        case SpvOpBitFieldInsert:
            return String("BitFieldInsert");
        case SpvOpBitFieldSExtract:
            return String("BitFieldSExtract");
        case SpvOpBitFieldUExtract:
            return String("BitFieldUExtract");
        case SpvOpBitReverse:
            return String("BitReverse");
        case SpvOpBitCount:
            return String("BitCount");
        case SpvOpDPdx:
            return String("DPdx");
        case SpvOpDPdy:
            return String("DPdy");
        case SpvOpFwidth:
            return String("Fwidth");
        case SpvOpDPdxFine:
            return String("DPdxFine");
        case SpvOpDPdyFine:
            return String("DPdyFine");
        case SpvOpFwidthFine:
            return String("FwidthFine");
        case SpvOpDPdxCoarse:
            return String("DPdxCoarse");
        case SpvOpDPdyCoarse:
            return String("DPdyCoarse");
        case SpvOpFwidthCoarse:
            return String("FwidthCoarse");
        case SpvOpEmitVertex:
            return String("EmitVertex");
        case SpvOpEndPrimitive:
            return String("EndPrimitive");
        case SpvOpEmitStreamVertex:
            return String("EmitStreamVertex");
        case SpvOpEndStreamPrimitive:
            return String("EndStreamPrimitive");
        case SpvOpControlBarrier:
            return String("ControlBarrier");
        case SpvOpMemoryBarrier:
            return String("MemoryBarrier");
        case SpvOpAtomicLoad:
            return String("AtomicLoad");
        case SpvOpAtomicStore:
            return String("AtomicStore");
        case SpvOpAtomicExchange:
            return String("AtomicExchange");
        case SpvOpAtomicCompareExchange:
            return String("AtomicCompareExchange");
        case SpvOpAtomicCompareExchangeWeak:
            return String("AtomicCompareExchangeWeak");
        case SpvOpAtomicIIncrement:
            return String("AtomicIIncrement");
        case SpvOpAtomicIDecrement:
            return String("AtomicIDecrement");
        case SpvOpAtomicIAdd:
            return String("AtomicIAdd");
        case SpvOpAtomicISub:
            return String("AtomicISub");
        case SpvOpAtomicSMin:
            return String("AtomicSMin");
        case SpvOpAtomicUMin:
            return String("AtomicUMin");
        case SpvOpAtomicSMax:
            return String("AtomicSMax");
        case SpvOpAtomicUMax:
            return String("AtomicUMax");
        case SpvOpAtomicAnd:
            return String("AtomicAnd");
        case SpvOpAtomicOr:
            return String("AtomicOr");
        case SpvOpAtomicXor:
            return String("AtomicXor");
        case SpvOpPhi:
            return String("Phi");
        case SpvOpLoopMerge:
            return String("LoopMerge");
        case SpvOpSelectionMerge:
            return String("SelectionMerge");
        case SpvOpLabel:
            return String("Label");
        case SpvOpBranch:
            return String("Branch");
        case SpvOpBranchConditional:
            return String("BranchConditional");
        case SpvOpSwitch:
            return String("Switch");
        case SpvOpKill:
            return String("Kill");
        case SpvOpReturn:
            return String("Return");
        case SpvOpReturnValue:
            return String("ReturnValue");
        case SpvOpUnreachable:
            return String("Unreachable");
        case SpvOpLifetimeStart:
            return String("LifetimeStart");
        case SpvOpLifetimeStop:
            return String("LifetimeStop");
        case SpvOpGroupAsyncCopy:
            return String("GroupAsyncCopy");
        case SpvOpGroupWaitEvents:
            return String("GroupWaitEvents");
        case SpvOpGroupAll:
            return String("GroupAll");
        case SpvOpGroupAny:
            return String("GroupAny");
        case SpvOpGroupBroadcast:
            return String("GroupBroadcast");
        case SpvOpGroupIAdd:
            return String("GroupIAdd");
        case SpvOpGroupFAdd:
            return String("GroupFAdd");
        case SpvOpGroupFMin:
            return String("GroupFMin");
        case SpvOpGroupUMin:
            return String("GroupUMin");
        case SpvOpGroupSMin:
            return String("GroupSMin");
        case SpvOpGroupFMax:
            return String("GroupFMax");
        case SpvOpGroupUMax:
            return String("GroupUMax");
        case SpvOpGroupSMax:
            return String("GroupSMax");
        case SpvOpReadPipe:
            return String("ReadPipe");
        case SpvOpWritePipe:
            return String("WritePipe");
        case SpvOpReservedReadPipe:
            return String("ReservedReadPipe");
        case SpvOpReservedWritePipe:
            return String("ReservedWritePipe");
        case SpvOpReserveReadPipePackets:
            return String("ReserveReadPipePackets");
        case SpvOpReserveWritePipePackets:
            return String("ReserveWritePipePackets");
        case SpvOpCommitReadPipe:
            return String("CommitReadPipe");
        case SpvOpCommitWritePipe:
            return String("CommitWritePipe");
        case SpvOpIsValidReserveId:
            return String("IsValidReserveId");
        case SpvOpGetNumPipePackets:
            return String("GetNumPipePackets");
        case SpvOpGetMaxPipePackets:
            return String("GetMaxPipePackets");
        case SpvOpGroupReserveReadPipePackets:
            return String("GroupReserveReadPipePackets");
        case SpvOpGroupReserveWritePipePackets:
            return String("GroupReserveWritePipePackets");
        case SpvOpGroupCommitReadPipe:
            return String("GroupCommitReadPipe");
        case SpvOpGroupCommitWritePipe:
            return String("GroupCommitWritePipe");
        case SpvOpEnqueueMarker:
            return String("EnqueueMarker");
        case SpvOpEnqueueKernel:
            return String("EnqueueKernel");
        case SpvOpGetKernelNDrangeSubGroupCount:
            return String("GetKernelNDrangeSubGroupCount");
        case SpvOpGetKernelNDrangeMaxSubGroupSize:
            return String("GetKernelNDrangeMaxSubGroupSize");
        case SpvOpGetKernelWorkGroupSize:
            return String("GetKernelWorkGroupSize");
        case SpvOpGetKernelPreferredWorkGroupSizeMultiple:
            return String("GetKernelPreferredWorkGroupSizeMultiple");
        case SpvOpRetainEvent:
            return String("RetainEvent");
        case SpvOpReleaseEvent:
            return String("ReleaseEvent");
        case SpvOpCreateUserEvent:
            return String("CreateUserEvent");
        case SpvOpIsValidEvent:
            return String("IsValidEvent");
        case SpvOpSetUserEventStatus:
            return String("SetUserEventStatus");
        case SpvOpCaptureEventProfilingInfo:
            return String("CaptureEventProfilingInfo");
        case SpvOpGetDefaultQueue:
            return String("GetDefaultQueue");
        case SpvOpBuildNDRange:
            return String("BuildNDRange");
        case SpvOpImageSparseSampleImplicitLod:
            return String("ImageSparseSampleImplicitLod");
        case SpvOpImageSparseSampleExplicitLod:
            return String("ImageSparseSampleExplicitLod");
        case SpvOpImageSparseSampleDrefImplicitLod:
            return String("ImageSparseSampleDrefImplicitLod");
        case SpvOpImageSparseSampleDrefExplicitLod:
            return String("ImageSparseSampleDrefExplicitLod");
        case SpvOpImageSparseSampleProjImplicitLod:
            return String("ImageSparseSampleProjImplicitLod");
        case SpvOpImageSparseSampleProjExplicitLod:
            return String("ImageSparseSampleProjExplicitLod");
        case SpvOpImageSparseSampleProjDrefImplicitLod:
            return String("ImageSparseSampleProjDrefImplicitLod");
        case SpvOpImageSparseSampleProjDrefExplicitLod:
            return String("ImageSparseSampleProjDrefExplicitLod");
        case SpvOpImageSparseFetch:
            return String("ImageSparseFetch");
        case SpvOpImageSparseGather:
            return String("ImageSparseGather");
        case SpvOpImageSparseDrefGather:
            return String("ImageSparseDrefGather");
        case SpvOpImageSparseTexelsResident:
            return String("ImageSparseTexelsResident");
        case SpvOpNoLine:
            return String("NoLine");
        case SpvOpAtomicFlagTestAndSet:
            return String("AtomicFlagTestAndSet");
        case SpvOpAtomicFlagClear:
            return String("AtomicFlagClear");
        case SpvOpImageSparseRead:
            return String("ImageSparseRead");
        default:
            ABORT("unsupported SPIR-V op");
    }
}
#endif

void SPIRVCodeGenerator::writeOpCode(SpvOp_ opCode, int length, OutputStream& out) {
    ASSERT(opCode != SpvOpUndef);
    switch (opCode) {
        case SpvOpReturn:      // fall through
        case SpvOpReturnValue: // fall through
        case SpvOpKill:        // fall through
        case SpvOpBranch:      // fall through
        case SpvOpBranchConditional:
            ASSERT(fCurrentBlock);
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
            ASSERT(fCurrentBlock);
    }
#if SPIRV_DEBUG
    out << std::endl << opcode_text(opCode) << " ";
#else
    this->writeWord((length << 16) | opCode, out);
#endif
}

void SPIRVCodeGenerator::writeLabel(SpvId label, OutputStream& out) {
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

void SPIRVCodeGenerator::writeString(const char* string, OutputStream& out) {
    size_t length = strlen(string);
    out.write(string, length);
    switch (length % 4) {
        case 1:
            out.write8(0);
            // fall through
        case 2:
            out.write8(0);
            // fall through
        case 3:
            out.write8(0);
            break;
        default:
            this->writeWord(0, out);
    }
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, const char* string, OutputStream& out) {
    int32_t length = (int32_t) strlen(string);
    this->writeOpCode(opCode, 1 + (length + 4) / 4, out);
    this->writeString(string, out);
}


void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, const char* string,
                                          OutputStream& out) {
    int32_t length = (int32_t) strlen(string);
    this->writeOpCode(opCode, 2 + (length + 4) / 4, out);
    this->writeWord(word1, out);
    this->writeString(string, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          const char* string, OutputStream& out) {
    int32_t length = (int32_t) strlen(string);
    this->writeOpCode(opCode, 3 + (length + 4) / 4, out);
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
}

SpvId SPIRVCodeGenerator::nextId() {
    return fIdCount++;
}

void SPIRVCodeGenerator::writeStruct(const Type& type, const MemoryLayout& memoryLayout,
                                     SpvId resultId) {
    this->writeInstruction(SpvOpName, resultId, type.name().c_str(), fNameBuffer);
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
        size_t size = memoryLayout.size(*type.fields()[i].fType);
        size_t alignment = memoryLayout.alignment(*type.fields()[i].fType);
        const Layout& fieldLayout = type.fields()[i].fModifiers.fLayout;
        if (fieldLayout.fOffset >= 0) {
            if (fieldLayout.fOffset < (int) offset) {
                fErrors.error(type.fPosition,
                              "offset of field '" + type.fields()[i].fName + "' must be at "
                              "least " + to_string((int) offset));
            }
            if (fieldLayout.fOffset % alignment) {
                fErrors.error(type.fPosition,
                              "offset of field '" + type.fields()[i].fName + "' must be a multiple"
                              " of " + to_string((int) alignment));
            }
            offset = fieldLayout.fOffset;
        } else {
            size_t mod = offset % alignment;
            if (mod) {
                offset += alignment - mod;
            }
        }
        this->writeInstruction(SpvOpMemberName, resultId, i, type.fields()[i].fName.c_str(),
                               fNameBuffer);
        this->writeLayout(fieldLayout, resultId, i);
        if (type.fields()[i].fModifiers.fLayout.fBuiltin < 0) {
            this->writeInstruction(SpvOpMemberDecorate, resultId, (SpvId) i, SpvDecorationOffset,
                                   (SpvId) offset, fDecorationBuffer);
        }
        if (type.fields()[i].fType->kind() == Type::kMatrix_Kind) {
            this->writeInstruction(SpvOpMemberDecorate, resultId, i, SpvDecorationColMajor,
                                   fDecorationBuffer);
            this->writeInstruction(SpvOpMemberDecorate, resultId, i, SpvDecorationMatrixStride,
                                   (SpvId) memoryLayout.stride(*type.fields()[i].fType),
                                   fDecorationBuffer);
        }
        offset += size;
        Type::Kind kind = type.fields()[i].fType->kind();
        if ((kind == Type::kArray_Kind || kind == Type::kStruct_Kind) && offset % alignment != 0) {
            offset += alignment - offset % alignment;
        }
    }
}

SpvId SPIRVCodeGenerator::getType(const Type& type) {
    return this->getType(type, fDefaultLayout);
}

SpvId SPIRVCodeGenerator::getType(const Type& type, const MemoryLayout& layout) {
    String key = type.name() + to_string((int) layout.fStd);
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId();
        switch (type.kind()) {
            case Type::kScalar_Kind:
                if (type == *fContext.fBool_Type) {
                    this->writeInstruction(SpvOpTypeBool, result, fConstantBuffer);
                } else if (type == *fContext.fInt_Type) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 1, fConstantBuffer);
                } else if (type == *fContext.fUInt_Type) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 0, fConstantBuffer);
                } else if (type == *fContext.fFloat_Type) {
                    this->writeInstruction(SpvOpTypeFloat, result, 32, fConstantBuffer);
                } else if (type == *fContext.fDouble_Type) {
                    this->writeInstruction(SpvOpTypeFloat, result, 64, fConstantBuffer);
                } else {
                    ASSERT(false);
                }
                break;
            case Type::kVector_Kind:
                this->writeInstruction(SpvOpTypeVector, result,
                                       this->getType(type.componentType(), layout),
                                       type.columns(), fConstantBuffer);
                break;
            case Type::kMatrix_Kind:
                this->writeInstruction(SpvOpTypeMatrix, result,
                                       this->getType(index_type(fContext, type), layout),
                                       type.columns(), fConstantBuffer);
                break;
            case Type::kStruct_Kind:
                this->writeStruct(type, layout, result);
                break;
            case Type::kArray_Kind: {
                if (type.columns() > 0) {
                    IntLiteral count(fContext, Position(), type.columns());
                    this->writeInstruction(SpvOpTypeArray, result,
                                           this->getType(type.componentType(), layout),
                                           this->writeIntLiteral(count), fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(type),
                                           fDecorationBuffer);
                } else {
                    ABORT("runtime-sized arrays are not yet supported");
                    this->writeInstruction(SpvOpTypeRuntimeArray, result,
                                           this->getType(type.componentType(), layout),
                                           fConstantBuffer);
                }
                break;
            }
            case Type::kSampler_Kind: {
                SpvId image = result;
                if (SpvDimSubpassData != type.dimensions()) {
                    image = this->nextId();
                }
                this->writeInstruction(SpvOpTypeImage, image,
                                       this->getType(*fContext.fFloat_Type, layout),
                                       type.dimensions(), type.isDepth(), type.isArrayed(),
                                       type.isMultisampled(), type.isSampled() ? 1 : 2,
                                       SpvImageFormatUnknown, fConstantBuffer);
                if (SpvDimSubpassData != type.dimensions()) {
                    this->writeInstruction(SpvOpTypeSampledImage, result, image, fConstantBuffer);
                }
                break;
            }
            default:
                if (type == *fContext.fVoid_Type) {
                    this->writeInstruction(SpvOpTypeVoid, result, fConstantBuffer);
                } else {
                    ABORT("invalid type: %s", type.description().c_str());
                }
        }
        fTypeMap[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::getFunctionType(const FunctionDeclaration& function) {
    String key = function.fReturnType.description() + "(";
    String separator;
    for (size_t i = 0; i < function.fParameters.size(); i++) {
        key += separator;
        separator = ", ";
        key += function.fParameters[i]->fType.description();
    }
    key += ")";
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId();
        int32_t length = 3 + (int32_t) function.fParameters.size();
        SpvId returnType = this->getType(function.fReturnType);
        std::vector<SpvId> parameterTypes;
        for (size_t i = 0; i < function.fParameters.size(); i++) {
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
//            if (is_out(function->fParameters[i])) {
                parameterTypes.push_back(this->getPointerType(function.fParameters[i]->fType,
                                                              SpvStorageClassFunction));
//            } else {
//                parameterTypes.push_back(this->getType(function.fParameters[i]->fType));
//            }
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

SpvId SPIRVCodeGenerator::getPointerType(const Type& type, const MemoryLayout& layout,
                                         SpvStorageClass_ storageClass) {
    String key = type.description() + "*" + to_string(layout.fStd) + to_string(storageClass);
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId();
        this->writeInstruction(SpvOpTypePointer, result, storageClass,
                               this->getType(type), fConstantBuffer);
        fTypeMap[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::writeExpression(const Expression& expr, OutputStream& out) {
    switch (expr.fKind) {
        case Expression::kBinary_Kind:
            return this->writeBinaryExpression((BinaryExpression&) expr, out);
        case Expression::kBoolLiteral_Kind:
            return this->writeBoolLiteral((BoolLiteral&) expr);
        case Expression::kConstructor_Kind:
            return this->writeConstructor((Constructor&) expr, out);
        case Expression::kIntLiteral_Kind:
            return this->writeIntLiteral((IntLiteral&) expr);
        case Expression::kFieldAccess_Kind:
            return this->writeFieldAccess(((FieldAccess&) expr), out);
        case Expression::kFloatLiteral_Kind:
            return this->writeFloatLiteral(((FloatLiteral&) expr));
        case Expression::kFunctionCall_Kind:
            return this->writeFunctionCall((FunctionCall&) expr, out);
        case Expression::kPrefix_Kind:
            return this->writePrefixExpression((PrefixExpression&) expr, out);
        case Expression::kPostfix_Kind:
            return this->writePostfixExpression((PostfixExpression&) expr, out);
        case Expression::kSwizzle_Kind:
            return this->writeSwizzle((Swizzle&) expr, out);
        case Expression::kVariableReference_Kind:
            return this->writeVariableReference((VariableReference&) expr, out);
        case Expression::kTernary_Kind:
            return this->writeTernaryExpression((TernaryExpression&) expr, out);
        case Expression::kIndex_Kind:
            return this->writeIndexExpression((IndexExpression&) expr, out);
        default:
            ABORT("unsupported expression: %s", expr.description().c_str());
    }
    return -1;
}

SpvId SPIRVCodeGenerator::writeIntrinsicCall(const FunctionCall& c, OutputStream& out) {
    auto intrinsic = fIntrinsicMap.find(c.fFunction.fName);
    ASSERT(intrinsic != fIntrinsicMap.end());
    const Type& type = c.fArguments[0]->fType;
    int32_t intrinsicId;
    if (std::get<0>(intrinsic->second) == kSpecial_IntrinsicKind || is_float(fContext, type)) {
        intrinsicId = std::get<1>(intrinsic->second);
    } else if (is_signed(fContext, type)) {
        intrinsicId = std::get<2>(intrinsic->second);
    } else if (is_unsigned(fContext, type)) {
        intrinsicId = std::get<3>(intrinsic->second);
    } else if (is_bool(fContext, type)) {
        intrinsicId = std::get<4>(intrinsic->second);
    } else {
        ABORT("invalid call %s, cannot operate on '%s'", c.description().c_str(),
              type.description().c_str());
    }
    switch (std::get<0>(intrinsic->second)) {
        case kGLSL_STD_450_IntrinsicKind: {
            SpvId result = this->nextId();
            std::vector<SpvId> arguments;
            for (size_t i = 0; i < c.fArguments.size(); i++) {
                arguments.push_back(this->writeExpression(*c.fArguments[i], out));
            }
            this->writeOpCode(SpvOpExtInst, 5 + (int32_t) arguments.size(), out);
            this->writeWord(this->getType(c.fType), out);
            this->writeWord(result, out);
            this->writeWord(fGLSLExtendedInstructions, out);
            this->writeWord(intrinsicId, out);
            for (SpvId id : arguments) {
                this->writeWord(id, out);
            }
            return result;
        }
        case kSPIRV_IntrinsicKind: {
            SpvId result = this->nextId();
            std::vector<SpvId> arguments;
            for (size_t i = 0; i < c.fArguments.size(); i++) {
                arguments.push_back(this->writeExpression(*c.fArguments[i], out));
            }
            this->writeOpCode((SpvOp_) intrinsicId, 3 + (int32_t) arguments.size(), out);
            this->writeWord(this->getType(c.fType), out);
            this->writeWord(result, out);
            for (SpvId id : arguments) {
                this->writeWord(id, out);
            }
            return result;
        }
        case kSpecial_IntrinsicKind:
            return this->writeSpecialIntrinsic(c, (SpecialIntrinsic) intrinsicId, out);
        default:
            ABORT("unsupported intrinsic kind");
    }
}

SpvId SPIRVCodeGenerator::writeSpecialIntrinsic(const FunctionCall& c, SpecialIntrinsic kind,
                                                OutputStream& out) {
    SpvId result = this->nextId();
    switch (kind) {
        case kAtan_SpecialIntrinsic: {
            std::vector<SpvId> arguments;
            for (size_t i = 0; i < c.fArguments.size(); i++) {
                arguments.push_back(this->writeExpression(*c.fArguments[i], out));
            }
            this->writeOpCode(SpvOpExtInst, 5 + (int32_t) arguments.size(), out);
            this->writeWord(this->getType(c.fType), out);
            this->writeWord(result, out);
            this->writeWord(fGLSLExtendedInstructions, out);
            this->writeWord(arguments.size() == 2 ? GLSLstd450Atan2 : GLSLstd450Atan, out);
            for (SpvId id : arguments) {
                this->writeWord(id, out);
            }
            return result;
        }
        case kTexture_SpecialIntrinsic: {
            SpvOp_ op = SpvOpImageSampleImplicitLod;
            switch (c.fArguments[0]->fType.dimensions()) {
                case SpvDim1D:
                    if (c.fArguments[1]->fType == *fContext.fVec2_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        ASSERT(c.fArguments[1]->fType == *fContext.fFloat_Type);
                    }
                    break;
                case SpvDim2D:
                    if (c.fArguments[1]->fType == *fContext.fVec3_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        ASSERT(c.fArguments[1]->fType == *fContext.fVec2_Type);
                    }
                    break;
                case SpvDim3D:
                    if (c.fArguments[1]->fType == *fContext.fVec4_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        ASSERT(c.fArguments[1]->fType == *fContext.fVec3_Type);
                    }
                    break;
                case SpvDimCube:   // fall through
                case SpvDimRect:   // fall through
                case SpvDimBuffer: // fall through
                case SpvDimSubpassData:
                    break;
            }
            SpvId type = this->getType(c.fType);
            SpvId sampler = this->writeExpression(*c.fArguments[0], out);
            SpvId uv = this->writeExpression(*c.fArguments[1], out);
            if (c.fArguments.size() == 3) {
                this->writeInstruction(op, type, result, sampler, uv,
                                       SpvImageOperandsBiasMask,
                                       this->writeExpression(*c.fArguments[2], out),
                                       out);
            } else {
                ASSERT(c.fArguments.size() == 2);
                this->writeInstruction(op, type, result, sampler, uv,
                                       out);
            }
            break;
        }
        case kSubpassLoad_SpecialIntrinsic: {
            SpvId img = this->writeExpression(*c.fArguments[0], out);
            std::vector<std::unique_ptr<Expression>> args;
            args.emplace_back(new FloatLiteral(fContext, Position(), 0.0));
            args.emplace_back(new FloatLiteral(fContext, Position(), 0.0));
            Constructor ctor(Position(), *fContext.fVec2_Type, std::move(args));
            SpvId coords = this->writeConstantVector(ctor);
            if (1 == c.fArguments.size()) {
                this->writeInstruction(SpvOpImageRead,
                                       this->getType(c.fType),
                                       result,
                                       img,
                                       coords,
                                       out);
            } else {
                ASSERT(2 == c.fArguments.size());
                SpvId sample = this->writeExpression(*c.fArguments[1], out);
                this->writeInstruction(SpvOpImageRead,
                                       this->getType(c.fType),
                                       result,
                                       img,
                                       coords,
                                       SpvImageOperandsSampleMask,
                                       sample,
                                       out);
            }
            break;
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFunctionCall(const FunctionCall& c, OutputStream& out) {
    const auto& entry = fFunctionMap.find(&c.fFunction);
    if (entry == fFunctionMap.end()) {
        return this->writeIntrinsicCall(c, out);
    }
    // stores (variable, type, lvalue) pairs to extract and save after the function call is complete
    std::vector<std::tuple<SpvId, SpvId, std::unique_ptr<LValue>>> lvalues;
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.fArguments.size(); i++) {
        // id of temporary variable that we will use to hold this argument, or 0 if it is being
        // passed directly
        SpvId tmpVar;
        // if we need a temporary var to store this argument, this is the value to store in the var
        SpvId tmpValueId;
        if (is_out(*c.fFunction.fParameters[i])) {
            std::unique_ptr<LValue> lv = this->getLValue(*c.fArguments[i], out);
            SpvId ptr = lv->getPointer();
            if (ptr) {
                arguments.push_back(ptr);
                continue;
            } else {
                // lvalue cannot simply be read and written via a pointer (e.g. a swizzle). Need to
                // copy it into a temp, call the function, read the value out of the temp, and then
                // update the lvalue.
                tmpValueId = lv->load(out);
                tmpVar = this->nextId();
                lvalues.push_back(std::make_tuple(tmpVar, this->getType(c.fArguments[i]->fType),
                                  std::move(lv)));
            }
        } else {
            // see getFunctionType for an explanation of why we're always using pointer parameters
            tmpValueId = this->writeExpression(*c.fArguments[i], out);
            tmpVar = this->nextId();
        }
        this->writeInstruction(SpvOpVariable,
                               this->getPointerType(c.fArguments[i]->fType,
                                                    SpvStorageClassFunction),
                               tmpVar,
                               SpvStorageClassFunction,
                               fVariableBuffer);
        this->writeInstruction(SpvOpStore, tmpVar, tmpValueId, out);
        arguments.push_back(tmpVar);
    }
    SpvId result = this->nextId();
    this->writeOpCode(SpvOpFunctionCall, 4 + (int32_t) c.fArguments.size(), out);
    this->writeWord(this->getType(c.fType), out);
    this->writeWord(result, out);
    this->writeWord(entry->second, out);
    for (SpvId id : arguments) {
        this->writeWord(id, out);
    }
    // now that the call is complete, we may need to update some lvalues with the new values of out
    // arguments
    for (const auto& tuple : lvalues) {
        SpvId load = this->nextId();
        this->writeInstruction(SpvOpLoad, std::get<1>(tuple), load, std::get<0>(tuple), out);
        std::get<2>(tuple)->store(load, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstantVector(const Constructor& c) {
    ASSERT(c.fType.kind() == Type::kVector_Kind && c.isConstant());
    SpvId result = this->nextId();
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.fArguments.size(); i++) {
        arguments.push_back(this->writeExpression(*c.fArguments[i], fConstantBuffer));
    }
    SpvId type = this->getType(c.fType);
    if (c.fArguments.size() == 1) {
        // with a single argument, a vector will have all of its entries equal to the argument
        this->writeOpCode(SpvOpConstantComposite, 3 + c.fType.columns(), fConstantBuffer);
        this->writeWord(type, fConstantBuffer);
        this->writeWord(result, fConstantBuffer);
        for (int i = 0; i < c.fType.columns(); i++) {
            this->writeWord(arguments[0], fConstantBuffer);
        }
    } else {
        this->writeOpCode(SpvOpConstantComposite, 3 + (int32_t) c.fArguments.size(),
                          fConstantBuffer);
        this->writeWord(type, fConstantBuffer);
        this->writeWord(result, fConstantBuffer);
        for (SpvId id : arguments) {
            this->writeWord(id, fConstantBuffer);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFloatConstructor(const Constructor& c, OutputStream& out) {
    ASSERT(c.fType == *fContext.fFloat_Type);
    ASSERT(c.fArguments.size() == 1);
    ASSERT(c.fArguments[0]->fType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.fArguments[0], out);
    if (c.fArguments[0]->fType == *fContext.fInt_Type) {
        this->writeInstruction(SpvOpConvertSToF, this->getType(c.fType), result, parameter,
                               out);
    } else if (c.fArguments[0]->fType == *fContext.fUInt_Type) {
        this->writeInstruction(SpvOpConvertUToF, this->getType(c.fType), result, parameter,
                               out);
    } else if (c.fArguments[0]->fType == *fContext.fFloat_Type) {
        return parameter;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeIntConstructor(const Constructor& c, OutputStream& out) {
    ASSERT(c.fType == *fContext.fInt_Type);
    ASSERT(c.fArguments.size() == 1);
    ASSERT(c.fArguments[0]->fType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.fArguments[0], out);
    if (c.fArguments[0]->fType == *fContext.fFloat_Type) {
        this->writeInstruction(SpvOpConvertFToS, this->getType(c.fType), result, parameter,
                               out);
    } else if (c.fArguments[0]->fType == *fContext.fUInt_Type) {
        this->writeInstruction(SpvOpSatConvertUToS, this->getType(c.fType), result, parameter,
                               out);
    } else if (c.fArguments[0]->fType == *fContext.fInt_Type) {
        return parameter;
    }
    return result;
}

void SPIRVCodeGenerator::writeUniformScaleMatrix(SpvId id, SpvId diagonal, const Type& type,
                                                 OutputStream& out) {
    FloatLiteral zero(fContext, Position(), 0);
    SpvId zeroId = this->writeFloatLiteral(zero);
    std::vector<SpvId> columnIds;
    for (int column = 0; column < type.columns(); column++) {
        this->writeOpCode(SpvOpCompositeConstruct, 3 + type.rows(),
                          out);
        this->writeWord(this->getType(type.componentType().toCompound(fContext, type.rows(), 1)),
                        out);
        SpvId columnId = this->nextId();
        this->writeWord(columnId, out);
        columnIds.push_back(columnId);
        for (int row = 0; row < type.columns(); row++) {
            this->writeWord(row == column ? diagonal : zeroId, out);
        }
    }
    this->writeOpCode(SpvOpCompositeConstruct, 3 + type.columns(),
                      out);
    this->writeWord(this->getType(type), out);
    this->writeWord(id, out);
    for (SpvId id : columnIds) {
        this->writeWord(id, out);
    }
}

void SPIRVCodeGenerator::writeMatrixCopy(SpvId id, SpvId src, const Type& srcType,
                                         const Type& dstType, OutputStream& out) {
    ABORT("unimplemented");
}

SpvId SPIRVCodeGenerator::writeMatrixConstructor(const Constructor& c, OutputStream& out) {
    ASSERT(c.fType.kind() == Type::kMatrix_Kind);
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.fArguments.size(); i++) {
        arguments.push_back(this->writeExpression(*c.fArguments[i], out));
    }
    SpvId result = this->nextId();
    int rows = c.fType.rows();
    int columns = c.fType.columns();
    if (arguments.size() == 1 && c.fArguments[0]->fType.kind() == Type::kScalar_Kind) {
        this->writeUniformScaleMatrix(result, arguments[0], c.fType, out);
    } else if (arguments.size() == 1 && c.fArguments[0]->fType.kind() == Type::kMatrix_Kind) {
        this->writeMatrixCopy(result, arguments[0], c.fArguments[0]->fType, c.fType, out);
    } else {
        std::vector<SpvId> columnIds;
        int currentCount = 0;
        for (size_t i = 0; i < arguments.size(); i++) {
            if (c.fArguments[i]->fType.kind() == Type::kVector_Kind) {
                ASSERT(currentCount == 0);
                columnIds.push_back(arguments[i]);
                currentCount = 0;
            } else {
                ASSERT(c.fArguments[i]->fType.kind() == Type::kScalar_Kind);
                if (currentCount == 0) {
                    this->writeOpCode(SpvOpCompositeConstruct, 3 + c.fType.rows(), out);
                    this->writeWord(this->getType(c.fType.componentType().toCompound(fContext, rows,
                                                                                     1)),
                                    out);
                    SpvId id = this->nextId();
                    this->writeWord(id, out);
                    columnIds.push_back(id);
                }
                this->writeWord(arguments[i], out);
                currentCount = (currentCount + 1) % rows;
            }
        }
        ASSERT(columnIds.size() == (size_t) columns);
        this->writeOpCode(SpvOpCompositeConstruct, 3 + columns, out);
        this->writeWord(this->getType(c.fType), out);
        this->writeWord(result, out);
        for (SpvId id : columnIds) {
            this->writeWord(id, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeVectorConstructor(const Constructor& c, OutputStream& out) {
    ASSERT(c.fType.kind() == Type::kVector_Kind);
    if (c.isConstant()) {
        return this->writeConstantVector(c);
    }
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.fArguments.size(); i++) {
        arguments.push_back(this->writeExpression(*c.fArguments[i], out));
    }
    SpvId result = this->nextId();
    if (arguments.size() == 1 && c.fArguments[0]->fType.kind() == Type::kScalar_Kind) {
        this->writeOpCode(SpvOpCompositeConstruct, 3 + c.fType.columns(), out);
        this->writeWord(this->getType(c.fType), out);
        this->writeWord(result, out);
        for (int i = 0; i < c.fType.columns(); i++) {
            this->writeWord(arguments[0], out);
        }
    } else {
        this->writeOpCode(SpvOpCompositeConstruct, 3 + (int32_t) c.fArguments.size(), out);
        this->writeWord(this->getType(c.fType), out);
        this->writeWord(result, out);
        for (SpvId id : arguments) {
            this->writeWord(id, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstructor(const Constructor& c, OutputStream& out) {
    if (c.fType == *fContext.fFloat_Type) {
        return this->writeFloatConstructor(c, out);
    } else if (c.fType == *fContext.fInt_Type) {
        return this->writeIntConstructor(c, out);
    }
    switch (c.fType.kind()) {
        case Type::kVector_Kind:
            return this->writeVectorConstructor(c, out);
        case Type::kMatrix_Kind:
            return this->writeMatrixConstructor(c, out);
        default:
            ABORT("unsupported constructor: %s", c.description().c_str());
    }
}

SpvStorageClass_ get_storage_class(const Modifiers& modifiers) {
    if (modifiers.fFlags & Modifiers::kIn_Flag) {
        ASSERT(!modifiers.fLayout.fPushConstant);
        return SpvStorageClassInput;
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        ASSERT(!modifiers.fLayout.fPushConstant);
        return SpvStorageClassOutput;
    } else if (modifiers.fFlags & Modifiers::kUniform_Flag) {
        if (modifiers.fLayout.fPushConstant) {
            return SpvStorageClassPushConstant;
        }
        return SpvStorageClassUniform;
    } else {
        return SpvStorageClassFunction;
    }
}

SpvStorageClass_ get_storage_class(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) expr).fVariable;
            if (var.fStorage != Variable::kGlobal_Storage) {
                return SpvStorageClassFunction;
            }
            return get_storage_class(var.fModifiers);
        }
        case Expression::kFieldAccess_Kind:
            return get_storage_class(*((FieldAccess&) expr).fBase);
        case Expression::kIndex_Kind:
            return get_storage_class(*((IndexExpression&) expr).fBase);
        default:
            return SpvStorageClassFunction;
    }
}

std::vector<SpvId> SPIRVCodeGenerator::getAccessChain(const Expression& expr, OutputStream& out) {
    std::vector<SpvId> chain;
    switch (expr.fKind) {
        case Expression::kIndex_Kind: {
            IndexExpression& indexExpr = (IndexExpression&) expr;
            chain = this->getAccessChain(*indexExpr.fBase, out);
            chain.push_back(this->writeExpression(*indexExpr.fIndex, out));
            break;
        }
        case Expression::kFieldAccess_Kind: {
            FieldAccess& fieldExpr = (FieldAccess&) expr;
            chain = this->getAccessChain(*fieldExpr.fBase, out);
            IntLiteral index(fContext, Position(), fieldExpr.fFieldIndex);
            chain.push_back(this->writeIntLiteral(index));
            break;
        }
        default:
            chain.push_back(this->getLValue(expr, out)->getPointer());
    }
    return chain;
}

class PointerLValue : public SPIRVCodeGenerator::LValue {
public:
    PointerLValue(SPIRVCodeGenerator& gen, SpvId pointer, SpvId type)
    : fGen(gen)
    , fPointer(pointer)
    , fType(type) {}

    virtual SpvId getPointer() override {
        return fPointer;
    }

    virtual SpvId load(OutputStream& out) override {
        SpvId result = fGen.nextId();
        fGen.writeInstruction(SpvOpLoad, fType, result, fPointer, out);
        return result;
    }

    virtual void store(SpvId value, OutputStream& out) override {
        fGen.writeInstruction(SpvOpStore, fPointer, value, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fPointer;
    const SpvId fType;
};

class SwizzleLValue : public SPIRVCodeGenerator::LValue {
public:
    SwizzleLValue(SPIRVCodeGenerator& gen, SpvId vecPointer, const std::vector<int>& components,
                  const Type& baseType, const Type& swizzleType)
    : fGen(gen)
    , fVecPointer(vecPointer)
    , fComponents(components)
    , fBaseType(baseType)
    , fSwizzleType(swizzleType) {}

    virtual SpvId getPointer() override {
        return 0;
    }

    virtual SpvId load(OutputStream& out) override {
        SpvId base = fGen.nextId();
        fGen.writeInstruction(SpvOpLoad, fGen.getType(fBaseType), base, fVecPointer, out);
        SpvId result = fGen.nextId();
        fGen.writeOpCode(SpvOpVectorShuffle, 5 + (int32_t) fComponents.size(), out);
        fGen.writeWord(fGen.getType(fSwizzleType), out);
        fGen.writeWord(result, out);
        fGen.writeWord(base, out);
        fGen.writeWord(base, out);
        for (int component : fComponents) {
            fGen.writeWord(component, out);
        }
        return result;
    }

    virtual void store(SpvId value, OutputStream& out) override {
        // use OpVectorShuffle to mix and match the vector components. We effectively create
        // a virtual vector out of the concatenation of the left and right vectors, and then
        // select components from this virtual vector to make the result vector. For
        // instance, given:
        // vec3 L = ...;
        // vec3 R = ...;
        // L.xz = R.xy;
        // we end up with the virtual vector (L.x, L.y, L.z, R.x, R.y, R.z). Then we want
        // our result vector to look like (R.x, L.y, R.y), so we need to select indices
        // (3, 1, 4).
        SpvId base = fGen.nextId();
        fGen.writeInstruction(SpvOpLoad, fGen.getType(fBaseType), base, fVecPointer, out);
        SpvId shuffle = fGen.nextId();
        fGen.writeOpCode(SpvOpVectorShuffle, 5 + fBaseType.columns(), out);
        fGen.writeWord(fGen.getType(fBaseType), out);
        fGen.writeWord(shuffle, out);
        fGen.writeWord(base, out);
        fGen.writeWord(value, out);
        for (int i = 0; i < fBaseType.columns(); i++) {
            // current offset into the virtual vector, defaults to pulling the unmodified
            // value from the left side
            int offset = i;
            // check to see if we are writing this component
            for (size_t j = 0; j < fComponents.size(); j++) {
                if (fComponents[j] == i) {
                    // we're writing to this component, so adjust the offset to pull from
                    // the correct component of the right side instead of preserving the
                    // value from the left
                    offset = (int) (j + fBaseType.columns());
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
    const std::vector<int>& fComponents;
    const Type& fBaseType;
    const Type& fSwizzleType;
};

std::unique_ptr<SPIRVCodeGenerator::LValue> SPIRVCodeGenerator::getLValue(const Expression& expr,
                                                                          OutputStream& out) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) expr).fVariable;
            auto entry = fVariableMap.find(&var);
            ASSERT(entry != fVariableMap.end());
            return std::unique_ptr<SPIRVCodeGenerator::LValue>(new PointerLValue(
                                                                       *this,
                                                                       entry->second,
                                                                       this->getType(expr.fType)));
        }
        case Expression::kIndex_Kind: // fall through
        case Expression::kFieldAccess_Kind: {
            std::vector<SpvId> chain = this->getAccessChain(expr, out);
            SpvId member = this->nextId();
            this->writeOpCode(SpvOpAccessChain, (SpvId) (3 + chain.size()), out);
            this->writeWord(this->getPointerType(expr.fType, get_storage_class(expr)), out);
            this->writeWord(member, out);
            for (SpvId idx : chain) {
                this->writeWord(idx, out);
            }
            return std::unique_ptr<SPIRVCodeGenerator::LValue>(new PointerLValue(
                                                                       *this,
                                                                       member,
                                                                       this->getType(expr.fType)));
        }

        case Expression::kSwizzle_Kind: {
            Swizzle& swizzle = (Swizzle&) expr;
            size_t count = swizzle.fComponents.size();
            SpvId base = this->getLValue(*swizzle.fBase, out)->getPointer();
            ASSERT(base);
            if (count == 1) {
                IntLiteral index(fContext, Position(), swizzle.fComponents[0]);
                SpvId member = this->nextId();
                this->writeInstruction(SpvOpAccessChain,
                                       this->getPointerType(swizzle.fType,
                                                            get_storage_class(*swizzle.fBase)),
                                       member,
                                       base,
                                       this->writeIntLiteral(index),
                                       out);
                return std::unique_ptr<SPIRVCodeGenerator::LValue>(new PointerLValue(
                                                                       *this,
                                                                       member,
                                                                       this->getType(expr.fType)));
            } else {
                return std::unique_ptr<SPIRVCodeGenerator::LValue>(new SwizzleLValue(
                                                                              *this,
                                                                              base,
                                                                              swizzle.fComponents,
                                                                              swizzle.fBase->fType,
                                                                              expr.fType));
            }
        }

        default:
            // expr isn't actually an lvalue, create a dummy variable for it. This case happens due
            // to the need to store values in temporary variables during function calls (see
            // comments in getFunctionType); erroneous uses of rvalues as lvalues should have been
            // caught by IRGenerator
            SpvId result = this->nextId();
            SpvId type = this->getPointerType(expr.fType, SpvStorageClassFunction);
            this->writeInstruction(SpvOpVariable, type, result, SpvStorageClassFunction,
                                   fVariableBuffer);
            this->writeInstruction(SpvOpStore, result, this->writeExpression(expr, out), out);
            return std::unique_ptr<SPIRVCodeGenerator::LValue>(new PointerLValue(
                                                                       *this,
                                                                       result,
                                                                       this->getType(expr.fType)));
    }
}

SpvId SPIRVCodeGenerator::writeVariableReference(const VariableReference& ref, OutputStream& out) {
    SpvId result = this->nextId();
    auto entry = fVariableMap.find(&ref.fVariable);
    ASSERT(entry != fVariableMap.end());
    SpvId var = entry->second;
    this->writeInstruction(SpvOpLoad, this->getType(ref.fVariable.fType), result, var, out);
    if (ref.fVariable.fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN &&
        fProgram.fSettings.fFlipY) {
        // need to remap to a top-left coordinate system
        if (fRTHeightStructId == (SpvId) -1) {
            // height variable hasn't been written yet
            std::shared_ptr<SymbolTable> st(new SymbolTable(&fErrors));
            ASSERT(fRTHeightFieldIndex == (SpvId) -1);
            std::vector<Type::Field> fields;
            fields.emplace_back(Modifiers(), String(SKSL_RTHEIGHT_NAME),
                                fContext.fFloat_Type.get());
            String name("sksl_synthetic_uniforms");
            Type intfStruct(Position(), name, fields);
            Layout layout(-1, -1, 1, -1, -1, -1, -1, false, false, false,
                          Layout::Format::kUnspecified, false, Layout::kUnspecified_Primitive, -1,
                          -1);
            Variable* intfVar = new Variable(Position(),
                                             Modifiers(layout, Modifiers::kUniform_Flag),
                                             name,
                                             intfStruct,
                                             Variable::kGlobal_Storage);
            fSynthetics.takeOwnership(intfVar);
            InterfaceBlock intf(Position(), intfVar, name, String(""),
                                std::vector<std::unique_ptr<Expression>>(), st);
            fRTHeightStructId = this->writeInterfaceBlock(intf);
            fRTHeightFieldIndex = 0;
        }
        ASSERT(fRTHeightFieldIndex != (SpvId) -1);
        // write vec4(gl_FragCoord.x, u_skRTHeight - gl_FragCoord.y, 0.0, 1.0)
        SpvId xId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), xId,
                               result, 0, out);
        IntLiteral fieldIndex(fContext, Position(), fRTHeightFieldIndex);
        SpvId fieldIndexId = this->writeIntLiteral(fieldIndex);
        SpvId heightPtr = this->nextId();
        this->writeOpCode(SpvOpAccessChain, 5, out);
        this->writeWord(this->getPointerType(*fContext.fFloat_Type, SpvStorageClassUniform), out);
        this->writeWord(heightPtr, out);
        this->writeWord(fRTHeightStructId, out);
        this->writeWord(fieldIndexId, out);
        SpvId heightRead = this->nextId();
        this->writeInstruction(SpvOpLoad, this->getType(*fContext.fFloat_Type), heightRead,
                               heightPtr, out);
        SpvId rawYId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), rawYId,
                               result, 1, out);
        SpvId flippedYId = this->nextId();
        this->writeInstruction(SpvOpFSub, this->getType(*fContext.fFloat_Type), flippedYId,
                               heightRead, rawYId, out);
        FloatLiteral zero(fContext, Position(), 0.0);
        SpvId zeroId = writeFloatLiteral(zero);
        FloatLiteral one(fContext, Position(), 1.0);
        SpvId oneId = writeFloatLiteral(one);
        SpvId flipped = this->nextId();
        this->writeOpCode(SpvOpCompositeConstruct, 7, out);
        this->writeWord(this->getType(*fContext.fVec4_Type), out);
        this->writeWord(flipped, out);
        this->writeWord(xId, out);
        this->writeWord(flippedYId, out);
        this->writeWord(zeroId, out);
        this->writeWord(oneId, out);
        return flipped;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeIndexExpression(const IndexExpression& expr, OutputStream& out) {
    return getLValue(expr, out)->load(out);
}

SpvId SPIRVCodeGenerator::writeFieldAccess(const FieldAccess& f, OutputStream& out) {
    return getLValue(f, out)->load(out);
}

SpvId SPIRVCodeGenerator::writeSwizzle(const Swizzle& swizzle, OutputStream& out) {
    SpvId base = this->writeExpression(*swizzle.fBase, out);
    SpvId result = this->nextId();
    size_t count = swizzle.fComponents.size();
    if (count == 1) {
        this->writeInstruction(SpvOpCompositeExtract, this->getType(swizzle.fType), result, base,
                               swizzle.fComponents[0], out);
    } else {
        this->writeOpCode(SpvOpVectorShuffle, 5 + (int32_t) count, out);
        this->writeWord(this->getType(swizzle.fType), out);
        this->writeWord(result, out);
        this->writeWord(base, out);
        this->writeWord(base, out);
        for (int component : swizzle.fComponents) {
            this->writeWord(component, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeBinaryOperation(const Type& resultType,
                                               const Type& operandType, SpvId lhs,
                                               SpvId rhs, SpvOp_ ifFloat, SpvOp_ ifInt,
                                               SpvOp_ ifUInt, SpvOp_ ifBool, OutputStream& out) {
    SpvId result = this->nextId();
    if (is_float(fContext, operandType)) {
        this->writeInstruction(ifFloat, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_signed(fContext, operandType)) {
        this->writeInstruction(ifInt, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_unsigned(fContext, operandType)) {
        this->writeInstruction(ifUInt, this->getType(resultType), result, lhs, rhs, out);
    } else if (operandType == *fContext.fBool_Type) {
        this->writeInstruction(ifBool, this->getType(resultType), result, lhs, rhs, out);
    } else {
        ABORT("invalid operandType: %s", operandType.description().c_str());
    }
    return result;
}

bool is_assignment(Token::Kind op) {
    switch (op) {
        case Token::EQ:           // fall through
        case Token::PLUSEQ:       // fall through
        case Token::MINUSEQ:      // fall through
        case Token::STAREQ:       // fall through
        case Token::SLASHEQ:      // fall through
        case Token::PERCENTEQ:    // fall through
        case Token::SHLEQ:        // fall through
        case Token::SHREQ:        // fall through
        case Token::BITWISEOREQ:  // fall through
        case Token::BITWISEXOREQ: // fall through
        case Token::BITWISEANDEQ: // fall through
        case Token::LOGICALOREQ:  // fall through
        case Token::LOGICALXOREQ: // fall through
        case Token::LOGICALANDEQ:
            return true;
        default:
            return false;
    }
}

SpvId SPIRVCodeGenerator::foldToBool(SpvId id, const Type& operandType, OutputStream& out) {
    if (operandType.kind() == Type::kVector_Kind) {
        SpvId result = this->nextId();
        this->writeInstruction(SpvOpAll, this->getType(*fContext.fBool_Type), result, id, out);
        return result;
    }
    return id;
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const BinaryExpression& b, OutputStream& out) {
    // handle cases where we don't necessarily evaluate both LHS and RHS
    switch (b.fOperator) {
        case Token::EQ: {
            SpvId rhs = this->writeExpression(*b.fRight, out);
            this->getLValue(*b.fLeft, out)->store(rhs, out);
            return rhs;
        }
        case Token::LOGICALAND:
            return this->writeLogicalAnd(b, out);
        case Token::LOGICALOR:
            return this->writeLogicalOr(b, out);
        default:
            break;
    }

    // "normal" operators
    const Type& resultType = b.fType;
    std::unique_ptr<LValue> lvalue;
    SpvId lhs;
    if (is_assignment(b.fOperator)) {
        lvalue = this->getLValue(*b.fLeft, out);
        lhs = lvalue->load(out);
    } else {
        lvalue = nullptr;
        lhs = this->writeExpression(*b.fLeft, out);
    }
    SpvId rhs = this->writeExpression(*b.fRight, out);
    // component type we are operating on: float, int, uint
    const Type* operandType;
    // IR allows mismatched types in expressions (e.g. vec2 * float), but they need special handling
    // in SPIR-V
    if (b.fLeft->fType != b.fRight->fType) {
        if (b.fLeft->fType.kind() == Type::kVector_Kind &&
            b.fRight->fType.isNumber()) {
            // promote number to vector
            SpvId vec = this->nextId();
            this->writeOpCode(SpvOpCompositeConstruct, 3 + b.fType.columns(), out);
            this->writeWord(this->getType(resultType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < resultType.columns(); i++) {
                this->writeWord(rhs, out);
            }
            rhs = vec;
            operandType = &b.fRight->fType;
        } else if (b.fRight->fType.kind() == Type::kVector_Kind &&
                   b.fLeft->fType.isNumber()) {
            // promote number to vector
            SpvId vec = this->nextId();
            this->writeOpCode(SpvOpCompositeConstruct, 3 + b.fType.columns(), out);
            this->writeWord(this->getType(resultType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < resultType.columns(); i++) {
                this->writeWord(lhs, out);
            }
            lhs = vec;
            ASSERT(!lvalue);
            operandType = &b.fLeft->fType;
        } else if (b.fLeft->fType.kind() == Type::kMatrix_Kind) {
            SpvOp_ op;
            if (b.fRight->fType.kind() == Type::kMatrix_Kind) {
                op = SpvOpMatrixTimesMatrix;
            } else if (b.fRight->fType.kind() == Type::kVector_Kind) {
                op = SpvOpMatrixTimesVector;
            } else {
                ASSERT(b.fRight->fType.kind() == Type::kScalar_Kind);
                op = SpvOpMatrixTimesScalar;
            }
            SpvId result = this->nextId();
            this->writeInstruction(op, this->getType(b.fType), result, lhs, rhs, out);
            if (b.fOperator == Token::STAREQ) {
                lvalue->store(result, out);
            } else {
                ASSERT(b.fOperator == Token::STAR);
            }
            return result;
        } else if (b.fRight->fType.kind() == Type::kMatrix_Kind) {
            SpvId result = this->nextId();
            if (b.fLeft->fType.kind() == Type::kVector_Kind) {
                this->writeInstruction(SpvOpVectorTimesMatrix, this->getType(b.fType), result,
                                       lhs, rhs, out);
            } else {
                ASSERT(b.fLeft->fType.kind() == Type::kScalar_Kind);
                this->writeInstruction(SpvOpMatrixTimesScalar, this->getType(b.fType), result, rhs,
                                       lhs, out);
            }
            if (b.fOperator == Token::STAREQ) {
                lvalue->store(result, out);
            } else {
                ASSERT(b.fOperator == Token::STAR);
            }
            return result;
        } else {
            ABORT("unsupported binary expression: %s", b.description().c_str());
        }
    } else {
        operandType = &b.fLeft->fType;
        ASSERT(*operandType == b.fRight->fType);
    }
    switch (b.fOperator) {
        case Token::EQEQ: {
            ASSERT(resultType == *fContext.fBool_Type);
            return this->foldToBool(this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                               SpvOpFOrdEqual, SpvOpIEqual,
                                                               SpvOpIEqual, SpvOpLogicalEqual, out),
                                    *operandType, out);
        }
        case Token::NEQ:
            ASSERT(resultType == *fContext.fBool_Type);
            return this->foldToBool(this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                               SpvOpFOrdNotEqual, SpvOpINotEqual,
                                                               SpvOpINotEqual, SpvOpLogicalNotEqual,
                                                               out),
                                    *operandType, out);
        case Token::GT:
            ASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThan, SpvOpSGreaterThan,
                                              SpvOpUGreaterThan, SpvOpUndef, out);
        case Token::LT:
            ASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFOrdLessThan,
                                              SpvOpSLessThan, SpvOpULessThan, SpvOpUndef, out);
        case Token::GTEQ:
            ASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThanEqual, SpvOpSGreaterThanEqual,
                                              SpvOpUGreaterThanEqual, SpvOpUndef, out);
        case Token::LTEQ:
            ASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdLessThanEqual, SpvOpSLessThanEqual,
                                              SpvOpULessThanEqual, SpvOpUndef, out);
        case Token::PLUS:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                              SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
        case Token::MINUS:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFSub,
                                              SpvOpISub, SpvOpISub, SpvOpUndef, out);
        case Token::STAR:
            if (b.fLeft->fType.kind() == Type::kMatrix_Kind &&
                b.fRight->fType.kind() == Type::kMatrix_Kind) {
                // matrix multiply
                SpvId result = this->nextId();
                this->writeInstruction(SpvOpMatrixTimesMatrix, this->getType(resultType), result,
                                       lhs, rhs, out);
                return result;
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMul,
                                              SpvOpIMul, SpvOpIMul, SpvOpUndef, out);
        case Token::SLASH:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFDiv,
                                              SpvOpSDiv, SpvOpUDiv, SpvOpUndef, out);
        case Token::PLUSEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                                      SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
            ASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::MINUSEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFSub,
                                                      SpvOpISub, SpvOpISub, SpvOpUndef, out);
            ASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::STAREQ: {
            if (b.fLeft->fType.kind() == Type::kMatrix_Kind &&
                b.fRight->fType.kind() == Type::kMatrix_Kind) {
                // matrix multiply
                SpvId result = this->nextId();
                this->writeInstruction(SpvOpMatrixTimesMatrix, this->getType(resultType), result,
                                       lhs, rhs, out);
                ASSERT(lvalue);
                lvalue->store(result, out);
                return result;
            }
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMul,
                                                      SpvOpIMul, SpvOpIMul, SpvOpUndef, out);
            ASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::SLASHEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFDiv,
                                                      SpvOpSDiv, SpvOpUDiv, SpvOpUndef, out);
            ASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        default:
            // FIXME: missing support for some operators (bitwise, &&=, ||=, shift...)
            ABORT("unsupported binary expression: %s", b.description().c_str());
    }
}

SpvId SPIRVCodeGenerator::writeLogicalAnd(const BinaryExpression& a, OutputStream& out) {
    ASSERT(a.fOperator == Token::LOGICALAND);
    BoolLiteral falseLiteral(fContext, Position(), false);
    SpvId falseConstant = this->writeBoolLiteral(falseLiteral);
    SpvId lhs = this->writeExpression(*a.fLeft, out);
    SpvId rhsLabel = this->nextId();
    SpvId end = this->nextId();
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, rhsLabel, end, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(*a.fRight, out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId();
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fBool_Type), result, falseConstant,
                           lhsBlock, rhs, rhsBlock, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalOr(const BinaryExpression& o, OutputStream& out) {
    ASSERT(o.fOperator == Token::LOGICALOR);
    BoolLiteral trueLiteral(fContext, Position(), true);
    SpvId trueConstant = this->writeBoolLiteral(trueLiteral);
    SpvId lhs = this->writeExpression(*o.fLeft, out);
    SpvId rhsLabel = this->nextId();
    SpvId end = this->nextId();
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, end, rhsLabel, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(*o.fRight, out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId();
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fBool_Type), result, trueConstant,
                           lhsBlock, rhs, rhsBlock, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeTernaryExpression(const TernaryExpression& t, OutputStream& out) {
    SpvId test = this->writeExpression(*t.fTest, out);
    if (t.fIfTrue->isConstant() && t.fIfFalse->isConstant()) {
        // both true and false are constants, can just use OpSelect
        SpvId result = this->nextId();
        SpvId trueId = this->writeExpression(*t.fIfTrue, out);
        SpvId falseId = this->writeExpression(*t.fIfFalse, out);
        this->writeInstruction(SpvOpSelect, this->getType(t.fType), result, test, trueId, falseId,
                               out);
        return result;
    }
    // was originally using OpPhi to choose the result, but for some reason that is crashing on
    // Adreno. Switched to storing the result in a temp variable as glslang does.
    SpvId var = this->nextId();
    this->writeInstruction(SpvOpVariable, this->getPointerType(t.fType, SpvStorageClassFunction),
                           var, SpvStorageClassFunction, fVariableBuffer);
    SpvId trueLabel = this->nextId();
    SpvId falseLabel = this->nextId();
    SpvId end = this->nextId();
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, test, trueLabel, falseLabel, out);
    this->writeLabel(trueLabel, out);
    this->writeInstruction(SpvOpStore, var, this->writeExpression(*t.fIfTrue, out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(falseLabel, out);
    this->writeInstruction(SpvOpStore, var, this->writeExpression(*t.fIfFalse, out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId();
    this->writeInstruction(SpvOpLoad, this->getType(t.fType), result, var, out);
    return result;
}

std::unique_ptr<Expression> create_literal_1(const Context& context, const Type& type) {
    if (type == *context.fInt_Type) {
        return std::unique_ptr<Expression>(new IntLiteral(context, Position(), 1));
    }
    else if (type == *context.fFloat_Type) {
        return std::unique_ptr<Expression>(new FloatLiteral(context, Position(), 1.0));
    } else {
        ABORT("math is unsupported on type '%s'", type.name().c_str());
    }
}

SpvId SPIRVCodeGenerator::writePrefixExpression(const PrefixExpression& p, OutputStream& out) {
    if (p.fOperator == Token::MINUS) {
        SpvId result = this->nextId();
        SpvId typeId = this->getType(p.fType);
        SpvId expr = this->writeExpression(*p.fOperand, out);
        if (is_float(fContext, p.fType)) {
            this->writeInstruction(SpvOpFNegate, typeId, result, expr, out);
        } else if (is_signed(fContext, p.fType)) {
            this->writeInstruction(SpvOpSNegate, typeId, result, expr, out);
        } else {
            ABORT("unsupported prefix expression %s", p.description().c_str());
        };
        return result;
    }
    switch (p.fOperator) {
        case Token::PLUS:
            return this->writeExpression(*p.fOperand, out);
        case Token::PLUSPLUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.fOperand, out);
            SpvId one = this->writeExpression(*create_literal_1(fContext, p.fType), out);
            SpvId result = this->writeBinaryOperation(p.fType, p.fType, lv->load(out), one,
                                                      SpvOpFAdd, SpvOpIAdd, SpvOpIAdd, SpvOpUndef,
                                                      out);
            lv->store(result, out);
            return result;
        }
        case Token::MINUSMINUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.fOperand, out);
            SpvId one = this->writeExpression(*create_literal_1(fContext, p.fType), out);
            SpvId result = this->writeBinaryOperation(p.fType, p.fType, lv->load(out), one,
                                                      SpvOpFSub, SpvOpISub, SpvOpISub, SpvOpUndef,
                                                      out);
            lv->store(result, out);
            return result;
        }
        case Token::LOGICALNOT: {
            ASSERT(p.fOperand->fType == *fContext.fBool_Type);
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpLogicalNot, this->getType(p.fOperand->fType), result,
                                   this->writeExpression(*p.fOperand, out), out);
            return result;
        }
        case Token::BITWISENOT: {
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpNot, this->getType(p.fOperand->fType), result,
                                   this->writeExpression(*p.fOperand, out), out);
            return result;
        }
        default:
            ABORT("unsupported prefix expression: %s", p.description().c_str());
    }
}

SpvId SPIRVCodeGenerator::writePostfixExpression(const PostfixExpression& p, OutputStream& out) {
    std::unique_ptr<LValue> lv = this->getLValue(*p.fOperand, out);
    SpvId result = lv->load(out);
    SpvId one = this->writeExpression(*create_literal_1(fContext, p.fType), out);
    switch (p.fOperator) {
        case Token::PLUSPLUS: {
            SpvId temp = this->writeBinaryOperation(p.fType, p.fType, result, one, SpvOpFAdd,
                                                    SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
            lv->store(temp, out);
            return result;
        }
        case Token::MINUSMINUS: {
            SpvId temp = this->writeBinaryOperation(p.fType, p.fType, result, one, SpvOpFSub,
                                                    SpvOpISub, SpvOpISub, SpvOpUndef, out);
            lv->store(temp, out);
            return result;
        }
        default:
            ABORT("unsupported postfix expression %s", p.description().c_str());
    }
}

SpvId SPIRVCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    if (b.fValue) {
        if (fBoolTrue == 0) {
            fBoolTrue = this->nextId();
            this->writeInstruction(SpvOpConstantTrue, this->getType(b.fType), fBoolTrue,
                                   fConstantBuffer);
        }
        return fBoolTrue;
    } else {
        if (fBoolFalse == 0) {
            fBoolFalse = this->nextId();
            this->writeInstruction(SpvOpConstantFalse, this->getType(b.fType), fBoolFalse,
                                   fConstantBuffer);
        }
        return fBoolFalse;
    }
}

SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    if (i.fType == *fContext.fInt_Type) {
        auto entry = fIntConstants.find(i.fValue);
        if (entry == fIntConstants.end()) {
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpConstant, this->getType(i.fType), result, (SpvId) i.fValue,
                                   fConstantBuffer);
            fIntConstants[i.fValue] = result;
            return result;
        }
        return entry->second;
    } else {
        ASSERT(i.fType == *fContext.fUInt_Type);
        auto entry = fUIntConstants.find(i.fValue);
        if (entry == fUIntConstants.end()) {
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpConstant, this->getType(i.fType), result, (SpvId) i.fValue,
                                   fConstantBuffer);
            fUIntConstants[i.fValue] = result;
            return result;
        }
        return entry->second;
    }
}

SpvId SPIRVCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    if (f.fType == *fContext.fFloat_Type) {
        float value = (float) f.fValue;
        auto entry = fFloatConstants.find(value);
        if (entry == fFloatConstants.end()) {
            SpvId result = this->nextId();
            uint32_t bits;
            ASSERT(sizeof(bits) == sizeof(value));
            memcpy(&bits, &value, sizeof(bits));
            this->writeInstruction(SpvOpConstant, this->getType(f.fType), result, bits,
                                   fConstantBuffer);
            fFloatConstants[value] = result;
            return result;
        }
        return entry->second;
    } else {
        ASSERT(f.fType == *fContext.fDouble_Type);
        auto entry = fDoubleConstants.find(f.fValue);
        if (entry == fDoubleConstants.end()) {
            SpvId result = this->nextId();
            uint64_t bits;
            ASSERT(sizeof(bits) == sizeof(f.fValue));
            memcpy(&bits, &f.fValue, sizeof(bits));
            this->writeInstruction(SpvOpConstant, this->getType(f.fType), result,
                                   bits & 0xffffffff, bits >> 32, fConstantBuffer);
            fDoubleConstants[f.fValue] = result;
            return result;
        }
        return entry->second;
    }
}

SpvId SPIRVCodeGenerator::writeFunctionStart(const FunctionDeclaration& f, OutputStream& out) {
    SpvId result = fFunctionMap[&f];
    this->writeInstruction(SpvOpFunction, this->getType(f.fReturnType), result,
                           SpvFunctionControlMaskNone, this->getFunctionType(f), out);
    this->writeInstruction(SpvOpName, result, f.fName.c_str(), fNameBuffer);
    for (size_t i = 0; i < f.fParameters.size(); i++) {
        SpvId id = this->nextId();
        fVariableMap[f.fParameters[i]] = id;
        SpvId type;
        type = this->getPointerType(f.fParameters[i]->fType, SpvStorageClassFunction);
        this->writeInstruction(SpvOpFunctionParameter, type, id, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFunction(const FunctionDefinition& f, OutputStream& out) {
    fVariableBuffer.reset();
    SpvId result = this->writeFunctionStart(f.fDeclaration, out);
    this->writeLabel(this->nextId(), out);
    if (f.fDeclaration.fName == "main") {
        write_stringstream(fGlobalInitializersBuffer, out);
    }
    StringStream bodyBuffer;
    this->writeBlock(*f.fBody, bodyBuffer);
    write_stringstream(fVariableBuffer, out);
    write_stringstream(bodyBuffer, out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpReturn, out);
    }
    this->writeInstruction(SpvOpFunctionEnd, out);
    return result;
}

void SPIRVCodeGenerator::writeLayout(const Layout& layout, SpvId target) {
    if (layout.fLocation >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationLocation, layout.fLocation,
                               fDecorationBuffer);
    }
    if (layout.fBinding >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationBinding, layout.fBinding,
                               fDecorationBuffer);
    }
    if (layout.fIndex >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationIndex, layout.fIndex,
                               fDecorationBuffer);
    }
    if (layout.fSet >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationDescriptorSet, layout.fSet,
                               fDecorationBuffer);
    }
    if (layout.fInputAttachmentIndex >= 0) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationInputAttachmentIndex,
                               layout.fInputAttachmentIndex, fDecorationBuffer);
    }
    if (layout.fBuiltin >= 0 && layout.fBuiltin != SK_FRAGCOLOR_BUILTIN) {
        this->writeInstruction(SpvOpDecorate, target, SpvDecorationBuiltIn, layout.fBuiltin,
                               fDecorationBuffer);
    }
}

void SPIRVCodeGenerator::writeLayout(const Layout& layout, SpvId target, int member) {
    if (layout.fLocation >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationLocation,
                               layout.fLocation, fDecorationBuffer);
    }
    if (layout.fBinding >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationBinding,
                               layout.fBinding, fDecorationBuffer);
    }
    if (layout.fIndex >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationIndex,
                               layout.fIndex, fDecorationBuffer);
    }
    if (layout.fSet >= 0) {
        this->writeInstruction(SpvOpMemberDecorate, target, member, SpvDecorationDescriptorSet,
                               layout.fSet, fDecorationBuffer);
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

SpvId SPIRVCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    MemoryLayout layout = intf.fVariable.fModifiers.fLayout.fPushConstant ?
                          MemoryLayout(MemoryLayout::k430_Standard) :
                          fDefaultLayout;
    SpvId result = this->nextId();
    const Type* type = &intf.fVariable.fType;
    if (fProgram.fInputs.fRTHeight) {
        ASSERT(fRTHeightStructId == (SpvId) -1);
        ASSERT(fRTHeightFieldIndex == (SpvId) -1);
        std::vector<Type::Field> fields = type->fields();
        fRTHeightStructId = result;
        fRTHeightFieldIndex = fields.size();
        fields.emplace_back(Modifiers(), String(SKSL_RTHEIGHT_NAME), fContext.fFloat_Type.get());
        type = new Type(type->fPosition, type->name(), fields);
    }
    SpvId typeId = this->getType(*type, layout);
    this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBlock, fDecorationBuffer);
    SpvStorageClass_ storageClass = get_storage_class(intf.fVariable.fModifiers);
    SpvId ptrType = this->nextId();
    this->writeInstruction(SpvOpTypePointer, ptrType, storageClass, typeId, fConstantBuffer);
    this->writeInstruction(SpvOpVariable, ptrType, result, storageClass, fConstantBuffer);
    this->writeLayout(intf.fVariable.fModifiers.fLayout, result);
    fVariableMap[&intf.fVariable] = result;
    if (fProgram.fInputs.fRTHeight) {
        delete type;
    }
    return result;
}

#define BUILTIN_IGNORE 9999
void SPIRVCodeGenerator::writeGlobalVars(Program::Kind kind, const VarDeclarations& decl,
                                         OutputStream& out) {
    for (size_t i = 0; i < decl.fVars.size(); i++) {
        const VarDeclaration& varDecl = decl.fVars[i];
        const Variable* var = varDecl.fVar;
        // These haven't been implemented in our SPIR-V generator yet and we only currently use them
        // in the OpenGL backend.
        ASSERT(!(var->fModifiers.fFlags & (Modifiers::kReadOnly_Flag |
                                           Modifiers::kWriteOnly_Flag |
                                           Modifiers::kCoherent_Flag |
                                           Modifiers::kVolatile_Flag |
                                           Modifiers::kRestrict_Flag)));
        if (var->fModifiers.fLayout.fBuiltin == BUILTIN_IGNORE) {
            continue;
        }
        if (var->fModifiers.fLayout.fBuiltin == SK_FRAGCOLOR_BUILTIN &&
            kind != Program::kFragment_Kind) {
            continue;
        }
        if (!var->fReadCount && !var->fWriteCount &&
                !(var->fModifiers.fFlags & (Modifiers::kIn_Flag |
                                            Modifiers::kOut_Flag |
                                            Modifiers::kUniform_Flag))) {
            // variable is dead and not an input / output var (the Vulkan debug layers complain if
            // we elide an interface var, even if it's dead)
            continue;
        }
        SpvStorageClass_ storageClass;
        if (var->fModifiers.fFlags & Modifiers::kIn_Flag) {
            storageClass = SpvStorageClassInput;
        } else if (var->fModifiers.fFlags & Modifiers::kOut_Flag) {
            storageClass = SpvStorageClassOutput;
        } else if (var->fModifiers.fFlags & Modifiers::kUniform_Flag) {
            if (var->fType.kind() == Type::kSampler_Kind) {
                storageClass = SpvStorageClassUniformConstant;
            } else {
                storageClass = SpvStorageClassUniform;
            }
        } else {
            storageClass = SpvStorageClassPrivate;
        }
        SpvId id = this->nextId();
        fVariableMap[var] = id;
        SpvId type = this->getPointerType(var->fType, storageClass);
        this->writeInstruction(SpvOpVariable, type, id, storageClass, fConstantBuffer);
        this->writeInstruction(SpvOpName, id, var->fName.c_str(), fNameBuffer);
        if (var->fType.kind() == Type::kMatrix_Kind) {
            this->writeInstruction(SpvOpMemberDecorate, id, (SpvId) i, SpvDecorationColMajor,
                                   fDecorationBuffer);
            this->writeInstruction(SpvOpMemberDecorate, id, (SpvId) i, SpvDecorationMatrixStride,
                                   (SpvId) fDefaultLayout.stride(var->fType), fDecorationBuffer);
        }
        if (varDecl.fValue) {
            ASSERT(!fCurrentBlock);
            fCurrentBlock = -1;
            SpvId value = this->writeExpression(*varDecl.fValue, fGlobalInitializersBuffer);
            this->writeInstruction(SpvOpStore, id, value, fGlobalInitializersBuffer);
            fCurrentBlock = 0;
        }
        this->writeLayout(var->fModifiers.fLayout, id);
    }
}

void SPIRVCodeGenerator::writeVarDeclarations(const VarDeclarations& decl, OutputStream& out) {
    for (const auto& varDecl : decl.fVars) {
        const Variable* var = varDecl.fVar;
        // These haven't been implemented in our SPIR-V generator yet and we only currently use them
        // in the OpenGL backend.
        ASSERT(!(var->fModifiers.fFlags & (Modifiers::kReadOnly_Flag |
                                           Modifiers::kWriteOnly_Flag |
                                           Modifiers::kCoherent_Flag |
                                           Modifiers::kVolatile_Flag |
                                           Modifiers::kRestrict_Flag)));
        SpvId id = this->nextId();
        fVariableMap[var] = id;
        SpvId type = this->getPointerType(var->fType, SpvStorageClassFunction);
        this->writeInstruction(SpvOpVariable, type, id, SpvStorageClassFunction, fVariableBuffer);
        this->writeInstruction(SpvOpName, id, var->fName.c_str(), fNameBuffer);
        if (varDecl.fValue) {
            SpvId value = this->writeExpression(*varDecl.fValue, out);
            this->writeInstruction(SpvOpStore, id, value, out);
        }
    }
}

void SPIRVCodeGenerator::writeStatement(const Statement& s, OutputStream& out) {
    switch (s.fKind) {
        case Statement::kBlock_Kind:
            this->writeBlock((Block&) s, out);
            break;
        case Statement::kExpression_Kind:
            this->writeExpression(*((ExpressionStatement&) s).fExpression, out);
            break;
        case Statement::kReturn_Kind:
            this->writeReturnStatement((ReturnStatement&) s, out);
            break;
        case Statement::kVarDeclarations_Kind:
            this->writeVarDeclarations(*((VarDeclarationsStatement&) s).fDeclaration, out);
            break;
        case Statement::kIf_Kind:
            this->writeIfStatement((IfStatement&) s, out);
            break;
        case Statement::kFor_Kind:
            this->writeForStatement((ForStatement&) s, out);
            break;
        case Statement::kWhile_Kind:
            this->writeWhileStatement((WhileStatement&) s, out);
            break;
        case Statement::kDo_Kind:
            this->writeDoStatement((DoStatement&) s, out);
            break;
        case Statement::kBreak_Kind:
            this->writeInstruction(SpvOpBranch, fBreakTarget.top(), out);
            break;
        case Statement::kContinue_Kind:
            this->writeInstruction(SpvOpBranch, fContinueTarget.top(), out);
            break;
        case Statement::kDiscard_Kind:
            this->writeInstruction(SpvOpKill, out);
            break;
        default:
            ABORT("unsupported statement: %s", s.description().c_str());
    }
}

void SPIRVCodeGenerator::writeBlock(const Block& b, OutputStream& out) {
    for (size_t i = 0; i < b.fStatements.size(); i++) {
        this->writeStatement(*b.fStatements[i], out);
    }
}

void SPIRVCodeGenerator::writeIfStatement(const IfStatement& stmt, OutputStream& out) {
    SpvId test = this->writeExpression(*stmt.fTest, out);
    SpvId ifTrue = this->nextId();
    SpvId ifFalse = this->nextId();
    if (stmt.fIfFalse) {
        SpvId end = this->nextId();
        this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
        this->writeInstruction(SpvOpBranchConditional, test, ifTrue, ifFalse, out);
        this->writeLabel(ifTrue, out);
        this->writeStatement(*stmt.fIfTrue, out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, end, out);
        }
        this->writeLabel(ifFalse, out);
        this->writeStatement(*stmt.fIfFalse, out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, end, out);
        }
        this->writeLabel(end, out);
    } else {
        this->writeInstruction(SpvOpSelectionMerge, ifFalse, SpvSelectionControlMaskNone, out);
        this->writeInstruction(SpvOpBranchConditional, test, ifTrue, ifFalse, out);
        this->writeLabel(ifTrue, out);
        this->writeStatement(*stmt.fIfTrue, out);
        if (fCurrentBlock) {
            this->writeInstruction(SpvOpBranch, ifFalse, out);
        }
        this->writeLabel(ifFalse, out);
    }
}

void SPIRVCodeGenerator::writeForStatement(const ForStatement& f, OutputStream& out) {
    if (f.fInitializer) {
        this->writeStatement(*f.fInitializer, out);
    }
    SpvId header = this->nextId();
    SpvId start = this->nextId();
    SpvId body = this->nextId();
    SpvId next = this->nextId();
    fContinueTarget.push(next);
    SpvId end = this->nextId();
    fBreakTarget.push(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, out);
    this->writeInstruction(SpvOpLoopMerge, end, next, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, out);
    if (f.fTest) {
        SpvId test = this->writeExpression(*f.fTest, out);
        this->writeInstruction(SpvOpBranchConditional, test, body, end, out);
    }
    this->writeLabel(body, out);
    this->writeStatement(*f.fStatement, out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
    }
    this->writeLabel(next, out);
    if (f.fNext) {
        this->writeExpression(*f.fNext, out);
    }
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(end, out);
    fBreakTarget.pop();
    fContinueTarget.pop();
}

void SPIRVCodeGenerator::writeWhileStatement(const WhileStatement& w, OutputStream& out) {
    // We believe the while loop code below will work, but Skia doesn't actually use them and
    // adequately testing this code in the absence of Skia exercising it isn't straightforward. For
    // the time being, we just fail with an error due to the lack of testing. If you encounter this
    // message, simply remove the error call below to see whether our while loop support actually
    // works.
    fErrors.error(w.fPosition, "internal error: while loop support has been disabled in SPIR-V, "
                  "see SkSLSPIRVCodeGenerator.cpp for details");

    SpvId header = this->nextId();
    SpvId start = this->nextId();
    SpvId body = this->nextId();
    fContinueTarget.push(start);
    SpvId end = this->nextId();
    fBreakTarget.push(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, out);
    this->writeInstruction(SpvOpLoopMerge, end, start, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, out);
    SpvId test = this->writeExpression(*w.fTest, out);
    this->writeInstruction(SpvOpBranchConditional, test, body, end, out);
    this->writeLabel(body, out);
    this->writeStatement(*w.fStatement, out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, start, out);
    }
    this->writeLabel(end, out);
    fBreakTarget.pop();
    fContinueTarget.pop();
}

void SPIRVCodeGenerator::writeDoStatement(const DoStatement& d, OutputStream& out) {
    // We believe the do loop code below will work, but Skia doesn't actually use them and
    // adequately testing this code in the absence of Skia exercising it isn't straightforward. For
    // the time being, we just fail with an error due to the lack of testing. If you encounter this
    // message, simply remove the error call below to see whether our do loop support actually
    // works.
    fErrors.error(d.fPosition, "internal error: do loop support has been disabled in SPIR-V, see "
                  "SkSLSPIRVCodeGenerator.cpp for details");

    SpvId header = this->nextId();
    SpvId start = this->nextId();
    SpvId next = this->nextId();
    fContinueTarget.push(next);
    SpvId end = this->nextId();
    fBreakTarget.push(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, out);
    this->writeInstruction(SpvOpLoopMerge, end, start, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, out);
    this->writeStatement(*d.fStatement, out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
    }
    this->writeLabel(next, out);
    SpvId test = this->writeExpression(*d.fTest, out);
    this->writeInstruction(SpvOpBranchConditional, test, start, end, out);
    this->writeLabel(end, out);
    fBreakTarget.pop();
    fContinueTarget.pop();
}

void SPIRVCodeGenerator::writeReturnStatement(const ReturnStatement& r, OutputStream& out) {
    if (r.fExpression) {
        this->writeInstruction(SpvOpReturnValue, this->writeExpression(*r.fExpression, out),
                               out);
    } else {
        this->writeInstruction(SpvOpReturn, out);
    }
}

void SPIRVCodeGenerator::writeInstructions(const Program& program, OutputStream& out) {
    fGLSLExtendedInstructions = this->nextId();
    StringStream body;
    std::set<SpvId> interfaceVars;
    // assign IDs to functions
    for (size_t i = 0; i < program.fElements.size(); i++) {
        if (program.fElements[i]->fKind == ProgramElement::kFunction_Kind) {
            FunctionDefinition& f = (FunctionDefinition&) *program.fElements[i];
            fFunctionMap[&f.fDeclaration] = this->nextId();
        }
    }
    for (size_t i = 0; i < program.fElements.size(); i++) {
        if (program.fElements[i]->fKind == ProgramElement::kInterfaceBlock_Kind) {
            InterfaceBlock& intf = (InterfaceBlock&) *program.fElements[i];
            SpvId id = this->writeInterfaceBlock(intf);
            if ((intf.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) ||
                (intf.fVariable.fModifiers.fFlags & Modifiers::kOut_Flag)) {
                interfaceVars.insert(id);
            }
        }
    }
    for (size_t i = 0; i < program.fElements.size(); i++) {
        if (program.fElements[i]->fKind == ProgramElement::kVar_Kind) {
            this->writeGlobalVars(program.fKind, ((VarDeclarations&) *program.fElements[i]),
                                  body);
        }
    }
    for (size_t i = 0; i < program.fElements.size(); i++) {
        if (program.fElements[i]->fKind == ProgramElement::kFunction_Kind) {
            this->writeFunction(((FunctionDefinition&) *program.fElements[i]), body);
        }
    }
    const FunctionDeclaration* main = nullptr;
    for (auto entry : fFunctionMap) {
        if (entry.first->fName == "main") {
            main = entry.first;
        }
    }
    ASSERT(main);
    for (auto entry : fVariableMap) {
        const Variable* var = entry.first;
        if (var->fStorage == Variable::kGlobal_Storage &&
                ((var->fModifiers.fFlags & Modifiers::kIn_Flag) ||
                 (var->fModifiers.fFlags & Modifiers::kOut_Flag))) {
            interfaceVars.insert(entry.second);
        }
    }
    this->writeCapabilities(out);
    this->writeInstruction(SpvOpExtInstImport, fGLSLExtendedInstructions, "GLSL.std.450", out);
    this->writeInstruction(SpvOpMemoryModel, SpvAddressingModelLogical, SpvMemoryModelGLSL450, out);
    this->writeOpCode(SpvOpEntryPoint, (SpvId) (3 + (strlen(main->fName.c_str()) + 4) / 4) +
                      (int32_t) interfaceVars.size(), out);
    switch (program.fKind) {
        case Program::kVertex_Kind:
            this->writeWord(SpvExecutionModelVertex, out);
            break;
        case Program::kFragment_Kind:
            this->writeWord(SpvExecutionModelFragment, out);
            break;
        case Program::kGeometry_Kind:
            this->writeWord(SpvExecutionModelGeometry, out);
            break;
    }
    this->writeWord(fFunctionMap[main], out);
    this->writeString(main->fName.c_str(), out);
    for (int var : interfaceVars) {
        this->writeWord(var, out);
    }
    if (program.fKind == Program::kFragment_Kind) {
        this->writeInstruction(SpvOpExecutionMode,
                               fFunctionMap[main],
                               SpvExecutionModeOriginUpperLeft,
                               out);
    }
    for (size_t i = 0; i < program.fElements.size(); i++) {
        if (program.fElements[i]->fKind == ProgramElement::kExtension_Kind) {
            this->writeInstruction(SpvOpSourceExtension,
                                   ((Extension&) *program.fElements[i]).fName.c_str(),
                                   out);
        }
    }

    write_stringstream(fExtraGlobalsBuffer, out);
    write_stringstream(fNameBuffer, out);
    write_stringstream(fDecorationBuffer, out);
    write_stringstream(fConstantBuffer, out);
    write_stringstream(fExternalFunctionsBuffer, out);
    write_stringstream(body, out);
}

bool SPIRVCodeGenerator::generateCode() {
    ASSERT(!fErrors.errorCount());
    this->writeWord(SpvMagicNumber, *fOut);
    this->writeWord(SpvVersion, *fOut);
    this->writeWord(SKSL_MAGIC, *fOut);
    StringStream buffer;
    this->writeInstructions(fProgram, buffer);
    this->writeWord(fIdCount, *fOut);
    this->writeWord(0, *fOut); // reserved, always zero
    write_stringstream(buffer, *fOut);
    return 0 == fErrors.errorCount();
}

}
