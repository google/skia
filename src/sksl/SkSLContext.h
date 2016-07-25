/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include "ir/SkSLType.h"

namespace SkSL {

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context()
    : fVoid_Type(new Type("void"))
    , fDouble_Type(new Type("double", true))
    , fDVec2_Type(new Type("dvec2", *fDouble_Type, 2))
    , fDVec3_Type(new Type("dvec3", *fDouble_Type, 3))
    , fDVec4_Type(new Type("dvec4", *fDouble_Type, 4))
    , fFloat_Type(new Type("float", true, { fDouble_Type.get() }))
    , fVec2_Type(new Type("vec2", *fFloat_Type, 2))
    , fVec3_Type(new Type("vec3", *fFloat_Type, 3))
    , fVec4_Type(new Type("vec4", *fFloat_Type, 4))
    , fUInt_Type(new Type("uint", true, { fFloat_Type.get(), fDouble_Type.get() }))
    , fUVec2_Type(new Type("uvec2", *fUInt_Type, 2))
    , fUVec3_Type(new Type("uvec3", *fUInt_Type, 3))
    , fUVec4_Type(new Type("uvec4", *fUInt_Type, 4))
    , fInt_Type(new Type("int", true, { fUInt_Type.get(), fFloat_Type.get(), fDouble_Type.get() }))
    , fIVec2_Type(new Type("ivec2", *fInt_Type, 2))
    , fIVec3_Type(new Type("ivec3", *fInt_Type, 3))
    , fIVec4_Type(new Type("ivec4", *fInt_Type, 4))
    , fBool_Type(new Type("bool", false))
    , fBVec2_Type(new Type("bvec2", *fBool_Type, 2))
    , fBVec3_Type(new Type("bvec3", *fBool_Type, 3))
    , fBVec4_Type(new Type("bvec4", *fBool_Type, 4))
    , fMat2x2_Type(new Type("mat2",   *fFloat_Type, 2, 2))
    , fMat2x3_Type(new Type("mat2x3", *fFloat_Type, 2, 3))
    , fMat2x4_Type(new Type("mat2x4", *fFloat_Type, 2, 4))
    , fMat3x2_Type(new Type("mat3x2", *fFloat_Type, 3, 2))
    , fMat3x3_Type(new Type("mat3",   *fFloat_Type, 3, 3))
    , fMat3x4_Type(new Type("mat3x4", *fFloat_Type, 3, 4))
    , fMat4x2_Type(new Type("mat4x2", *fFloat_Type, 4, 2))
    , fMat4x3_Type(new Type("mat4x3", *fFloat_Type, 4, 3))
    , fMat4x4_Type(new Type("mat4",   *fFloat_Type, 4, 4))
    , fDMat2x2_Type(new Type("dmat2",   *fFloat_Type, 2, 2))
    , fDMat2x3_Type(new Type("dmat2x3", *fFloat_Type, 2, 3))
    , fDMat2x4_Type(new Type("dmat2x4", *fFloat_Type, 2, 4))
    , fDMat3x2_Type(new Type("dmat3x2", *fFloat_Type, 3, 2))
    , fDMat3x3_Type(new Type("dmat3",   *fFloat_Type, 3, 3))
    , fDMat3x4_Type(new Type("dmat3x4", *fFloat_Type, 3, 4))
    , fDMat4x2_Type(new Type("dmat4x2", *fFloat_Type, 4, 2))
    , fDMat4x3_Type(new Type("dmat4x3", *fFloat_Type, 4, 3))
    , fDMat4x4_Type(new Type("dmat4",   *fFloat_Type, 4, 4))
    , fSampler1D_Type(new Type("sampler1D", SpvDim1D, false, false, false, true))
    , fSampler2D_Type(new Type("sampler2D", SpvDim2D, false, false, false, true))
    , fSampler3D_Type(new Type("sampler3D", SpvDim3D, false, false, false, true))
    , fSamplerCube_Type(new Type("samplerCube"))
    , fSampler2DRect_Type(new Type("sampler2DRect"))
    , fSampler1DArray_Type(new Type("sampler1DArray"))
    , fSampler2DArray_Type(new Type("sampler2DArray"))
    , fSamplerCubeArray_Type(new Type("samplerCubeArray"))
    , fSamplerBuffer_Type(new Type("samplerBuffer"))
    , fSampler2DMS_Type(new Type("sampler2DMS"))
    , fSampler2DMSArray_Type(new Type("sampler2DMSArray"))
    , fSampler1DShadow_Type(new Type("sampler1DShadow"))
    , fSampler2DShadow_Type(new Type("sampler2DShadow"))
    , fSamplerCubeShadow_Type(new Type("samplerCubeShadow"))
    , fSampler2DRectShadow_Type(new Type("sampler2DRectShadow"))
    , fSampler1DArrayShadow_Type(new Type("sampler1DArrayShadow"))
    , fSampler2DArrayShadow_Type(new Type("sampler2DArrayShadow"))
    , fSamplerCubeArrayShadow_Type(new Type("samplerCubeArrayShadow"))
    // FIXME figure out what we're supposed to do with the gsampler et al. types)
    , fGSampler1D_Type(new Type("$gsampler1D", static_type(*fSampler1D_Type)))
    , fGSampler2D_Type(new Type("$gsampler2D", static_type(*fSampler2D_Type)))
    , fGSampler3D_Type(new Type("$gsampler3D", static_type(*fSampler3D_Type)))
    , fGSamplerCube_Type(new Type("$gsamplerCube", static_type(*fSamplerCube_Type)))
    , fGSampler2DRect_Type(new Type("$gsampler2DRect", static_type(*fSampler2DRect_Type)))
    , fGSampler1DArray_Type(new Type("$gsampler1DArray", static_type(*fSampler1DArray_Type)))
    , fGSampler2DArray_Type(new Type("$gsampler2DArray", static_type(*fSampler2DArray_Type)))
    , fGSamplerCubeArray_Type(new Type("$gsamplerCubeArray", static_type(*fSamplerCubeArray_Type)))
    , fGSamplerBuffer_Type(new Type("$gsamplerBuffer", static_type(*fSamplerBuffer_Type)))
    , fGSampler2DMS_Type(new Type("$gsampler2DMS", static_type(*fSampler2DMS_Type)))
    , fGSampler2DMSArray_Type(new Type("$gsampler2DMSArray", static_type(*fSampler2DMSArray_Type)))
    , fGSampler2DArrayShadow_Type(new Type("$gsampler2DArrayShadow", 
                                           static_type(*fSampler2DArrayShadow_Type)))
    , fGSamplerCubeArrayShadow_Type(new Type("$gsamplerCubeArrayShadow",
                                             static_type(*fSamplerCubeArrayShadow_Type)))
    , fGenType_Type(new Type("$genType", { fFloat_Type.get(), fVec2_Type.get(), fVec3_Type.get(), 
                                           fVec4_Type.get() }))
    , fGenDType_Type(new Type("$genDType", { fDouble_Type.get(), fDVec2_Type.get(), 
                                             fDVec3_Type.get(), fDVec4_Type.get() }))
    , fGenIType_Type(new Type("$genIType", { fInt_Type.get(), fIVec2_Type.get(), fIVec3_Type.get(), 
                                             fIVec4_Type.get() }))
    , fGenUType_Type(new Type("$genUType", { fUInt_Type.get(), fUVec2_Type.get(), fUVec3_Type.get(), 
                                             fUVec4_Type.get() }))
    , fGenBType_Type(new Type("$genBType", { fBool_Type.get(), fBVec2_Type.get(), fBVec3_Type.get(), 
                                             fBVec4_Type.get() }))
    , fMat_Type(new Type("$mat"))
    , fVec_Type(new Type("$vec", { fVec2_Type.get(), fVec2_Type.get(), fVec3_Type.get(),
                                   fVec4_Type.get() }))
    , fGVec_Type(new Type("$gvec"))
    , fGVec2_Type(new Type("$gvec2"))
    , fGVec3_Type(new Type("$gvec3"))
    , fGVec4_Type(new Type("$gvec4", static_type(*fVec4_Type)))
    , fDVec_Type(new Type("$dvec"))
    , fIVec_Type(new Type("$ivec"))
    , fUVec_Type(new Type("$uvec"))
    , fBVec_Type(new Type("$bvec", { fBVec2_Type.get(), fBVec2_Type.get(), fBVec3_Type.get(), 
                                     fBVec4_Type.get() }))
    , fInvalid_Type(new Type("<INVALID>")) {}

