/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSPIRVCodeGenerator.h"

#include "src/sksl/GLSL.std.450.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#endif

#define kLast_Capability SpvCapabilityMultiViewport

namespace SkSL {

static const int32_t SKSL_MAGIC  = 0x0; // FIXME: we should probably register a magic number

void SPIRVCodeGenerator::setupIntrinsics() {
#define ALL_GLSL(x) std::make_tuple(kGLSL_STD_450_IntrinsicKind, GLSLstd450 ## x, GLSLstd450 ## x, \
                                    GLSLstd450 ## x, GLSLstd450 ## x)
#define BY_TYPE_GLSL(ifFloat, ifInt, ifUInt) std::make_tuple(kGLSL_STD_450_IntrinsicKind, \
                                                             GLSLstd450 ## ifFloat, \
                                                             GLSLstd450 ## ifInt, \
                                                             GLSLstd450 ## ifUInt, \
                                                             SpvOpUndef)
#define ALL_SPIRV(x) std::make_tuple(kSPIRV_IntrinsicKind, SpvOp ## x, SpvOp ## x, SpvOp ## x, \
                                                           SpvOp ## x)
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
    fIntrinsicMap[String("inverse")]       = ALL_GLSL(MatrixInverse);
    fIntrinsicMap[String("outerProduct")]  = ALL_SPIRV(OuterProduct);
    fIntrinsicMap[String("transpose")]     = ALL_SPIRV(Transpose);
    fIntrinsicMap[String("isinf")]         = ALL_SPIRV(IsInf);
    fIntrinsicMap[String("isnan")]         = ALL_SPIRV(IsNan);
    fIntrinsicMap[String("inversesqrt")]   = ALL_GLSL(InverseSqrt);
    fIntrinsicMap[String("determinant")]   = ALL_GLSL(Determinant);
    fIntrinsicMap[String("matrixCompMult")] = SPECIAL(MatrixCompMult);
    fIntrinsicMap[String("matrixInverse")] = ALL_GLSL(MatrixInverse);
    fIntrinsicMap[String("mod")]           = SPECIAL(Mod);
    fIntrinsicMap[String("modf")]          = ALL_GLSL(Modf);
    fIntrinsicMap[String("min")]           = SPECIAL(Min);
    fIntrinsicMap[String("max")]           = SPECIAL(Max);
    fIntrinsicMap[String("clamp")]         = SPECIAL(Clamp);
    fIntrinsicMap[String("saturate")]      = SPECIAL(Saturate);
    fIntrinsicMap[String("dot")]           = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDot,
                                                             SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("mix")]           = SPECIAL(Mix);
    fIntrinsicMap[String("step")]          = SPECIAL(Step);
    fIntrinsicMap[String("smoothstep")]    = SPECIAL(SmoothStep);
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
    fIntrinsicMap[String("faceforward")] = ALL_GLSL(FaceForward);
    fIntrinsicMap[String("reflect")]     = ALL_GLSL(Reflect);
    fIntrinsicMap[String("refract")]     = ALL_GLSL(Refract);
    fIntrinsicMap[String("bitCount")]    = ALL_SPIRV(BitCount);
    fIntrinsicMap[String("findLSB")]     = ALL_GLSL(FindILsb);
    fIntrinsicMap[String("findMSB")]     = BY_TYPE_GLSL(FindSMsb, FindSMsb, FindUMsb);
    fIntrinsicMap[String("dFdx")]        = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDPdx,
                                                           SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("dFdy")]        = SPECIAL(DFdy);
    fIntrinsicMap[String("fwidth")]      = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpFwidth,
                                                           SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("makeSampler2D")] = SPECIAL(SampledImage);

    fIntrinsicMap[String("sample")]      = SPECIAL(Texture);
    fIntrinsicMap[String("subpassLoad")] = SPECIAL(SubpassLoad);

    fIntrinsicMap[String("floatBitsToInt")]  = ALL_SPIRV(Bitcast);
    fIntrinsicMap[String("floatBitsToUint")] = ALL_SPIRV(Bitcast);
    fIntrinsicMap[String("intBitsToFloat")]  = ALL_SPIRV(Bitcast);
    fIntrinsicMap[String("uintBitsToFloat")] = ALL_SPIRV(Bitcast);

    fIntrinsicMap[String("any")]              = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpUndef,
                                                                SpvOpUndef, SpvOpUndef, SpvOpAny);
    fIntrinsicMap[String("all")]              = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpUndef,
                                                                SpvOpUndef, SpvOpUndef, SpvOpAll);
    fIntrinsicMap[String("not")]              = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpUndef,
                                                                SpvOpUndef, SpvOpUndef,
                                                                SpvOpLogicalNot);
    fIntrinsicMap[String("equal")]            = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                SpvOpFOrdEqual, SpvOpIEqual,
                                                                SpvOpIEqual, SpvOpLogicalEqual);
    fIntrinsicMap[String("notEqual")]         = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                SpvOpFOrdNotEqual, SpvOpINotEqual,
                                                                SpvOpINotEqual,
                                                                SpvOpLogicalNotEqual);
    fIntrinsicMap[String("lessThan")]         = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                SpvOpFOrdLessThan, SpvOpSLessThan,
                                                                SpvOpULessThan, SpvOpUndef);
    fIntrinsicMap[String("lessThanEqual")]    = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                SpvOpFOrdLessThanEqual,
                                                                SpvOpSLessThanEqual,
                                                                SpvOpULessThanEqual,
                                                                SpvOpUndef);
    fIntrinsicMap[String("greaterThan")]      = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                SpvOpFOrdGreaterThan,
                                                                SpvOpSGreaterThan,
                                                                SpvOpUGreaterThan,
                                                                SpvOpUndef);
    fIntrinsicMap[String("greaterThanEqual")] = std::make_tuple(kSPIRV_IntrinsicKind,
                                                                SpvOpFOrdGreaterThanEqual,
                                                                SpvOpSGreaterThanEqual,
                                                                SpvOpUGreaterThanEqual,
                                                                SpvOpUndef);
    fIntrinsicMap[String("EmitVertex")]       = ALL_SPIRV(EmitVertex);
    fIntrinsicMap[String("EndPrimitive")]     = ALL_SPIRV(EndPrimitive);
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
    return type.isEnum() ||
           ((type.isScalar() || type.isVector()) && type.componentType().isSigned());
}

static bool is_unsigned(const Context& context, const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isUnsigned();
}

static bool is_bool(const Context& context, const Type& type) {
    return (type.isScalar() || type.isVector()) && type.componentType().isBoolean();
}

static bool is_out(const Variable& var) {
    return (var.modifiers().fFlags & Modifiers::kOut_Flag) != 0;
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
            SkASSERT(fCurrentBlock);
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

void SPIRVCodeGenerator::writeString(const char* string, size_t length, OutputStream& out) {
    out.write(string, length);
    switch (length % 4) {
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
    }
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, StringFragment string, OutputStream& out) {
    this->writeOpCode(opCode, 1 + (string.fLength + 4) / 4, out);
    this->writeString(string.fChars, string.fLength, out);
}


void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, StringFragment string,
                                          OutputStream& out) {
    this->writeOpCode(opCode, 2 + (string.fLength + 4) / 4, out);
    this->writeWord(word1, out);
    this->writeString(string.fChars, string.fLength, out);
}

