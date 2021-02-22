/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrColorSpaceXform.h"

#include "include/core/SkColorSpace.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/GrColorInfo.h"
#include "src/gpu/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(SkColorSpace* src, SkAlphaType srcAT,
                                                 SkColorSpace* dst, SkAlphaType dstAT) {
    SkColorSpaceXformSteps steps(src, srcAT, dst, dstAT);
    return steps.flags.mask() == 0 ? nullptr  /* Noop transform */
                                   : sk_make_sp<GrColorSpaceXform>(steps);
}

sk_sp<GrColorSpaceXform> GrColorSpaceXform::Make(const GrColorInfo& srcInfo,
                                                 const GrColorInfo& dstInfo) {
    return Make(srcInfo.colorSpace(), srcInfo.alphaType(),
                dstInfo.colorSpace(), dstInfo.alphaType());
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

SkColor4f GrColorSpaceXform::apply(const SkColor4f& srcColor) {
    SkColor4f result = srcColor;
    fSteps.apply(result.vec());
    return result;
}

//////////////////////////////////////////////////////////////////////////////

class GrGLColorSpaceXformEffect : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrColorSpaceXformEffect& proc = args.fFp.cast<GrColorSpaceXformEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

        fColorSpaceHelper.emitCode(uniformHandler, proc.colorXform());

        SkString childColor = this->invokeChild(0, args);

        SkString xformedColor;
        fragBuilder->appendColorGamutXform(&xformedColor, childColor.c_str(), &fColorSpaceHelper);
        fragBuilder->codeAppendf("return %s;", xformedColor.c_str());
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& fp) override {
        const GrColorSpaceXformEffect& proc = fp.cast<GrColorSpaceXformEffect>();
        fColorSpaceHelper.setData(pdman, proc.colorXform());
    }

    GrGLSLColorSpaceXformHelper fColorSpaceHelper;

    using INHERITED = GrGLSLFragmentProcessor;
};

//////////////////////////////////////////////////////////////////////////////

GrColorSpaceXformEffect::GrColorSpaceXformEffect(std::unique_ptr<GrFragmentProcessor> child,
                                                 sk_sp<GrColorSpaceXform> colorXform)
        : INHERITED(kGrColorSpaceXformEffect_ClassID, OptFlags(child.get()))
        , fColorXform(std::move(colorXform)) {
    this->registerChild(std::move(child));
}

GrColorSpaceXformEffect::GrColorSpaceXformEffect(const GrColorSpaceXformEffect& that)
        : INHERITED(kGrColorSpaceXformEffect_ClassID, that.optimizationFlags())
        , fColorXform(that.fColorXform) {
    this->cloneAndRegisterAllChildProcessors(that);
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrColorSpaceXformEffect(*this));
}

bool GrColorSpaceXformEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrColorSpaceXformEffect& other = s.cast<GrColorSpaceXformEffect>();
    return GrColorSpaceXform::Equals(fColorXform.get(), other.fColorXform.get());
}

void GrColorSpaceXformEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                                    GrProcessorKeyBuilder* b) const {
    b->add32(GrColorSpaceXform::XformKey(fColorXform.get()));
}

std::unique_ptr<GrGLSLFragmentProcessor> GrColorSpaceXformEffect::onMakeProgramImpl() const {
    return std::make_unique<GrGLColorSpaceXformEffect>();
}

GrFragmentProcessor::OptimizationFlags GrColorSpaceXformEffect::OptFlags(
        const GrFragmentProcessor* child) {
    return (child ? ProcessorOptimizationFlags(child) : kAll_OptimizationFlags) &
              (kCompatibleWithCoverageAsAlpha_OptimizationFlag |
               kPreservesOpaqueInput_OptimizationFlag |
               kConstantOutputForConstantInput_OptimizationFlag);
}

SkPMColor4f GrColorSpaceXformEffect::constantOutputForConstantInput(
        const SkPMColor4f& input) const {
    const auto c0 = ConstantOutputForConstantInput(this->childProcessor(0), input);
    return this->fColorXform->apply(c0.unpremul()).premul();
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::Make(
        std::unique_ptr<GrFragmentProcessor> child,
        SkColorSpace* src, SkAlphaType srcAT,
        SkColorSpace* dst, SkAlphaType dstAT) {
    return Make(std::move(child), GrColorSpaceXform::Make(src, srcAT, dst, dstAT));
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::Make(
        std::unique_ptr<GrFragmentProcessor> child,
        const GrColorInfo& srcInfo,
        const GrColorInfo& dstInfo) {
    return Make(std::move(child), GrColorSpaceXform::Make(srcInfo, dstInfo));
}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::Make(
        std::unique_ptr<GrFragmentProcessor> child,
        sk_sp<GrColorSpaceXform> colorXform) {
    if (!colorXform) {
        return child;
    }

    return std::unique_ptr<GrFragmentProcessor>(new GrColorSpaceXformEffect(std::move(child),
                                                                            std::move(colorXform)));
}
