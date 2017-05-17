/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNonlinearColorSpaceXformEffect.h"

#include "GrProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

#include "SkColorSpace_Base.h"

class GrGLNonlinearColorSpaceXformEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrNonlinearColorSpaceXformEffect& csxe =
                args.fFp.cast<GrNonlinearColorSpaceXformEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const SkColorSpaceTransferFn& srcTransferFn = csxe.srcTransferFn();
        const SkColorSpaceTransferFn& dstTransferFn = csxe.dstTransferFn();
        const SkMatrix44& gamutXform = csxe.gamutXform();

        // Helper function to apply the src to linear transfer function to a single value
        SkString srcTfFuncName;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kSrcTransfer_Op)) {
          static const GrShaderVar gTransferFnFuncArgs[] = {
              GrShaderVar("x", kFloat_GrSLType),
          };
          SkString transferFnBody;
          transferFnBody.printf("return (x < %f) ? (%f * x) + %f : pow(%f * x + %f, %f) + %f;",
              srcTransferFn.fD, srcTransferFn.fC, srcTransferFn.fF, srcTransferFn.fA,
              srcTransferFn.fB, srcTransferFn.fG, srcTransferFn.fE);
          fragBuilder->emitFunction(kFloat_GrSLType, "src_transfer_fn",
                                    SK_ARRAY_COUNT(gTransferFnFuncArgs), gTransferFnFuncArgs,
                                    transferFnBody.c_str(), &srcTfFuncName);
        }

        // Likewise for linear to dst
        SkString dstTfFuncName;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kDstTransfer_Op)) {
          static const GrShaderVar gTransferFnFuncArgs[] = {
              GrShaderVar("x", kFloat_GrSLType),
          };
          SkString transferFnBody;
          transferFnBody.printf("return (x < %f) ? (%f * x) + %f : pow(%f * x + %f, %f) + %f;",
              dstTransferFn.fD, dstTransferFn.fC, dstTransferFn.fF, dstTransferFn.fA,
              dstTransferFn.fB, dstTransferFn.fG, dstTransferFn.fE);
          fragBuilder->emitFunction(kFloat_GrSLType, "dst_transfer_fn",
                                    SK_ARRAY_COUNT(gTransferFnFuncArgs), gTransferFnFuncArgs,
                                    transferFnBody.c_str(), &dstTfFuncName);
        }
        if (nullptr == args.fInputColor) {
            args.fInputColor = "vec4(1)";
        }
        fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);

        // 1: Un-premultiply the input color (if necessary)
        fragBuilder->codeAppendf("float nonZeroAlpha = max(color.a, 0.00001);");
        fragBuilder->codeAppendf("color = vec4(color.rgb / nonZeroAlpha, nonZeroAlpha);");

        // 2: Apply src transfer function (to get to linear RGB)
        if (!srcTfFuncName.isEmpty()) {
            fragBuilder->codeAppendf("color.r = %s(color.r);", srcTfFuncName.c_str());
            fragBuilder->codeAppendf("color.g = %s(color.g);", srcTfFuncName.c_str());
            fragBuilder->codeAppendf("color.b = %s(color.b);", srcTfFuncName.c_str());
        }

        // 3: Apply gamut matrix
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kGamutXform_Op)) {
            // Color is unpremultiplied at this point, so clamp to [0, 1]
            fragBuilder->codeAppendf("mat4 gamutXform = mat4(%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f);",
                gamutXform.get(0, 0), gamutXform.get(1, 0), gamutXform.get(2, 0), gamutXform.get(3, 0),
                gamutXform.get(0, 1), gamutXform.get(1, 1), gamutXform.get(2, 1), gamutXform.get(3, 1),
                gamutXform.get(0, 2), gamutXform.get(1, 2), gamutXform.get(2, 2), gamutXform.get(3, 2),
                gamutXform.get(0, 3), gamutXform.get(1, 3), gamutXform.get(2, 3), gamutXform.get(3, 3));
            fragBuilder->codeAppendf(
                "color.rgb = clamp((gamutXform * vec4(color.rgb, 1.0)).rgb, 0.0, 1.0);");
        }

        // 4: Apply dst transfer fn
        if (!dstTfFuncName.isEmpty()) {
            fragBuilder->codeAppendf("color.r = %s(color.r);", dstTfFuncName.c_str());
            fragBuilder->codeAppendf("color.g = %s(color.g);", dstTfFuncName.c_str());
            fragBuilder->codeAppendf("color.b = %s(color.b);", dstTfFuncName.c_str());
        }

        // 5: Premultiply again
        fragBuilder->codeAppendf("%s = vec4(color.rgb * color.a, color.a);", args.fOutputColor);
    }

    static inline void GenKey(const GrProcessor& processor, const GrShaderCaps&,
                              GrProcessorKeyBuilder* b) {
        const GrNonlinearColorSpaceXformEffect& csxe =
                processor.cast<GrNonlinearColorSpaceXformEffect>();
        b->add32(csxe.ops());
    }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& processor) override {
    }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrNonlinearColorSpaceXformEffect::GrNonlinearColorSpaceXformEffect(
    uint32_t ops, const SkColorSpaceTransferFn& srcTransferFn,
    const SkColorSpaceTransferFn& dstTransferFn, const SkMatrix44& gamutXform)
        : INHERITED(kPreservesOpaqueInput_OptimizationFlag)
        , fGamutXform(gamutXform)
        , fOps(ops) {
    this->initClassID<GrNonlinearColorSpaceXformEffect>();
    fSrcTransferFn = srcTransferFn;
    fDstTransferFn = dstTransferFn;
}

bool GrNonlinearColorSpaceXformEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrNonlinearColorSpaceXformEffect& other = s.cast<GrNonlinearColorSpaceXformEffect>();
    if (other.fOps != fOps) {
        return false;
    }
    if (SkToBool(fOps & kSrcTransfer_Op) &&
        memcmp(&other.fSrcTransferFn, &fSrcTransferFn, sizeof(fSrcTransferFn))) {
        return false;
    }
    if (SkToBool(fOps & kDstTransfer_Op) &&
        memcmp(&other.fDstTransferFn, &fDstTransferFn, sizeof(fDstTransferFn))) {
        return false;
    }
    if (SkToBool(fOps & kGamutXform_Op) && other.fGamutXform != fGamutXform) {
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrNonlinearColorSpaceXformEffect);

#if GR_TEST_UTILS
sk_sp<GrFragmentProcessor> GrNonlinearColorSpaceXformEffect::TestCreate(GrProcessorTestData* d) {
    // TODO: Generate a random variety of color spaces for this effect (it can handle wacky
    // transfer functions, etc...)
    sk_sp<SkColorSpace> srcSpace = SkColorSpace::MakeSRGBLinear();
    sk_sp<SkColorSpace> dstSpace = SkColorSpace::MakeSRGB();
    return GrNonlinearColorSpaceXformEffect::Make(srcSpace.get(), dstSpace.get());
}
#endif

///////////////////////////////////////////////////////////////////////////////

void GrNonlinearColorSpaceXformEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                             GrProcessorKeyBuilder* b) const {
    GrGLNonlinearColorSpaceXformEffect::GenKey(*this, caps, b);
}

GrGLSLFragmentProcessor* GrNonlinearColorSpaceXformEffect::onCreateGLSLInstance() const {
    return new GrGLNonlinearColorSpaceXformEffect();
}

sk_sp<GrFragmentProcessor> GrNonlinearColorSpaceXformEffect::Make(const SkColorSpace* src,
                                                                  const SkColorSpace* dst) {
    if (!src || !dst || SkColorSpace::Equals(src, dst)) {
        // No conversion possible (or necessary)
        return nullptr;
    }

    uint32_t ops = 0;

    // We rely on GrColorSpaceXform to build the gamut xform matrix for us (to get caching)
    auto gamutXform = GrColorSpaceXform::Make(src, dst);
    SkMatrix44 srcToDstMtx(SkMatrix44::kUninitialized_Constructor);
    if (gamutXform) {
        ops |= kGamutXform_Op;
        srcToDstMtx = gamutXform->srcToDst();
    }

    SkColorSpaceTransferFn srcTransferFn;
    if (!src->gammaIsLinear()) {
        if (src->isNumericalTransferFn(&srcTransferFn)) {
            ops |= kSrcTransfer_Op;
        } else {
            return nullptr;
        }
    }

    SkColorSpaceTransferFn dstTransferFn;
    if (!dst->gammaIsLinear()) {
        if (dst->isNumericalTransferFn(&dstTransferFn)) {
            dstTransferFn = dstTransferFn.invert();
            ops |= kDstTransfer_Op;
        } else {
            return nullptr;
        }
    }

    return sk_sp<GrFragmentProcessor>(new GrNonlinearColorSpaceXformEffect(
            ops, srcTransferFn, dstTransferFn, srcToDstMtx));
}
