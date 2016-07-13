/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#include "SkSLType.h"

namespace SkSL {

bool Type::determineCoercionCost(std::shared_ptr<Type> other, int* outCost) const {
    if (this == other.get()) {
        *outCost = 0;
        return true;
    }
    if (this->kind() == kVector_Kind && other->kind() == kVector_Kind) {
        if (this->columns() == other->columns()) {
            return this->componentType()->determineCoercionCost(other->componentType(), outCost);
        }
        return false;
    }
    if (this->kind() == kMatrix_Kind) {
        if (this->columns() == other->columns() && 
            this->rows() == other->rows()) {
            return this->componentType()->determineCoercionCost(other->componentType(), outCost);
        }
        return false;
    }
    for (size_t i = 0; i < fCoercibleTypes.size(); i++) {
        if (fCoercibleTypes[i] == other) {
            *outCost = (int) i + 1;
            return true;
        }
    }
    return false;
}

std::shared_ptr<Type> Type::toCompound(int columns, int rows) {
    ASSERT(this->kind() == Type::kScalar_Kind);
    if (columns == 1 && rows == 1) {
        return std::shared_ptr<Type>(this);
    }
    if (*this == *kFloat_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return kVec2_Type;
                    case 3: return kVec3_Type;
                    case 4: return kVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return kMat2x2_Type;
                    case 3: return kMat3x2_Type;
                    case 4: return kMat4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return kMat2x3_Type;
                    case 3: return kMat3x3_Type;
                    case 4: return kMat4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return kMat2x4_Type;
                    case 3: return kMat3x4_Type;
                    case 4: return kMat4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *kDouble_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return kDVec2_Type;
                    case 3: return kDVec3_Type;
                    case 4: return kDVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            case 2:
                switch (columns) {
                    case 2: return kDMat2x2_Type;
                    case 3: return kDMat3x2_Type;
                    case 4: return kDMat4x2_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 3:
                switch (columns) {
                    case 2: return kDMat2x3_Type;
                    case 3: return kDMat3x3_Type;
                    case 4: return kDMat4x3_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            case 4:
                switch (columns) {
                    case 2: return kDMat2x4_Type;
                    case 3: return kDMat3x4_Type;
                    case 4: return kDMat4x4_Type;
                    default: ABORT("unsupported matrix column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *kInt_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return kIVec2_Type;
                    case 3: return kIVec3_Type;
                    case 4: return kIVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    } else if (*this == *kUInt_Type) {
        switch (rows) {
            case 1:
                switch (columns) {
                    case 2: return kUVec2_Type;
                    case 3: return kUVec3_Type;
                    case 4: return kUVec4_Type;
                    default: ABORT("unsupported vector column count (%d)", columns);
                }
            default: ABORT("unsupported row count (%d)", rows);
        }
    }
    ABORT("unsupported scalar_to_compound type %s", this->description().c_str());
}

const std::shared_ptr<Type> kVoid_Type(new Type("void"));

const std::shared_ptr<Type> kDouble_Type(new Type("double", true));
const std::shared_ptr<Type> kDVec2_Type(new Type("dvec2", kDouble_Type, 2));
const std::shared_ptr<Type> kDVec3_Type(new Type("dvec3", kDouble_Type, 3));
const std::shared_ptr<Type> kDVec4_Type(new Type("dvec4", kDouble_Type, 4));

const std::shared_ptr<Type> kFloat_Type(new Type("float", true, { kDouble_Type }));
const std::shared_ptr<Type> kVec2_Type(new Type("vec2", kFloat_Type, 2));
const std::shared_ptr<Type> kVec3_Type(new Type("vec3", kFloat_Type, 3));
const std::shared_ptr<Type> kVec4_Type(new Type("vec4", kFloat_Type, 4));

const std::shared_ptr<Type> kUInt_Type(new Type("uint", true, { kFloat_Type, kDouble_Type }));
const std::shared_ptr<Type> kUVec2_Type(new Type("uvec2", kUInt_Type, 2));
const std::shared_ptr<Type> kUVec3_Type(new Type("uvec3", kUInt_Type, 3));
const std::shared_ptr<Type> kUVec4_Type(new Type("uvec4", kUInt_Type, 4));

const std::shared_ptr<Type> kInt_Type(new Type("int", true, { kUInt_Type, kFloat_Type, 
                                                              kDouble_Type }));
const std::shared_ptr<Type> kIVec2_Type(new Type("ivec2", kInt_Type, 2));
const std::shared_ptr<Type> kIVec3_Type(new Type("ivec3", kInt_Type, 3));
const std::shared_ptr<Type> kIVec4_Type(new Type("ivec4", kInt_Type, 4));

const std::shared_ptr<Type> kBool_Type(new Type("bool", false));
const std::shared_ptr<Type> kBVec2_Type(new Type("bvec2", kBool_Type, 2));
const std::shared_ptr<Type> kBVec3_Type(new Type("bvec3", kBool_Type, 3));
const std::shared_ptr<Type> kBVec4_Type(new Type("bvec4", kBool_Type, 4));

const std::shared_ptr<Type> kMat2x2_Type(new Type("mat2",   kFloat_Type, 2, 2));
const std::shared_ptr<Type> kMat2x3_Type(new Type("mat2x3", kFloat_Type, 2, 3));
const std::shared_ptr<Type> kMat2x4_Type(new Type("mat2x4", kFloat_Type, 2, 4));
const std::shared_ptr<Type> kMat3x2_Type(new Type("mat3x2", kFloat_Type, 3, 2));
const std::shared_ptr<Type> kMat3x3_Type(new Type("mat3",   kFloat_Type, 3, 3));
const std::shared_ptr<Type> kMat3x4_Type(new Type("mat3x4", kFloat_Type, 3, 4));
const std::shared_ptr<Type> kMat4x2_Type(new Type("mat4x2", kFloat_Type, 4, 2));
const std::shared_ptr<Type> kMat4x3_Type(new Type("mat4x3", kFloat_Type, 4, 3));
const std::shared_ptr<Type> kMat4x4_Type(new Type("mat4",   kFloat_Type, 4, 4));

const std::shared_ptr<Type> kDMat2x2_Type(new Type("dmat2",   kFloat_Type, 2, 2));
const std::shared_ptr<Type> kDMat2x3_Type(new Type("dmat2x3", kFloat_Type, 2, 3));
const std::shared_ptr<Type> kDMat2x4_Type(new Type("dmat2x4", kFloat_Type, 2, 4));
const std::shared_ptr<Type> kDMat3x2_Type(new Type("dmat3x2", kFloat_Type, 3, 2));
const std::shared_ptr<Type> kDMat3x3_Type(new Type("dmat3",   kFloat_Type, 3, 3));
const std::shared_ptr<Type> kDMat3x4_Type(new Type("dmat3x4", kFloat_Type, 3, 4));
const std::shared_ptr<Type> kDMat4x2_Type(new Type("dmat4x2", kFloat_Type, 4, 2));
const std::shared_ptr<Type> kDMat4x3_Type(new Type("dmat4x3", kFloat_Type, 4, 3));
const std::shared_ptr<Type> kDMat4x4_Type(new Type("dmat4",   kFloat_Type, 4, 4));

const std::shared_ptr<Type> kSampler1D_Type(new Type("sampler1D", SpvDim1D, false, false, false, true));
const std::shared_ptr<Type> kSampler2D_Type(new Type("sampler2D", SpvDim2D, false, false, false, true));
const std::shared_ptr<Type> kSampler3D_Type(new Type("sampler3D", SpvDim3D, false, false, false, true));
const std::shared_ptr<Type> kSamplerCube_Type(new Type("samplerCube"));
const std::shared_ptr<Type> kSampler2DRect_Type(new Type("sampler2DRect"));
const std::shared_ptr<Type> kSampler1DArray_Type(new Type("sampler1DArray"));
const std::shared_ptr<Type> kSampler2DArray_Type(new Type("sampler2DArray"));
const std::shared_ptr<Type> kSamplerCubeArray_Type(new Type("samplerCubeArray"));
const std::shared_ptr<Type> kSamplerBuffer_Type(new Type("samplerBuffer"));
const std::shared_ptr<Type> kSampler2DMS_Type(new Type("sampler2DMS"));
const std::shared_ptr<Type> kSampler2DMSArray_Type(new Type("sampler2DMSArray"));
const std::shared_ptr<Type> kSampler1DShadow_Type(new Type("sampler1DShadow"));
const std::shared_ptr<Type> kSampler2DShadow_Type(new Type("sampler2DShadow"));
const std::shared_ptr<Type> kSamplerCubeShadow_Type(new Type("samplerCubeShadow"));
const std::shared_ptr<Type> kSampler2DRectShadow_Type(new Type("sampler2DRectShadow"));
const std::shared_ptr<Type> kSampler1DArrayShadow_Type(new Type("sampler1DArrayShadow"));
const std::shared_ptr<Type> kSampler2DArrayShadow_Type(new Type("sampler2DArrayShadow"));
const std::shared_ptr<Type> kSamplerCubeArrayShadow_Type(new Type("samplerCubeArrayShadow"));

static std::vector<std::shared_ptr<Type>> type(std::shared_ptr<Type> t) {
    return { t, t, t, t };   
}

// FIXME figure out what we're supposed to do with the gsampler et al. types
const std::shared_ptr<Type> kGSampler1D_Type(new Type("$gsampler1D", type(kSampler1D_Type)));
const std::shared_ptr<Type> kGSampler2D_Type(new Type("$gsampler2D", type(kSampler2D_Type)));
const std::shared_ptr<Type> kGSampler3D_Type(new Type("$gsampler3D", type(kSampler3D_Type)));
const std::shared_ptr<Type> kGSamplerCube_Type(new Type("$gsamplerCube", type(kSamplerCube_Type)));
const std::shared_ptr<Type> kGSampler2DRect_Type(new Type("$gsampler2DRect", 
                                                 type(kSampler2DRect_Type)));
const std::shared_ptr<Type> kGSampler1DArray_Type(new Type("$gsampler1DArray", 
                                                  type(kSampler1DArray_Type)));
const std::shared_ptr<Type> kGSampler2DArray_Type(new Type("$gsampler2DArray", 
                                                  type(kSampler2DArray_Type)));
const std::shared_ptr<Type> kGSamplerCubeArray_Type(new Type("$gsamplerCubeArray", 
                                                    type(kSamplerCubeArray_Type)));
const std::shared_ptr<Type> kGSamplerBuffer_Type(new Type("$gsamplerBuffer", 
                                                 type(kSamplerBuffer_Type)));
const std::shared_ptr<Type> kGSampler2DMS_Type(new Type("$gsampler2DMS", 
                                               type(kSampler2DMS_Type)));
const std::shared_ptr<Type> kGSampler2DMSArray_Type(new Type("$gsampler2DMSArray", 
                                                    type(kSampler2DMSArray_Type)));
const std::shared_ptr<Type> kGSampler2DArrayShadow_Type(new Type("$gsampler2DArrayShadow", 
                                                        type(kSampler2DArrayShadow_Type)));
const std::shared_ptr<Type> kGSamplerCubeArrayShadow_Type(new Type("$gsamplerCubeArrayShadow", 
                                                          type(kSamplerCubeArrayShadow_Type)));

const std::shared_ptr<Type> kGenType_Type(new Type("$genType", { kFloat_Type, kVec2_Type,
                                                                 kVec3_Type, kVec4_Type }));
const std::shared_ptr<Type> kGenDType_Type(new Type("$genDType", { kDouble_Type, kDVec2_Type,
                                                                   kDVec3_Type, kDVec4_Type }));
const std::shared_ptr<Type> kGenIType_Type(new Type("$genIType", { kInt_Type, kIVec2_Type,
                                                                   kIVec3_Type, kIVec4_Type }));
const std::shared_ptr<Type> kGenUType_Type(new Type("$genUType", { kUInt_Type, kUVec2_Type,
                                                                   kUVec3_Type, kUVec4_Type }));
const std::shared_ptr<Type> kGenBType_Type(new Type("$genBType", { kBool_Type, kBVec2_Type,
                                                                   kBVec3_Type, kBVec4_Type }));

const std::shared_ptr<Type> kMat_Type(new Type("$mat"));

const std::shared_ptr<Type> kVec_Type(new Type("$vec", { kVec2_Type, kVec2_Type, kVec3_Type, 
                                                         kVec4_Type }));

const std::shared_ptr<Type> kGVec_Type(new Type("$gvec"));
const std::shared_ptr<Type> kGVec2_Type(new Type("$gvec2"));
const std::shared_ptr<Type> kGVec3_Type(new Type("$gvec3"));
const std::shared_ptr<Type> kGVec4_Type(new Type("$gvec4", type(kVec4_Type)));
const std::shared_ptr<Type> kDVec_Type(new Type("$dvec"));
const std::shared_ptr<Type> kIVec_Type(new Type("$ivec"));
const std::shared_ptr<Type> kUVec_Type(new Type("$uvec"));

const std::shared_ptr<Type> kBVec_Type(new Type("$bvec", { kBVec2_Type, kBVec2_Type,
                                                           kBVec3_Type, kBVec4_Type }));

const std::shared_ptr<Type> kInvalid_Type(new Type("<INVALID>"));

} // namespace
