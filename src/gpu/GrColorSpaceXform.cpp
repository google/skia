/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrColorSpaceXform.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkMatrix44.h"
#include "glsl/GrGLSLColorSpaceXformHelper.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(const SkColorSpace* src,
                                                 const SkColorSpace* dst) {
    // No transformation is performed in legacy mode
    if (!dst) {
        return nullptr;
    }

    // Treat null sources as sRGB
    if (!src) {
        src = SkColorSpace::MakeSRGB().get();
    }

    // TODO: Plumb source alpha type
    SkColorSpaceXformSteps steps(src, kUnpremul_SkAlphaType, dst);

    return steps.flags.mask() ? sk_make_sp<GrColorSpaceXform>(steps) : nullptr;
}

bool GrColorSpaceXform::Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b) {
    if (a == b) {
        return true;
    }

    if (!a || !b || a->fSteps.flags.mask() != b->fSteps.flags.mask()) {
        return false;
    }

    if (a->fSteps.flags.linearize &&
        0 != memcmp(&a->fSteps.srcTF, &b->fSteps.srcTF, sizeof(a->fSteps.srcTF))) {
        return false;
    }

    if (a->fSteps.flags.gamut_transform &&
        0 != memcmp(&a->fSteps.src_to_dst_matrix, &b->fSteps.src_to_dst_matrix,
                    sizeof(a->fSteps.src_to_dst_matrix))) {
        return false;
    }

    if (a->fSteps.flags.encode &&
        0 != memcmp(&a->fSteps.dstTFInv, &b->fSteps.dstTFInv, sizeof(a->fSteps.dstTFInv))) {
        return false;
    }

    return true;
}

GrColor4f GrColorSpaceXform::unclampedXform(const GrColor4f& srcColor) {
    GrColor4f result = srcColor;
    if (fSteps.flags.unpremul) {
        result = result.unpremul();
    }
    if (fSteps.flags.linearize) {
        for (int i = 0; i < 3; ++i) {
            result.fRGBA[i] = fSteps.srcTF(result.fRGBA[i]);
        }
    }
    if (fSteps.flags.gamut_transform) {
        GrColor4f temp = result;
        for (int i = 0; i < 3; ++i) {
            result.fRGBA[i] = fSteps.src_to_dst_matrix[3 * i + 0] * temp.fRGBA[0] +
                              fSteps.src_to_dst_matrix[3 * i + 1] * temp.fRGBA[1] +
                              fSteps.src_to_dst_matrix[3 * i + 2] * temp.fRGBA[2];
        }
    }
    if (fSteps.flags.encode) {
        for (int i = 0; i < 3; ++i) {
            result.fRGBA[i] = fSteps.dstTFInv(result.fRGBA[i]);
        }
    }
    if (fSteps.flags.premul) {
        result = result.premul();
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////////

class GrGLColorSpaceXformEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrColorSpaceXformEffect& csxe = args.fFp.cast<GrColorSpaceXformEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        fColorSpaceHelper.emitCode(uniformHandler, csxe.colorXform());

        SkString childColor("src_color");
        this->emitChild(0, &childColor, args);

        SkString xformedColor;
        fragBuilder->appendColorGamutXform(&xformedColor, childColor.c_str(), &fColorSpaceHelper);
        fragBuilder->codeAppendf("%s = %s * %s;", args.fOutputColor, xformedColor.c_str(),
                                 args.fInputColor);
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& processor) override {
        const GrColorSpaceXformEffect& csxe = processor.cast<GrColorSpaceXformEffect>();
        if (fColorSpaceHelper.isValid()) {
            fColorSpaceHelper.setData(pdman, csxe.colorXform());
        }
    }

    GrGLSLColorSpaceXformHelper fColorSpaceHelper;

    typedef GrGLSLFragmentProcessor INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

GrColorSpaceXformEffect::GrColorSpaceXformEffect(sk_sp<GrColorSpaceXform> colorXform)
        : INHERITED(kGrColorSpaceXformEffect_ClassID,
                    // TODO: Implement constant output for constant input
                    kCompatibleWithCoverageAsAlpha_OptimizationFlag
                    | kPreservesOpaqueInput_OptimizationFlag)
        , fColorXform(std::move(colorXform)) {}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrColorSpaceXformEffect(fColorXform));
}

bool GrColorSpaceXformEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrColorSpaceXformEffect& other = s.cast<GrColorSpaceXformEffect>();
    return GrColorSpaceXform::Equals(fColorXform.get(), other.fColorXform.get());
}

void GrColorSpaceXformEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    b->add32(GrColorSpaceXform::XformKey(fColorXform.get()));
}

GrGLSLFragmentProcessor* GrColorSpaceXformEffect::onCreateGLSLInstance() const {
    return new GrGLColorSpaceXformEffect();
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::Make(const SkColorSpace* src,
                                                                   const SkColorSpace* dst) {
    auto xform = GrColorSpaceXform::Make(src, dst);
    if (xform) {
        return std::unique_ptr<GrFragmentProcessor>(new GrColorSpaceXformEffect(std::move(xform)));
    } else {
        return nullptr;
    }
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::Make(
        std::unique_ptr<GrFragmentProcessor> child,
        const SkColorSpace* src,
        const SkColorSpace* dst) {
    if (!child) {
        return nullptr;
    }

    auto xformFP = Make(src, dst);
    if (!xformFP) {
        return child;
    }

    std::unique_ptr<GrFragmentProcessor> fpPipeline[] = { std::move(child), std::move(xformFP) };
    return GrFragmentProcessor::RunInSeries(fpPipeline, 2);
}
