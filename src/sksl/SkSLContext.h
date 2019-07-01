/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CONTEXT
#define SKSL_CONTEXT

#include "src/sksl/SkSLIRNode.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * Contains compiler-wide objects, which currently means the core types.
 */
class Context {
public:
    Context(IRGenerator* irGenerator);

    IRNode::ID fInvalid_Type;
    IRNode::ID fVoid_Type;
    IRNode::ID fNull_Type;
    IRNode::ID fFloatLiteral_Type;
    IRNode::ID fIntLiteral_Type;

    IRNode::ID fDouble_Type;
    IRNode::ID fDouble2_Type;
    IRNode::ID fDouble3_Type;
    IRNode::ID fDouble4_Type;

    IRNode::ID fFloat_Type;
    IRNode::ID fFloat2_Type;
    IRNode::ID fFloat3_Type;
    IRNode::ID fFloat4_Type;

    IRNode::ID fHalf_Type;
    IRNode::ID fHalf2_Type;
    IRNode::ID fHalf3_Type;
    IRNode::ID fHalf4_Type;

    IRNode::ID fUInt_Type;
    IRNode::ID fUInt2_Type;
    IRNode::ID fUInt3_Type;
    IRNode::ID fUInt4_Type;

    IRNode::ID fInt_Type;
    IRNode::ID fInt2_Type;
    IRNode::ID fInt3_Type;
    IRNode::ID fInt4_Type;

    IRNode::ID fUShort_Type;
    IRNode::ID fUShort2_Type;
    IRNode::ID fUShort3_Type;
    IRNode::ID fUShort4_Type;

    IRNode::ID fShort_Type;
    IRNode::ID fShort2_Type;
    IRNode::ID fShort3_Type;
    IRNode::ID fShort4_Type;

    IRNode::ID fUByte_Type;
    IRNode::ID fUByte2_Type;
    IRNode::ID fUByte3_Type;
    IRNode::ID fUByte4_Type;

    IRNode::ID fByte_Type;
    IRNode::ID fByte2_Type;
    IRNode::ID fByte3_Type;
    IRNode::ID fByte4_Type;

    IRNode::ID fBool_Type;
    IRNode::ID fBool2_Type;
    IRNode::ID fBool3_Type;
    IRNode::ID fBool4_Type;

    IRNode::ID fFloat2x2_Type;
    IRNode::ID fFloat2x3_Type;
    IRNode::ID fFloat2x4_Type;
    IRNode::ID fFloat3x2_Type;
    IRNode::ID fFloat3x3_Type;
    IRNode::ID fFloat3x4_Type;
    IRNode::ID fFloat4x2_Type;
    IRNode::ID fFloat4x3_Type;
    IRNode::ID fFloat4x4_Type;

    IRNode::ID fHalf2x2_Type;
    IRNode::ID fHalf2x3_Type;
    IRNode::ID fHalf2x4_Type;
    IRNode::ID fHalf3x2_Type;
    IRNode::ID fHalf3x3_Type;
    IRNode::ID fHalf3x4_Type;
    IRNode::ID fHalf4x2_Type;
    IRNode::ID fHalf4x3_Type;
    IRNode::ID fHalf4x4_Type;

    IRNode::ID fDouble2x2_Type;
    IRNode::ID fDouble2x3_Type;
    IRNode::ID fDouble2x4_Type;
    IRNode::ID fDouble3x2_Type;
    IRNode::ID fDouble3x3_Type;
    IRNode::ID fDouble3x4_Type;
    IRNode::ID fDouble4x2_Type;
    IRNode::ID fDouble4x3_Type;
    IRNode::ID fDouble4x4_Type;

    IRNode::ID fSampler1D_Type;
    IRNode::ID fSampler2D_Type;
    IRNode::ID fSampler3D_Type;
    IRNode::ID fSamplerExternalOES_Type;
    IRNode::ID fSamplerCube_Type;
    IRNode::ID fSampler2DRect_Type;
    IRNode::ID fSampler1DArray_Type;
    IRNode::ID fSampler2DArray_Type;
    IRNode::ID fSamplerCubeArray_Type;
    IRNode::ID fSamplerBuffer_Type;
    IRNode::ID fSampler2DMS_Type;
    IRNode::ID fSampler2DMSArray_Type;
    IRNode::ID fSampler1DShadow_Type;
    IRNode::ID fSampler2DShadow_Type;
    IRNode::ID fSamplerCubeShadow_Type;
    IRNode::ID fSampler2DRectShadow_Type;
    IRNode::ID fSampler1DArrayShadow_Type;
    IRNode::ID fSampler2DArrayShadow_Type;
    IRNode::ID fSamplerCubeArrayShadow_Type;

    IRNode::ID fISampler2D_Type;

    IRNode::ID fImage2D_Type;
    IRNode::ID fIImage2D_Type;

    IRNode::ID fSubpassInput_Type;
    IRNode::ID fSubpassInputMS_Type;

    IRNode::ID fGSampler1D_Type;
    IRNode::ID fGSampler2D_Type;
    IRNode::ID fGSampler3D_Type;
    IRNode::ID fGSamplerCube_Type;
    IRNode::ID fGSampler2DRect_Type;
    IRNode::ID fGSampler1DArray_Type;
    IRNode::ID fGSampler2DArray_Type;
    IRNode::ID fGSamplerCubeArray_Type;
    IRNode::ID fGSamplerBuffer_Type;
    IRNode::ID fGSampler2DMS_Type;
    IRNode::ID fGSampler2DMSArray_Type;
    IRNode::ID fGSampler2DArrayShadow_Type;
    IRNode::ID fGSamplerCubeArrayShadow_Type;

    IRNode::ID fGenType_Type;
    IRNode::ID fGenHType_Type;
    IRNode::ID fGenDType_Type;
    IRNode::ID fGenIType_Type;
    IRNode::ID fGenUType_Type;
    IRNode::ID fGenBType_Type;

    IRNode::ID fMat_Type;

    IRNode::ID fVec_Type;

    IRNode::ID fGVec_Type;
    IRNode::ID fGVec2_Type;
    IRNode::ID fGVec3_Type;
    IRNode::ID fGVec4_Type;
    IRNode::ID fHVec_Type;
    IRNode::ID fDVec_Type;
    IRNode::ID fIVec_Type;
    IRNode::ID fUVec_Type;
    IRNode::ID fSVec_Type;
    IRNode::ID fUSVec_Type;
    IRNode::ID fByteVec_Type;
    IRNode::ID fUByteVec_Type;

    IRNode::ID fBVec_Type;

    IRNode::ID fSkCaps_Type;
    IRNode::ID fSkArgs_Type;
    IRNode::ID fFragmentProcessor_Type;
    IRNode::ID fSkRasterPipeline_Type;

    // dummy expression used to mark that a variable has a value during dataflow analysis (when it
    // could have several different values, or the analyzer is otherwise unable to assign it a
    // specific expression)
    IRNode::ID fDefined_Expression;

private:
    class Defined : public Expression {
    public:
        Defined(IRNode::ID type)
        : INHERITED(nullptr, -1, kDefined_Kind, type) {}

        bool hasSideEffects() const override {
            return false;
        }

        String description() const override {
            return "<defined>";
        }

        IRNode::ID clone() const override;

        typedef Expression INHERITED;
    };
};

} // namespace

#endif
