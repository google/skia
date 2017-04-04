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
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        const char* srcCoeffsName = nullptr;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kSrcTransfer_Op)) {
            fSrcTransferFnUni = uniformHandler->addUniformArray(
                    kFragment_GrShaderFlag, kFloat_GrSLType, kDefault_GrSLPrecision,
                    "SrcTransferFn", GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs,
                    &srcCoeffsName);
        }

        const char* dstCoeffsName = nullptr;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kDstTransfer_Op)) {
            fDstTransferFnUni = uniformHandler->addUniformArray(
                    kFragment_GrShaderFlag, kFloat_GrSLType, kDefault_GrSLPrecision,
                    "DstTransferFn", GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs,
                    &dstCoeffsName);
        }

        const char* gamutXformName = nullptr;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kGamutXform_Op)) {
            fGamutXformUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kMat44f_GrSLType,
                                                        kDefault_GrSLPrecision, "GamutXform",
                                                        &gamutXformName);
        }

        // Helper function to apply a transfer function to a single value
        SkString tfFuncNameString;
        static const GrShaderVar gTransferFnFuncArgs[] = {
            GrShaderVar("x", kFloat_GrSLType),
            GrShaderVar("coeffs", kFloat_GrSLType,
                        GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs),
        };
        SkString transferFnBody;
        // Temporaries to make evaluation line readable
        transferFnBody.printf("float A = coeffs[0];");
        transferFnBody.append("float B = coeffs[1];");
        transferFnBody.append("float C = coeffs[2];");
        transferFnBody.append("float D = coeffs[3];");
        transferFnBody.append("float E = coeffs[4];");
        transferFnBody.append("float F = coeffs[5];");
        transferFnBody.append("float G = coeffs[6];");
        transferFnBody.appendf("return (x < D) ? (C * x) + F : pow(A * x + B, G) + E;");
        fragBuilder->emitFunction(kFloat_GrSLType, "transfer_fn",
                                  SK_ARRAY_COUNT(gTransferFnFuncArgs), gTransferFnFuncArgs,
                                  transferFnBody.c_str(), &tfFuncNameString);
        const char* tfFuncName = tfFuncNameString.c_str();

        if (nullptr == args.fInputColor) {
            args.fInputColor = "vec4(1)";
        }
        fragBuilder->codeAppendf("vec4 color = %s;", args.fInputColor);

        // 1: Un-premultiply the input color (if necessary)
        fragBuilder->codeAppendf("float nonZeroAlpha = max(color.a, 0.00001);");
        fragBuilder->codeAppendf("color = vec4(color.rgb / nonZeroAlpha, nonZeroAlpha);");

        // 2: Apply src transfer function (to get to linear RGB)
        if (srcCoeffsName) {
            fragBuilder->codeAppendf("color.r = %s(color.r, %s);", tfFuncName, srcCoeffsName);
            fragBuilder->codeAppendf("color.g = %s(color.g, %s);", tfFuncName, srcCoeffsName);
            fragBuilder->codeAppendf("color.b = %s(color.b, %s);", tfFuncName, srcCoeffsName);
        }

        // 3: Apply gamut matrix
        if (gamutXformName) {
            // Color is unpremultiplied at this point, so clamp to [0, 1]
            fragBuilder->codeAppendf(
                "color.rgb = clamp((%s * vec4(color.rgb, 1.0)).rgb, 0.0, 1.0);", gamutXformName);
        }

        // 4: Apply dst transfer fn
        if (dstCoeffsName) {
            fragBuilder->codeAppendf("color.r = %s(color.r, %s);", tfFuncName, dstCoeffsName);
            fragBuilder->codeAppendf("color.g = %s(color.g, %s);", tfFuncName, dstCoeffsName);
            fragBuilder->codeAppendf("color.b = %s(color.b, %s);", tfFuncName, dstCoeffsName);
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
        const GrNonlinearColorSpaceXformEffect& csxe =
                processor.cast<GrNonlinearColorSpaceXformEffect>();
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kSrcTransfer_Op)) {
            pdman.set1fv(fSrcTransferFnUni, GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs,
                         csxe.srcTransferFnCoeffs());
        }
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kDstTransfer_Op)) {
            pdman.set1fv(fDstTransferFnUni, GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs,
                         csxe.dstTransferFnCoeffs());
        }
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kGamutXform_Op)) {
            pdman.setSkMatrix44(fGamutXformUni, csxe.gamutXform());
        }
    }

private:
    GrGLSLProgramDataManager::UniformHandle fSrcTransferFnUni;
    GrGLSLProgramDataManager::UniformHandle fDstTransferFnUni;
    GrGLSLProgramDataManager::UniformHandle fGamutXformUni;

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

    fSrcTransferFnCoeffs[0] = srcTransferFn.fA;
    fSrcTransferFnCoeffs[1] = srcTransferFn.fB;
    fSrcTransferFnCoeffs[2] = srcTransferFn.fC;
    fSrcTransferFnCoeffs[3] = srcTransferFn.fD;
    fSrcTransferFnCoeffs[4] = srcTransferFn.fE;
    fSrcTransferFnCoeffs[5] = srcTransferFn.fF;
    fSrcTransferFnCoeffs[6] = srcTransferFn.fG;

    fDstTransferFnCoeffs[0] = dstTransferFn.fA;
    fDstTransferFnCoeffs[1] = dstTransferFn.fB;
    fDstTransferFnCoeffs[2] = dstTransferFn.fC;
    fDstTransferFnCoeffs[3] = dstTransferFn.fD;
    fDstTransferFnCoeffs[4] = dstTransferFn.fE;
    fDstTransferFnCoeffs[5] = dstTransferFn.fF;
    fDstTransferFnCoeffs[6] = dstTransferFn.fG;
}

bool GrNonlinearColorSpaceXformEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrNonlinearColorSpaceXformEffect& other = s.cast<GrNonlinearColorSpaceXformEffect>();
    if (other.fOps != fOps) {
        return false;
    }
    if (SkToBool(fOps & kSrcTransfer_Op) &&
        memcmp(&other.fSrcTransferFnCoeffs, &fSrcTransferFnCoeffs, sizeof(fSrcTransferFnCoeffs))) {
        return false;
    }
    if (SkToBool(fOps & kDstTransfer_Op) &&
        memcmp(&other.fDstTransferFnCoeffs, &fDstTransferFnCoeffs, sizeof(fDstTransferFnCoeffs))) {
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
