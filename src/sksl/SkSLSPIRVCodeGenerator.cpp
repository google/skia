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
    fIntrinsicMap[String("transpose")]     = ALL_SPIRV(Transpose);
    fIntrinsicMap[String("inversesqrt")]   = ALL_GLSL(InverseSqrt);
    fIntrinsicMap[String("determinant")]   = ALL_GLSL(Determinant);
    fIntrinsicMap[String("matrixInverse")] = ALL_GLSL(MatrixInverse);
    fIntrinsicMap[String("mod")]           = SPECIAL(Mod);
    fIntrinsicMap[String("min")]           = SPECIAL(Min);
    fIntrinsicMap[String("max")]           = SPECIAL(Max);
    fIntrinsicMap[String("clamp")]         = SPECIAL(Clamp);
    fIntrinsicMap[String("saturate")]      = SPECIAL(Saturate);
    fIntrinsicMap[String("dot")]           = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpDot,
                                                             SpvOpUndef, SpvOpUndef, SpvOpUndef);
    fIntrinsicMap[String("mix")]           = SPECIAL(Mix);
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
    fIntrinsicMap[String("dFdy")]        = SPECIAL(DFdy);
    fIntrinsicMap[String("fwidth")]      = std::make_tuple(kSPIRV_IntrinsicKind, SpvOpFwidth,
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
    return type == *context.fFloat_Type || type == *context.fHalf_Type ||
           type == *context.fDouble_Type;
}

static bool is_signed(const Context& context, const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return is_signed(context, type.componentType());
    }
    return type == *context.fInt_Type || type == *context.fShort_Type ||
           type == *context.fByte_Type;
}