void SPIRVCodeGenerator::writeInstruction(SpvOp_ opCode, int32_t word1, int32_t word2,
                                          StringFragment string, OutputStream& out) {
    this->writeOpCode(opCode, 3 + (string.fLength + 4) / 4, out);
    this->writeWord(word1, out);
    this->writeWord(word2, out);
    this->writeString(string.fChars, string.fLength, out);
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
    if (fProgram.fConfig->fKind == ProgramKind::kGeometry) {
        this->writeInstruction(SpvOpCapability, SpvCapabilityGeometry, out);
    }
    else {
        this->writeInstruction(SpvOpCapability, SpvCapabilityShader, out);
    }
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
            fErrors.error(type.fOffset, "type '" + field.fType->name() + "' is not permitted here");
            return;
        }
        size_t size = memoryLayout.size(*field.fType);
        size_t alignment = memoryLayout.alignment(*field.fType);
        const Layout& fieldLayout = field.fModifiers.fLayout;
        if (fieldLayout.fOffset >= 0) {
            if (fieldLayout.fOffset < (int) offset) {
                fErrors.error(type.fOffset,
                              "offset of field '" + field.fName + "' must be at "
                              "least " + to_string((int) offset));
            }
            if (fieldLayout.fOffset % alignment) {
                fErrors.error(type.fOffset,
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
        this->writeLayout(fieldLayout, resultId, i);
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
    if (type.isSigned() || type.isEnum()) {
        return *fContext.fTypes.fInt;
    }
    if (type.isUnsigned()) {
        return *fContext.fTypes.fUInt;
    }
    if (type.isMatrix() || type.isVector()) {
        if (type.componentType() == *fContext.fTypes.fHalf) {
            return fContext.fTypes.fFloat->toCompound(fContext, type.columns(), type.rows());
        }
        if (type.componentType() == *fContext.fTypes.fShort ||
            type.componentType() == *fContext.fTypes.fByte) {
            return fContext.fTypes.fInt->toCompound(fContext, type.columns(), type.rows());
        }
        if (type.componentType() == *fContext.fTypes.fUShort ||
            type.componentType() == *fContext.fTypes.fUByte) {
            return fContext.fTypes.fUInt->toCompound(fContext, type.columns(), type.rows());
        }
    }
    return type;
}

SpvId SPIRVCodeGenerator::getType(const Type& type) {
    return this->getType(type, fDefaultLayout);
}

SpvId SPIRVCodeGenerator::getType(const Type& rawType, const MemoryLayout& layout) {
    const Type& type = this->getActualType(rawType);
    String key = type.name();
    if (type.isStruct() || type.isArray()) {
        key += to_string((int)layout.fStd);
#ifdef SK_DEBUG
        SkASSERT(layout.fStd == MemoryLayout::Standard::k140_Standard ||
                 layout.fStd == MemoryLayout::Standard::k430_Standard);
        MemoryLayout::Standard otherStd = layout.fStd == MemoryLayout::Standard::k140_Standard
                                                  ? MemoryLayout::Standard::k430_Standard
                                                  : MemoryLayout::Standard::k140_Standard;
        String otherKey = type.name() + to_string((int)otherStd);
        SkASSERT(fTypeMap.find(otherKey) == fTypeMap.end());
#endif
    }
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId(nullptr);
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                if (type.isBoolean()) {
                    this->writeInstruction(SpvOpTypeBool, result, fConstantBuffer);
                } else if (type == *fContext.fTypes.fInt || type == *fContext.fTypes.fShort ||
                           type == *fContext.fTypes.fIntLiteral) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 1, fConstantBuffer);
                } else if (type == *fContext.fTypes.fUInt || type == *fContext.fTypes.fUShort) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 0, fConstantBuffer);
                } else if (type == *fContext.fTypes.fFloat || type == *fContext.fTypes.fHalf ||
                           type == *fContext.fTypes.fFloatLiteral) {
                    this->writeInstruction(SpvOpTypeFloat, result, 32, fConstantBuffer);
                } else {
                    SkASSERT(false);
                }
                break;
            case Type::TypeKind::kEnum:
                this->writeInstruction(SpvOpTypeInt, result, 32, 1, fConstantBuffer);
                break;
            case Type::TypeKind::kVector:
                this->writeInstruction(SpvOpTypeVector, result,
                                       this->getType(type.componentType(), layout),
                                       type.columns(), fConstantBuffer);
                break;
            case Type::TypeKind::kMatrix:
                this->writeInstruction(
                        SpvOpTypeMatrix,
                        result,
                        this->getType(IndexExpression::IndexType(fContext, type), layout),
                        type.columns(),
                        fConstantBuffer);
                break;
            case Type::TypeKind::kStruct:
                this->writeStruct(type, layout, result);
                break;
            case Type::TypeKind::kArray: {
                if (!MemoryLayout::LayoutIsSupported(type)) {
                    fErrors.error(type.fOffset, "type '" + type.name() + "' is not permitted here");
                    return this->nextId(nullptr);
                }
                if (type.columns() > 0) {
                    SpvId typeId = this->getType(type.componentType(), layout);
                    IntLiteral countLiteral(/*offset=*/-1, type.columns(),
                                            fContext.fTypes.fInt.get());
                    SpvId countId = this->writeIntLiteral(countLiteral);
                    this->writeInstruction(SpvOpTypeArray, result, typeId, countId,
                                           fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(type),
                                           fDecorationBuffer);
                } else {
                    // We shouldn't have any runtime-sized arrays right now
                    fErrors.error(type.fOffset, "runtime-sized arrays are not supported in SPIR-V");
                    this->writeInstruction(SpvOpTypeRuntimeArray, result,
                                           this->getType(type.componentType(), layout),
                                           fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(type),
                                           fDecorationBuffer);
                }
                break;
            }
            case Type::TypeKind::kSampler: {
                SpvId image = result;
                if (SpvDimSubpassData != type.dimensions()) {
                    image = this->getType(type.textureType(), layout);
                }
                if (SpvDimBuffer == type.dimensions()) {
                    fCapabilities |= (((uint64_t) 1) << SpvCapabilitySampledBuffer);
                }
                if (SpvDimSubpassData != type.dimensions()) {
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
                                       type.dimensions(), type.isDepth(), type.isArrayedTexture(),
                                       type.isMultisampled(), type.isSampled() ? 1 : 2,
                                       SpvImageFormatUnknown, fConstantBuffer);
                fImageTypeMap[key] = result;
                break;
            }
            default:
                if (type.isVoid()) {
                    this->writeInstruction(SpvOpTypeVoid, result, fConstantBuffer);
                } else {
                    SkDEBUGFAILF("invalid type: %s", type.description().c_str());
                }
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
//            if (is_out(function->fParameters[i])) {
                parameterTypes.push_back(this->getPointerType(parameters[i]->type(),
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
        case Expression::Kind::kBoolLiteral:
            return this->writeBoolLiteral(expr.as<BoolLiteral>());
        case Expression::Kind::kConstructor:
            return this->writeConstructor(expr.as<Constructor>(), out);
        case Expression::Kind::kIntLiteral:
            return this->writeIntLiteral(expr.as<IntLiteral>());
        case Expression::Kind::kFieldAccess:
            return this->writeFieldAccess(expr.as<FieldAccess>(), out);
        case Expression::Kind::kFloatLiteral:
            return this->writeFloatLiteral(expr.as<FloatLiteral>());
        case Expression::Kind::kFunctionCall:
            return this->writeFunctionCall(expr.as<FunctionCall>(), out);
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
    auto intrinsic = fIntrinsicMap.find(function.name());
    if (intrinsic == fIntrinsicMap.end()) {
        fErrors.error(c.fOffset, "unsupported intrinsic '" + function.description() + "'");
        return -1;
    }
    int32_t intrinsicId;
    const ExpressionArray& arguments = c.arguments();
    if (arguments.size() > 0) {
        const Type& type = arguments[0]->type();
        if (std::get<0>(intrinsic->second) == kSpecial_IntrinsicKind || is_float(fContext, type)) {
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
        case kGLSL_STD_450_IntrinsicKind: {
            SpvId result = this->nextId(nullptr);
            std::vector<SpvId> argumentIds;
            for (size_t i = 0; i < arguments.size(); i++) {
                if (function.parameters()[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
                    // TODO(skia:11052): swizzled lvalues won't work with getPointer()
                    argumentIds.push_back(this->getLValue(*arguments[i], out)->getPointer());
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
            return result;
        }
        case kSPIRV_IntrinsicKind: {
            // GLSL supports dot(float, float), but SPIR-V does not. Convert it to FMul
            if (intrinsicId == SpvOpDot && arguments[0]->type().isScalar()) {
                intrinsicId = SpvOpFMul;
            }
            SpvId result = this->nextId(nullptr);
            std::vector<SpvId> argumentIds;
            for (size_t i = 0; i < arguments.size(); i++) {
                if (function.parameters()[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
                    // TODO(skia:11052): swizzled lvalues won't work with getPointer()
                    argumentIds.push_back(this->getLValue(*arguments[i], out)->getPointer());
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
            return result;
        }
        case kSpecial_IntrinsicKind:
            return this->writeSpecialIntrinsic(c, (SpecialIntrinsic) intrinsicId, out);
        default:
            fErrors.error(c.fOffset, "unsupported intrinsic '" + function.description() + "'");
            return -1;
    }
}

std::vector<SpvId> SPIRVCodeGenerator::vectorize(const ExpressionArray& args, OutputStream& out) {
    int vectorSize = 0;
    for (const auto& a : args) {
        if (a->type().isVector()) {
            if (vectorSize) {
                SkASSERT(a->type().columns() == vectorSize);
            }
            else {
                vectorSize = a->type().columns();
            }
        }
    }
    std::vector<SpvId> result;
    result.reserve(args.size());
    for (const auto& arg : args) {
        const Type& argType = arg->type();
        SpvId raw = this->writeExpression(*arg, out);
        if (vectorSize && argType.isScalar()) {
            SpvId vector = this->nextId(&arg->type());
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vectorSize, out);
            this->writeWord(this->getType(argType.toCompound(fContext, vectorSize, 1)), out);
            this->writeWord(vector, out);
            for (int i = 0; i < vectorSize; i++) {
                this->writeWord(raw, out);
            }
            result.push_back(vector);
        } else {
            result.push_back(raw);
        }
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
            args.push_back(IntLiteral::Make(fContext, /*offset=*/-1, /*value=*/0));
            args.push_back(IntLiteral::Make(fContext, /*offset=*/-1, /*value=*/0));
            Constructor ctor(/*offset=*/-1, *fContext.fTypes.fInt2, std::move(args));
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
                    if (arg1Type == *fContext.fTypes.fFloat2) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type == *fContext.fTypes.fFloat);
                    }
                    break;
                case SpvDim2D:
                    if (arg1Type == *fContext.fTypes.fFloat3) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type == *fContext.fTypes.fFloat2);
                    }
                    break;
                case SpvDim3D:
                    if (arg1Type == *fContext.fTypes.fFloat4) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type == *fContext.fTypes.fFloat3);
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
                    FloatLiteral lodBias(/*offset=*/-1, /*value=*/-0.5,
                                         fContext.fTypes.fFloat.get());
                    this->writeInstruction(op, type, result, sampler, uv,
                                           SpvImageOperandsBiasMask,
                                           this->writeFloatLiteral(lodBias),
                                           out);
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
            if (fProgram.fConfig->fSettings.fFlipY) {
                // Flipping Y also negates the Y derivatives.
                SpvId flipped = this->nextId(&callType);
                this->writeInstruction(SpvOpFNegate, this->getType(callType), flipped, result,
                                       out);
                result = flipped;
            }
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
            this->writeGLSLExtendedInstruction(callType, result, GLSLstd450FMix, SpvOpUndef,
                                               SpvOpUndef, args, out);
            break;
        }
        case kSaturate_SpecialIntrinsic: {
            SkASSERT(arguments.size() == 1);
            ExpressionArray finalArgs;
            finalArgs.reserve_back(3);
            finalArgs.push_back(arguments[0]->clone());
            finalArgs.push_back(FloatLiteral::Make(fContext, /*offset=*/-1, /*value=*/0));
            finalArgs.push_back(FloatLiteral::Make(fContext, /*offset=*/-1, /*value=*/1));
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
            result = this->writeComponentwiseMatrixBinary(callType, lhs, rhs, SpvOpFMul, SpvOpUndef,
                                                          out);
            break;
        }
    }
    return result;
}

namespace {
struct TempVar {
    SpvId spvId;
    const Type* type;
    std::unique_ptr<SPIRVCodeGenerator::LValue> lvalue;
};
}

SpvId SPIRVCodeGenerator::writeFunctionCall(const FunctionCall& c, OutputStream& out) {
    const FunctionDeclaration& function = c.function();
    if (function.isBuiltin() && !function.definition()) {
        return this->writeIntrinsicCall(c, out);
    }
    const ExpressionArray& arguments = c.arguments();
    const auto& entry = fFunctionMap.find(&function);
    if (entry == fFunctionMap.end()) {
        fErrors.error(c.fOffset, "function '" + function.description() + "' is not defined");
        return -1;
    }
    // Temp variables are used to write back out-parameters after the function call is complete.
    std::vector<TempVar> tempVars;
    std::vector<SpvId> argumentIds;
    for (size_t i = 0; i < arguments.size(); i++) {
        // id of temporary variable that we will use to hold this argument, or 0 if it is being
        // passed directly
        SpvId tmpVar;
        // if we need a temporary var to store this argument, this is the value to store in the var
        SpvId tmpValueId;
        if (is_out(*function.parameters()[i])) {
            std::unique_ptr<LValue> lv = this->getLValue(*arguments[i], out);
            SpvId ptr = lv->getPointer();
            if (ptr != (SpvId) -1 && lv->isMemoryObjectPointer()) {
                argumentIds.push_back(ptr);
                continue;
            } else {
                // lvalue cannot simply be read and written via a pointer (e.g. a swizzle). Need to
                // copy it into a temp, call the function, read the value out of the temp, and then
                // update the lvalue.
                tmpValueId = lv->load(out);
                tmpVar = this->nextId(nullptr);
                tempVars.push_back(TempVar{tmpVar, &arguments[i]->type(), std::move(lv)});
            }
        } else {
            // See getFunctionType for an explanation of why we're always using pointer parameters.
            tmpValueId = this->writeExpression(*arguments[i], out);
            tmpVar = this->nextId(nullptr);
        }
        this->writeInstruction(SpvOpVariable,
                               this->getPointerType(arguments[i]->type(), SpvStorageClassFunction),
                               tmpVar,
                               SpvStorageClassFunction,
                               fVariableBuffer);
        this->writeInstruction(SpvOpStore, tmpVar, tmpValueId, out);
        argumentIds.push_back(tmpVar);
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
    for (const TempVar& tempVar : tempVars) {
        SpvId load = this->nextId(tempVar.type);
        this->writeInstruction(SpvOpLoad, getType(*tempVar.type), load, tempVar.spvId, out);
        tempVar.lvalue->store(load, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstantVector(const Constructor& c) {
    const Type& type = c.type();
    SkASSERT(type.isVector() && c.isCompileTimeConstant());

    // Get each of the constructor components as SPIR-V constants.
    SPIRVVectorConstant key{this->getType(type),
                            /*fValueId=*/{SpvId(-1), SpvId(-1), SpvId(-1), SpvId(-1)}};

    if (c.componentType().isFloat()) {
        for (int i = 0; i < type.columns(); i++) {
            FloatLiteral literal(c.fOffset, c.getFVecComponent(i), &c.componentType());
            key.fValueId[i] = this->writeFloatLiteral(literal);
        }
    } else if (c.componentType().isInteger()) {
        for (int i = 0; i < type.columns(); i++) {
            IntLiteral literal(c.fOffset, c.getIVecComponent(i), &c.componentType());
            key.fValueId[i] = this->writeIntLiteral(literal);
        }
    } else if (c.componentType().isBoolean()) {
        for (int i = 0; i < type.columns(); i++) {
            BoolLiteral literal(c.fOffset, c.getBVecComponent(i), &c.componentType());
            key.fValueId[i] = this->writeBoolLiteral(literal);
        }
    } else {
        SkDEBUGFAILF("unexpected vector component type: %s",
                     c.componentType().displayName().c_str());
        return SpvId(-1);
    }

    // Check to see if we've already synthesized this vector constant.
    auto [iter, newlyCreated] = fVectorConstants.insert({key, (SpvId)-1});
    if (newlyCreated) {
        // Emit an OpConstantComposite instruction for this constant.
        SpvId result = this->nextId(nullptr);
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

SpvId SPIRVCodeGenerator::writeFloatConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.arguments().size() == 1);
    SkASSERT(c.type().isFloat());
    const Expression& ctorExpr = *c.arguments()[0];
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
    SpvId result = this->nextId(nullptr);
    if (inputType.isBoolean()) {
        // Use OpSelect to convert the boolean argument to a literal 1.0 or 0.0.
        FloatLiteral one(/*offset=*/-1, /*value=*/1, fContext.fTypes.fFloat.get());
        SpvId        oneID = this->writeFloatLiteral(one);
        FloatLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fFloat.get());
        SpvId        zeroID = this->writeFloatLiteral(zero);
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

SpvId SPIRVCodeGenerator::writeIntConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.arguments().size() == 1);
    SkASSERT(c.type().isSigned());
    const Expression& ctorExpr = *c.arguments()[0];
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
    SpvId result = this->nextId(nullptr);
    if (inputType.isBoolean()) {
        // Use OpSelect to convert the boolean argument to a literal 1 or 0.
        IntLiteral one(/*offset=*/-1, /*value=*/1, fContext.fTypes.fInt.get());
        SpvId      oneID = this->writeIntLiteral(one);
        IntLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fInt.get());
        SpvId      zeroID = this->writeIntLiteral(zero);
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

SpvId SPIRVCodeGenerator::writeUIntConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.arguments().size() == 1);
    SkASSERT(c.type().isUnsigned());
    const Expression& ctorExpr = *c.arguments()[0];
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
    SpvId result = this->nextId(nullptr);
    if (inputType.isBoolean()) {
        // Use OpSelect to convert the boolean argument to a literal 1u or 0u.
        IntLiteral one(/*offset=*/-1, /*value=*/1, fContext.fTypes.fUInt.get());
        SpvId      oneID = this->writeIntLiteral(one);
        IntLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fUInt.get());
        SpvId      zeroID = this->writeIntLiteral(zero);
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

SpvId SPIRVCodeGenerator::writeBooleanConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.arguments().size() == 1);
    SkASSERT(c.type().isBoolean());
    const Expression& ctorExpr = *c.arguments()[0];
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
        IntLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fInt.get());
        SpvId      zeroID = this->writeIntLiteral(zero);
        this->writeInstruction(SpvOpINotEqual, this->getType(outputType), result,
                               inputId, zeroID, out);
    } else if (inputType.isUnsigned()) {
        // Synthesize a boolean result by comparing the input against an unsigned zero literal.
        IntLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fUInt.get());
        SpvId      zeroID = this->writeIntLiteral(zero);
        this->writeInstruction(SpvOpINotEqual, this->getType(outputType), result,
                               inputId, zeroID, out);
    } else if (inputType.isFloat()) {
        // Synthesize a boolean result by comparing the input against a floating-point zero literal.
        FloatLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fFloat.get());
        SpvId        zeroID = this->writeFloatLiteral(zero);
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
    FloatLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fFloat.get());
    SpvId zeroId = this->writeFloatLiteral(zero);
    std::vector<SpvId> columnIds;
    for (int column = 0; column < type.columns(); column++) {
        this->writeOpCode(SpvOpCompositeConstruct, 3 + type.rows(),
                          out);
        this->writeWord(this->getType(type.componentType().toCompound(fContext, type.rows(), 1)),
                        out);
        SpvId columnId = this->nextId(&type);
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
    for (SpvId columnId : columnIds) {
        this->writeWord(columnId, out);
    }
}