    static std::vector<const Type*> static_type(const Type& t) {
        return { &t, &t, &t, &t };   
    }

    const std::unique_ptr<Type> fVoid_Type;

    const std::unique_ptr<Type> fDouble_Type;
    const std::unique_ptr<Type> fDVec2_Type;
    const std::unique_ptr<Type> fDVec3_Type;
    const std::unique_ptr<Type> fDVec4_Type;

    const std::unique_ptr<Type> fFloat_Type;
    const std::unique_ptr<Type> fVec2_Type;
    const std::unique_ptr<Type> fVec3_Type;
    const std::unique_ptr<Type> fVec4_Type;

    const std::unique_ptr<Type> fUInt_Type;
    const std::unique_ptr<Type> fUVec2_Type;
    const std::unique_ptr<Type> fUVec3_Type;
    const std::unique_ptr<Type> fUVec4_Type;

    const std::unique_ptr<Type> fInt_Type;
    const std::unique_ptr<Type> fIVec2_Type;
    const std::unique_ptr<Type> fIVec3_Type;
    const std::unique_ptr<Type> fIVec4_Type;

    const std::unique_ptr<Type> fBool_Type;
    const std::unique_ptr<Type> fBVec2_Type;
    const std::unique_ptr<Type> fBVec3_Type;
    const std::unique_ptr<Type> fBVec4_Type;