static bool is_unsigned(const Context& context, const Type& type) {
    if (type.kind() == Type::kVector_Kind) {
        return is_unsigned(context, type.componentType());
    }
    return type == *context.fUInt_Type || type == *context.fUShort_Type ||
           type == *context.fUByte_Type;
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

void SPIRVCodeGenerator::writeOpCode(SpvOp_ opCode, int length, OutputStream& out) {
    SkASSERT(opCode != SpvOpLoad || &out != &fConstantBuffer);
    SkASSERT(opCode != SpvOpUndef);
    switch (opCode) {
        case SpvOpReturn:      // fall through
        case SpvOpReturnValue: // fall through
        case SpvOpKill:        // fall through
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
        const Type::Field& field = type.fields()[i];
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
        if (field.fType->kind() == Type::kMatrix_Kind) {
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
        Type::Kind kind = field.fType->kind();
        if ((kind == Type::kArray_Kind || kind == Type::kStruct_Kind) && offset % alignment != 0) {
            offset += alignment - offset % alignment;
        }
    }
}

Type SPIRVCodeGenerator::getActualType(const Type& type) {
    if (type.isFloat()) {
        return *fContext.fFloat_Type;
    }
    if (type.isSigned()) {
        return *fContext.fInt_Type;
    }
    if (type.isUnsigned()) {
        return *fContext.fUInt_Type;
    }
    if (type.kind() == Type::kMatrix_Kind || type.kind() == Type::kVector_Kind) {
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
    Type type = this->getActualType(rawType);
    String key = type.name() + to_string((int) layout.fStd);
    auto entry = fTypeMap.find(key);
    if (entry == fTypeMap.end()) {
        SpvId result = this->nextId();
        switch (type.kind()) {
            case Type::kScalar_Kind:
                if (type == *fContext.fBool_Type) {
                    this->writeInstruction(SpvOpTypeBool, result, fConstantBuffer);
                } else if (type == *fContext.fInt_Type || type == *fContext.fShort_Type ||
                           type == *fContext.fIntLiteral_Type) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 1, fConstantBuffer);
                } else if (type == *fContext.fUInt_Type || type == *fContext.fUShort_Type) {
                    this->writeInstruction(SpvOpTypeInt, result, 32, 0, fConstantBuffer);
                } else if (type == *fContext.fFloat_Type || type == *fContext.fHalf_Type ||
                           type == *fContext.fFloatLiteral_Type) {
                    this->writeInstruction(SpvOpTypeFloat, result, 32, fConstantBuffer);
                } else if (type == *fContext.fDouble_Type) {
                    this->writeInstruction(SpvOpTypeFloat, result, 64, fConstantBuffer);
                } else {
                    SkASSERT(false);
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
                    IntLiteral count(fContext, -1, type.columns());
                    this->writeInstruction(SpvOpTypeArray, result,
                                           this->getType(type.componentType(), layout),
                                           this->writeIntLiteral(count), fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(type),
                                           fDecorationBuffer);
                } else {
                    SkASSERT(false); // we shouldn't have any runtime-sized arrays right now
                    this->writeInstruction(SpvOpTypeRuntimeArray, result,
                                           this->getType(type.componentType(), layout),
                                           fConstantBuffer);
                    this->writeInstruction(SpvOpDecorate, result, SpvDecorationArrayStride,
                                           (int32_t) layout.stride(type),
                                           fDecorationBuffer);
                }
                break;
            }
            case Type::kSampler_Kind: {
                SpvId image = result;
                if (SpvDimSubpassData != type.dimensions()) {
                    image = this->nextId();
                }
                if (SpvDimBuffer == type.dimensions()) {
                    fCapabilities |= (((uint64_t) 1) << SpvCapabilitySampledBuffer);
                }
                this->writeInstruction(SpvOpTypeImage, image,
                                       this->getType(*fContext.fFloat_Type, layout),
                                       type.dimensions(), type.isDepth(), type.isArrayed(),
                                       type.isMultisampled(), type.isSampled() ? 1 : 2,
                                       SpvImageFormatUnknown, fConstantBuffer);
                fImageTypeMap[key] = image;
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

SpvId SPIRVCodeGenerator::getImageType(const Type& type) {
    SkASSERT(type.kind() == Type::kSampler_Kind);
    this->getType(type);
    String key = type.name() + to_string((int) fDefaultLayout.fStd);
    SkASSERT(fImageTypeMap.find(key) != fImageTypeMap.end());
    return fImageTypeMap[key];
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

SpvId SPIRVCodeGenerator::getPointerType(const Type& rawType, const MemoryLayout& layout,
                                         SpvStorageClass_ storageClass) {
    Type type = this->getActualType(rawType);
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
    SkASSERT(intrinsic != fIntrinsicMap.end());
    int32_t intrinsicId;
    if (c.fArguments.size() > 0) {
        const Type& type = c.fArguments[0]->fType;
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
            std::vector<SpvId> arguments;
            for (size_t i = 0; i < c.fArguments.size(); i++) {
                if (c.fFunction.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag) {
                    arguments.push_back(this->getLValue(*c.fArguments[i], out)->getPointer());
                } else {
                    arguments.push_back(this->writeExpression(*c.fArguments[i], out));
                }
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
                if (c.fFunction.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag) {
                    arguments.push_back(this->getLValue(*c.fArguments[i], out)->getPointer());
                } else {
                    arguments.push_back(this->writeExpression(*c.fArguments[i], out));
                }
            }
            if (c.fType != *fContext.fVoid_Type) {
                this->writeOpCode((SpvOp_) intrinsicId, 3 + (int32_t) arguments.size(), out);
                this->writeWord(this->getType(c.fType), out);
                this->writeWord(result, out);
            } else {
                this->writeOpCode((SpvOp_) intrinsicId, 1 + (int32_t) arguments.size(), out);
            }
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

std::vector<SpvId> SPIRVCodeGenerator::vectorize(
                                               const std::vector<std::unique_ptr<Expression>>& args,
                                               OutputStream& out) {
    int vectorSize = 0;
    for (const auto& a : args) {
        if (a->fType.kind() == Type::kVector_Kind) {
            if (vectorSize) {
                SkASSERT(a->fType.columns() == vectorSize);
            }
            else {
                vectorSize = a->fType.columns();
            }
        }
    }
    std::vector<SpvId> result;
    for (const auto& a : args) {
        SpvId raw = this->writeExpression(*a, out);
        if (vectorSize && a->fType.kind() == Type::kScalar_Kind) {
            SpvId vector = this->nextId();
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vectorSize, out);
            this->writeWord(this->getType(a->fType.toCompound(fContext, vectorSize, 1)), out);
            this->writeWord(vector, out);
            for (int i = 0; i < vectorSize; i++) {
                this->writeWord(raw, out);
            }
            this->writePrecisionModifier(a->fType, vector);
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
            break;
        }
        case kSubpassLoad_SpecialIntrinsic: {
            SpvId img = this->writeExpression(*c.fArguments[0], out);
            std::vector<std::unique_ptr<Expression>> args;
            args.emplace_back(new FloatLiteral(fContext, -1, 0.0));
            args.emplace_back(new FloatLiteral(fContext, -1, 0.0));
            Constructor ctor(-1, *fContext.fFloat2_Type, std::move(args));
            SpvId coords = this->writeConstantVector(ctor);
            if (1 == c.fArguments.size()) {
                this->writeInstruction(SpvOpImageRead,
                                       this->getType(c.fType),
                                       result,
                                       img,
                                       coords,
                                       out);
            } else {
                SkASSERT(2 == c.fArguments.size());
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
        case kTexture_SpecialIntrinsic: {
            SpvOp_ op = SpvOpImageSampleImplicitLod;
            switch (c.fArguments[0]->fType.dimensions()) {
                case SpvDim1D:
                    if (c.fArguments[1]->fType == *fContext.fFloat2_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(c.fArguments[1]->fType == *fContext.fFloat_Type);
                    }
                    break;
                case SpvDim2D:
                    if (c.fArguments[1]->fType == *fContext.fFloat3_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(c.fArguments[1]->fType == *fContext.fFloat2_Type);
                    }
                    break;
                case SpvDim3D:
                    if (c.fArguments[1]->fType == *fContext.fFloat4_Type) {
                        op = SpvOpImageSampleProjImplicitLod;
                    } else {
                        SkASSERT(c.fArguments[1]->fType == *fContext.fFloat3_Type);
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
                SkASSERT(c.fArguments.size() == 2);
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
            std::vector<SpvId> args = this->vectorize(c.fArguments, out);
            SkASSERT(args.size() == 2);
            const Type& operandType = c.fArguments[0]->fType;
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
            SpvId fn = this->writeExpression(*c.fArguments[0], out);
            this->writeOpCode(SpvOpDPdy, 4, out);
            this->writeWord(this->getType(c.fType), out);
            this->writeWord(result, out);
            this->writeWord(fn, out);
            if (fProgram.fSettings.fFlipY) {
                // Flipping Y also negates the Y derivatives.
                SpvId flipped = this->nextId();
                this->writeInstruction(SpvOpFNegate, this->getType(c.fType), flipped, result, out);
                this->writePrecisionModifier(c.fType, flipped);
                return flipped;
            }
            break;
        }
        case kClamp_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(c.fArguments, out);
            SkASSERT(args.size() == 3);
            this->writeGLSLExtendedInstruction(c.fType, result, GLSLstd450FClamp, GLSLstd450SClamp,
                                               GLSLstd450UClamp, args, out);
            break;
        }
        case kMax_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(c.fArguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(c.fType, result, GLSLstd450FMax, GLSLstd450SMax,
                                               GLSLstd450UMax, args, out);
            break;
        }
        case kMin_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(c.fArguments, out);
            SkASSERT(args.size() == 2);
            this->writeGLSLExtendedInstruction(c.fType, result, GLSLstd450FMin, GLSLstd450SMin,
                                               GLSLstd450UMin, args, out);
            break;
        }
        case kMix_SpecialIntrinsic: {
            std::vector<SpvId> args = this->vectorize(c.fArguments, out);
            SkASSERT(args.size() == 3);
            this->writeGLSLExtendedInstruction(c.fType, result, GLSLstd450FMix, SpvOpUndef,
                                               SpvOpUndef, args, out);
            break;
        }
        case kSaturate_SpecialIntrinsic: {
            SkASSERT(c.fArguments.size() == 1);
            std::vector<std::unique_ptr<Expression>> finalArgs;
            finalArgs.push_back(c.fArguments[0]->clone());
            finalArgs.emplace_back(new FloatLiteral(fContext, -1, 0));
            finalArgs.emplace_back(new FloatLiteral(fContext, -1, 1));
            std::vector<SpvId> spvArgs = this->vectorize(finalArgs, out);
            this->writeGLSLExtendedInstruction(c.fType, result, GLSLstd450FClamp, GLSLstd450SClamp,
                                               GLSLstd450UClamp, spvArgs, out);
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
    std::vector<std::tuple<SpvId, const Type*, std::unique_ptr<LValue>>> lvalues;
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
                lvalues.push_back(std::make_tuple(tmpVar, &c.fArguments[i]->fType, std::move(lv)));
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
        this->writeInstruction(SpvOpLoad, getType(*std::get<1>(tuple)), load, std::get<0>(tuple),
                               out);
        this->writePrecisionModifier(*std::get<1>(tuple), load);
        std::get<2>(tuple)->store(load, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstantVector(const Constructor& c) {
    SkASSERT(c.fType.kind() == Type::kVector_Kind && c.isConstant());
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
    SkASSERT(c.fType.isFloat());
    SkASSERT(c.fArguments.size() == 1);
    SkASSERT(c.fArguments[0]->fType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.fArguments[0], out);
    if (c.fArguments[0]->fType.isSigned()) {
        this->writeInstruction(SpvOpConvertSToF, this->getType(c.fType), result, parameter,
                               out);
    } else {
        SkASSERT(c.fArguments[0]->fType.isUnsigned());
        this->writeInstruction(SpvOpConvertUToF, this->getType(c.fType), result, parameter,
                               out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeIntConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.fType.isSigned());
    SkASSERT(c.fArguments.size() == 1);
    SkASSERT(c.fArguments[0]->fType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.fArguments[0], out);
    if (c.fArguments[0]->fType.isFloat()) {
        this->writeInstruction(SpvOpConvertFToS, this->getType(c.fType), result, parameter,
                               out);
    }
    else {
        SkASSERT(c.fArguments[0]->fType.isUnsigned());
        this->writeInstruction(SpvOpBitcast, this->getType(c.fType), result, parameter,
                               out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeUIntConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.fType.isUnsigned());
    SkASSERT(c.fArguments.size() == 1);
    SkASSERT(c.fArguments[0]->fType.isNumber());
    SpvId result = this->nextId();
    SpvId parameter = this->writeExpression(*c.fArguments[0], out);
    if (c.fArguments[0]->fType.isFloat()) {
        this->writeInstruction(SpvOpConvertFToU, this->getType(c.fType), result, parameter,
                               out);
    } else {
        SkASSERT(c.fArguments[0]->fType.isSigned());
        this->writeInstruction(SpvOpBitcast, this->getType(c.fType), result, parameter,
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
    for (SpvId id : columnIds) {
        this->writeWord(id, out);
    }
    this->writePrecisionModifier(type, id);
}

void SPIRVCodeGenerator::writeMatrixCopy(SpvId id, SpvId src, const Type& srcType,
                                         const Type& dstType, OutputStream& out) {
    SkASSERT(srcType.kind() == Type::kMatrix_Kind);
    SkASSERT(dstType.kind() == Type::kMatrix_Kind);
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
                for (int i = 0; i < delta; ++i) {
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
                for (int i = 0; i < count; i++) {
                    this->writeWord(i, out);
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
                for (int i = 0; i < dstType.rows(); ++i) {
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
    SkASSERT(c.fType.kind() == Type::kMatrix_Kind);
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
    } else if (arguments.size() == 1 && c.fArguments[0]->fType.kind() == Type::kVector_Kind) {
        SkASSERT(c.fType.rows() == 2 && c.fType.columns() == 2);
        SkASSERT(c.fArguments[0]->fType.columns() == 4);
        SpvId componentType = this->getType(c.fType.componentType());
        SpvId v[4];
        for (int i = 0; i < 4; ++i) {
            v[i] = this->nextId();
            this->writeInstruction(SpvOpCompositeExtract, componentType, v[i], arguments[0], i, out);
        }
        SpvId columnType = this->getType(c.fType.componentType().toCompound(fContext, 2, 1));
        SpvId column1 = this->nextId();
        this->writeInstruction(SpvOpCompositeConstruct, columnType, column1, v[0], v[1], out);
        SpvId column2 = this->nextId();
        this->writeInstruction(SpvOpCompositeConstruct, columnType, column2, v[2], v[3], out);
        this->writeInstruction(SpvOpCompositeConstruct, this->getType(c.fType), result, column1,
                               column2, out);
    } else {
        SpvId columnType = this->getType(c.fType.componentType().toCompound(fContext, rows, 1));
        std::vector<SpvId> columnIds;
        // ids of vectors and scalars we have written to the current column so far
        std::vector<SpvId> currentColumn;
        // the total number of scalars represented by currentColumn's entries
        int currentCount = 0;
        Precision precision = c.fType.highPrecision() ? Precision::kHigh : Precision::kLow;
        for (size_t i = 0; i < arguments.size(); i++) {
            if (currentCount == 0 && c.fArguments[i]->fType.kind() == Type::kVector_Kind &&
                    c.fArguments[i]->fType.columns() == c.fType.rows()) {
                // this is a complete column by itself
                columnIds.push_back(arguments[i]);
            } else {
                if (c.fArguments[i]->fType.columns() == 1) {
                    this->addColumnEntry(columnType, precision, &currentColumn, &columnIds,
                                         &currentCount, rows, arguments[i], out);
                } else {
                    SpvId componentType = this->getType(c.fArguments[i]->fType.componentType());
                    for (int j = 0; j < c.fArguments[i]->fType.columns(); ++j) {
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
        this->writeWord(this->getType(c.fType), out);
        this->writeWord(result, out);
        for (SpvId id : columnIds) {
            this->writeWord(id, out);
        }
    }
    this->writePrecisionModifier(c.fType, result);
    return result;
}

SpvId SPIRVCodeGenerator::writeVectorConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.fType.kind() == Type::kVector_Kind);
    if (c.isConstant()) {
        return this->writeConstantVector(c);
    }
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.fArguments.size(); i++) {
        if (c.fArguments[i]->fType.kind() == Type::kVector_Kind) {
            // SPIR-V doesn't support vector(vector-of-different-type) directly, so we need to
            // extract the components and convert them in that case manually. On top of that,
            // as of this writing there's a bug in the Intel Vulkan driver where OpCreateComposite
            // doesn't handle vector arguments at all, so we always extract vector components and
            // pass them into OpCreateComposite individually.
            SpvId vec = this->writeExpression(*c.fArguments[i], out);
            SpvOp_ op = SpvOpUndef;
            const Type& src = c.fArguments[i]->fType.componentType();
            const Type& dst = c.fType.componentType();
            if (dst == *fContext.fFloat_Type || dst == *fContext.fHalf_Type) {
                if (src == *fContext.fFloat_Type || src == *fContext.fHalf_Type) {
                    if (c.fArguments.size() == 1) {
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
                    if (c.fArguments.size() == 1) {
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
                    if (c.fArguments.size() == 1) {
                        return vec;
                    }
                } else {
                    SkASSERT(false);
                }
            }
            for (int j = 0; j < c.fArguments[i]->fType.columns(); j++) {
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
            arguments.push_back(this->writeExpression(*c.fArguments[i], out));
        }
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
        SkASSERT(arguments.size() > 1);
        this->writeOpCode(SpvOpCompositeConstruct, 3 + (int32_t) arguments.size(), out);
        this->writeWord(this->getType(c.fType), out);
        this->writeWord(result, out);
        for (SpvId id : arguments) {
            this->writeWord(id, out);
        }
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeArrayConstructor(const Constructor& c, OutputStream& out) {
    SkASSERT(c.fType.kind() == Type::kArray_Kind);
    // go ahead and write the arguments so we don't try to write new instructions in the middle of
    // an instruction
    std::vector<SpvId> arguments;
    for (size_t i = 0; i < c.fArguments.size(); i++) {
        arguments.push_back(this->writeExpression(*c.fArguments[i], out));
    }
    SpvId result = this->nextId();
    this->writeOpCode(SpvOpCompositeConstruct, 3 + (int32_t) c.fArguments.size(), out);
    this->writeWord(this->getType(c.fType), out);
    this->writeWord(result, out);
    for (SpvId id : arguments) {
        this->writeWord(id, out);
    }
    return result;
}

SpvId SPIRVCodeGenerator::writeConstructor(const Constructor& c, OutputStream& out) {
    if (c.fArguments.size() == 1 &&
        this->getActualType(c.fType) == this->getActualType(c.fArguments[0]->fType)) {
        return this->writeExpression(*c.fArguments[0], out);
    }
    if (c.fType == *fContext.fFloat_Type || c.fType == *fContext.fHalf_Type) {
        return this->writeFloatConstructor(c, out);
    } else if (c.fType == *fContext.fInt_Type ||
               c.fType == *fContext.fShort_Type ||
               c.fType == *fContext.fByte_Type) {
        return this->writeIntConstructor(c, out);
    } else if (c.fType == *fContext.fUInt_Type ||
               c.fType == *fContext.fUShort_Type ||
               c.fType == *fContext.fUByte_Type) {
        return this->writeUIntConstructor(c, out);
    }
    switch (c.fType.kind()) {
        case Type::kVector_Kind:
            return this->writeVectorConstructor(c, out);
        case Type::kMatrix_Kind:
            return this->writeMatrixConstructor(c, out);
        case Type::kArray_Kind:
            return this->writeArrayConstructor(c, out);
        default:
            ABORT("unsupported constructor: %s", c.description().c_str());
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
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) expr).fVariable;
            if (var.fStorage != Variable::kGlobal_Storage) {
                return SpvStorageClassFunction;
            }
            SpvStorageClass_ result = get_storage_class(var.fModifiers);
            if (result == SpvStorageClassFunction) {
                result = SpvStorageClassPrivate;
            }
            return result;
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
            IntLiteral index(fContext, -1, fieldExpr.fFieldIndex);
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

    virtual SpvId getPointer() override {
        return fPointer;
    }

    virtual SpvId load(OutputStream& out) override {
        SpvId result = fGen.nextId();
        fGen.writeInstruction(SpvOpLoad, fType, result, fPointer, out);
        fGen.writePrecisionModifier(fPrecision, result);
        return result;
    }

    virtual void store(SpvId value, OutputStream& out) override {
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
    SwizzleLValue(SPIRVCodeGenerator& gen, SpvId vecPointer, const std::vector<int>& components,
                  const Type& baseType, const Type& swizzleType,
                  SPIRVCodeGenerator::Precision precision)
    : fGen(gen)
    , fVecPointer(vecPointer)
    , fComponents(components)
    , fBaseType(baseType)
    , fSwizzleType(swizzleType)
    , fPrecision(precision) {}

    virtual SpvId getPointer() override {
        return 0;
    }

    virtual SpvId load(OutputStream& out) override {
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

    virtual void store(SpvId value, OutputStream& out) override {
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
    const std::vector<int>& fComponents;
    const Type& fBaseType;
    const Type& fSwizzleType;
    const SPIRVCodeGenerator::Precision fPrecision;
};

std::unique_ptr<SPIRVCodeGenerator::LValue> SPIRVCodeGenerator::getLValue(const Expression& expr,
                                                                          OutputStream& out) {
    Precision precision = expr.fType.highPrecision() ? Precision::kHigh : Precision::kLow;
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            SpvId type;
            const Variable& var = ((VariableReference&) expr).fVariable;
            if (var.fModifiers.fLayout.fBuiltin == SK_IN_BUILTIN) {
                type = this->getType(Type("sk_in", Type::kArray_Kind, var.fType.componentType(),
                                          fSkInCount));
            } else {
                type = this->getType(expr.fType);
            }
            auto entry = fVariableMap.find(&var);
            SkASSERT(entry != fVariableMap.end());
            return std::unique_ptr<SPIRVCodeGenerator::LValue>(new PointerLValue(*this,
                                                                                 entry->second,
                                                                                 type,
                                                                                 precision));
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
                                                                        this->getType(expr.fType),
                                                                        precision));
        }
        case Expression::kSwizzle_Kind: {
            Swizzle& swizzle = (Swizzle&) expr;
            size_t count = swizzle.fComponents.size();
            SpvId base = this->getLValue(*swizzle.fBase, out)->getPointer();
            SkASSERT(base);
            if (count == 1) {
                IntLiteral index(fContext, -1, swizzle.fComponents[0]);
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
                                                                       this->getType(expr.fType),
                                                                       precision));
            } else {
                return std::unique_ptr<SPIRVCodeGenerator::LValue>(new SwizzleLValue(
                                                                              *this,
                                                                              base,
                                                                              swizzle.fComponents,
                                                                              swizzle.fBase->fType,
                                                                              expr.fType,
                                                                              precision));
            }
        }
        case Expression::kTernary_Kind: {
            TernaryExpression& t = (TernaryExpression&) expr;
            SpvId test = this->writeExpression(*t.fTest, out);
            SpvId end = this->nextId();
            SpvId ifTrueLabel = this->nextId();
            SpvId ifFalseLabel = this->nextId();
            this->writeInstruction(SpvOpSelectionMerge, end, SpvSelectionControlMaskNone, out);
            this->writeInstruction(SpvOpBranchConditional, test, ifTrueLabel, ifFalseLabel, out);
            this->writeLabel(ifTrueLabel, out);
            SpvId ifTrue = this->getLValue(*t.fIfTrue, out)->getPointer();
            SkASSERT(ifTrue);
            this->writeInstruction(SpvOpBranch, end, out);
            ifTrueLabel = fCurrentBlock;
            SpvId ifFalse = this->getLValue(*t.fIfFalse, out)->getPointer();
            SkASSERT(ifFalse);
            ifFalseLabel = fCurrentBlock;
            this->writeInstruction(SpvOpBranch, end, out);
            SpvId result = this->nextId();
            this->writeInstruction(SpvOpPhi, this->getType(*fContext.fBool_Type), result, ifTrue,
                       ifTrueLabel, ifFalse, ifFalseLabel, out);
            return std::unique_ptr<SPIRVCodeGenerator::LValue>(new PointerLValue(
                                                                       *this,
                                                                       result,
                                                                       this->getType(expr.fType),
                                                                       precision));
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
                                                                       this->getType(expr.fType),
                                                                       precision));
    }
}

SpvId SPIRVCodeGenerator::writeVariableReference(const VariableReference& ref, OutputStream& out) {
    SpvId result = this->nextId();
    auto entry = fVariableMap.find(&ref.fVariable);
    SkASSERT(entry != fVariableMap.end());
    SpvId var = entry->second;
    this->writeInstruction(SpvOpLoad, this->getType(ref.fVariable.fType), result, var, out);
    this->writePrecisionModifier(ref.fVariable.fType, result);
    if (ref.fVariable.fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN &&
        fProgram.fSettings.fFlipY) {
        // need to remap to a top-left coordinate system
        if (fRTHeightStructId == (SpvId) -1) {
            // height variable hasn't been written yet
            std::shared_ptr<SymbolTable> st(new SymbolTable(&fErrors));
            SkASSERT(fRTHeightFieldIndex == (SpvId) -1);
            std::vector<Type::Field> fields;
            fields.emplace_back(Modifiers(), SKSL_RTHEIGHT_NAME, fContext.fFloat_Type.get());
            StringFragment name("sksl_synthetic_uniforms");
            Type intfStruct(-1, name, fields);
            Layout layout(0, -1, -1, 1, -1, -1, -1, -1, Layout::Format::kUnspecified,
                          Layout::kUnspecified_Primitive, -1, -1, "", Layout::kNo_Key,
                          Layout::CType::kDefault);
            Variable* intfVar = new Variable(-1,
                                             Modifiers(layout, Modifiers::kUniform_Flag),
                                             name,
                                             intfStruct,
                                             Variable::kGlobal_Storage);
            fSynthetics.takeOwnership(intfVar);
            InterfaceBlock intf(-1, intfVar, name, String(""),
                                std::vector<std::unique_ptr<Expression>>(), st);
            fRTHeightStructId = this->writeInterfaceBlock(intf);
            fRTHeightFieldIndex = 0;
        }
        SkASSERT(fRTHeightFieldIndex != (SpvId) -1);
        // write float4(gl_FragCoord.x, u_skRTHeight - gl_FragCoord.y, 0.0, gl_FragCoord.w)
        SpvId xId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), xId,
                               result, 0, out);
        IntLiteral fieldIndex(fContext, -1, fRTHeightFieldIndex);
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
        FloatLiteral zero(fContext, -1, 0.0);
        SpvId zeroId = writeFloatLiteral(zero);
        FloatLiteral one(fContext, -1, 1.0);
        SpvId wId = this->nextId();
        this->writeInstruction(SpvOpCompositeExtract, this->getType(*fContext.fFloat_Type), wId,
                               result, 3, out);
        SpvId flipped = this->nextId();
        this->writeOpCode(SpvOpCompositeConstruct, 7, out);
        this->writeWord(this->getType(*fContext.fFloat4_Type), out);
        this->writeWord(flipped, out);
        this->writeWord(xId, out);
        this->writeWord(flippedYId, out);
        this->writeWord(zeroId, out);
        this->writeWord(wId, out);
        return flipped;
    }
    if (ref.fVariable.fModifiers.fLayout.fBuiltin == SK_CLOCKWISE_BUILTIN &&
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
    if (expr.fBase->fType.kind() == Type::Kind::kVector_Kind) {
        SpvId base = this->writeExpression(*expr.fBase, out);
        SpvId index = this->writeExpression(*expr.fIndex, out);
        SpvId result = this->nextId();
        this->writeInstruction(SpvOpVectorExtractDynamic, this->getType(expr.fType), result, base,
                               index, out);
        return result;
    }
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
        SpvId other;
        int last = swizzle.fComponents.back();
        if (last < 0) {
            if (!fConstantZeroOneVector) {
                FloatLiteral zero(fContext, -1, 0);
                SpvId zeroId = this->writeFloatLiteral(zero);
                FloatLiteral one(fContext, -1, 1);
                SpvId oneId = this->writeFloatLiteral(one);
                SpvId type = this->getType(*fContext.fFloat2_Type);
                fConstantZeroOneVector = this->nextId();
                this->writeOpCode(SpvOpConstantComposite, 5, fConstantBuffer);
                this->writeWord(type, fConstantBuffer);
                this->writeWord(fConstantZeroOneVector, fConstantBuffer);
                this->writeWord(zeroId, fConstantBuffer);
                this->writeWord(oneId, fConstantBuffer);
            }
            other = fConstantZeroOneVector;
        } else {
            other = base;
        }
        this->writeWord(other, out);
        for (int component : swizzle.fComponents) {
            if (component == SKSL_SWIZZLE_0) {
                this->writeWord(swizzle.fBase->fType.columns(), out);
            } else if (component == SKSL_SWIZZLE_1) {
                this->writeWord(swizzle.fBase->fType.columns() + 1, out);
            } else {
                this->writeWord(component, out);
            }
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
        return result; // skip RelaxedPrecision check
    } else {
        ABORT("invalid operandType: %s", operandType.description().c_str());
    }
    if (getActualType(resultType) == operandType && !resultType.highPrecision()) {
        this->writeInstruction(SpvOpDecorate, result, SpvDecorationRelaxedPrecision,
                               fDecorationBuffer);
    }
    return result;
}

SpvId SPIRVCodeGenerator::foldToBool(SpvId id, const Type& operandType, SpvOp op,
                                     OutputStream& out) {
    if (operandType.kind() == Type::kVector_Kind) {
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
    SkASSERT(operandType.kind() == Type::kMatrix_Kind);
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
    SkASSERT(operandType.kind() == Type::kMatrix_Kind);
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
    if (b.fOperator == Token::COMMA) {
        return rhs;
    }
    Type tmp("<invalid>");
    // overall type we are operating on: float2, int, uint4...
    const Type* operandType;
    // IR allows mismatched types in expressions (e.g. float2 * float), but they need special
    // handling in SPIR-V
    if (this->getActualType(b.fLeft->fType) != this->getActualType(b.fRight->fType)) {
        if (b.fLeft->fType.kind() == Type::kVector_Kind &&
            b.fRight->fType.isNumber()) {
            // promote number to vector
            SpvId vec = this->nextId();
            const Type& vecType = b.fLeft->fType;
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vecType.columns(), out);
            this->writeWord(this->getType(vecType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < vecType.columns(); i++) {
                this->writeWord(rhs, out);
            }
            rhs = vec;
            operandType = &b.fLeft->fType;
        } else if (b.fRight->fType.kind() == Type::kVector_Kind &&
                   b.fLeft->fType.isNumber()) {
            // promote number to vector
            SpvId vec = this->nextId();
            const Type& vecType = b.fRight->fType;
            this->writeOpCode(SpvOpCompositeConstruct, 3 + vecType.columns(), out);
            this->writeWord(this->getType(vecType), out);
            this->writeWord(vec, out);
            for (int i = 0; i < vecType.columns(); i++) {
                this->writeWord(lhs, out);
            }
            lhs = vec;
            SkASSERT(!lvalue);
            operandType = &b.fRight->fType;
        } else if (b.fLeft->fType.kind() == Type::kMatrix_Kind) {
            SpvOp_ op;
            if (b.fRight->fType.kind() == Type::kMatrix_Kind) {
                op = SpvOpMatrixTimesMatrix;
            } else if (b.fRight->fType.kind() == Type::kVector_Kind) {
                op = SpvOpMatrixTimesVector;
            } else {
                SkASSERT(b.fRight->fType.kind() == Type::kScalar_Kind);
                op = SpvOpMatrixTimesScalar;
            }
            SpvId result = this->nextId();
            this->writeInstruction(op, this->getType(b.fType), result, lhs, rhs, out);
            if (b.fOperator == Token::STAREQ) {
                lvalue->store(result, out);
            } else {
                SkASSERT(b.fOperator == Token::STAR);
            }
            return result;
        } else if (b.fRight->fType.kind() == Type::kMatrix_Kind) {
            SpvId result = this->nextId();
            if (b.fLeft->fType.kind() == Type::kVector_Kind) {
                this->writeInstruction(SpvOpVectorTimesMatrix, this->getType(b.fType), result,
                                       lhs, rhs, out);
            } else {
                SkASSERT(b.fLeft->fType.kind() == Type::kScalar_Kind);
                this->writeInstruction(SpvOpMatrixTimesScalar, this->getType(b.fType), result, rhs,
                                       lhs, out);
            }
            if (b.fOperator == Token::STAREQ) {
                lvalue->store(result, out);
            } else {
                SkASSERT(b.fOperator == Token::STAR);
            }
            return result;
        } else {
            ABORT("unsupported binary expression: %s (%s, %s)", b.description().c_str(),
                  b.fLeft->fType.description().c_str(), b.fRight->fType.description().c_str());
        }
    } else {
        tmp = this->getActualType(b.fLeft->fType);
        operandType = &tmp;
        SkASSERT(*operandType == this->getActualType(b.fRight->fType));
    }
    switch (b.fOperator) {
        case Token::EQEQ: {
            if (operandType->kind() == Type::kMatrix_Kind) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFOrdEqual,
                                                   SpvOpIEqual, SpvOpAll, SpvOpLogicalAnd, out);
            }
            SkASSERT(resultType == *fContext.fBool_Type);
            const Type* tmpType;
            if (operandType->kind() == Type::kVector_Kind) {
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
        case Token::NEQ:
            if (operandType->kind() == Type::kMatrix_Kind) {
                return this->writeMatrixComparison(*operandType, lhs, rhs, SpvOpFOrdNotEqual,
                                                   SpvOpINotEqual, SpvOpAny, SpvOpLogicalOr, out);
            }
            SkASSERT(resultType == *fContext.fBool_Type);
            const Type* tmpType;
            if (operandType->kind() == Type::kVector_Kind) {
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
        case Token::GT:
            SkASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThan, SpvOpSGreaterThan,
                                              SpvOpUGreaterThan, SpvOpUndef, out);
        case Token::LT:
            SkASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFOrdLessThan,
                                              SpvOpSLessThan, SpvOpULessThan, SpvOpUndef, out);
        case Token::GTEQ:
            SkASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdGreaterThanEqual, SpvOpSGreaterThanEqual,
                                              SpvOpUGreaterThanEqual, SpvOpUndef, out);
        case Token::LTEQ:
            SkASSERT(resultType == *fContext.fBool_Type);
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                              SpvOpFOrdLessThanEqual, SpvOpSLessThanEqual,
                                              SpvOpULessThanEqual, SpvOpUndef, out);
        case Token::PLUS:
            if (b.fLeft->fType.kind() == Type::kMatrix_Kind &&
                b.fRight->fType.kind() == Type::kMatrix_Kind) {
                SkASSERT(b.fLeft->fType == b.fRight->fType);
                return this->writeComponentwiseMatrixBinary(b.fLeft->fType, lhs, rhs,
                                                            SpvOpFAdd, SpvOpIAdd, out);
            }
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                              SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
        case Token::MINUS:
            if (b.fLeft->fType.kind() == Type::kMatrix_Kind &&
                b.fRight->fType.kind() == Type::kMatrix_Kind) {
                SkASSERT(b.fLeft->fType == b.fRight->fType);
                return this->writeComponentwiseMatrixBinary(b.fLeft->fType, lhs, rhs,
                                                            SpvOpFSub, SpvOpISub, out);
            }
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
        case Token::PERCENT:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMod,
                                              SpvOpSMod, SpvOpUMod, SpvOpUndef, out);
        case Token::SHL:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpShiftLeftLogical, SpvOpShiftLeftLogical,
                                              SpvOpUndef, out);
        case Token::SHR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpShiftRightArithmetic, SpvOpShiftRightLogical,
                                              SpvOpUndef, out);
        case Token::BITWISEAND:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseAnd, SpvOpBitwiseAnd, SpvOpUndef, out);
        case Token::BITWISEOR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseOr, SpvOpBitwiseOr, SpvOpUndef, out);
        case Token::BITWISEXOR:
            return this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpUndef,
                                              SpvOpBitwiseXor, SpvOpBitwiseXor, SpvOpUndef, out);
        case Token::PLUSEQ: {
            SpvId result;
            if (b.fLeft->fType.kind() == Type::kMatrix_Kind &&
                b.fRight->fType.kind() == Type::kMatrix_Kind) {
                SkASSERT(b.fLeft->fType == b.fRight->fType);
                result = this->writeComponentwiseMatrixBinary(b.fLeft->fType, lhs, rhs,
                                                            SpvOpFAdd, SpvOpIAdd, out);
            }
            else {
                result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFAdd,
                                                      SpvOpIAdd, SpvOpIAdd, SpvOpUndef, out);
            }
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::MINUSEQ: {
            SpvId result;
            if (b.fLeft->fType.kind() == Type::kMatrix_Kind &&
                b.fRight->fType.kind() == Type::kMatrix_Kind) {
                SkASSERT(b.fLeft->fType == b.fRight->fType);
                result = this->writeComponentwiseMatrixBinary(b.fLeft->fType, lhs, rhs,
                                                            SpvOpFSub, SpvOpISub, out);
            }
            else {
                result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFSub,
                                                      SpvOpISub, SpvOpISub, SpvOpUndef, out);
            }
            SkASSERT(lvalue);
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
                SkASSERT(lvalue);
                lvalue->store(result, out);
                return result;
            }
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMul,
                                                      SpvOpIMul, SpvOpIMul, SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::SLASHEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFDiv,
                                                      SpvOpSDiv, SpvOpUDiv, SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::PERCENTEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs, SpvOpFMod,
                                                      SpvOpSMod, SpvOpUMod, SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::SHLEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                      SpvOpUndef, SpvOpShiftLeftLogical,
                                                      SpvOpShiftLeftLogical, SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::SHREQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                      SpvOpUndef, SpvOpShiftRightArithmetic,
                                                      SpvOpShiftRightLogical, SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::BITWISEANDEQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                      SpvOpUndef, SpvOpBitwiseAnd, SpvOpBitwiseAnd,
                                                      SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::BITWISEOREQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                      SpvOpUndef, SpvOpBitwiseOr, SpvOpBitwiseOr,
                                                      SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        case Token::BITWISEXOREQ: {
            SpvId result = this->writeBinaryOperation(resultType, *operandType, lhs, rhs,
                                                      SpvOpUndef, SpvOpBitwiseXor, SpvOpBitwiseXor,
                                                      SpvOpUndef, out);
            SkASSERT(lvalue);
            lvalue->store(result, out);
            return result;
        }
        default:
            ABORT("unsupported binary expression: %s", b.description().c_str());
    }
}

SpvId SPIRVCodeGenerator::writeLogicalAnd(const BinaryExpression& a, OutputStream& out) {
    SkASSERT(a.fOperator == Token::LOGICALAND);
    BoolLiteral falseLiteral(fContext, -1, false);
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
    SkASSERT(o.fOperator == Token::LOGICALOR);
    BoolLiteral trueLiteral(fContext, -1, true);
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
    if (t.fIfTrue->fType.columns() == 1 && t.fIfTrue->isConstant() && t.fIfFalse->isConstant()) {
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
    this->writePrecisionModifier(t.fType, result);
    return result;
}

std::unique_ptr<Expression> create_literal_1(const Context& context, const Type& type) {
    if (type.isInteger()) {
        return std::unique_ptr<Expression>(new IntLiteral(-1, 1, &type));
    }
    else if (type.isFloat()) {
        return std::unique_ptr<Expression>(new FloatLiteral(-1, 1.0, &type));
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
        }
        this->writePrecisionModifier(p.fType, result);
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
            SkASSERT(p.fOperand->fType == *fContext.fBool_Type);
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
    ConstantType type;
    if (i.fType == *fContext.fInt_Type) {
        type = ConstantType::kInt;
    } else if (i.fType == *fContext.fUInt_Type) {
        type = ConstantType::kUInt;
    } else if (i.fType == *fContext.fShort_Type) {
        type = ConstantType::kShort;
    } else if (i.fType == *fContext.fUShort_Type) {
        type = ConstantType::kUShort;
    }
    std::pair<ConstantValue, ConstantType> key(i.fValue, type);
    auto entry = fNumberConstants.find(key);
    if (entry == fNumberConstants.end()) {
        SpvId result = this->nextId();
        this->writeInstruction(SpvOpConstant, this->getType(i.fType), result, (SpvId) i.fValue,
                               fConstantBuffer);
        fNumberConstants[key] = result;
        return result;
    }
    return entry->second;
}

SpvId SPIRVCodeGenerator::writeFloatLiteral(const FloatLiteral& f) {
    if (f.fType != *fContext.fDouble_Type) {
        ConstantType type;
        if (f.fType == *fContext.fHalf_Type) {
            type = ConstantType::kHalf;
        } else {
            type = ConstantType::kFloat;
        }
        float value = (float) f.fValue;
        std::pair<ConstantValue, ConstantType> key(f.fValue, type);
        auto entry = fNumberConstants.find(key);
        if (entry == fNumberConstants.end()) {
            SpvId result = this->nextId();
            uint32_t bits;
            SkASSERT(sizeof(bits) == sizeof(value));
            memcpy(&bits, &value, sizeof(bits));
            this->writeInstruction(SpvOpConstant, this->getType(f.fType), result, bits,
                                   fConstantBuffer);
            fNumberConstants[key] = result;
            return result;
        }
        return entry->second;
    } else {
        std::pair<ConstantValue, ConstantType> key(f.fValue, ConstantType::kDouble);
        auto entry = fNumberConstants.find(key);
        if (entry == fNumberConstants.end()) {
            SpvId result = this->nextId();
            uint64_t bits;
            SkASSERT(sizeof(bits) == sizeof(f.fValue));
            memcpy(&bits, &f.fValue, sizeof(bits));
            this->writeInstruction(SpvOpConstant, this->getType(f.fType), result,
                                   bits & 0xffffffff, bits >> 32, fConstantBuffer);
            fNumberConstants[key] = result;
            return result;
        }
        return entry->second;
    }
}

SpvId SPIRVCodeGenerator::writeFunctionStart(const FunctionDeclaration& f, OutputStream& out) {
    SpvId result = fFunctionMap[&f];
    this->writeInstruction(SpvOpFunction, this->getType(f.fReturnType), result,
                           SpvFunctionControlMaskNone, this->getFunctionType(f), out);
    this->writeInstruction(SpvOpName, result, f.fName, fNameBuffer);
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
    StringStream bodyBuffer;
    this->writeBlock((Block&) *f.fBody, bodyBuffer);
    write_stringstream(fVariableBuffer, out);
    if (f.fDeclaration.fName == "main") {
        write_stringstream(fGlobalInitializersBuffer, out);
    }
    write_stringstream(bodyBuffer, out);
    if (fCurrentBlock) {
        if (f.fDeclaration.fReturnType == *fContext.fVoid_Type) {
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

SpvId SPIRVCodeGenerator::writeInterfaceBlock(const InterfaceBlock& intf) {
    bool isBuffer = (0 != (intf.fVariable.fModifiers.fFlags & Modifiers::kBuffer_Flag));
    bool pushConstant = (0 != (intf.fVariable.fModifiers.fLayout.fFlags &
                               Layout::kPushConstant_Flag));
    MemoryLayout memoryLayout = (pushConstant || isBuffer) ?
                                MemoryLayout(MemoryLayout::k430_Standard) :
                                fDefaultLayout;
    SpvId result = this->nextId();
    const Type* type = &intf.fVariable.fType;
    if (fProgram.fInputs.fRTHeight) {
        SkASSERT(fRTHeightStructId == (SpvId) -1);
        SkASSERT(fRTHeightFieldIndex == (SpvId) -1);
        std::vector<Type::Field> fields = type->fields();
        fRTHeightStructId = result;
        fRTHeightFieldIndex = fields.size();
        fields.emplace_back(Modifiers(), StringFragment(SKSL_RTHEIGHT_NAME), fContext.fFloat_Type.get());
        type = new Type(type->fOffset, type->name(), fields);
    }
    SpvId typeId;
    if (intf.fVariable.fModifiers.fLayout.fBuiltin == SK_IN_BUILTIN) {
        for (const auto& e : fProgram) {
            if (e.fKind == ProgramElement::kModifiers_Kind) {
                const Modifiers& m = ((ModifiersDeclaration&) e).fModifiers;
                update_sk_in_count(m, &fSkInCount);
            }
        }
        typeId = this->getType(Type("sk_in", Type::kArray_Kind, intf.fVariable.fType.componentType(),
                                  fSkInCount), memoryLayout);
    } else {
        typeId = this->getType(*type, memoryLayout);
    }
    if (intf.fVariable.fModifiers.fFlags & Modifiers::kBuffer_Flag) {
        this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBufferBlock, fDecorationBuffer);
    } else if (intf.fVariable.fModifiers.fLayout.fBuiltin == -1) {
        this->writeInstruction(SpvOpDecorate, typeId, SpvDecorationBlock, fDecorationBuffer);
    }
    SpvStorageClass_ storageClass = get_storage_class(intf.fVariable.fModifiers);
    SpvId ptrType = this->nextId();
    this->writeInstruction(SpvOpTypePointer, ptrType, storageClass, typeId, fConstantBuffer);
    this->writeInstruction(SpvOpVariable, ptrType, result, storageClass, fConstantBuffer);
    Layout layout = intf.fVariable.fModifiers.fLayout;
    if (intf.fVariable.fModifiers.fFlags & Modifiers::kUniform_Flag && layout.fSet == -1) {
        layout.fSet = 0;
    }
    this->writeLayout(layout, result);
    fVariableMap[&intf.fVariable] = result;
    if (fProgram.fInputs.fRTHeight) {
        delete type;
    }
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

#define BUILTIN_IGNORE 9999
void SPIRVCodeGenerator::writeGlobalVars(Program::Kind kind, const VarDeclarations& decl,
                                         OutputStream& out) {
    for (size_t i = 0; i < decl.fVars.size(); i++) {
        if (decl.fVars[i]->fKind == Statement::kNop_Kind) {
            continue;
        }
        const VarDeclaration& varDecl = (VarDeclaration&) *decl.fVars[i];
        const Variable* var = varDecl.fVar;
        // These haven't been implemented in our SPIR-V generator yet and we only currently use them
        // in the OpenGL backend.
        SkASSERT(!(var->fModifiers.fFlags & (Modifiers::kReadOnly_Flag |
                                           Modifiers::kWriteOnly_Flag |
                                           Modifiers::kCoherent_Flag |
                                           Modifiers::kVolatile_Flag |
                                           Modifiers::kRestrict_Flag)));
        if (var->fModifiers.fLayout.fBuiltin == BUILTIN_IGNORE) {
            continue;
        }
        if (var->fModifiers.fLayout.fBuiltin == SK_FRAGCOLOR_BUILTIN &&
            kind != Program::kFragment_Kind) {
            SkASSERT(!fProgram.fSettings.fFragColorIsInOut);
            continue;
        }
        if (!var->fReadCount && !var->fWriteCount &&
                !(var->fModifiers.fFlags & (Modifiers::kIn_Flag |
                                            Modifiers::kOut_Flag |
                                            Modifiers::kUniform_Flag |
                                            Modifiers::kBuffer_Flag))) {
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
        SpvId type;
        if (var->fModifiers.fLayout.fBuiltin == SK_IN_BUILTIN) {
            type = this->getPointerType(Type("sk_in", Type::kArray_Kind,
                                             var->fType.componentType(), fSkInCount),
                                        storageClass);
        } else {
            type = this->getPointerType(var->fType, storageClass);
        }
        this->writeInstruction(SpvOpVariable, type, id, storageClass, fConstantBuffer);
        this->writeInstruction(SpvOpName, id, var->fName, fNameBuffer);
        this->writePrecisionModifier(var->fType, id);
        if (varDecl.fValue) {
            SkASSERT(!fCurrentBlock);
            fCurrentBlock = -1;
            SpvId value = this->writeExpression(*varDecl.fValue, fGlobalInitializersBuffer);
            this->writeInstruction(SpvOpStore, id, value, fGlobalInitializersBuffer);
            fCurrentBlock = 0;
        }
        this->writeLayout(var->fModifiers.fLayout, id);
        if (var->fModifiers.fFlags & Modifiers::kFlat_Flag) {
            this->writeInstruction(SpvOpDecorate, id, SpvDecorationFlat, fDecorationBuffer);
        }
        if (var->fModifiers.fFlags & Modifiers::kNoPerspective_Flag) {
            this->writeInstruction(SpvOpDecorate, id, SpvDecorationNoPerspective,
                                   fDecorationBuffer);
        }
    }
}

void SPIRVCodeGenerator::writeVarDeclarations(const VarDeclarations& decl, OutputStream& out) {
    for (const auto& stmt : decl.fVars) {
        SkASSERT(stmt->fKind == Statement::kVarDeclaration_Kind);
        VarDeclaration& varDecl = (VarDeclaration&) *stmt;
        const Variable* var = varDecl.fVar;
        // These haven't been implemented in our SPIR-V generator yet and we only currently use them
        // in the OpenGL backend.
        SkASSERT(!(var->fModifiers.fFlags & (Modifiers::kReadOnly_Flag |
                                           Modifiers::kWriteOnly_Flag |
                                           Modifiers::kCoherent_Flag |
                                           Modifiers::kVolatile_Flag |
                                           Modifiers::kRestrict_Flag)));
        SpvId id = this->nextId();
        fVariableMap[var] = id;
        SpvId type = this->getPointerType(var->fType, SpvStorageClassFunction);
        this->writeInstruction(SpvOpVariable, type, id, SpvStorageClassFunction, fVariableBuffer);
        this->writeInstruction(SpvOpName, id, var->fName, fNameBuffer);
        if (varDecl.fValue) {
            SpvId value = this->writeExpression(*varDecl.fValue, out);
            this->writeInstruction(SpvOpStore, id, value, out);
        }
    }
}

void SPIRVCodeGenerator::writeStatement(const Statement& s, OutputStream& out) {
    switch (s.fKind) {
        case Statement::kNop_Kind:
            break;
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
        case Statement::kSwitch_Kind:
            this->writeSwitchStatement((SwitchStatement&) s, out);
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
    SpvId header = this->nextId();
    SpvId start = this->nextId();
    SpvId body = this->nextId();
    SpvId continueTarget = this->nextId();
    fContinueTarget.push(continueTarget);
    SpvId end = this->nextId();
    fBreakTarget.push(end);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(header, out);
    this->writeInstruction(SpvOpLoopMerge, end, continueTarget, SpvLoopControlMaskNone, out);
    this->writeInstruction(SpvOpBranch, start, out);
    this->writeLabel(start, out);
    SpvId test = this->writeExpression(*w.fTest, out);
    this->writeInstruction(SpvOpBranchConditional, test, body, end, out);
    this->writeLabel(body, out);
    this->writeStatement(*w.fStatement, out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, continueTarget, out);
    }
    this->writeLabel(continueTarget, out);
    this->writeInstruction(SpvOpBranch, header, out);
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
    fErrors.error(d.fOffset, "internal error: do loop support has been disabled in SPIR-V, see "
                  "SkSLSPIRVCodeGenerator.cpp for details");

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
    this->writeStatement(*d.fStatement, out);
    if (fCurrentBlock) {
        this->writeInstruction(SpvOpBranch, next, out);
    }
    this->writeLabel(next, out);
    SpvId test = this->writeExpression(*d.fTest, out);
    this->writeInstruction(SpvOpBranchConditional, test, continueTarget, end, out);
    this->writeLabel(continueTarget, out);
    this->writeInstruction(SpvOpBranch, header, out);
    this->writeLabel(end, out);
    fBreakTarget.pop();
    fContinueTarget.pop();
}

void SPIRVCodeGenerator::writeSwitchStatement(const SwitchStatement& s, OutputStream& out) {
    SpvId value = this->writeExpression(*s.fValue, out);
    std::vector<SpvId> labels;
    SpvId end = this->nextId();
    SpvId defaultLabel = end;
    fBreakTarget.push(end);
    int size = 3;
    for (const auto& c : s.fCases) {
        SpvId label = this->nextId();
        labels.push_back(label);
        if (c->fValue) {
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
    for (size_t i = 0; i < s.fCases.size(); ++i) {
        if (!s.fCases[i]->fValue) {
            continue;
        }
        SkASSERT(s.fCases[i]->fValue->fKind == Expression::kIntLiteral_Kind);
        this->writeWord(((IntLiteral&) *s.fCases[i]->fValue).fValue, out);
        this->writeWord(labels[i], out);
    }
    for (size_t i = 0; i < s.fCases.size(); ++i) {
        this->writeLabel(labels[i], out);
        for (const auto& stmt : s.fCases[i]->fStatements) {
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
    if (r.fExpression) {
        this->writeInstruction(SpvOpReturnValue, this->writeExpression(*r.fExpression, out),
                               out);
    } else {
        this->writeInstruction(SpvOpReturn, out);
    }
}

void SPIRVCodeGenerator::writeGeometryShaderExecutionMode(SpvId entryPoint, OutputStream& out) {
    SkASSERT(fProgram.fKind == Program::kGeometry_Kind);
    int invocations = 1;
    for (const auto& e : fProgram) {
        if (e.fKind == ProgramElement::kModifiers_Kind) {
            const Modifiers& m = ((ModifiersDeclaration&) e).fModifiers;
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
    // assign IDs to functions, determine sk_in size
    int skInSize = -1;
    for (const auto& e : program) {
        switch (e.fKind) {
            case ProgramElement::kFunction_Kind: {
                FunctionDefinition& f = (FunctionDefinition&) e;
                fFunctionMap[&f.fDeclaration] = this->nextId();
                break;
            }
            case ProgramElement::kModifiers_Kind: {
                Modifiers& m = ((ModifiersDeclaration&) e).fModifiers;
                if (m.fFlags & Modifiers::kIn_Flag) {
                    switch (m.fLayout.fPrimitive) {
                        case Layout::kPoints_Primitive: // break
                        case Layout::kLines_Primitive:
                            skInSize = 1;
                            break;
                        case Layout::kLinesAdjacency_Primitive: // break
                            skInSize = 2;
                            break;
                        case Layout::kTriangles_Primitive: // break
                        case Layout::kTrianglesAdjacency_Primitive:
                            skInSize = 3;
                            break;
                        default:
                            break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    for (const auto& e : program) {
        if (e.fKind == ProgramElement::kInterfaceBlock_Kind) {
            InterfaceBlock& intf = (InterfaceBlock&) e;
            if (SK_IN_BUILTIN == intf.fVariable.fModifiers.fLayout.fBuiltin) {
                SkASSERT(skInSize != -1);
                intf.fSizes.emplace_back(new IntLiteral(fContext, -1, skInSize));
            }
            SpvId id = this->writeInterfaceBlock(intf);
            if (((intf.fVariable.fModifiers.fFlags & Modifiers::kIn_Flag) ||
                (intf.fVariable.fModifiers.fFlags & Modifiers::kOut_Flag)) &&
                intf.fVariable.fModifiers.fLayout.fBuiltin == -1) {
                interfaceVars.insert(id);
            }
        }
    }
    for (const auto& e : program) {
        if (e.fKind == ProgramElement::kVar_Kind) {
            this->writeGlobalVars(program.fKind, ((VarDeclarations&) e), body);
        }
    }
    for (const auto& e : program) {
        if (e.fKind == ProgramElement::kFunction_Kind) {
            this->writeFunction(((FunctionDefinition&) e), body);
        }
    }
    const FunctionDeclaration* main = nullptr;
    for (auto entry : fFunctionMap) {
        if (entry.first->fName == "main") {
            main = entry.first;
        }
    }
    if (!main) {
        fErrors.error(0, "program does not contain a main() function");
        return;
    }
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
    this->writeOpCode(SpvOpEntryPoint, (SpvId) (3 + (main->fName.fLength + 4) / 4) +
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
    this->writeString(main->fName.fChars, main->fName.fLength, out);
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
    for (const auto& e : program) {
        if (e.fKind == ProgramElement::kExtension_Kind) {
            this->writeInstruction(SpvOpSourceExtension, ((Extension&) e).fName.c_str(), out);
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

}