void SPIRVCodeGenerator::writeMatrixCopy(SpvId id, SpvId src, const Type& srcType,
                                         const Type& dstType, OutputStream& out) {
    SkASSERT(srcType.isMatrix());
    SkASSERT(dstType.isMatrix());
    SkASSERT(srcType.componentType() == dstType.componentType());
    SpvId srcColumnType = this->getType(srcType.componentType().toCompound(fContext,
                                                                           srcType.rows(),
                                                                           1));
    SpvId dstColumnType = this->getType(dstType.componentType().toCompound(fContext,
                                                                           dstType.rows(),
                                                                           1));
    SpvId zeroId;
    if (dstType.componentType() == *fContext.fTypes.fFloat) {
        FloatLiteral zero(/*offset=*/-1, /*value=*/0.0, fContext.fTypes.fFloat.get());
        zeroId = this->writeFloatLiteral(zero);
    } else if (dstType.componentType() == *fContext.fTypes.fInt) {
        IntLiteral zero(/*offset=*/-1, /*value=*/0, fContext.fTypes.fInt.get());
        zeroId = this->writeIntLiteral(zero);
    } else {
        SK_ABORT("unsupported matrix component type");
    }
    SpvId zeroColumn = 0;
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
                for (int j = 0; j < delta; ++j) {
                    this->writeWord(zeroId, out);
                }
            }
            else {
                // dst column is smaller, need to swizzle the src column
                dstColumn = this->nextId(&dstType);
                int count = dstType.rows();
                this->writeOpCode(SpvOpVectorShuffle, 5 + count, out);
                this->writeWord(dstColumnType, out);
                this->writeWord(dstColumn, out);
                this->writeWord(srcColumn, out);
                this->writeWord(srcColumn, out);
                for (int j = 0; j < count; j++) {
                    this->writeWord(j, out);
                }
            }
            columns[i] = dstColumn;
        } else {
            // we're past the end of the src matrix, need a vector of zeroes
            if (!zeroColumn) {
                zeroColumn = this->nextId(&dstType);
                this->writeOpCode(SpvOpCompositeConstruct, 3 + dstType.rows(), out);
                this->writeWord(dstColumnType, out);
                this->writeWord(zeroColumn, out);
                for (int j = 0; j < dstType.rows(); ++j) {
                    this->writeWord(zeroId, out);
                }
            }
            columns[i] = zeroColumn;
        }
    }
    this->writeOpCode(SpvOpCompositeConstruct, 3 + dstType.columns(), out);
    this->writeWord(this->getType(dstType), out);
    this->writeWord(id, out);
    for (int i = 0; i < dstType.columns(); i++) {
        this->writeWord(columns[i], out);
    }
}