    const std::unique_ptr<Type> fMat2x2_Type;
    const std::unique_ptr<Type> fMat2x3_Type;
    const std::unique_ptr<Type> fMat2x4_Type;
    const std::unique_ptr<Type> fMat3x2_Type;
    const std::unique_ptr<Type> fMat3x3_Type;
    const std::unique_ptr<Type> fMat3x4_Type;
    const std::unique_ptr<Type> fMat4x2_Type;
    const std::unique_ptr<Type> fMat4x3_Type;
    const std::unique_ptr<Type> fMat4x4_Type;

    const std::unique_ptr<Type> fDMat2x2_Type;
    const std::unique_ptr<Type> fDMat2x3_Type;
    const std::unique_ptr<Type> fDMat2x4_Type;
    const std::unique_ptr<Type> fDMat3x2_Type;
    const std::unique_ptr<Type> fDMat3x3_Type;
    const std::unique_ptr<Type> fDMat3x4_Type;
    const std::unique_ptr<Type> fDMat4x2_Type;
    const std::unique_ptr<Type> fDMat4x3_Type;
    const std::unique_ptr<Type> fDMat4x4_Type;

    const std::unique_ptr<Type> fSampler1D_Type;
    const std::unique_ptr<Type> fSampler2D_Type;
    const std::unique_ptr<Type> fSampler3D_Type;
    const std::unique_ptr<Type> fSamplerCube_Type;
    const std::unique_ptr<Type> fSampler2DRect_Type;
    const std::unique_ptr<Type> fSampler1DArray_Type;
    const std::unique_ptr<Type> fSampler2DArray_Type;
    const std::unique_ptr<Type> fSamplerCubeArray_Type;
    const std::unique_ptr<Type> fSamplerBuffer_Type;
    const std::unique_ptr<Type> fSampler2DMS_Type;
    const std::unique_ptr<Type> fSampler2DMSArray_Type;
    const std::unique_ptr<Type> fSampler1DShadow_Type;
    const std::unique_ptr<Type> fSampler2DShadow_Type;
    const std::unique_ptr<Type> fSamplerCubeShadow_Type;
    const std::unique_ptr<Type> fSampler2DRectShadow_Type;
    const std::unique_ptr<Type> fSampler1DArrayShadow_Type;
    const std::unique_ptr<Type> fSampler2DArrayShadow_Type;
    const std::unique_ptr<Type> fSamplerCubeArrayShadow_Type;

    const std::unique_ptr<Type> fGSampler1D_Type;
    const std::unique_ptr<Type> fGSampler2D_Type;
    const std::unique_ptr<Type> fGSampler3D_Type;
    const std::unique_ptr<Type> fGSamplerCube_Type;
    const std::unique_ptr<Type> fGSampler2DRect_Type;
    const std::unique_ptr<Type> fGSampler1DArray_Type;
    const std::unique_ptr<Type> fGSampler2DArray_Type;
    const std::unique_ptr<Type> fGSamplerCubeArray_Type;
    const std::unique_ptr<Type> fGSamplerBuffer_Type;
    const std::unique_ptr<Type> fGSampler2DMS_Type;
    const std::unique_ptr<Type> fGSampler2DMSArray_Type;
    const std::unique_ptr<Type> fGSampler2DArrayShadow_Type;
    const std::unique_ptr<Type> fGSamplerCubeArrayShadow_Type;

    const std::unique_ptr<Type> fGenType_Type;
    const std::unique_ptr<Type> fGenDType_Type;
    const std::unique_ptr<Type> fGenIType_Type;
    const std::unique_ptr<Type> fGenUType_Type;
    const std::unique_ptr<Type> fGenBType_Type;

    const std::unique_ptr<Type> fMat_Type;

    const std::unique_ptr<Type> fVec_Type;

    const std::unique_ptr<Type> fGVec_Type;
    const std::unique_ptr<Type> fGVec2_Type;
    const std::unique_ptr<Type> fGVec3_Type;
    const std::unique_ptr<Type> fGVec4_Type;
    const std::unique_ptr<Type> fDVec_Type;
    const std::unique_ptr<Type> fIVec_Type;
    const std::unique_ptr<Type> fUVec_Type;

    const std::unique_ptr<Type> fBVec_Type;

    const std::unique_ptr<Type> fInvalid_Type;
};

} // namespace

#endif
