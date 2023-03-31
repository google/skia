/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"


#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"

using namespace skgpu::graphite;

namespace {

PaintParamsKey create_key(PaintParamsKeyBuilder* builder, int snippetID) {
    SkDEBUGCODE(builder->checkReset());

    builder->beginBlock(snippetID);
    builder->endBlock();

    return builder->lockAsKey();
}

bool coeff_equal(SkBlendModeCoeff skCoeff, skgpu::BlendCoeff gpuCoeff) {
    switch(skCoeff) {
        case SkBlendModeCoeff::kZero: return skgpu::BlendCoeff::kZero == gpuCoeff;
        case SkBlendModeCoeff::kOne:  return skgpu::BlendCoeff::kOne == gpuCoeff;
        case SkBlendModeCoeff::kSC:   return skgpu::BlendCoeff::kSC == gpuCoeff;
        case SkBlendModeCoeff::kISC:  return skgpu::BlendCoeff::kISC == gpuCoeff;
        case SkBlendModeCoeff::kDC:   return skgpu::BlendCoeff::kDC == gpuCoeff;
        case SkBlendModeCoeff::kIDC:  return skgpu::BlendCoeff::kIDC == gpuCoeff;
        case SkBlendModeCoeff::kSA:   return skgpu::BlendCoeff::kSA == gpuCoeff;
        case SkBlendModeCoeff::kISA:  return skgpu::BlendCoeff::kISA == gpuCoeff;
        case SkBlendModeCoeff::kDA:   return skgpu::BlendCoeff::kDA == gpuCoeff;
        case SkBlendModeCoeff::kIDA:  return skgpu::BlendCoeff::kIDA == gpuCoeff;
        default:                      return false;
    }
}

} // anonymous namespace

// These are intended to be unit tests of the PaintParamsKeyBuilder and PaintParamsKey.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(KeyWithInvalidCodeSnippetIDTest, reporter, context) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();
    PaintParamsKeyBuilder builder(dict);

    // Invalid code snippet ID, key creation fails.
    PaintParamsKey key = create_key(&builder, kBuiltInCodeSnippetIDCount);
    REPORTER_ASSERT(reporter, key.isErrorKey());
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(KeyEqualityChecksSnippetID, reporter, context) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    int userSnippetID1 = dict->addUserDefinedSnippet("key1");
    int userSnippetID2 = dict->addUserDefinedSnippet("key2");

    PaintParamsKeyBuilder builderA(dict);
    PaintParamsKeyBuilder builderB(dict);
    PaintParamsKeyBuilder builderC(dict);
    PaintParamsKey keyA = create_key(&builderA, userSnippetID1);
    PaintParamsKey keyB = create_key(&builderB, userSnippetID1);
    PaintParamsKey keyC = create_key(&builderC, userSnippetID2);

    // Verify that keyA matches keyB, and that it does not match keyC.
    REPORTER_ASSERT(reporter, keyA == keyB);
    REPORTER_ASSERT(reporter, keyA != keyC);
    REPORTER_ASSERT(reporter, !(keyA == keyC));
    REPORTER_ASSERT(reporter, !(keyA != keyB));
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(KeySetsBlendInfoOnShaderInfo, reporter, context) {
    ShaderCodeDictionary* dict = context->priv().shaderCodeDictionary();

    for (int bm = 0; bm <= (int) SkBlendMode::kLastCoeffMode; ++bm) {
        PaintParamsKeyBuilder builder(dict);
        PaintParamsKey key = create_key(&builder, bm + kFixedFunctionBlendModeIDOffset);

        ShaderInfo shaderInfo;
        key.toShaderInfo(dict, &shaderInfo);

        SkBlendModeCoeff expectedSrc, expectedDst;
        REPORTER_ASSERT(reporter, SkBlendMode_AsCoeff(static_cast<SkBlendMode>(bm),
                                                      &expectedSrc, &expectedDst));
        REPORTER_ASSERT(reporter, coeff_equal(expectedSrc, shaderInfo.blendInfo().fSrcBlend));
        REPORTER_ASSERT(reporter, coeff_equal(expectedDst, shaderInfo.blendInfo().fDstBlend));
        REPORTER_ASSERT(reporter, shaderInfo.blendInfo().fEquation == skgpu::BlendEquation::kAdd);
        REPORTER_ASSERT(reporter, shaderInfo.blendInfo().fBlendConstant == SK_PMColor4fTRANSPARENT);

        bool expectedWriteColor = BlendModifiesDst(skgpu::BlendEquation::kAdd,
                                                   shaderInfo.blendInfo().fSrcBlend,
                                                   shaderInfo.blendInfo().fDstBlend);
        REPORTER_ASSERT(reporter, shaderInfo.blendInfo().fWritesColor == expectedWriteColor);
    }
}

// TODO: Add unit tests for converting a complex key to a ShaderInfo