void SPIRVCodeGenerator::addColumnEntry(SpvId columnType, Precision precision,
                                        std::vector<SpvId>* currentColumn,
                                        std::vector<SpvId>* columnIds,
                                        int* currentCount, int rows, SpvId entry,
                                        OutputStream& out) {
    SkASSERT(*currentCount < rows);
    ++(*currentCount);
    currentColumn->push_back(entry);
    if (*currentCount == rows) {
        *currentCount = 0;
        this->writeOpCode(SpvOpCompositeConstruct, 3 + currentColumn->size(), out);
        this->writeWord(columnType, out);
        SpvId columnId = this->nextId(precision);
        this->writeWord(columnId, out);
        columnIds->push_back(columnId);
        for (SpvId id : *currentColumn) {
            this->writeWord(id, out);
        }
        currentColumn->clear();
    }
}

SpvId SPIRVCodeGenerator::writeMatrixConstructor(const Constructor& c, OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isMatrix());
    SkASSERT(c.arguments().size() > 0);
    const Type& arg0Type = c.arguments()[0]->type();
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.arguments().size(); i++) {
        arguments.push_back(this->writeExpression(*c.arguments()[i], out));
    }
    SpvId result = this->nextId(&type);
    int rows = type.rows();
    int columns = type.columns();
    if (arguments.size() == 1 && arg0Type.isScalar()) {
        this->writeUniformScaleMatrix(result, arguments[0], type, out);
    } else if (arguments.size() == 1 && arg0Type.isMatrix()) {
        this->writeMatrixCopy(result, arguments[0], arg0Type, type, out);
    } else if (arguments.size() == 1 &&
               arg0Type.isVector()) {
        SkASSERT(type.rows() == 2 && type.columns() == 2);
        SkASSERT(arg0Type.columns() == 4);
        SpvId componentType = this->getType(type.componentType());
        SpvId v[4];
        for (int i = 0; i < 4; ++i) {
            v[i] = this->nextId(nullptr);
            this->writeInstruction(SpvOpCompositeExtract, componentType, v[i], arguments[0], i,
                                   out);
        }
        SpvId columnType = this->getType(type.componentType().toCompound(fContext, 2, 1));
        SpvId column1 = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeConstruct, columnType, column1, v[0], v[1], out);
        SpvId column2 = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeConstruct, columnType, column2, v[2], v[3], out);
        this->writeInstruction(SpvOpCompositeConstruct, this->getType(type), result, column1,
                               column2, out);
    } else {
        SpvId columnType = this->getType(type.componentType().toCompound(fContext, rows, 1));
        std::vector<SpvId> columnIds;
        // ids of vectors and scalars we have written to the current column so far
        std::vector<SpvId> currentColumn;
        // the total number of scalars represented by currentColumn's entries
        int currentCount = 0;
        Precision precision = type.highPrecision() ? Precision::kDefault : Precision::kRelaxed;
        for (size_t i = 0; i < arguments.size(); i++) {
            const Type& argType = c.arguments()[i]->type();
            if (currentCount == 0 && argType.isVector() &&
                argType.columns() == type.rows()) {
                // this is a complete column by itself
                columnIds.push_back(arguments[i]);
            } else {
                if (argType.columns() == 1) {
                    this->addColumnEntry(columnType, precision, &currentColumn, &columnIds,
                                         &currentCount, rows, arguments[i], out);
                } else {
                    SpvId componentType = this->getType(argType.componentType());
                    for (int j = 0; j < argType.columns(); ++j) {
                        SpvId swizzle = this->nextId(nullptr);
                        this->writeInstruction(SpvOpCompositeExtract, componentType, swizzle,
                                               arguments[i], j, out);
                        this->addColumnEntry(columnType, precision, &currentColumn, &columnIds,
                                             &currentCount, rows, swizzle, out);
                    }
                }
            }
        }
        SkASSERT(columnIds.size() == (size_t) columns);
        this->writeOpCode(SpvOpCompositeConstruct, 3 + columns, out);
        this->writeWord(this->getType(type), out);
        this->writeWord(result, out);
        for (SpvId id : columnIds) {
            this->writeWord(id, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeVectorConstructor(const Constructor& c, OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isVector());
    // Constructing a vector from another vector (even if it's constant) requires our general
    // case code, to deal with (possible) per-element type conversion.
    bool vectorToVector = c.arguments().size() == 1 && c.arguments()[0]->type().isVector();
    if (c.isCompileTimeConstant() && !vectorToVector) {
        return this->writeConstantVector(c);
    }
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.arguments().size(); i++) {
        const Type& argType = c.arguments()[i]->type();
        if (argType.isVector()) {
            // SPIR-V doesn't support vector(vector-of-different-type) directly, so we need to
            // extract the components and convert them in that case manually. On top of that,
            // as of this writing there's a bug in the Intel Vulkan driver where
            // OpCompositeConstruct doesn't handle vector arguments at all, so we always extract
            // vector components and pass them into OpCompositeConstruct individually.
            SpvId vec = this->writeExpression(*c.arguments()[i], out);
            const Type& src = argType.componentType();
            const Type& dst = type.componentType();
            if (c.arguments().size() == 1 && src.numberKind() == dst.numberKind()) {
                return vec;
            }

            for (int j = 0; j < argType.columns(); j++) {
                SpvId swizzle = this->nextId(nullptr);
                this->writeInstruction(SpvOpCompositeExtract, this->getType(src), swizzle, vec, j,
                                       out);
                if (dst.isFloat()) {
                    arguments.push_back(this->castScalarToFloat(swizzle, src, dst, out));
                } else if (dst.isSigned()) {
                    arguments.push_back(this->castScalarToSignedInt(swizzle, src, dst, out));
                } else if (dst.isUnsigned()) {
                    arguments.push_back(this->castScalarToUnsignedInt(swizzle, src, dst, out));
                } else if (dst.isBoolean()) {
                    arguments.push_back(this->castScalarToBoolean(swizzle, src, dst, out));
                } else {
                    arguments.push_back(swizzle);
                    fErrors.error(c.arguments()[i]->fOffset, "unsupported cast in SPIR-V: vector " +
                                                             src.description() + " to " +
                                                             dst.description());
                }
            }
        } else {
            arguments.push_back(this->writeExpression(*c.arguments()[i], out));
        }
    }
    SpvId result = this->nextId(nullptr);
    if (arguments.size() == 1 && c.arguments()[0]->type().isScalar()) {
        this->writeOpCode(SpvOpCompositeConstruct, 3 + type.columns(), out);
        this->writeWord(this->getType(type), out);
        this->writeWord(result, out);
        for (int i = 0; i < type.columns(); i++) {
            this->writeWord(arguments[0], out);
        }
    } else {
        SkASSERT(arguments.size() > 1);
        this->writeOpCode(SpvOpCompositeConstruct, 3 + (int32_t) arguments.size(), out);
        this->writeWord(this->getType(type), out);
        this->writeWord(result, out);
        for (SpvId id : arguments) {
            this->writeWord(id, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeArrayConstructor(const Constructor& c, OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isArray());
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.arguments().size(); i++) {
        arguments.push_back(this->writeExpression(*c.arguments()[i], out));
    }
    SpvId result = this->nextId(nullptr);
    this->writeOpCode(SpvOpCompositeConstruct, 3 + (int32_t) c.arguments().size(), out);
    this->writeWord(this->getType(type), out);
    this->writeWord(result, out);
    for (SpvId id : arguments) {
        this->writeWord(id, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstructor(const Constructor& c, OutputStream& out) {
    const Type& type = c.type();
    if (c.arguments().size() == 1 &&
        this->getActualType(type) == this->getActualType(c.arguments()[0]->type())) {
        return this->writeExpression(*c.arguments()[0], out);
    }
    if (type.isFloat()) {
        return this->writeFloatConstructor(c, out);
    } else if (type.isSigned()) {
        return this->writeIntConstructor(c, out);
    } else if (type.isUnsigned()) {
        return this->writeUIntConstructor(c, out);
    } else if (type.isBoolean()) {
        return this->writeBooleanConstructor(c, out);
    }
    switch (type.typeKind()) {
        case Type::TypeKind::kVector:
            return this->writeVectorConstructor(c, out);
        case Type::TypeKind::kMatrix:
            return this->writeMatrixConstructor(c, out);
        case Type::TypeKind::kArray:
            return this->writeArrayConstructor(c, out);
        default:
            fErrors.error(c.fOffset, "unsupported constructor: " + c.description());
            return -1;
    }
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
            IntLiteral index(/*offset=*/-1, fieldExpr.fieldIndex(), fContext.fTypes.fInt.get());
            chain.push_back(this->writeIntLiteral(index));
            break;
        }
        default: {
            SpvId id = this->getLValue(expr, out)->getPointer();
            SkASSERT(id != (SpvId) -1);
            chain.push_back(id);
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
        SpvId base = fGen.nextId(nullptr);
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
                IntLiteral uniformIdxLiteral{/*offset=*/-1, uniformIdx, fContext.fTypes.fInt.get()};
                SpvId memberId = this->nextId(nullptr);
                SpvId typeId = this->getPointerType(type, SpvStorageClassUniform);
                SpvId uniformIdxId = this->writeIntLiteral(uniformIdxLiteral);
                this->writeInstruction(SpvOpAccessChain, typeId, memberId, fUniformBufferId,
                                       uniformIdxId, out);
                return std::make_unique<PointerLValue>(*this, memberId,
                                                       /*isMemoryObjectPointer=*/true,
                                                       this->getType(type), precision);
            }
            SpvId typeId;
            if (var.modifiers().fLayout.fBuiltin == SK_IN_BUILTIN) {
                typeId = this->getType(*Type::MakeArrayType("sk_in", var.type().componentType(),
                                                            fSkInCount));
            } else {
                typeId = this->getType(type, this->memoryLayoutForVariable(var));
            }
            auto entry = fVariableMap.find(&var);
            SkASSERT(entry != fVariableMap.end());
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
                fErrors.error(swizzle.fOffset, "unable to retrieve lvalue from swizzle");
            }
            if (swizzle.components().size() == 1) {
                SpvId member = this->nextId(nullptr);
                SpvId typeId = this->getPointerType(type, get_storage_class(*swizzle.base()));
                IntLiteral index(/*offset=*/-1, swizzle.components()[0],
                                 fContext.fTypes.fInt.get());
                SpvId indexId = this->writeIntLiteral(index);
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
            // expr isn't actually an lvalue, create a dummy variable for it. This case happens due
            // to the need to store values in temporary variables during function calls (see
            // comments in getFunctionType); erroneous uses of rvalues as lvalues should have been
            // caught by IRGenerator
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
    SpvId result = this->getLValue(ref, out)->load(out);

    // Handle the "flipY" setting when reading sk_FragCoord.
    const Variable* variable = ref.variable();
    if (variable->modifiers().fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN &&
        fProgram.fConfig->fSettings.fFlipY) {
        // The x component never changes, so just grab it
        SpvId xId = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fTypes.fFloat), xId,
                               result, 0, out);

        // Calculate the y component which may need to be flipped
        SpvId rawYId = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fTypes.fFloat),
                               rawYId, result, 1, out);
        SpvId flippedYId = 0;
        if (fProgram.fConfig->fSettings.fFlipY) {
            // need to remap to a top-left coordinate system
            if (fRTHeightStructId == (SpvId)-1) {
                // height variable hasn't been written yet
                SkASSERT(fRTHeightFieldIndex == (SpvId)-1);
                std::vector<Type::Field> fields;
                if (fProgram.fConfig->fSettings.fRTHeightOffset < 0) {
                    fErrors.error(ref.fOffset, "RTHeightOffset is negative");
                }
                fields.emplace_back(
                        Modifiers(Layout(/*flags=*/0, /*location=*/-1,
                                         fProgram.fConfig->fSettings.fRTHeightOffset,
                                         /*binding=*/-1, /*index=*/-1, /*set=*/-1, /*builtin=*/-1,
                                         /*inputAttachmentIndex=*/-1,
                                         Layout::kUnspecified_Primitive, /*maxVertices=*/1,
                                         /*invocations=*/-1, /*marker=*/"", /*when=*/"",
                                         Layout::CType::kDefault),
                                  /*flags=*/0),
                        SKSL_RTHEIGHT_NAME, fContext.fTypes.fFloat.get());
                StringFragment name("sksl_synthetic_uniforms");
                std::unique_ptr<Type> intfStruct = Type::MakeStructType(/*offset=*/-1, name,
                                                                        fields);
                int binding = fProgram.fConfig->fSettings.fRTHeightBinding;
                if (binding == -1) {
                    fErrors.error(ref.fOffset, "layout(binding=...) is required in SPIR-V");
                }
                int set = fProgram.fConfig->fSettings.fRTHeightSet;
                if (set == -1) {
                    fErrors.error(ref.fOffset, "layout(set=...) is required in SPIR-V");
                }
                bool usePushConstants = fProgram.fConfig->fSettings.fUsePushConstants;
                int flags = usePushConstants ? Layout::Flag::kPushConstant_Flag : 0;
                Modifiers modifiers(
                        Layout(flags, /*location=*/-1, /*offset=*/-1, binding, /*index=*/-1,
                               set, /*builtin=*/-1, /*inputAttachmentIndex=*/-1,
                               Layout::kUnspecified_Primitive,
                               /*maxVertices=*/-1, /*invocations=*/-1, /*marker=*/"", /*when=*/"",
                               Layout::CType::kDefault),
                        Modifiers::kUniform_Flag);
                const Variable* intfVar = fSynthetics.takeOwnershipOfSymbol(
                        std::make_unique<Variable>(/*offset=*/-1,
                                                   fProgram.fModifiers->addToPool(modifiers),
                                                   name,
                                                   intfStruct.get(),
                                                   /*builtin=*/false,
                                                   Variable::Storage::kGlobal));
                InterfaceBlock intf(/*offset=*/-1, intfVar, name,
                                    /*instanceName=*/"", /*arraySize=*/0,
                                    std::make_shared<SymbolTable>(&fErrors, /*builtin=*/false));

                fRTHeightStructId = this->writeInterfaceBlock(intf, false);
                fRTHeightFieldIndex = 0;
                fRTHeightStorageClass = usePushConstants ? SpvStorageClassPushConstant
                                                         : SpvStorageClassUniform;
            }
            SkASSERT(fRTHeightFieldIndex != (SpvId)-1);

            IntLiteral fieldIndex(/*offset=*/-1, fRTHeightFieldIndex, fContext.fTypes.fInt.get());
            SpvId fieldIndexId = this->writeIntLiteral(fieldIndex);
            SpvId heightPtr = this->nextId(nullptr);
            this->writeOpCode(SpvOpAccessChain, 5, out);
            this->writeWord(this->getPointerType(*fContext.fTypes.fFloat, fRTHeightStorageClass),
                            out);
            this->writeWord(heightPtr, out);
            this->writeWord(fRTHeightStructId, out);
            this->writeWord(fieldIndexId, out);
            SpvId heightRead = this->nextId(nullptr);
            this->writeInstruction(SpvOpLoad, this->getType(*fContext.fTypes.fFloat), heightRead,
                                   heightPtr, out);

            flippedYId = this->nextId(nullptr);
            this->writeInstruction(SpvOpFSub, this->getType(*fContext.fTypes.fFloat), flippedYId,
                                   heightRead, rawYId, out);
        }

        // The z component will always be zero so we just get an id to the 0 literal
        FloatLiteral zero(/*offset=*/-1, /*value=*/0.0, fContext.fTypes.fFloat.get());
        SpvId zeroId = writeFloatLiteral(zero);

        // Calculate the w component
        SpvId rawWId = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fTypes.fFloat),
                               rawWId, result, 3, out);

        // Fill in the new fragcoord with the components from above
        SpvId adjusted = this->nextId(nullptr);
        this->writeOpCode(SpvOpCompositeConstruct, 7, out);
        this->writeWord(this->getType(*fContext.fTypes.fFloat4), out);
        this->writeWord(adjusted, out);
        this->writeWord(xId, out);
        if (fProgram.fConfig->fSettings.fFlipY) {
            this->writeWord(flippedYId, out);
        } else {
            this->writeWord(rawYId, out);
        }
        this->writeWord(zeroId, out);
        this->writeWord(rawWId, out);

        return adjusted;
    }

    // Handle the "flipY" setting when reading sk_Clockwise.
    if (variable->modifiers().fLayout.fBuiltin == SK_CLOCKWISE_BUILTIN &&
        !fProgram.fConfig->fSettings.fFlipY) {
        // FrontFacing in Vulkan is defined in terms of a top-down render target. In skia, we use
        // the default convention of "counter-clockwise face is front".
        SpvId inverse = this->nextId(nullptr);
        this->writeInstruction(SpvOpLogicalNot, this->getType(*fContext.fTypes.fBool), inverse,
                               result, out);
        return inverse;
    }

    return result;
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
    SpvId result = this->nextId(nullptr);
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
        fErrors.error(operandType.fOffset,
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
                                                         SpvId rhs, SpvOp_ floatOperator,
                                                         SpvOp_ intOperator,
                                                         OutputStream& out) {
    SpvOp_ op = is_float(fContext, operandType) ? floatOperator : intOperator;
    SkASSERT(operandType.isMatrix());
    SpvId columnType = this->getType(operandType.componentType().toCompound(fContext,
                                                                            operandType.rows(),
                                                                            1));
    SpvId columns[4];
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId columnL = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnL, lhs, i, out);
        SpvId columnR = this->nextId(nullptr);
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnR, rhs, i, out);
        columns[i] = this->nextId(nullptr);
        this->writeInstruction(op, columnType, columns[i], columnL, columnR, out);
    }
    SpvId result = this->nextId(nullptr);
    this->writeOpCode(SpvOpCompositeConstruct, 3 + operandType.columns(), out);
    this->writeWord(this->getType(operandType), out);
    this->writeWord(result, out);
    for (int i = 0; i < operandType.columns(); i++) {
        this->writeWord(columns[i], out);
    }
    return result;
}

