/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLSPIRVCodeGenerator.h"

#include "src/sksl/GLSL.std.450.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkCaps.h"
#endif

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
    if (type.columns() > 1) {
        return is_float(context, type.componentType());
    }
    return type.isFloat();
}

static bool is_signed(const Context& context, const Type& type) {
    if (type.isVector()) {
        return is_signed(context, type.componentType());
    }
    return type.isSigned();
}

static bool is_unsigned(const Context& context, const Type& type) {
    if (type.isVector()) {
        return is_unsigned(context, type.componentType());
    }
    return type.isUnsigned();
}

static bool is_bool(const Context& context, const Type& type) {
    if (type.isVector()) {
        return is_bool(context, type.componentType());
    }
    return type.isBoolean();
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
            SkASSERT(fCurrentBlock);
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
    if (fProgram.fKind == Program::kGeometry_Kind) {
        this->writeInstruction(SpvOpCapability, SpvCapabilityGeometry, out);
    }
    else {
        this->writeInstruction(SpvOpCapability, SpvCapabilityShader, out);
    }
}

SpvId SPIRVCodeGenerator::nextId() {
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
        return *fContext.fFloat_Type;
    }
    if (type.isSigned()) {
        return *fContext.fInt_Type;
    }
    if (type.isUnsigned()) {
        return *fContext.fUInt_Type;
    }
    if (type.isMatrix() || type.isVector()) {
        if (type.componentType() == *fContext.fHalf_Type) {
            return fContext.fFloat_Type->toCompound(fContext, type.columns(), type.rows());
        }
        if (type.componentType() == *fContext.fShort_Type ||
            type.componentType() == *fContext.fByte_Type) {
            return fContext.fInt_Type->toCompound(fContext, type.columns(), type.rows());
        }
        if (type.componentType() == *fContext.fUShort_Type ||
            type.componentType() == *fContext.fUByte_Type) {
            return fContext.fUInt_Type->toCompound(fContext, type.columns(), type.rows());
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
    }
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId();
        switch (type.typeKind()) {
            case Type::TypeKind::kScalar:
                if (type.isBoolean()) {
                    this->writeInstruction(SpvOpTypeBool, result, fConstantBuffer);
                } else if (type == *fContext.fInt_Type || type == *fContext.fShort_Type ||
                           type == *fContext.fIntLiteral_Type) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 1, fConstantBuffer);
                } else if (type == *fContext.fUInt_Type || type == *fContext.fUShort_Type) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 0, fConstantBuffer);
                } else if (type == *fContext.fFloat_Type || type == *fContext.fHalf_Type ||
                           type == *fContext.fFloatLiteral_Type) {
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
                this->writeInstruction(SpvOpTypeMatrix, result,
                                       this->getType(index_type(fContext, type), layout),
                                       type.columns(), fConstantBuffer);
                break;
            case Type::TypeKind::kStruct:
                this->writeStruct(type, layout, result);
                break;
            case Type::TypeKind::kArray: {
                if (!MemoryLayout::LayoutIsSupported(type)) {
                    fErrors.error(type.fOffset, "type '" + type.name() + "' is not permitted here");
                    return this->nextId();
                }
                if (type.columns() > 0) {
                    IntLiteral count(fContext, -1, type.columns());
                    this->writeInstruction(SpvOpTypeArray, result,
                                           this->getType(type.componentType(), layout),
                                           this->writeIntLiteral(count), fConstantBuffer);
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
                                       this->getType(*fContext.fFloat_Type, layout),
                                       type.dimensions(), type.isDepth(), type.isArrayedTexture(),
                                       type.isMultisampled(), type.isSampled() ? 1 : 2,
                                       SpvImageFormatUnknown, fConstantBuffer);
                fImageTypeMap[key] = result;
                break;
            }
            default:
                if (type == *fContext.fVoid_Type) {
                    this->writeInstruction(SpvOpTypeVoid, result, fConstantBuffer);
                } else {
#ifdef SK_DEBUG
                    ABORT("invalid type: %s", type.description().c_str());
#endif
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
        SpvId result = this->nextId();
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
        SpvId result = this->nextId();
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
#ifdef SK_DEBUG
            ABORT("unsupported expression: %s", expr.description().c_str());
#endif
            break;
    }
    return -1;
}

