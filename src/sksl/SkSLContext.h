/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include "ir/SkSLType.h"
#include "ir/SkSLExpression.h"

namespace SkSL {

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context()
    : fInvalid_Type(new Type(String("<INVALID>")))
    , fVoid_Type(new Type(String("void")))
    , fDouble_Type(new Type(String("double"), true))
    , fDVec2_Type(new Type(String("dvec2"), *fDouble_Type, 2))
    , fDVec3_Type(new Type(String("dvec3"), *fDouble_Type, 3))
    , fDVec4_Type(new Type(String("dvec4"), *fDouble_Type, 4))
    , fFloat_Type(new Type(String("float"), true, { fDouble_Type.get() }))
    , fVec2_Type(new Type(String("vec2"), *fFloat_Type, 2))
    , fVec3_Type(new Type(String("vec3"), *fFloat_Type, 3))
    , fVec4_Type(new Type(String("vec4"), *fFloat_Type, 4))
    , fUInt_Type(new Type(String("uint"), true, { fFloat_Type.get(), fDouble_Type.get() }))
    , fUVec2_Type(new Type(String("uvec2"), *fUInt_Type, 2))
    , fUVec3_Type(new Type(String("uvec3"), *fUInt_Type, 3))
    , fUVec4_Type(new Type(String("uvec4"), *fUInt_Type, 4))
    , fInt_Type(new Type(String("int"), true, { fUInt_Type.get(), fFloat_Type.get(),
                                                  fDouble_Type.get() }))
    , fIVec2_Type(new Type(String("ivec2"), *fInt_Type, 2))
    , fIVec3_Type(new Type(String("ivec3"), *fInt_Type, 3))
    , fIVec4_Type(new Type(String("ivec4"), *fInt_Type, 4))
    , fBool_Type(new Type(String("bool"), false))
    , fBVec2_Type(new Type(String("bvec2"), *fBool_Type, 2))
    , fBVec3_Type(new Type(String("bvec3"), *fBool_Type, 3))
    , fBVec4_Type(new Type(String("bvec4"), *fBool_Type, 4))
    , fMat2x2_Type(new Type(String("mat2"),   *fFloat_Type, 2, 2))
    , fMat2x3_Type(new Type(String("mat2x3"), *fFloat_Type, 2, 3))
    , fMat2x4_Type(new Type(String("mat2x4"), *fFloat_Type, 2, 4))
    , fMat3x2_Type(new Type(String("mat3x2"), *fFloat_Type, 3, 2))
    , fMat3x3_Type(new Type(String("mat3"),   *fFloat_Type, 3, 3))
    , fMat3x4_Type(new Type(String("mat3x4"), *fFloat_Type, 3, 4))
    , fMat4x2_Type(new Type(String("mat4x2"), *fFloat_Type, 4, 2))
    , fMat4x3_Type(new Type(String("mat4x3"), *fFloat_Type, 4, 3))
    , fMat4x4_Type(new Type(String("mat4"),   *fFloat_Type, 4, 4))
    , fDMat2x2_Type(new Type(String("dmat2"),   *fFloat_Type, 2, 2))
    , fDMat2x3_Type(new Type(String("dmat2x3"), *fFloat_Type, 2, 3))
    , fDMat2x4_Type(new Type(String("dmat2x4"), *fFloat_Type, 2, 4))
    , fDMat3x2_Type(new Type(String("dmat3x2"), *fFloat_Type, 3, 2))
    , fDMat3x3_Type(new Type(String("dmat3"),   *fFloat_Type, 3, 3))
    , fDMat3x4_Type(new Type(String("dmat3x4"), *fFloat_Type, 3, 4))
    , fDMat4x2_Type(new Type(String("dmat4x2"), *fFloat_Type, 4, 2))
    , fDMat4x3_Type(new Type(String("dmat4x3"), *fFloat_Type, 4, 3))
    , fDMat4x4_Type(new Type(String("dmat4"),   *fFloat_Type, 4, 4))
    , fSampler1D_Type(new Type(String("sampler1D"), SpvDim1D, false, false, false, true))
    , fSampler2D_Type(new Type(String("sampler2D"), SpvDim2D, false, false, false, true))
    , fSampler3D_Type(new Type(String("sampler3D"), SpvDim3D, false, false, false, true))
    , fSamplerExternalOES_Type(new Type(String("samplerExternalOES"), SpvDim2D, false, false,
                                        false, true))
    , fSamplerCube_Type(new Type(String("samplerCube"), SpvDimCube, false, false, false, true))
    , fSampler2DRect_Type(new Type(String("sampler2DRect"), SpvDimRect, false, false, false,
                                   true))
    , fSampler1DArray_Type(new Type(String("sampler1DArray")))
    , fSampler2DArray_Type(new Type(String("sampler2DArray")))
    , fSamplerCubeArray_Type(new Type(String("samplerCubeArray")))
    , fSamplerBuffer_Type(new Type(String("samplerBuffer")))
    , fSampler2DMS_Type(new Type(String("sampler2DMS")))
    , fSampler2DMSArray_Type(new Type(String("sampler2DMSArray")))
    , fSampler1DShadow_Type(new Type(String("sampler1DShadow")))
    , fSampler2DShadow_Type(new Type(String("sampler2DShadow")))
    , fSamplerCubeShadow_Type(new Type(String("samplerCubeShadow")))
    , fSampler2DRectShadow_Type(new Type(String("sampler2DRectShadow")))
    , fSampler1DArrayShadow_Type(new Type(String("sampler1DArrayShadow")))
    , fSampler2DArrayShadow_Type(new Type(String("sampler2DArrayShadow")))
    , fSamplerCubeArrayShadow_Type(new Type(String("samplerCubeArrayShadow")))

