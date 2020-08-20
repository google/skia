/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/mtl/GrMtlDepthStencil.h"
#include "src/gpu/mtl/GrMtlGpu.h"

MTLStencilOperation skia_stencil_op_to_mtl(GrStencilOp op) {
    switch (op) {
        case GrStencilOp::kKeep:
            return MTLStencilOperationKeep;
        case GrStencilOp::kZero:
            return MTLStencilOperationZero;
        case GrStencilOp::kReplace:
            return MTLStencilOperationReplace;
        case GrStencilOp::kInvert:
            return MTLStencilOperationInvert;
        case GrStencilOp::kIncWrap:
            return MTLStencilOperationIncrementWrap;
        case GrStencilOp::kDecWrap:
            return MTLStencilOperationDecrementWrap;
        case GrStencilOp::kIncClamp:
            return MTLStencilOperationIncrementClamp;
        case GrStencilOp::kDecClamp:
            return MTLStencilOperationDecrementClamp;
    }
}

sk_cf_obj<MTLStencilDescriptor*> skia_stencil_to_mtl(GrStencilSettings::Face face) {
    MTLStencilDescriptor* result = [[MTLStencilDescriptor alloc] init];
    switch (face.fTest) {
        case GrStencilTest::kAlways:
            result.stencilCompareFunction = MTLCompareFunctionAlways;
            break;
        case GrStencilTest::kNever:
            result.stencilCompareFunction = MTLCompareFunctionNever;
            break;
        case GrStencilTest::kGreater:
            result.stencilCompareFunction = MTLCompareFunctionGreater;
            break;
        case GrStencilTest::kGEqual:
            result.stencilCompareFunction = MTLCompareFunctionGreaterEqual;
            break;
        case GrStencilTest::kLess:
            result.stencilCompareFunction = MTLCompareFunctionLess;
            break;
        case GrStencilTest::kLEqual:
            result.stencilCompareFunction = MTLCompareFunctionLessEqual;
            break;
        case GrStencilTest::kEqual:
            result.stencilCompareFunction = MTLCompareFunctionEqual;
            break;
        case GrStencilTest::kNotEqual:
            result.stencilCompareFunction = MTLCompareFunctionNotEqual;
            break;
    }
    result.readMask = face.fTestMask;
    result.writeMask = face.fWriteMask;
    result.depthStencilPassOperation = skia_stencil_op_to_mtl(face.fPassOp);
    result.stencilFailureOperation = skia_stencil_op_to_mtl(face.fFailOp);
    return sk_cf_obj<MTLStencilDescriptor*>(result);
}

GrMtlDepthStencil* GrMtlDepthStencil::Create(const GrMtlGpu* gpu,
                                             const GrStencilSettings& stencil,
                                             GrSurfaceOrigin origin) {
    sk_cf_obj<MTLDepthStencilDescriptor*> desc([[MTLDepthStencilDescriptor alloc] init]);
    if (!stencil.isDisabled()) {
        if (stencil.isTwoSided()) {
            (*desc).frontFaceStencil = skia_stencil_to_mtl(stencil.postOriginCCWFace(origin)).get();
            (*desc).backFaceStencil = skia_stencil_to_mtl(stencil.postOriginCWFace(origin)).get();
        }
        else {
            (*desc).frontFaceStencil = skia_stencil_to_mtl(stencil.singleSidedFace()).get();
            (*desc).backFaceStencil = (*desc).frontFaceStencil;
        }
    }

    sk_cf_obj<id<MTLDepthStencilState>> stencilState(
            [gpu->device() newDepthStencilStateWithDescriptor:desc.get()]);
    return new GrMtlDepthStencil(std::move(stencilState),
                                 GenerateKey(stencil, origin));
}

void skia_stencil_to_key(GrStencilSettings::Face face, GrMtlDepthStencil::Key::Face* faceKey) {
    const int kPassOpShift = 3;
    const int kFailOpShift = 6;

    faceKey->fReadMask = face.fTestMask;
    faceKey->fWriteMask = face.fWriteMask;

    SkASSERT(static_cast<int>(face.fTest) <= 7);
    faceKey->fOps = static_cast<uint32_t>(face.fTest);

    SkASSERT(static_cast<int>(face.fPassOp) <= 7);
    faceKey->fOps |= (static_cast<uint32_t>(face.fPassOp) << kPassOpShift);

    SkASSERT(static_cast<int>(face.fFailOp) <= 7);
    faceKey->fOps |= (static_cast<uint32_t>(face.fFailOp) << kFailOpShift);
}

GrMtlDepthStencil::Key GrMtlDepthStencil::GenerateKey(const GrStencilSettings& stencil,
                                                      GrSurfaceOrigin origin) {
    Key depthStencilKey;

    if (stencil.isDisabled()) {
        memset(&depthStencilKey, 0, sizeof(Key));
    } else {
        if (stencil.isTwoSided()) {
            skia_stencil_to_key(stencil.postOriginCCWFace(origin), &depthStencilKey.fFront);
            skia_stencil_to_key(stencil.postOriginCWFace(origin), &depthStencilKey.fBack);
        }
        else {
            skia_stencil_to_key(stencil.singleSidedFace(), &depthStencilKey.fFront);
            memcpy(&depthStencilKey.fBack, &depthStencilKey.fFront, sizeof(Key::Face));
        }
    }

    return depthStencilKey;
}
