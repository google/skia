/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNonlinearColorSpaceXformEffect.h"
#include "GrColorSpaceXform.h"
#include "GrProcessor.h"
#include "SkColorSpace_Base.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

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
                    kFragment_GrShaderFlag, kHalf_GrSLType, "SrcTransferFn",
                    GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs, &srcCoeffsName);
        }

        const char* dstCoeffsName = nullptr;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kDstTransfer_Op)) {
            fDstTransferFnUni = uniformHandler->addUniformArray(
                    kFragment_GrShaderFlag, kHalf_GrSLType, "DstTransferFn",
                    GrNonlinearColorSpaceXformEffect::kNumTransferFnCoeffs, &dstCoeffsName);
        }

        const char* gamutXformName = nullptr;
        if (SkToBool(csxe.ops() & GrNonlinearColorSpaceXformEffect::kGamutXform_Op)) {
            fGamutXformUni = uniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4x4_GrSLType,
                                                        "GamutXform", &gamutXformName);
        }

        // Helper function to apply the src or dst transfer function to a single value
        SkString tfFuncNames[2];
        for (size_t i = 0; i < 2; ++i) {
            const char* coeffsName = i ? dstCoeffsName : srcCoeffsName;
            if (!coeffsName) {
                continue;
            }
            const char* fnName = i ? "dst_transfer_fn" : "src_transfer_fn";
            static const GrShaderVar gTransferFnFuncArgs[] = {
                    GrShaderVar("x", kHalf_GrSLType),
            };
            SkString transferFnBody;
            // Temporaries to make evaluation line readable
            transferFnBody.printf("half A = %s[0];", coeffsName);
            transferFnBody.appendf("half B = %s[1];", coeffsName);
            transferFnBody.appendf("half C = %s[2];", coeffsName);
            transferFnBody.appendf("half D = %s[3];", coeffsName);
            transferFnBody.appendf("half E = %s[4];", coeffsName);
            transferFnBody.appendf("half F = %s[5];", coeffsName);
            transferFnBody.appendf("half G = %s[6];", coeffsName);
            transferFnBody.append("half s = sign(x);");
            transferFnBody.append("x = abs(x);");
            transferFnBody.appendf("return s * ((x < D) ? (C * x) + F : pow(A * x + B, G) + E);");
            fragBuilder->emitFunction(kHalf_GrSLType, fnName, SK_ARRAY_COUNT(gTransferFnFuncArgs),
                                      gTransferFnFuncArgs, transferFnBody.c_str(), &tfFuncNames[i]);
        }

        if (nullptr == args.fInputColor) {
            args.fInputColor = "half4(1)";
        }
        fragBuilder->codeAppendf("half4 color = %s;", args.fInputColor);

        // 1: Un-premultiply the input color (if necessary)
        fragBuilder->codeAppendf("half nonZeroAlpha = max(color.a, 0.00001);");
        fragBuilder->codeAppendf("color = half4(color.rgb / nonZeroAlpha, nonZeroAlpha);");

        // 2: Apply src transfer function (to get to linear RGB)
        if (srcCoeffsName) {
            fragBuilder->codeAppendf("color.r = %s(color.r);", tfFuncNames[0].c_str());
            fragBuilder->codeAppendf("color.g = %s(color.g);", tfFuncNames[0].c_str());
            fragBuilder->codeAppendf("color.b = %s(color.b);", tfFuncNames[0].c_str());
        }

        // 3: Apply gamut matrix
        if (gamutXformName) {
            fragBuilder->codeAppendf(
                "color.rgb = (%s * half4(color.rgb, 1.0)).rgb;", gamutXformName);
        }

        // 4: Apply dst transfer fn
        if (dstCoeffsName) {
            fragBuilder->codeAppendf("color.r = %s(color.r);", tfFuncNames[1].c_str());
            fragBuilder->codeAppendf("color.g = %s(color.g);", tfFuncNames[1].c_str());
            fragBuilder->codeAppendf("color.b = %s(color.b);", tfFuncNames[1].c_str());
        }

        // 5: Premultiply again
        fragBuilder->codeAppendf("%s = half4(color.rgb * color.a, color.a);", args.fOutputColor);
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
        : INHERITED(kGrNonlinearColorSpaceXformEffect_ClassID,
                    kPreservesOpaqueInput_OptimizationFlag)
        , fGamutXform(gamutXform)
        , fOps(ops) {

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

GrNonlinearColorSpaceXformEffect::GrNonlinearColorSpaceXformEffect(
        const GrNonlinearColorSpaceXformEffect& that)
        : INHERITED(kGrNonlinearColorSpaceXformEffect_ClassID,
                    kPreservesOpaqueInput_OptimizationFlag)
        , fGamutXform(that.fGamutXform)
        , fOps(that.fOps) {
    memcpy(fSrcTransferFnCoeffs, that.fSrcTransferFnCoeffs, sizeof(fSrcTransferFnCoeffs));
    memcpy(fDstTransferFnCoeffs, that.fDstTransferFnCoeffs, sizeof(fDstTransferFnCoeffs));
}

std::unique_ptr<GrFragmentProcessor> GrNonlinearColorSpaceXformEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrNonlinearColorSpaceXformEffect(*this));
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
std::unique_ptr<GrFragmentProcessor> GrNonlinearColorSpaceXformEffect::TestCreate(
        GrProcessorTestData* d) {
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

std::unique_ptr<GrFragmentProcessor> GrNonlinearColorSpaceXformEffect::Make(
        const SkColorSpace* src, const SkColorSpace* dst) {
    if (!src || !dst || SkColorSpace::Equals(src, dst)) {
        // No conversion possible (or necessary)
        return nullptr;
    }

    uint32_t ops = 0;

    // We rely on GrColorSpaceXform to build the gamut xform matrix for us (to get caching)
    auto gamutXform = GrColorSpaceXform::MakeGamutXform(src, dst);
    SkMatrix44 srcToDstMtx(SkMatrix44::kUninitialized_Constructor);
    if (gamutXform) {
        ops |= kGamutXform_Op;
        srcToDstMtx = gamutXform->gamutXform();
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

    return std::unique_ptr<GrFragmentProcessor>(
            new GrNonlinearColorSpaceXformEffect(ops, srcTransferFn, dstTransferFn, srcToDstMtx));
}