SpvId SPIRVCodeGenerator::writeIntrinsicCall(const FunctionCall& c, OutputStream& out) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
    auto intrinsic = fIntrinsicMap.find(function.name());
    if (intrinsic == fIntrinsicMap.end()) {
        fErrors.error(c.fOffset, "unsupported intrinsic '" + function.description() + "'");
        return -1;
    }
    int32_t intrinsicId;
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
            SpvId result = this->nextId();
            std::vector<SpvId> argumentIds;
            for (size_t i = 0; i < arguments.size(); i++) {
                if (function.parameters()[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
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
            SpvId result = this->nextId();
            std::vector<SpvId> argumentIds;
            for (size_t i = 0; i < arguments.size(); i++) {
                if (function.parameters()[i]->modifiers().fFlags & Modifiers::kOut_Flag) {
                    argumentIds.push_back(this->getLValue(*arguments[i], out)->getPointer());
                } else {
                    argumentIds.push_back(this->writeExpression(*arguments[i], out));
                }
            }
            if (c.type() != *fContext.fVoid_Type) {
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
            SpvId vector = this->nextId();
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vectorSize, out);
            this->writeWord(this->getType(argType.toCompound(fContext, vectorSize, 1)), out);
            this->writeWord(vector, out);
            for (int i = 0; i < vectorSize; i++) {
                this->writeWord(raw, out);
            }
            this->writePrecisionModifier(argType, vector);
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
    SpvId result = this->nextId();
    const Type& callType = c.type();
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
            args.push_back(std::make_unique<IntLiteral>(fContext, /*offset=*/-1, /*value=*/0));
            args.push_back(std::make_unique<IntLiteral>(fContext, /*offset=*/-1, /*value=*/0));
            Constructor ctor(-1, fContext.fInt2_Type.get(), std::move(args));
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
                    if (arg1Type == *fContext.fFloat2_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type == *fContext.fFloat_Type);
                    }
                    break;
                case SpvDim2D:
                    if (arg1Type == *fContext.fFloat3_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type == *fContext.fFloat2_Type);
                    }
                    break;
                case SpvDim3D:
                    if (arg1Type == *fContext.fFloat4_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(arg1Type == *fContext.fFloat3_Type);
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
                if (fProgram.fSettings.fSharpenTextures) {
                    FloatLiteral lodBias(fContext, -1, -0.5);
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
            if (fProgram.fSettings.fFlipY) {
                // Flipping Y also negates the Y derivatives.
                SpvId flipped = this->nextId();
                this->writeInstruction(SpvOpFNegate, this->getType(callType), flipped, result,
                                       out);
                this->writePrecisionModifier(callType, flipped);
                return flipped;
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
            finalArgs.push_back(std::make_unique<FloatLiteral>(fContext, /*offset=*/-1,
                                                               /*value=*/0));
            finalArgs.push_back(std::make_unique<FloatLiteral>(fContext, /*offset=*/-1,
                                                               /*value=*/1));
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
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFunctionCall(const FunctionCall& c, OutputStream& out) {
    const FunctionDeclaration& function = c.function();
    const ExpressionArray& arguments = c.arguments();
    const auto& entry = fFunctionMap.find(&function);
    if (entry == fFunctionMap.end()) {
        return this->writeIntrinsicCall(c, out);
    }
    // stores (variable, type, lvalue) pairs to extract and save after the function call is complete
    std::vector<std::tuple<SpvId, const Type*, std::unique_ptr<LValue>>> lvalues;
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
            if (ptr) {
                argumentIds.push_back(ptr);
                continue;
            } else {
                // lvalue cannot simply be read and written via a pointer (e.g. a swizzle). Need to
                // copy it into a temp, call the function, read the value out of the temp, and then
                // update the lvalue.
                tmpValueId = lv->load(out);
                tmpVar = this->nextId();
                lvalues.push_back(std::make_tuple(tmpVar, &arguments[i]->type(), std::move(lv)));
            }
        } else {
            // see getFunctionType for an explanation of why we're always using pointer parameters
            tmpValueId = this->writeExpression(*arguments[i], out);
            tmpVar = this->nextId();
        }
        this->writeInstruction(SpvOpVariable,
                               this->getPointerType(arguments[i]->type(),
                                                    SpvStorageClassFunction),
                               tmpVar,
                               SpvStorageClassFunction,
                               fVariableBuffer);
        this->writeInstruction(SpvOpStore, tmpVar, tmpValueId, out);
        argumentIds.push_back(tmpVar);
    }
    SpvId result = this->nextId();
    this->writeOpCode(SpvOpFunctionCall, 4 + (int32_t) arguments.size(), out);
    this->writeWord(this->getType(c.type()), out);
    this->writeWord(result, out);
    this->writeWord(entry->second, out);
    for (SpvId id : argumentIds) {
        this->writeWord(id, out);
    }
    // now that the call is complete, we may need to update some lvalues with the new values of out
    // arguments
    for (const auto& tuple : lvalues) {
        SpvId load = this->nextId();
        this->writeInstruction(SpvOpLoad, getType(*std::get<1>(tuple)), load, std::get<0>(tuple),
                               out);
        this->writePrecisionModifier(*std::get<1>(tuple), load);
        std::get<2>(tuple)->store(load, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstantVector(const Constructor& c) {
    const Type& type = c.type();
    SkASSERT(type.isVector() && c.isCompileTimeConstant());
    SpvId result = this->nextId();
    std::vector<SpvId> arguments;
    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
        arguments.push_back(this->writeExpression(*arg, fConstantBuffer));
    }
    SpvId typeId = this->getType(type);
    if (c.arguments().size() == 1) {
        // with a single argument, a vector will have all of its entries equal to the argument
        this->writeOpCode(SpvOpConstantComposite, 3 + type.columns(), fConstantBuffer);
        this->writeWord(typeId, fConstantBuffer);
        this->writeWord(result, fConstantBuffer);
        for (int i = 0; i < type.columns(); i++) {
            this->writeWord(arguments[0], fConstantBuffer);
        }
    } else {
        this->writeOpCode(SpvOpConstantComposite, 3 + (int32_t) c.arguments().size(),
                          fConstantBuffer);
        this->writeWord(typeId, fConstantBuffer);
        this->writeWord(result, fConstantBuffer);
        for (SpvId id : arguments) {
            this->writeWord(id, fConstantBuffer);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeFloatConstructor(const Constructor& c, OutputStream& out) {
    const Type& constructorType = c.type();
    SkASSERT(c.arguments().size() == 1);
    const Type& argType = c.arguments()[0]->type();
    SkASSERT(constructorType.isFloat());
    SkASSERT(argType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.arguments()[0], out);
    if (argType.isSigned()) {
        this->writeInstruction(SpvOpConvertSToF, this->getType(constructorType), result, parameter,
                               out);
    } else {
        SkASSERT(argType.isUnsigned());
        this->writeInstruction(SpvOpConvertUToF, this->getType(constructorType), result, parameter,
                               out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeIntConstructor(const Constructor& c, OutputStream& out) {
    const Type& constructorType = c.type();
    SkASSERT(c.arguments().size() == 1);
    const Type& argType = c.arguments()[0]->type();
    SkASSERT(constructorType.isSigned());
    SkASSERT(argType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.arguments()[0], out);
    if (argType.isFloat()) {
        this->writeInstruction(SpvOpConvertFToS, this->getType(constructorType), result, parameter,
                               out);
    }
    else {
        SkASSERT(argType.isUnsigned());
        this->writeInstruction(SpvOpBitcast, this->getType(constructorType), result, parameter,
                               out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeUIntConstructor(const Constructor& c, OutputStream& out) {
    const Type& constructorType = c.type();
    SkASSERT(c.arguments().size() == 1);
    const Type& argType = c.arguments()[0]->type();
    SkASSERT(constructorType.isUnsigned());
    SkASSERT(argType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.arguments()[0], out);
    if (argType.isFloat()) {
        this->writeInstruction(SpvOpConvertFToU, this->getType(constructorType), result, parameter,
                               out);
    } else {
        SkASSERT(argType.isSigned());
        this->writeInstruction(SpvOpBitcast, this->getType(constructorType), result, parameter,
                               out);
    }
    return result;
}

void SPIRVCodeGenerator::writeUniformScaleMatrix(SpvId id, SpvId diagonal, const Type& type,
                                                 OutputStream& out) {
    FloatLiteral zero(fContext, -1, 0);
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
        this->writePrecisionModifier(type, columnId);
    }
    this->writeOpCode(SpvOpCompositeConstruct, 3 + type.columns(),
                      out);
    this->writeWord(this->getType(type), out);
    this->writeWord(id, out);
    for (SpvId columnId : columnIds) {
        this->writeWord(columnId, out);
    }
    this->writePrecisionModifier(type, id);
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
    if (dstType.componentType() == *fContext.fFloat_Type) {
        FloatLiteral zero(fContext, -1, 0.0);
        zeroId = this->writeFloatLiteral(zero);
    } else if (dstType.componentType() == *fContext.fInt_Type) {
        IntLiteral zero(fContext, -1, 0);
        zeroId = this->writeIntLiteral(zero);
    } else {
        ABORT("unsupported matrix component type");
    }
    SpvId zeroColumn = 0;
    SpvId columns[4];
    for (int i = 0; i < dstType.columns(); i++) {
        if (i < srcType.columns()) {
            // we're still inside the src matrix, copy the column
            SpvId srcColumn = this->nextId();
            this->writeInstruction(SpvOpCompositeExtract, srcColumnType, srcColumn, src, i, out);
            this->writePrecisionModifier(dstType, srcColumn);
            SpvId dstColumn;
            if (srcType.rows() == dstType.rows()) {
                // columns are equal size, don't need to do anything
                dstColumn = srcColumn;
            }
            else if (dstType.rows() > srcType.rows()) {
                // dst column is bigger, need to zero-pad it
                dstColumn = this->nextId();
                int delta = dstType.rows() - srcType.rows();
                this->writeOpCode(SpvOpCompositeConstruct, 4 + delta, out);
                this->writeWord(dstColumnType, out);
                this->writeWord(dstColumn, out);
                this->writeWord(srcColumn, out);
                for (int j = 0; j < delta; ++j) {
                    this->writeWord(zeroId, out);
                }
                this->writePrecisionModifier(dstType, dstColumn);
            }
            else {
                // dst column is smaller, need to swizzle the src column
                dstColumn = this->nextId();
                int count = dstType.rows();
                this->writeOpCode(SpvOpVectorShuffle, 5 + count, out);
                this->writeWord(dstColumnType, out);
                this->writeWord(dstColumn, out);
                this->writeWord(srcColumn, out);
                this->writeWord(srcColumn, out);
                for (int j = 0; j < count; j++) {
                    this->writeWord(j, out);
                }
                this->writePrecisionModifier(dstType, dstColumn);
            }
            columns[i] = dstColumn;
        } else {
            // we're past the end of the src matrix, need a vector of zeroes
            if (!zeroColumn) {
                zeroColumn = this->nextId();
                this->writeOpCode(SpvOpCompositeConstruct, 3 + dstType.rows(), out);
                this->writeWord(dstColumnType, out);
                this->writeWord(zeroColumn, out);
                for (int j = 0; j < dstType.rows(); ++j) {
                    this->writeWord(zeroId, out);
                }
                this->writePrecisionModifier(dstType, zeroColumn);
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
    this->writePrecisionModifier(dstType, id);
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
        SpvId columnId = this->nextId();
        this->writeWord(columnId, out);
        columnIds->push_back(columnId);
        for (SpvId id : *currentColumn) {
            this->writeWord(id, out);
        }
        currentColumn->clear();
        this->writePrecisionModifier(precision, columnId);
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
    SpvId result = this->nextId();
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
            v[i] = this->nextId();
            this->writeInstruction(SpvOpCompositeExtract, componentType, v[i], arguments[0], i,
                                   out);
        }
        SpvId columnType = this->getType(type.componentType().toCompound(fContext, 2, 1));
        SpvId column1 = this->nextId();
        this->writeInstruction(SpvOpCompositeConstruct, columnType, column1, v[0], v[1], out);
        SpvId column2 = this->nextId();
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
        Precision precision = type.highPrecision() ? Precision::kHigh : Precision::kLow;
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
                        SpvId swizzle = this->nextId();
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
    this->writePrecisionModifier(type, result);
    return result;
}

SpvId SPIRVCodeGenerator::writeVectorConstructor(const Constructor& c, OutputStream& out) {
    const Type& type = c.type();
    SkASSERT(type.isVector());
    if (c.isCompileTimeConstant()) {
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
            // as of this writing there's a bug in the Intel Vulkan driver where OpCreateComposite
            // doesn't handle vector arguments at all, so we always extract vector components and
            // pass them into OpCreateComposite individually.
            SpvId vec = this->writeExpression(*c.arguments()[i], out);
            SpvOp_ op = SpvOpUndef;
            const Type& src = argType.componentType();
            const Type& dst = type.componentType();
            if (dst == *fContext.fFloat_Type || dst == *fContext.fHalf_Type) {
                if (src == *fContext.fFloat_Type || src == *fContext.fHalf_Type) {
                    if (c.arguments().size() == 1) {
                        return vec;
                    }
                } else if (src == *fContext.fInt_Type ||
                           src == *fContext.fShort_Type ||
                           src == *fContext.fByte_Type) {
                    op = SpvOpConvertSToF;
                } else if (src == *fContext.fUInt_Type ||
                           src == *fContext.fUShort_Type ||
                           src == *fContext.fUByte_Type) {
                    op = SpvOpConvertUToF;
                } else {
                    SkASSERT(false);
                }
            } else if (dst == *fContext.fInt_Type ||
                       dst == *fContext.fShort_Type ||
                       dst == *fContext.fByte_Type) {
                if (src == *fContext.fFloat_Type || src == *fContext.fHalf_Type) {
                    op = SpvOpConvertFToS;
                } else if (src == *fContext.fInt_Type ||
                           src == *fContext.fShort_Type ||
                           src == *fContext.fByte_Type) {
                    if (c.arguments().size() == 1) {
                        return vec;
                    }
                } else if (src == *fContext.fUInt_Type ||
                           src == *fContext.fUShort_Type ||
                           src == *fContext.fUByte_Type) {
                    op = SpvOpBitcast;
                } else {
                    SkASSERT(false);
                }
            } else if (dst == *fContext.fUInt_Type ||
                       dst == *fContext.fUShort_Type ||
                       dst == *fContext.fUByte_Type) {
                if (src == *fContext.fFloat_Type || src == *fContext.fHalf_Type) {
                    op = SpvOpConvertFToS;
                } else if (src == *fContext.fInt_Type ||
                           src == *fContext.fShort_Type ||
                           src == *fContext.fByte_Type) {
                    op = SpvOpBitcast;
                } else if (src == *fContext.fUInt_Type ||
                           src == *fContext.fUShort_Type ||
                           src == *fContext.fUByte_Type) {
                    if (c.arguments().size() == 1) {
                        return vec;
                    }
                } else {
                    SkASSERT(false);
                }
            }
            for (int j = 0; j < argType.columns(); j++) {
                SpvId swizzle = this->nextId();
                this->writeInstruction(SpvOpCompositeExtract, this->getType(src), swizzle, vec, j,
                                       out);
                if (op != SpvOpUndef) {
                    SpvId cast = this->nextId();
                    this->writeInstruction(op, this->getType(dst), cast, swizzle, out);
                    arguments.push_back(cast);
                } else {
                    arguments.push_back(swizzle);
                }
            }
        } else {
            arguments.push_back(this->writeExpression(*c.arguments()[i], out));
        }
    }
    SpvId result = this->nextId();
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
    SpvId result = this->nextId();
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
    if (type == *fContext.fFloat_Type || type == *fContext.fHalf_Type) {
        return this->writeFloatConstructor(c, out);
    } else if (type == *fContext.fInt_Type ||
               type == *fContext.fShort_Type ||
               type == *fContext.fByte_Type) {
        return this->writeIntConstructor(c, out);
    } else if (type == *fContext.fUInt_Type ||
               type == *fContext.fUShort_Type ||
               type == *fContext.fUByte_Type) {
        return this->writeUIntConstructor(c, out);
    }
    switch (type.typeKind()) {
        case Type::TypeKind::kVector:
            return this->writeVectorConstructor(c, out);
        case Type::TypeKind::kMatrix:
            return this->writeMatrixConstructor(c, out);
        case Type::TypeKind::kArray:
            return this->writeArrayConstructor(c, out);
        default:
#ifdef SK_DEBUG
            ABORT("unsupported constructor: %s", c.description().c_str());
#endif
            return -1;
    }
}

SpvStorageClass_ get_storage_class(const Modifiers& modifiers) {
    if (modifiers.fFlags & Modifiers::kIn_Flag) {
        SkASSERT(!(modifiers.fLayout.fFlags & Layout::kPushConstant_Flag));
        return SpvStorageClassInput;
    } else if (modifiers.fFlags & Modifiers::kOut_Flag) {
        SkASSERT(!(modifiers.fLayout.fFlags & Layout::kPushConstant_Flag));
        return SpvStorageClassOutput;
    } else if (modifiers.fFlags & Modifiers::kUniform_Flag) {
        if (modifiers.fLayout.fFlags & Layout::kPushConstant_Flag) {
            return SpvStorageClassPushConstant;
        }
        return SpvStorageClassUniform;
    } else {
        return SpvStorageClassFunction;
    }
}

SpvStorageClass_ get_storage_class(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kVariableReference: {
            const Variable& var = *expr.as<VariableReference>().variable();
            if (var.storage() != Variable::Storage::kGlobal) {
                return SpvStorageClassFunction;
            }
            SpvStorageClass_ result = get_storage_class(var.modifiers());
            if (result == SpvStorageClassFunction) {
                result = SpvStorageClassPrivate;
            }
            return result;
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
            IndexExpression& indexExpr = (IndexExpression&) expr;
            chain = this->getAccessChain(*indexExpr.base(), out);
            chain.push_back(this->writeExpression(*indexExpr.index(), out));
            break;
        }
        case Expression::Kind::kFieldAccess: {
            FieldAccess& fieldExpr = (FieldAccess&) expr;
            chain = this->getAccessChain(*fieldExpr.base(), out);
            IntLiteral index(fContext, -1, fieldExpr.fieldIndex());
            chain.push_back(this->writeIntLiteral(index));
            break;
        }
        default: {
            SpvId id = this->getLValue(expr, out)->getPointer();
            SkASSERT(id != 0);
            chain.push_back(id);
        }
    }
    return chain;
}

class PointerLValue : public SPIRVCodeGenerator::LValue {
public:
    PointerLValue(SPIRVCodeGenerator& gen, SpvId pointer, SpvId type,
                  SPIRVCodeGenerator::Precision precision)
    : fGen(gen)
    , fPointer(pointer)
    , fType(type)
    , fPrecision(precision) {}

    SpvId getPointer() override {
        return fPointer;
    }

    SpvId load(OutputStream& out) override {
        SpvId result = fGen.nextId();
        fGen.writeInstruction(SpvOpLoad, fType, result, fPointer, out);
        fGen.writePrecisionModifier(fPrecision, result);
        return result;
    }

    void store(SpvId value, OutputStream& out) override {
        fGen.writeInstruction(SpvOpStore, fPointer, value, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fPointer;
    const SpvId fType;
    const SPIRVCodeGenerator::Precision fPrecision;
};

class SwizzleLValue : public SPIRVCodeGenerator::LValue {
public:
    SwizzleLValue(SPIRVCodeGenerator& gen, SpvId vecPointer, const ComponentArray& components,
                  const Type& baseType, const Type& swizzleType,
                  SPIRVCodeGenerator::Precision precision)
    : fGen(gen)
    , fVecPointer(vecPointer)
    , fComponents(components)
    , fBaseType(baseType)
    , fSwizzleType(swizzleType)
    , fPrecision(precision) {}

    SpvId getPointer() override {
        return 0;
    }

    SpvId load(OutputStream& out) override {
        SpvId base = fGen.nextId();
        fGen.writeInstruction(SpvOpLoad, fGen.getType(fBaseType), base, fVecPointer, out);
        fGen.writePrecisionModifier(fPrecision, base);
        SpvId result = fGen.nextId();
        fGen.writeOpCode(SpvOpVectorShuffle, 5 + (int32_t) fComponents.size(), out);
        fGen.writeWord(fGen.getType(fSwizzleType), out);
        fGen.writeWord(result, out);
        fGen.writeWord(base, out);
        fGen.writeWord(base, out);
        for (int component : fComponents) {
            fGen.writeWord(component, out);
        }
        fGen.writePrecisionModifier(fPrecision, result);
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
        fGen.writePrecisionModifier(fPrecision, shuffle);
        fGen.writeInstruction(SpvOpStore, fVecPointer, shuffle, out);
    }

private:
    SPIRVCodeGenerator& fGen;
    const SpvId fVecPointer;
    const ComponentArray& fComponents;
    const Type& fBaseType;
    const Type& fSwizzleType;
    const SPIRVCodeGenerator::Precision fPrecision;
};

std::unique_ptr<SPIRVCodeGenerator::LValue> SPIRVCodeGenerator::getLValue(const Expression& expr,
                                                                          OutputStream& out) {
    const Type& type = expr.type();
    Precision precision = type.highPrecision() ? Precision::kHigh : Precision::kLow;
    switch (expr.kind()) {
        case Expression::Kind::kVariableReference: {
            SpvId typeId;
            const Variable& var = *expr.as<VariableReference>().variable();
            if (var.modifiers().fLayout.fBuiltin == SK_IN_BUILTIN) {
                typeId = this->getType(*Type::MakeArrayType("sk_in", var.type().componentType(),
                                                            fSkInCount));
            } else {
                typeId = this->getType(type);
            }
            auto entry = fVariableMap.find(&var);
            SkASSERT(entry != fVariableMap.end());
            return std::make_unique<PointerLValue>(*this, entry->second, typeId, precision);
        }
        case Expression::Kind::kIndex: // fall through
        case Expression::Kind::kFieldAccess: {
            std::vector<SpvId> chain = this->getAccessChain(expr, out);
            SpvId member = this->nextId();
            this->writeOpCode(SpvOpAccessChain, (SpvId) (3 + chain.size()), out);
            this->writeWord(this->getPointerType(type, get_storage_class(expr)), out);
            this->writeWord(member, out);
            for (SpvId idx : chain) {
                this->writeWord(idx, out);
            }
            return std::make_unique<PointerLValue>(*this, member, this->getType(type), precision);
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& swizzle = (Swizzle&) expr;
            size_t count = swizzle.components().size();
            SpvId base = this->getLValue(*swizzle.base(), out)->getPointer();
            if (!base) {
                fErrors.error(swizzle.fOffset, "unable to retrieve lvalue from swizzle");
            }
            if (count == 1) {
                IntLiteral index(fContext, -1, swizzle.components()[0]);
                SpvId member = this->nextId();
                this->writeInstruction(SpvOpAccessChain,
                                       this->getPointerType(type,
                                                            get_storage_class(*swizzle.base())),
                                       member,
                                       base,
                                       this->writeIntLiteral(index),
                                       out);
                return std::make_unique<PointerLValue>(*this, member, this->getType(type),
                                                       precision);
            } else {
                return std::make_unique<SwizzleLValue>(*this, base, swizzle.components(),
                                                       swizzle.base()->type(), type, precision);
            }
        }
        case Expression::Kind::kTernary: {
            TernaryExpression& t = (TernaryExpression&) expr;
            SpvId test = this->writeExpression(*t.test(), out);
            SpvId end = this->nextId();
            SpvId ifTrueLabel = this->nextId();
            SpvId ifFalseLabel = this->nextId();
            this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
            this->writeInstruction(SpvOpBranchConditional, test, ifTrueLabel, ifFalseLabel, out);
            this->writeLabel(ifTrueLabel, out);
            SpvId ifTrue = this->getLValue(*t.ifTrue(), out)->getPointer();
            SkASSERT(ifTrue);
            this->writeInstruction(SpvOpBranch, end, out);
            ifTrueLabel = fCurrentBlock;
            SpvId ifFalse = this->getLValue(*t.ifFalse(), out)->getPointer();
            SkASSERT(ifFalse);
            ifFalseLabel = fCurrentBlock;
            this->writeInstruction(SpvOpBranch, end, out);
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpPhi, this->getType(*fContext.fBool_Type), result, ifTrue,
                       ifTrueLabel, ifFalse, ifFalseLabel, out);
            return std::make_unique<PointerLValue>(*this, result, this->getType(type), precision);
        }
        default: {
            // expr isn't actually an lvalue, create a dummy variable for it. This case happens due
            // to the need to store values in temporary variables during function calls (see
            // comments in getFunctionType); erroneous uses of rvalues as lvalues should have been
            // caught by IRGenerator
            SpvId result = this->nextId();
            SpvId pointerType = this->getPointerType(type, SpvStorageClassFunction);
            this->writeInstruction(SpvOpVariable, pointerType, result, SpvStorageClassFunction,
                                   fVariableBuffer);
            this->writeInstruction(SpvOpStore, result, this->writeExpression(expr, out), out);
            return std::make_unique<PointerLValue>(*this, result, this->getType(type), precision);
        }
    }
}

SpvId SPIRVCodeGenerator::writeVariableReference(const VariableReference& ref, OutputStream& out) {
    SpvId result = this->nextId();
    auto entry = fVariableMap.find(ref.variable());
    SkASSERT(entry != fVariableMap.end());
    SpvId var = entry->second;
    this->writeInstruction(SpvOpLoad, this->getType(ref.variable()->type()), result, var, out);
    this->writePrecisionModifier(ref.variable()->type(), result);
    if (ref.variable()->modifiers().fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN &&
        (fProgram.fSettings.fFlipY || fProgram.fSettings.fInverseW)) {
        // The x component never changes, so just grab it
        SpvId xId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), xId,
                               result, 0, out);

        // Calculate the y component which may need to be flipped
        SpvId rawYId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), rawYId,
                               result, 1, out);
        SpvId flippedYId = 0;
        if (fProgram.fSettings.fFlipY) {
            // need to remap to a top-left coordinate system
            if (fRTHeightStructId == (SpvId)-1) {
                // height variable hasn't been written yet
                SkASSERT(fRTHeightFieldIndex == (SpvId)-1);
                std::vector<Type::Field> fields;
                if (fProgram.fSettings.fRTHeightOffset < 0) {
                    fErrors.error(ref.fOffset, "RTHeightOffset is negative");
                }
                fields.emplace_back(
                        Modifiers(Layout(0, -1, fProgram.fSettings.fRTHeightOffset, -1, -1, -1, -1,
                                         -1, Layout::Format::kUnspecified,
                                         Layout::kUnspecified_Primitive, 1, -1, "", "",
                                         Layout::kNo_Key, Layout::CType::kDefault),
                                    0),
                        SKSL_RTHEIGHT_NAME, fContext.fFloat_Type.get());
                StringFragment name("sksl_synthetic_uniforms");
                std::unique_ptr<Type> intfStruct = Type::MakeStructType(/*offset=*/-1, name,
                                                                        fields);
                int binding = fProgram.fSettings.fRTHeightBinding;
                if (binding == -1) {
                    fErrors.error(ref.fOffset, "layout(binding=...) is required in SPIR-V");
                }
                int set = fProgram.fSettings.fRTHeightSet;
                if (set == -1) {
                    fErrors.error(ref.fOffset, "layout(set=...) is required in SPIR-V");
                }
                Layout layout(0, -1, -1, binding, -1, set, -1, -1, Layout::Format::kUnspecified,
                                Layout::kUnspecified_Primitive, -1, -1, "", "", Layout::kNo_Key,
                                Layout::CType::kDefault);
                Modifiers modifiers(layout, Modifiers::kUniform_Flag);
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
                fRTHeightStorageClass = SpvStorageClassUniform;
            }
            SkASSERT(fRTHeightFieldIndex != (SpvId)-1);

            IntLiteral fieldIndex(fContext, -1, fRTHeightFieldIndex);
            SpvId fieldIndexId = this->writeIntLiteral(fieldIndex);
            SpvId heightPtr = this->nextId();
            this->writeOpCode(SpvOpAccessChain, 5, out);
            this->writeWord(this->getPointerType(*fContext.fFloat_Type, fRTHeightStorageClass),
                            out);
            this->writeWord(heightPtr, out);
            this->writeWord(fRTHeightStructId, out);
            this->writeWord(fieldIndexId, out);
            SpvId heightRead = this->nextId();
            this->writeInstruction(SpvOpLoad, this->getType(*fContext.fFloat_Type), heightRead,
                                   heightPtr, out);

            flippedYId = this->nextId();
            this->writeInstruction(SpvOpFSub, this->getType(*fContext.fFloat_Type), flippedYId,
                                   heightRead, rawYId, out);
        }

        // The z component will always be zero so we just get an id to the 0 literal
        FloatLiteral zero(fContext, -1, 0.0);
        SpvId zeroId = writeFloatLiteral(zero);

        // Calculate the w component which may need to be inverted
        SpvId rawWId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), rawWId,
                               result, 3, out);
        SpvId invWId = 0;
        if (fProgram.fSettings.fInverseW) {
            // We need to invert w
            FloatLiteral one(fContext, -1, 1.0);
            SpvId oneId = writeFloatLiteral(one);
            invWId = this->nextId();
            this->writeInstruction(SpvOpFDiv, this->getType(*fContext.fFloat_Type), invWId, oneId,
                                   rawWId, out);
        }

        // Fill in the new fragcoord with the components from above
        SpvId adjusted = this->nextId();
        this->writeOpCode(SpvOpCompositeConstruct, 7, out);
        this->writeWord(this->getType(*fContext.fFloat4_Type), out);
        this->writeWord(adjusted, out);
        this->writeWord(xId, out);
        if (fProgram.fSettings.fFlipY) {
            this->writeWord(flippedYId, out);
        } else {
            this->writeWord(rawYId, out);
        }
        this->writeWord(zeroId, out);
        if (fProgram.fSettings.fInverseW) {
            this->writeWord(invWId, out);
        } else {
            this->writeWord(rawWId, out);
        }

        return adjusted;
    }
    if (ref.variable()->modifiers().fLayout.fBuiltin == SK_CLOCKWISE_BUILTIN &&
        !fProgram.fSettings.fFlipY) {
        // FrontFacing in Vulkan is defined in terms of a top-down render target. In skia, we use
        // the default convention of "counter-clockwise face is front".
        SpvId inverse = this->nextId();
        this->writeInstruction(SpvOpLogicalNot, this->getType(*fContext.fBool_Type), inverse,
                               result, out);
        return inverse;
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeIndexExpression(const IndexExpression& expr, OutputStream& out) {
    if (expr.base()->type().isVector()) {
        SpvId base = this->writeExpression(*expr.base(), out);
        SpvId index = this->writeExpression(*expr.index(), out);
        SpvId result = this->nextId();
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
    SpvId result = this->nextId();
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
    SpvId result = this->nextId();
    if (is_float(fContext, operandType)) {
        this->writeInstruction(ifFloat, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_signed(fContext, operandType)) {
        this->writeInstruction(ifInt, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_unsigned(fContext, operandType)) {
        this->writeInstruction(ifUInt, this->getType(resultType), result, lhs, rhs, out);
    } else if (is_bool(fContext, operandType)) {
        this->writeInstruction(ifBool, this->getType(resultType), result, lhs, rhs, out);
        return result; // skip RelaxedPrecision check
    } else {
        fErrors.error(operandType.fOffset,
                      "unsupported operand for binary expression: " + operandType.description());
        return result;
    }
    if (getActualType(resultType) == operandType && !resultType.highPrecision()) {
        this->writeInstruction(SpvOpDecorate, result, SpvDecorationRelaxedPrecision,
                               fDecorationBuffer);
    }
    return result;
}

SpvId SPIRVCodeGenerator::foldToBool(SpvId id, const Type& operandType, SpvOp op,
                                     OutputStream& out) {
    if (operandType.isVector()) {
        SpvId result = this->nextId();
        this->writeInstruction(op, this->getType(*fContext.fBool_Type), result, id, out);
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
    SpvId bvecType = this->getType(fContext.fBool_Type->toCompound(fContext,
                                                                    operandType.rows(),
                                                                    1));
    SpvId boolType = this->getType(*fContext.fBool_Type);
    SpvId result = 0;
    for (int i = 0; i < operandType.columns(); i++) {
        SpvId columnL = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnL, lhs, i, out);
        SpvId columnR = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnR, rhs, i, out);
        SpvId compare = this->nextId();
        this->writeInstruction(compareOp, bvecType, compare, columnL, columnR, out);
        SpvId merge = this->nextId();
        this->writeInstruction(vectorMergeOperator, boolType, merge, compare, out);
        if (result != 0) {
            SpvId next = this->nextId();
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
        SpvId columnL = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnL, lhs, i, out);
        SpvId columnR = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, columnType, columnR, rhs, i, out);
        columns[i] = this->nextId();
        this->writeInstruction(op, columnType, columns[i], columnL, columnR, out);
    }
    SpvId result = this->nextId();
    this->writeOpCode(SpvOpCompositeConstruct, 3 + operandType.columns(), out);
    this->writeWord(this->getType(operandType), out);
    this->writeWord(result, out);
    for (int i = 0; i < operandType.columns(); i++) {
        this->writeWord(columns[i], out);
    }
    return result;
}

std::unique_ptr<Expression> create_literal_1(const Context& context, const Type& type) {
    if (type.isInteger()) {
        return std::unique_ptr<Expression>(new IntLiteral(-1, 1, &type));
    }
    else if (type.isFloat()) {
        return std::unique_ptr<Expression>(new FloatLiteral(-1, 1.0, &type));
    } else {
        ABORT("math is unsupported on type '%s'", String(type.name()).c_str());
    }
}

SpvId SPIRVCodeGenerator::writeBinaryExpression(const Type& leftType, SpvId lhs, Token::Kind op,
                                                const Type& rightType, SpvId rhs,
                                                const Type& resultType, OutputStream& out) {
    // The comma operator ignores the type of the left-hand side entirely.
    if (op == Token::Kind::TK_COMMA) {
        return rhs;
    }
    // overall type we are operating on: float2, int, uint4...
    const Type* operandType;
    // IR allows mismatched types in expressions (e.g. float2 * float), but they need special
    // handling in SPIR-V
    if (this->getActualType(leftType) != this->getActualType(rightType)) {
        if (leftType.isVector() && rightType.isNumber()) {
            if (op == Token::Kind::TK_SLASH) {
                SpvId one = this->writeExpression(*create_literal_1(fContext, rightType), out);
                SpvId inverse = this->nextId();
                this->writeInstruction(SpvOpFDiv, this->getType(rightType), inverse, one, rhs, out);
                rhs = inverse;
                op = Token::Kind::TK_STAR;
            }
            if (op == Token::Kind::TK_STAR) {
                SpvId result = this->nextId();
                this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                       result, lhs, rhs, out);
                return result;
            }
            // promote number to vector
            SpvId vec = this->nextId();
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
            if (op == Token::Kind::TK_STAR) {
                SpvId result = this->nextId();
                this->writeInstruction(SpvOpVectorTimesScalar, this->getType(resultType),
                                       result, rhs, lhs, out);
                return result;
            }
            // promote number to vector
            SpvId vec = this->nextId();
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
            SpvId result = this->nextId();
            this->writeInstruction(spvop, this->getType(resultType), result, lhs, rhs, out);
            return result;
        } else if (rightType.isMatrix()) {
            SpvId result = this->nextId();
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
    switch (op) {
        case Token::Kind::TK_EQEQ: {
            if (operandType->isMatrix()) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFOrdEqual,
                                                   SpvOpIEqual, SpvOpAll, SpvOpLogicalAnd, out);
            }
            SkASSERT(resultType.isBoolean());
            const Type* tmpType;
            if (operandType->isVector()) {
                tmpType = &fContext.fBool_Type->toCompound(fContext,
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
                tmpType = &fContext.fBool_Type->toCompound(fContext,
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
                SpvId result = this->nextId();
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
    Token::Kind op = b.getOperator();
    // handle cases where we don't necessarily evaluate both LHS and RHS
    switch (op) {
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
    if (Compiler::IsAssignment(op)) {
        lvalue = this->getLValue(left, out);
        lhs = lvalue->load(out);
    } else {
        lvalue = nullptr;
        lhs = this->writeExpression(left, out);
    }
    SpvId rhs = this->writeExpression(right, out);
    SpvId result = this->writeBinaryExpression(left.type(), lhs, Compiler::RemoveAssignment(op),
                                               right.type(), rhs, b.type(), out);
    if (lvalue) {
        lvalue->store(result, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalAnd(const BinaryExpression& a, OutputStream& out) {
    SkASSERT(a.getOperator() == Token::Kind::TK_LOGICALAND);
    BoolLiteral falseLiteral(fContext, -1, false);
    SpvId falseConstant = this->writeBoolLiteral(falseLiteral);
    SpvId lhs = this->writeExpression(*a.left(), out);
    SpvId rhsLabel = this->nextId();
    SpvId end = this->nextId();
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, rhsLabel, end, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(*a.right(), out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId();
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fBool_Type), result, falseConstant,
                           lhsBlock, rhs, rhsBlock, out);
    return result;
}

SpvId SPIRVCodeGenerator::writeLogicalOr(const BinaryExpression& o, OutputStream& out) {
    SkASSERT(o.getOperator() == Token::Kind::TK_LOGICALOR);
    BoolLiteral trueLiteral(fContext, -1, true);
    SpvId trueConstant = this->writeBoolLiteral(trueLiteral);
    SpvId lhs = this->writeExpression(*o.left(), out);
    SpvId rhsLabel = this->nextId();
    SpvId end = this->nextId();
    SpvId lhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, lhs, end, rhsLabel, out);
    this->writeLabel(rhsLabel, out);
    SpvId rhs = this->writeExpression(*o.right(), out);
    SpvId rhsBlock = fCurrentBlock;
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId();
    this->writeInstruction(SpvOpPhi, this->getType(*fContext.fBool_Type), result, trueConstant,
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
        SpvId result = this->nextId();
        SpvId trueId = this->writeExpression(*t.ifTrue(), out);
        SpvId falseId = this->writeExpression(*t.ifFalse(), out);
        this->writeInstruction(SpvOpSelect, this->getType(type), result, test, trueId, falseId,
                               out);
        return result;
    }
    // was originally using OpPhi to choose the result, but for some reason that is crashing on
    // Adreno. Switched to storing the result in a temp variable as glslang does.
    SpvId var = this->nextId();
    this->writeInstruction(SpvOpVariable, this->getPointerType(type, SpvStorageClassFunction),
                           var, SpvStorageClassFunction, fVariableBuffer);
    SpvId trueLabel = this->nextId();
    SpvId falseLabel = this->nextId();
    SpvId end = this->nextId();
    this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
    this->writeInstruction(SpvOpBranchConditional, test, trueLabel, falseLabel, out);
    this->writeLabel(trueLabel, out);
    this->writeInstruction(SpvOpStore, var, this->writeExpression(*t.ifTrue(), out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(falseLabel, out);
    this->writeInstruction(SpvOpStore, var, this->writeExpression(*t.ifFalse(), out), out);
    this->writeInstruction(SpvOpBranch, end, out);
    this->writeLabel(end, out);
    SpvId result = this->nextId();
    this->writeInstruction(SpvOpLoad, this->getType(type), result, var, out);
    this->writePrecisionModifier(type, result);
    return result;
}

SpvId SPIRVCodeGenerator::writePrefixExpression(const PrefixExpression& p, OutputStream& out) {
    const Type& type = p.type();
    if (p.getOperator() == Token::Kind::TK_MINUS) {
        SpvId result = this->nextId();
        SpvId typeId = this->getType(type);
        SpvId expr = this->writeExpression(*p.operand(), out);
        if (is_float(fContext, type)) {
            this->writeInstruction(SpvOpFNegate, typeId, result, expr, out);
        } else if (is_signed(fContext, type)) {
            this->writeInstruction(SpvOpSNegate, typeId, result, expr, out);
        } else {
#ifdef SK_DEBUG
            ABORT("unsupported prefix expression %s", p.description().c_str());
#endif
        }
        this->writePrecisionModifier(type, result);
        return result;
    }
    switch (p.getOperator()) {
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
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpLogicalNot, this->getType(type), result,
                                   this->writeExpression(*p.operand(), out), out);
            return result;
        }
        case Token::Kind::TK_BITWISENOT: {
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpNot, this->getType(type), result,
                                   this->writeExpression(*p.operand(), out), out);
            return result;
        }
        default:
#ifdef SK_DEBUG
            ABORT("unsupported prefix expression: %s", p.description().c_str());
#endif
            return -1;
    }
}

SpvId SPIRVCodeGenerator::writePostfixExpression(const PostfixExpression& p, OutputStream& out) {
    const Type& type = p.type();
    std::unique_ptr<LValue> lv = this->getLValue(*p.operand(), out);
    SpvId result = lv->load(out);
    SpvId one = this->writeExpression(*create_literal_1(fContext, type), out);
    switch (p.getOperator()) {
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
#ifdef SK_DEBUG
            ABORT("unsupported postfix expression %s", p.description().c_str());
#endif
            return -1;
    }
}

SpvId SPIRVCodeGenerator::writeBoolLiteral(const BoolLiteral& b) {
    if (b.value()) {
        if (fBoolTrue == 0) {
            fBoolTrue = this->nextId();
            this->writeInstruction(SpvOpConstantTrue, this->getType(b.type()), fBoolTrue,
                                   fConstantBuffer);
        }
        return fBoolTrue;
    } else {
        if (fBoolFalse == 0) {
            fBoolFalse = this->nextId();
            this->writeInstruction(SpvOpConstantFalse, this->getType(b.type()), fBoolFalse,
                                   fConstantBuffer);
        }
        return fBoolFalse;
    }
}

SpvId SPIRVCodeGenerator::writeIntLiteral(const IntLiteral& i) {
    const Type& type = i.type();
    ConstantType constantType;
    if (type == *fContext.fInt_Type || type.typeKind() == Type::TypeKind::kEnum) {
        constantType = ConstantType::kInt;
    } else if (type == *fContext.fUInt_Type) {
        constantType = ConstantType::kUInt;
    } else if (type == *fContext.fShort_Type || type == *fContext.fByte_Type) {
        constantType = ConstantType::kShort;
    } else if (type == *fContext.fUShort_Type || type == *fContext.fUByte_Type) {
        constantType = ConstantType::kUShort;
    } else {
        SkASSERT(false);
    }
    std::pair<ConstantValue, ConstantType> key(i.value(), constantType);
    auto entry = fNumberConstants.find(key);
    if (entry == fNumberConstants.end()) {
        SpvId result = this->nextId();
        this->writeInstruction(SpvOpConstant, this->getType(type), result, (SpvId) i.value(),
                               fConstantBuffer);
        fNumberConstants[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    const Type& type = f.type();
    ConstantType constantType;
    if (type == *fContext.fHalf_Type) {
        constantType = ConstantType::kHalf;
    } else {
        constantType = ConstantType::kFloat;
    }
    float value = (float) f.value();
    std::pair<ConstantValue, ConstantType> key(f.value(), constantType);
    auto entry = fNumberConstants.find(key);
    if (entry == fNumberConstants.end()) {
        SpvId result = this->nextId();
        uint32_t bits;
        SkASSERT(sizeof(bits) == sizeof(value));
        memcpy(&bits, &value, sizeof(bits));
        this->writeInstruction(SpvOpConstant, this->getType(type), result, bits,
                               fConstantBuffer);
        fNumberConstants[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::writeFunctionStart(const FunctionDeclaration& f, OutputStream& out) {
    SpvId result = fFunctionMap[&f];
    this->writeInstruction(SpvOpFunction, this->getType(f.returnType()), result,
                           SpvFunctionControlMaskNone, this->getFunctionType(f), out);
    this->writeInstruction(SpvOpName, result, f.name(), fNameBuffer);
    const std::vector<const Variable*>& parameters = f.parameters();
    for (size_t i = 0; i < parameters.size(); i++) {
        SpvId id = this->nextId();
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
    this->writeLabel(this->nextId(), out);
    StringStream bodyBuffer;
    this->writeBlock((Block&) *f.body(), bodyBuffer);
    write_stringstream(fVariableBuffer, out);
    if (f.declaration().name() == "main") {
        write_stringstream(fGlobalInitializersBuffer, out);
    }
    write_stringstream(bodyBuffer, out);
    if (fCurrentBlock) {
        if (f.declaration().returnType() == *fContext.fVoid_Type) {
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
    bool isBuffer = ((intf.variable().modifiers().fFlags & Modifiers::kBuffer_Flag) != 0);
    bool pushConstant = ((intf.variable().modifiers().fLayout.fFlags &
                          Layout::kPushConstant_Flag) != 0);
    MemoryLayout memoryLayout = (pushConstant || isBuffer) ?
                                MemoryLayout(MemoryLayout::k430_Standard) :
                                fDefaultLayout;
    SpvId result = this->nextId();
    std::unique_ptr<Type> rtHeightStructType;
    const Type* type = &intf.variable().type();
    if (!MemoryLayout::LayoutIsSupported(*type)) {
        fErrors.error(type->fOffset, "type '" + type->name() + "' is not permitted here");
        return this->nextId();
    }
    Modifiers intfModifiers = intf.variable().modifiers();
    SpvStorageClass_ storageClass = get_storage_class(intfModifiers);
    if (fProgram.fInputs.fRTHeight && appendRTHeight) {
        SkASSERT(fRTHeightStructId == (SpvId) -1);
        SkASSERT(fRTHeightFieldIndex == (SpvId) -1);
        std::vector<Type::Field> fields = type->fields();
        fRTHeightStructId = result;
        fRTHeightFieldIndex = fields.size();
        fRTHeightStorageClass = storageClass;
        fields.emplace_back(Modifiers(), StringFragment(SKSL_RTHEIGHT_NAME),
                            fContext.fFloat_Type.get());
        rtHeightStructType = Type::MakeStructType(type->fOffset, type->name(), std::move(fields));
        type = rtHeightStructType.get();
    }
    SpvId typeId;
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
    if (intfModifiers.fFlags & Modifiers::kBuffer_Flag) {
        this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBufferBlock, fDecorationBuffer);
    } else if (intfModifiers.fLayout.fBuiltin == -1) {
        this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBlock, fDecorationBuffer);
    }
    SpvId ptrType = this->nextId();
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

void SPIRVCodeGenerator::writePrecisionModifier(const Type& type, SpvId id) {
    this->writePrecisionModifier(type.highPrecision() ? Precision::kHigh : Precision::kLow, id);
}

void SPIRVCodeGenerator::writePrecisionModifier(Precision precision, SpvId id) {
    if (precision == Precision::kLow) {
        this->writeInstruction(SpvOpDecorate, id, SpvDecorationRelaxedPrecision, fDecorationBuffer);
    }
}

bool is_dead(const Variable& var, const ProgramUsage* usage) {
    ProgramUsage::VariableCounts counts = usage->get(var);
    if (counts.fRead || counts.fWrite) {
        return false;
    }
    // not entirely sure what the rules are for when it's safe to elide interface variables, but it
    // causes various problems to elide some of them even when dead. But it also causes problems
    // *not* to elide sk_SampleMask when it's not being used.
    if (!(var.modifiers().fFlags & (Modifiers::kIn_Flag |
                                    Modifiers::kOut_Flag |
                                    Modifiers::kUniform_Flag |
                                    Modifiers::kBuffer_Flag))) {
        return true;
    }
    return var.modifiers().fLayout.fBuiltin == SK_SAMPLEMASK_BUILTIN;
}

#define BUILTIN_IGNORE 9999
void SPIRVCodeGenerator::writeGlobalVar(Program::Kind kind, const VarDeclaration& varDecl,
                                        OutputStream& out) {
    const Variable& var = varDecl.var();
    // These haven't been implemented in our SPIR-V generator yet and we only currently use them
    // in the OpenGL backend.
    SkASSERT(!(var.modifiers().fFlags & (Modifiers::kReadOnly_Flag |
                                          Modifiers::kWriteOnly_Flag |
                                          Modifiers::kCoherent_Flag |
                                          Modifiers::kVolatile_Flag |
                                          Modifiers::kRestrict_Flag)));
    if (var.modifiers().fLayout.fBuiltin == BUILTIN_IGNORE) {
        return;
    }
    if (var.modifiers().fLayout.fBuiltin == SK_FRAGCOLOR_BUILTIN &&
        kind != Program::kFragment_Kind) {
        SkASSERT(!fProgram.fSettings.fFragColorIsInOut);
        return;
    }
    if (is_dead(var, fProgram.fUsage.get())) {
        return;
    }
    const Type& type = var.type();
    SpvStorageClass_ storageClass;
    if (var.modifiers().fFlags & Modifiers::kIn_Flag) {
        storageClass = SpvStorageClassInput;
    } else if (var.modifiers().fFlags & Modifiers::kOut_Flag) {
        storageClass = SpvStorageClassOutput;
    } else if (var.modifiers().fFlags & Modifiers::kUniform_Flag) {
        if (type.typeKind() == Type::TypeKind::kSampler ||
            type.typeKind() == Type::TypeKind::kSeparateSampler ||
            type.typeKind() == Type::TypeKind::kTexture) {
            storageClass = SpvStorageClassUniformConstant;
        } else {
            storageClass = SpvStorageClassUniform;
        }
    } else {
        storageClass = SpvStorageClassPrivate;
    }
    SpvId id = this->nextId();
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
    this->writePrecisionModifier(type, id);
    if (varDecl.value()) {
        SkASSERT(!fCurrentBlock);
        fCurrentBlock = -1;
        SpvId value = this->writeExpression(*varDecl.value(), fGlobalInitializersBuffer);
        this->writeInstruction(SpvOpStore, id, value, fGlobalInitializersBuffer);
        fCurrentBlock = 0;
    }
    this->writeLayout(var.modifiers().fLayout, id);
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
    // These haven't been implemented in our SPIR-V generator yet and we only currently use them
    // in the OpenGL backend.
    SkASSERT(!(var.modifiers().fFlags & (Modifiers::kReadOnly_Flag |
                                         Modifiers::kWriteOnly_Flag |
                                         Modifiers::kCoherent_Flag |
                                         Modifiers::kVolatile_Flag |
                                         Modifiers::kRestrict_Flag)));
    SpvId id = this->nextId();
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
            this->writeBlock((Block&) s, out);
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
#ifdef SK_DEBUG
            ABORT("unsupported statement: %s", s.description().c_str());
#endif
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
    SpvId ifTrue = this->nextId();
    SpvId ifFalse = this->nextId();
    if (stmt.ifFalse()) {
        SpvId end = this->nextId();
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
    SpvId header = this->nextId();
    SpvId start = this->nextId();
    SpvId next = this->nextId();
    SpvId continueTarget = this->nextId();
    fContinueTarget.push(continueTarget);
    SpvId end = this->nextId();
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
    SpvId end = this->nextId();
    SpvId defaultLabel = end;
    fBreakTarget.push(end);
    int size = 3;
    auto& cases = s.cases();
    for (const std::unique_ptr<SwitchCase>& c : cases) {
        SpvId label = this->nextId();
        labels.push_back(label);
        if (c->value()) {
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
        if (!cases[i]->value()) {
            continue;
        }
        this->writeWord(cases[i]->value()->as<IntLiteral>().value(), out);
        this->writeWord(labels[i], out);
    }
    for (size_t i = 0; i < cases.size(); ++i) {
        this->writeLabel(labels[i], out);
        for (const auto& stmt : cases[i]->statements()) {
            this->writeStatement(*stmt, out);
        }
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
    SkASSERT(fProgram.fKind == Program::kGeometry_Kind);
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

void SPIRVCodeGenerator::writeInstructions(const Program& program, OutputStream& out) {
    fGLSLExtendedInstructions = this->nextId();
    StringStream body;
    std::set<SpvId> interfaceVars;
    // assign IDs to functions
    for (const ProgramElement* e : program.elements()) {
        switch (e->kind()) {
            case ProgramElement::Kind::kFunction: {
                const FunctionDefinition& f = e->as<FunctionDefinition>();
                fFunctionMap[&f.declaration()] = this->nextId();
                break;
            }
            default:
                break;
        }
    }
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
    for (const ProgramElement* e : program.elements()) {
        if (e->is<GlobalVarDeclaration>()) {
            this->writeGlobalVar(program.fKind,
                                 e->as<GlobalVarDeclaration>().declaration()->as<VarDeclaration>(),
                                 body);
        }
    }
    for (const ProgramElement* e : program.elements()) {
        if (e->is<FunctionDefinition>()) {
            this->writeFunction(e->as<FunctionDefinition>(), body);
        }
    }
    const FunctionDeclaration* main = nullptr;
    for (auto entry : fFunctionMap) {
        if (entry.first->name() == "main") {
            main = entry.first;
        }
    }
    if (!main) {
        fErrors.error(0, "program does not contain a main() function");
        return;
    }
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
        default:
            ABORT("cannot write this kind of program to SPIR-V\n");
    }
    SpvId entryPoint = fFunctionMap[main];
    this->writeWord(entryPoint, out);
    this->writeString(main->name().fChars, main->name().fLength, out);
    for (int var : interfaceVars) {
        this->writeWord(var, out);
    }
    if (program.fKind == Program::kGeometry_Kind) {
        this->writeGeometryShaderExecutionMode(entryPoint, out);
    }
    if (program.fKind == Program::kFragment_Kind) {
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