static std::unique_ptr<Expression> create_literal_1(const Context& context, const Type& type) {
    if (type.isInteger()) {
        return IntLiteral::Make(/*offset=*/-1, /*value=*/1, &type);
    }
    else if (type.isFloat()) {
        return FloatLiteral::Make(/*offset=*/-1, /*value=*/1.0, &type);
    } else {
        SK_ABORT("math is unsupported on type '%s'", String(type.name()).c_str());
    }
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
    if (this->getActualType(leftType) != this->getActualType(rightType)) {
        if (leftType.isVector() && rightType.isNumber()) {
            if (op.kind() == Token::Kind::TK_SLASH) {
                SpvId one = this->writeExpression(*create_literal_1(fContext, rightType), out);
                SpvId inverse = this->nextId(nullptr);
                this->writeInstruction(SpvOpFDiv, this->getType(rightType), inverse, one, rhs, out);
                rhs = inverse;
                op = Token::Kind::TK_STAR;
            }
            if (op.kind() == Token::Kind::TK_STAR) {
                SpvId result = this->nextId(nullptr);
                this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                       result, lhs, rhs, out);
                return result;
            }
            // promote number to vector
            SpvId vec = this->nextId(nullptr);
            const Type& vecType = leftType;
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vecType.columns(), out);
            this->writeWord(this->getType(vecType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < vecType.columns(); i++) {
                this->writeWord(rhs, out);
            }
            rhs = vec;
            operandType = &leftType;
        } else if (rightType.isVector() && leftType.isNumber()) {
            if (op.kind() == Token::Kind::TK_STAR) {
                SpvId result = this->nextId(nullptr);
                this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                       result, rhs, lhs, out);
                return result;
            }
            // promote number to vector
            SpvId vec = this->nextId(nullptr);
            const Type& vecType = rightType;
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vecType.columns(), out);
            this->writeWord(this->getType(vecType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < vecType.columns(); i++) {
                this->writeWord(lhs, out);
            }
            lhs = vec;
            operandType = &rightType;
        } else if (leftType.isMatrix()) {
            SpvOp_ spvop;
            if (rightType.isMatrix()) {
                spvop = SpvOpMatrixTimesMatrix;
            } else if (rightType.isVector()) {
                spvop = SpvOpMatrixTimesVector;
            } else {
                SkASSERT(rightType.isScalar());
                spvop = SpvOpMatrixTimesScalar;
            }
            SpvId result = this->nextId(nullptr);
            this->writeInstruction(spvop, this->getType(resultType), result, lhs, rhs, out);
            return result;
        } else if (rightType.isMatrix()) {
            SpvId result = this->nextId(nullptr);
            if (leftType.isVector()) {
                this->writeInstruction(SpvOpVectorTimesMatrix, this->getType(resultType), result,
                                       lhs, rhs, out);
            } else {
                SkASSERT(leftType.isScalar());
                this->writeInstruction(SpvOpMatrixTimesScalar, this->getType(resultType), result,
                                       rhs, lhs, out);
            }
            return result;
        } else {
            fErrors.error(leftType.fOffset, "unsupported mixed-type expression");
            return -1;
        }
    } else {
        operandType = &this->getActualType(leftType);
        SkASSERT(*operandType == this->getActualType(rightType));
    }
    switch (op.kind()) {
        case Token::Kind::TK_EQEQ: {
            if (operandType->isMatrix()) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFOrdEqual,
                                                   SpvOpIEqual, SpvOpAll, SpvOpLogicalAnd, out);
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
                SkASSERT(leftType == rightType);
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs,
                                                            SpvOpFAdd, SpvOpIAdd, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                              SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
        case Token::Kind::TK_MINUS:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                SkASSERT(leftType == rightType);
                return this->writeComponentwiseMatrixBinary(leftType, lhs, rhs,
                                                            SpvOpFSub, SpvOpISub, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFSub,
                                              SpvOpISub, SpvOpISub, SpvOpUndef, out);
        case Token::Kind::TK_STAR:
            if (leftType.isMatrix() && rightType.isMatrix()) {
                // matrix multiply
                SpvId result = this->nextId(nullptr);
                this->writeInstruction(SpvOpMatrixTimesMatrix, this->getType(resultType), result,
                                       lhs, rhs, out);
                return result;
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMul,
                                              SpvOpIMul, SpvOpIMul, SpvOpUndef, out);
        case Token::Kind::TK_SLASH:
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
            fErrors.error(0, "unsupported token");
            return -1;
    }
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const BinaryExpression& b, OutputStream& out) {
    const Expression& left = *b.left();
    const Expression& right = *b.right();
    Operator op = b.getOperator();
    // handle cases where we don't necessarily evaluate both LHS and RHS
    switch (op.kind()) {
        case Token::Kind::TK_EQ: {
            SpvId rhs = this->writeExpression(right, out);
            this->getLValue(left, out)->store(rhs, out);
            return rhs;
        }
        case Token::Kind::TK_LOGICALAND:
            return this->writeLogicalAnd(b, out);
        case Token::Kind::TK_LOGICALOR:
            return this->writeLogicalOr(b, out);
        default:
            break;
    }

    std::unique_ptr<LValue> lvalue;
    SpvId lhs;
    if (op.isAssignment()) {
        lvalue = this->getLValue(left, out);
        lhs = lvalue->load(out);
    } else {
        lvalue = nullptr;
        lhs = this->writeExpression(left, out);
    }
    SpvId rhs = this->writeExpression(right, out);
    SpvId result = this->writeBinaryExpression(left.type(), lhs, op.removeAssignment(),
                                               right.type(), rhs, b.type(), out);
    if (lvalue) {
        lvalue->store(result, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalAnd(const BinaryExpression& a, OutputStream& out) {
    SkASSERT(a.getOperator().kind() == Token::Kind::TK_LOGICALAND);
    BoolLiteral falseLiteral(/*offset=*/-1, /*value=*/false, fContext.fTypes.fBool.get());
    SpvId falseConstant = this->writeBoolLiteral(falseLiteral);
    SpvId lhs = this->writeExpression(*a.left(), out);
    SpvId rhsLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, rhsLabel, end, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(*a.right(), out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId(nullptr);
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fTypes.fBool), result, falseConstant,
                           lhsBlock, rhs, rhsBlock, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalOr(const BinaryExpression& o, OutputStream& out) {
    SkASSERT(o.getOperator().kind() == Token::Kind::TK_LOGICALOR);
    BoolLiteral trueLiteral(/*offset=*/-1, /*value=*/true, fContext.fTypes.fBool.get());
    SpvId trueConstant = this->writeBoolLiteral(trueLiteral);
    SpvId lhs = this->writeExpression(*o.left(), out);
    SpvId rhsLabel = this->nextId(nullptr);
    SpvId end = this->nextId(nullptr);
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, end, rhsLabel, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(*o.right(), out);
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
        } else if (is_signed(fContext, type)) {
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
            SpvId one = this->writeExpression(*create_literal_1(fContext, type), out);
            SpvId result = this->writeBinaryOperation(type, type, lv->load(out), one,
                                                      SpvOpFAdd, SpvOpIAdd, SpvOpIAdd, SpvOpUndef,
                                                      out);
            lv->store(result, out);
            return result;
        }
        case Token::Kind::TK_MINUSMINUS: {
            std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
            SpvId one = this->writeExpression(*create_literal_1(fContext, type), out);
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
    SpvId one = this->writeExpression(*create_literal_1(fContext, type), out);
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

SpvId SPIRVCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    if (b.value()) {
        if (fBoolTrue == 0) {
            fBoolTrue = this->nextId(nullptr);
            this->writeInstruction(SpvOpConstantTrue, this->getType(b.type()), fBoolTrue,
                                   fConstantBuffer);
        }
        return fBoolTrue;
    } else {
        if (fBoolFalse == 0) {
            fBoolFalse = this->nextId(nullptr);
            this->writeInstruction(SpvOpConstantFalse, this->getType(b.type()), fBoolFalse,
                                   fConstantBuffer);
        }
        return fBoolFalse;
    }
}

SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    SPIRVNumberConstant key{i.value(), i.type().numberKind()};
    auto [iter, newlyCreated] = fNumberConstants.insert({key, (SpvId)-1});
    if (newlyCreated) {
        SpvId result = this->nextId(nullptr);
        this->writeInstruction(SpvOpConstant, this->getType(i.type()), result, (SpvId) i.value(),
                               fConstantBuffer);
        iter->second = result;
    }
    return iter->second;
}

SpvId SPIRVCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    // Convert the float literal into its bit-representation.
    float value = f.value();
    uint32_t valueBits;
    static_assert(sizeof(valueBits) == sizeof(value));
    memcpy(&valueBits, &value, sizeof(value));

    SPIRVNumberConstant key{valueBits, f.type().numberKind()};
    auto [iter, newlyCreated] = fNumberConstants.insert({key, (SpvId)-1});
    if (newlyCreated) {
        SpvId result = this->nextId(nullptr);
        this->writeInstruction(SpvOpConstant, this->getType(f.type()), result, (SpvId) valueBits,
                               fConstantBuffer);
        iter->second = result;
    }
    return iter->second;
}

SpvId SPIRVCodeGenerator::writeFunctionStart(const FunctionDeclaration& f, OutputStream& out) {
    SpvId result = fFunctionMap[&f];
    SpvId returnTypeId = this->getType(f.returnType());
    SpvId functionTypeId = this->getFunctionType(f);
    this->writeInstruction(SpvOpFunction, returnTypeId, result,
                           SpvFunctionControlMaskNone, functionTypeId, out);
    this->writeInstruction(SpvOpName, result, f.name(), fNameBuffer);
    const std::vector<const Variable*>& parameters = f.parameters();
    for (size_t i = 0; i < parameters.size(); i++) {
        SpvId id = this->nextId(nullptr);
        fVariableMap[parameters[i]] = id;
        SpvId type;
        type = this->getPointerType(parameters[i]->type(), SpvStorageClassFunction);
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
    if (f.declaration().name() == "main") {
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
        fCapabilities |= (((uint64_t) 1) << SpvCapabilityInputAttachment);
    }
    if (layout.fBuiltin >= 0 && layout.fBuiltin != SK_FRAGCOLOR_BUILTIN &&
        layout.fBuiltin != SK_IN_BUILTIN && layout.fBuiltin != SK_OUT_BUILTIN) {
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

MemoryLayout SPIRVCodeGenerator::memoryLayoutForVariable(const Variable& v) const {
    bool pushConstant = ((v.modifiers().fLayout.fFlags & Layout::kPushConstant_Flag) != 0);
    return pushConstant ? MemoryLayout(MemoryLayout::k430_Standard) : fDefaultLayout;
}

static void update_sk_in_count(const Modifiers& m, int* outSkInCount) {
    switch (m.fLayout.fPrimitive) {
        case Layout::kPoints_Primitive:
            *outSkInCount = 1;
            break;
        case Layout::kLines_Primitive:
            *outSkInCount = 2;
            break;
        case Layout::kLinesAdjacency_Primitive:
            *outSkInCount = 4;
            break;
        case Layout::kTriangles_Primitive:
            *outSkInCount = 3;
            break;
        case Layout::kTrianglesAdjacency_Primitive:
            *outSkInCount = 6;
            break;
        default:
            return;
    }
}

SpvId SPIRVCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf, bool appendRTHeight) {
    MemoryLayout memoryLayout = this->memoryLayoutForVariable(intf.variable());
    SpvId result = this->nextId(nullptr);
    std::unique_ptr<Type> rtHeightStructType;
    const Type* type = &intf.variable().type();
    if (!MemoryLayout::LayoutIsSupported(*type)) {
        fErrors.error(type->fOffset, "type '" + type->name() + "' is not permitted here");
        return this->nextId(nullptr);
    }
    SpvStorageClass_ storageClass = get_storage_class(intf.variable(), SpvStorageClassFunction);
    if (fProgram.fInputs.fRTHeight && appendRTHeight) {
        SkASSERT(fRTHeightStructId == (SpvId) -1);
        SkASSERT(fRTHeightFieldIndex == (SpvId) -1);
        std::vector<Type::Field> fields = type->fields();
        fRTHeightStructId = result;
        fRTHeightFieldIndex = fields.size();
        fRTHeightStorageClass = storageClass;
        fields.emplace_back(Modifiers(), StringFragment(SKSL_RTHEIGHT_NAME),
                            fContext.fTypes.fFloat.get());
        rtHeightStructType = Type::MakeStructType(type->fOffset, type->name(), std::move(fields));
        type = rtHeightStructType.get();
    }
    SpvId typeId;
    const Modifiers& intfModifiers = intf.variable().modifiers();
    if (intfModifiers.fLayout.fBuiltin == SK_IN_BUILTIN) {
        for (const ProgramElement* e : fProgram.elements()) {
            if (e->is<ModifiersDeclaration>()) {
                const Modifiers& m = e->as<ModifiersDeclaration>().modifiers();
                update_sk_in_count(m, &fSkInCount);
            }
        }
        typeId = this->getType(
                *Type::MakeArrayType("sk_in", intf.variable().type().componentType(), fSkInCount),
                memoryLayout);
    } else {
        typeId = this->getType(*type, memoryLayout);
    }
    if (intfModifiers.fLayout.fBuiltin == -1) {
        this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBlock, fDecorationBuffer);
    }
    SpvId ptrType = this->nextId(nullptr);
    this->writeInstruction(SpvOpTypePointer, ptrType, storageClass, typeId, fConstantBuffer);
    this->writeInstruction(SpvOpVariable, ptrType, result, storageClass, fConstantBuffer);
    Layout layout = intfModifiers.fLayout;
    if (intfModifiers.fFlags & Modifiers::kUniform_Flag && layout.fSet == -1) {
        layout.fSet = 0;
    }
    this->writeLayout(layout, result);
    fVariableMap[&intf.variable()] = result;
    return result;
}

static bool is_dead(const Variable& var, const ProgramUsage* usage) {
    ProgramUsage::VariableCounts counts = usage->get(var);
    if (counts.fRead || counts.fWrite) {
        return false;
    }
    // not entirely sure what the rules are for when it's safe to elide interface variables, but it
    // causes various problems to elide some of them even when dead. But it also causes problems
    // *not* to elide sk_SampleMask when it's not being used.
    if (!(var.modifiers().fFlags & (Modifiers::kIn_Flag |
                                    Modifiers::kOut_Flag |
                                    Modifiers::kUniform_Flag))) {
        return true;
    }
    return var.modifiers().fLayout.fBuiltin == SK_SAMPLEMASK_BUILTIN;
}

void SPIRVCodeGenerator::writeGlobalVar(ProgramKind kind, const VarDeclaration& varDecl) {
    const Variable& var = varDecl.var();
    // 9999 is a sentinel value used in our built-in modules that causes us to ignore these
    // declarations, beyond adding them to the symbol table.
    constexpr int kBuiltinIgnore = 9999;
    if (var.modifiers().fLayout.fBuiltin == kBuiltinIgnore) {
        return;
    }
    if (var.modifiers().fLayout.fBuiltin == SK_FRAGCOLOR_BUILTIN &&
        kind != ProgramKind::kFragment) {
        SkASSERT(!fProgram.fConfig->fSettings.fFragColorIsInOut);
        return;
    }
    if (is_dead(var, fProgram.fUsage.get())) {
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
    SpvId typeId;
    if (var.modifiers().fLayout.fBuiltin == SK_IN_BUILTIN) {
        typeId = this->getPointerType(
                *Type::MakeArrayType("sk_in", type.componentType(), fSkInCount),
                storageClass);
    } else {
        typeId = this->getPointerType(type, storageClass);
    }
    this->writeInstruction(SpvOpVariable, typeId, id, storageClass, fConstantBuffer);
    this->writeInstruction(SpvOpName, id, var.name(), fNameBuffer);
    if (varDecl.value()) {
        SkASSERT(!fCurrentBlock);
        fCurrentBlock = -1;
        SpvId value = this->writeExpression(*varDecl.value(), fGlobalInitializersBuffer);
        this->writeInstruction(SpvOpStore, id, value, fGlobalInitializersBuffer);
        fCurrentBlock = 0;
    }
    this->writeLayout(layout, id);
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
    SpvId id = this->nextId(nullptr);
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
    SpvId test = this->writeExpression(*d.test(), out);
    this->writeInstruction(SpvOpBranchConditional, test, continueTarget, end, out);
    this->writeLabel(continueTarget, out);
    this->writeInstruction(SpvOpBranch, header, out);
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
        if (c.value()) {
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
        if (!c.value()) {
            continue;
        }
        this->writeWord(c.value()->as<IntLiteral>().value(), out);
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

void SPIRVCodeGenerator::writeGeometryShaderExecutionMode(SpvId entryPoint, OutputStream& out) {
    SkASSERT(fProgram.fConfig->fKind == ProgramKind::kGeometry);
    int invocations = 1;
    for (const ProgramElement* e : fProgram.elements()) {
        if (e->is<ModifiersDeclaration>()) {
            const Modifiers& m = e->as<ModifiersDeclaration>().modifiers();
            if (m.fFlags & Modifiers::kIn_Flag) {
                if (m.fLayout.fInvocations != -1) {
                    invocations = m.fLayout.fInvocations;
                }
                SpvId input;
                switch (m.fLayout.fPrimitive) {
                    case Layout::kPoints_Primitive:
                        input = SpvExecutionModeInputPoints;
                        break;
                    case Layout::kLines_Primitive:
                        input = SpvExecutionModeInputLines;
                        break;
                    case Layout::kLinesAdjacency_Primitive:
                        input = SpvExecutionModeInputLinesAdjacency;
                        break;
                    case Layout::kTriangles_Primitive:
                        input = SpvExecutionModeTriangles;
                        break;
                    case Layout::kTrianglesAdjacency_Primitive:
                        input = SpvExecutionModeInputTrianglesAdjacency;
                        break;
                    default:
                        input = 0;
                        break;
                }
                update_sk_in_count(m, &fSkInCount);
                if (input) {
                    this->writeInstruction(SpvOpExecutionMode, entryPoint, input, out);
                }
            } else if (m.fFlags & Modifiers::kOut_Flag) {
                SpvId output;
                switch (m.fLayout.fPrimitive) {
                    case Layout::kPoints_Primitive:
                        output = SpvExecutionModeOutputPoints;
                        break;
                    case Layout::kLineStrip_Primitive:
                        output = SpvExecutionModeOutputLineStrip;
                        break;
                    case Layout::kTriangleStrip_Primitive:
                        output = SpvExecutionModeOutputTriangleStrip;
                        break;
                    default:
                        output = 0;
                        break;
                }
                if (output) {
                    this->writeInstruction(SpvOpExecutionMode, entryPoint, output, out);
                }
                if (m.fLayout.fMaxVertices != -1) {
                    this->writeInstruction(SpvOpExecutionMode, entryPoint,
                                           SpvExecutionModeOutputVertices, m.fLayout.fMaxVertices,
                                           out);
                }
            }
        }
    }
    this->writeInstruction(SpvOpExecutionMode, entryPoint, SpvExecutionModeInvocations,
                           invocations, out);
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
    auto skFragColorRef = std::make_unique<VariableReference>(/*offset=*/-1, &skFragColorVar,
                                                              VariableReference::RefKind::kWrite);
    // Synthesize a call to the `main()` function.
    if (main.returnType() != skFragColorRef->type()) {
        fErrors.error(main.fOffset, "SPIR-V does not support returning '" +
                                    main.returnType().description() + "' from main()");
        return {};
    }
    auto callMainFn = std::make_unique<FunctionCall>(/*offset=*/-1, &main.returnType(), &main,
                                                     /*arguments=*/ExpressionArray{});

    // Synthesize `skFragColor = main()` as a BinaryExpression.
    auto assignmentStmt = std::make_unique<ExpressionStatement>(std::make_unique<BinaryExpression>(
            /*offset=*/-1,
            std::move(skFragColorRef),
            Token::Kind::TK_EQ,
            std::move(callMainFn),
            &main.returnType()));

    // Function bodies are always wrapped in a Block.
    StatementArray entrypointStmts;
    entrypointStmts.push_back(std::move(assignmentStmt));
    auto entrypointBlock = Block::Make(/*offset=*/-1, std::move(entrypointStmts),
                                       symbolTable, /*isScope=*/true);
    // Declare an entrypoint function.
    EntrypointAdapter adapter;
    adapter.fLayout = {};
    adapter.fModifiers = Modifiers{adapter.fLayout, Modifiers::kHasSideEffects_Flag};
    adapter.entrypointDecl =
            std::make_unique<FunctionDeclaration>(/*offset=*/-1,
                                                  &adapter.fModifiers,
                                                  "_entrypoint",
                                                  /*parameters=*/std::vector<const Variable*>{},
                                                  /*returnType=*/fContext.fTypes.fVoid.get(),
                                                  /*builtin=*/false);
    // Define it.
    adapter.entrypointDef =
            std::make_unique<FunctionDefinition>(/*offset=*/-1, adapter.entrypointDecl.get(),
                                                 /*builtin=*/false,
                                                 /*body=*/std::move(entrypointBlock));

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
    fUniformBuffer.fStruct = Type::MakeStructType(/*offset=*/-1, kUniformBufferName,
                                                 std::move(fields));

    // Create a global variable to contain this struct.
    Layout layout;
    layout.fBinding = fProgram.fConfig->fSettings.fDefaultUniformBinding;
    layout.fSet     = fProgram.fConfig->fSettings.fDefaultUniformSet;
    Modifiers modifiers{layout, Modifiers::kUniform_Flag};

    fUniformBuffer.fInnerVariable = std::make_unique<Variable>(
            /*offset=*/-1, fProgram.fModifiers->addToPool(modifiers), kUniformBufferName,
            fUniformBuffer.fStruct.get(), /*builtin=*/false, Variable::Storage::kGlobal);

    // Create an interface block object for this global variable.
    fUniformBuffer.fInterfaceBlock = std::make_unique<InterfaceBlock>(
            /*offset=*/-1, fUniformBuffer.fInnerVariable.get(), kUniformBufferName,
            kUniformBufferName, /*arraySize=*/0, topLevelSymbolTable);

    // Generate an interface block and hold onto its ID.
    fUniformBufferId = this->writeInterfaceBlock(*fUniformBuffer.fInterfaceBlock);
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
            if (funcDecl.name() == "main") {
                main = &funcDecl;
            }
        }
    }
    // Make sure we have a main() function.
    if (!main) {
        fErrors.error(/*offset=*/0, "program does not contain a main() function");
        return;
    }
    // Emit interface blocks.
    std::set<SpvId> interfaceVars;
    for (const ProgramElement* e : program.elements()) {
        if (e->is<InterfaceBlock>()) {
            const InterfaceBlock& intf = e->as<InterfaceBlock>();
            SpvId id = this->writeInterfaceBlock(intf);

            const Modifiers& modifiers = intf.variable().modifiers();
            if (((modifiers.fFlags & Modifiers::kIn_Flag) ||
                 (modifiers.fFlags & Modifiers::kOut_Flag)) &&
                modifiers.fLayout.fBuiltin == -1 &&
                !is_dead(intf.variable(), fProgram.fUsage.get())) {
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
    if (main->returnType() == *fContext.fTypes.fHalf4) {
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
            ((var->modifiers().fFlags & Modifiers::kIn_Flag) ||
             (var->modifiers().fFlags & Modifiers::kOut_Flag)) &&
            !is_dead(*var, fProgram.fUsage.get())) {
            interfaceVars.insert(entry.second);
        }
    }
    this->writeCapabilities(out);
    this->writeInstruction(SpvOpExtInstImport, fGLSLExtendedInstructions, "GLSL.std.450", out);
    this->writeInstruction(SpvOpMemoryModel, SpvAddressingModelLogical, SpvMemoryModelGLSL450, out);
    this->writeOpCode(SpvOpEntryPoint, (SpvId) (3 + (main->name().fLength + 4) / 4) +
                      (int32_t) interfaceVars.size(), out);
    switch (program.fConfig->fKind) {
        case ProgramKind::kVertex:
            this->writeWord(SpvExecutionModelVertex, out);
            break;
        case ProgramKind::kFragment:
            this->writeWord(SpvExecutionModelFragment, out);
            break;
        case ProgramKind::kGeometry:
            this->writeWord(SpvExecutionModelGeometry, out);
            break;
        default:
            SK_ABORT("cannot write this kind of program to SPIR-V\n");
    }
    SpvId entryPoint = fFunctionMap[main];
    this->writeWord(entryPoint, out);
    this->writeString(main->name().fChars, main->name().fLength, out);
    for (int var : interfaceVars) {
        this->writeWord(var, out);
    }
    if (program.fConfig->fKind == ProgramKind::kGeometry) {
        this->writeGeometryShaderExecutionMode(entryPoint, out);
    }
    if (program.fConfig->fKind == ProgramKind::kFragment) {
        this->writeInstruction(SpvOpExecutionMode,
                               fFunctionMap[main],
                               SpvExecutionModeOriginUpperLeft,
                               out);
    }
    for (const ProgramElement* e : program.elements()) {
        if (e->is<Extension>()) {
            this->writeInstruction(SpvOpSourceExtension, e->as<Extension>().name().c_str(), out);
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
    SkASSERT(!fErrors.errorCount());
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

}  // namespace SkSL