    // Related to below FIXME, gsampler*s don't currently expand to cover integer case.
    , fISampler2D_Type(new Type(String("isampler2D"), SpvDim2D, false, false, false, true))

    // FIXME express these as "gimage2D" that expand to image2D, iimage2D, and uimage2D.
    , fImage2D_Type(new Type(String("image2D"), SpvDim2D, false, false, false, true))
    , fIImage2D_Type(new Type(String("iimage2D"), SpvDim2D, false, false, false, true))

    // FIXME express these as "gsubpassInput" that expand to subpassInput, isubpassInput,
    // and usubpassInput.
    , fSubpassInput_Type(new Type(String("subpassInput"), SpvDimSubpassData, false, false,
                                           false, false))
    , fSubpassInputMS_Type(new Type(String("subpassInputMS"), SpvDimSubpassData, false, false,
                                             true, false))

    // FIXME figure out what we're supposed to do with the gsampler et al. types)
    , fGSampler1D_Type(new Type(String("$gsampler1D"), static_type(*fSampler1D_Type)))
    , fGSampler2D_Type(new Type(String("$gsampler2D"), static_type(*fSampler2D_Type)))
    , fGSampler3D_Type(new Type(String("$gsampler3D"), static_type(*fSampler3D_Type)))
    , fGSamplerCube_Type(new Type(String("$gsamplerCube"), static_type(*fSamplerCube_Type)))
    , fGSampler2DRect_Type(new Type(String("$gsampler2DRect"), static_type(*fSampler2DRect_Type)))
    , fGSampler1DArray_Type(new Type(String("$gsampler1DArray"),
                                     static_type(*fSampler1DArray_Type)))
    , fGSampler2DArray_Type(new Type(String("$gsampler2DArray"),
                                     static_type(*fSampler2DArray_Type)))
    , fGSamplerCubeArray_Type(new Type(String("$gsamplerCubeArray"),
                                       static_type(*fSamplerCubeArray_Type)))
    , fGSamplerBuffer_Type(new Type(String("$gsamplerBuffer"), static_type(*fSamplerBuffer_Type)))
    , fGSampler2DMS_Type(new Type(String("$gsampler2DMS"), static_type(*fSampler2DMS_Type)))
    , fGSampler2DMSArray_Type(new Type(String("$gsampler2DMSArray"),
                                       static_type(*fSampler2DMSArray_Type)))
    , fGSampler2DArrayShadow_Type(new Type(String("$gsampler2DArrayShadow"),
                                           static_type(*fSampler2DArrayShadow_Type)))
    , fGSamplerCubeArrayShadow_Type(new Type(String("$gsamplerCubeArrayShadow"),
                                             static_type(*fSamplerCubeArrayShadow_Type)))
    , fGenType_Type(new Type(String("$genType"), { fFloat_Type.get(), fVec2_Type.get(),
                                                     fVec3_Type.get(), fVec4_Type.get() }))
    , fGenDType_Type(new Type(String("$genDType"), { fDouble_Type.get(), fDVec2_Type.get(),
                                                       fDVec3_Type.get(), fDVec4_Type.get() }))
    , fGenIType_Type(new Type(String("$genIType"), { fInt_Type.get(), fIVec2_Type.get(),
                                                       fIVec3_Type.get(), fIVec4_Type.get() }))
    , fGenUType_Type(new Type(String("$genUType"), { fUInt_Type.get(), fUVec2_Type.get(),
                                                       fUVec3_Type.get(), fUVec4_Type.get() }))
    , fGenBType_Type(new Type(String("$genBType"), { fBool_Type.get(), fBVec2_Type.get(),
                                                       fBVec3_Type.get(), fBVec4_Type.get() }))
    , fMat_Type(new Type(String("$mat"), { fMat2x2_Type.get(), fMat2x3_Type.get(),
                                             fMat2x4_Type.get(), fMat3x2_Type.get(),
                                             fMat3x3_Type.get(), fMat3x4_Type.get(),
                                             fMat4x2_Type.get(), fMat4x3_Type.get(),
                                             fMat4x4_Type.get(), fDMat2x2_Type.get(),
                                             fDMat2x3_Type.get(), fDMat2x4_Type.get(),
                                             fDMat3x2_Type.get(), fDMat3x3_Type.get(),
                                             fDMat3x4_Type.get(), fDMat4x2_Type.get(),
                                             fDMat4x3_Type.get(), fDMat4x4_Type.get() }))
    , fVec_Type(new Type(String("$vec"), { fInvalid_Type.get(), fVec2_Type.get(),
                                             fVec3_Type.get(), fVec4_Type.get() }))
    , fGVec_Type(new Type(String("$gvec")))
    , fGVec2_Type(new Type(String("$gvec2")))
    , fGVec3_Type(new Type(String("$gvec3")))
    , fGVec4_Type(new Type(String("$gvec4"), static_type(*fVec4_Type)))
    , fDVec_Type(new Type(String("$dvec"), { fInvalid_Type.get(), fDVec2_Type.get(),
                                              fDVec3_Type.get(), fDVec4_Type.get() }))
    , fIVec_Type(new Type(String("$ivec"), { fInvalid_Type.get(), fIVec2_Type.get(),
                                               fIVec3_Type.get(), fIVec4_Type.get() }))
    , fUVec_Type(new Type(String("$uvec"), { fInvalid_Type.get(), fUVec2_Type.get(),
                                               fUVec3_Type.get(), fUVec4_Type.get() }))
    , fBVec_Type(new Type(String("$bvec"), { fInvalid_Type.get(), fBVec2_Type.get(),
                                               fBVec3_Type.get(), fBVec4_Type.get() }))
    , fSkCaps_Type(new Type(String("$sk_Caps")))
    , fDefined_Expression(new Defined(*fInvalid_Type)) {}

    static std::vector<const Type*> static_type(const Type& t) {
        return { &t, &t, &t, &t };
    }

    const std::unique_ptr<Type> fInvalid_Type;
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
    const std::unique_ptr<Type> fSamplerExternalOES_Type;
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


    const std::unique_ptr<Type> fISampler2D_Type;

    const std::unique_ptr<Type> fImage2D_Type;
    const std::unique_ptr<Type> fIImage2D_Type;

    const std::unique_ptr<Type> fSubpassInput_Type;
    const std::unique_ptr<Type> fSubpassInputMS_Type;

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

    const std::unique_ptr<Type> fSkCaps_Type;

    // dummy expression used to mark that a variable has a value during dataflow analysis (when it
    // could have several different values, or the analyzer is otherwise unable to assign it a
    // specific expression)
    const std::unique_ptr<Expression> fDefined_Expression;

private:
    class Defined : public Expression {
    public:
        Defined(const Type& type)
        : INHERITED(Position(), kDefined_Kind, type) {}

        virtual String description() const override {
            return String("<defined>");
        }

        typedef Expression INHERITED;
    };
};

} // namespace

#endif
