/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrColorSpaceXform.h"

#include "include/core/SkString.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLColorSpaceXformHelper.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include <string.h>
#include <utility>

class GrGLSLProgramDataManager;
class GrGLSLUniformHandler;
struct GrShaderCaps;

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

uint32_t GrColorSpaceXform::XformKey(const GrColorSpaceXform* xform) {
    // Code generation depends on which steps we apply,
    // and the kinds of transfer functions (if we're applying those).
    if (!xform) { return 0; }

    const SkColorSpaceXformSteps& steps(xform->fSteps);
    uint32_t key = steps.flags.mask();
    if (steps.flags.linearize) {
        key |= skcms_TransferFunction_getType(&steps.srcTF)    << 8;
    }
    if (steps.flags.encode) {
        key |= skcms_TransferFunction_getType(&steps.dstTFInv) << 16;
    }
    return key;
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

GrColorSpaceXformEffect::GrColorSpaceXformEffect(std::unique_ptr<GrFragmentProcessor> child,
                                                 sk_sp<GrColorSpaceXform> colorXform)
        : INHERITED(kGrColorSpaceXformEffect_ClassID, OptFlags(child.get()))
        , fColorXform(std::move(colorXform)) {
    this->registerChild(std::move(child));
}

GrColorSpaceXformEffect::GrColorSpaceXformEffect(const GrColorSpaceXformEffect& that)
        : INHERITED(that)
        , fColorXform(that.fColorXform) {}

std::unique_ptr<GrFragmentProcessor> GrColorSpaceXformEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrColorSpaceXformEffect(*this));
}

bool GrColorSpaceXformEffect::onIsEqual(const GrFragmentProcessor& s) const {
    const GrColorSpaceXformEffect& other = s.cast<GrColorSpaceXformEffect>();
    return GrColorSpaceXform::Equals(fColorXform.get(), other.fColorXform.get());
}

void GrColorSpaceXformEffect::onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const {
    b->add32(GrColorSpaceXform::XformKey(fColorXform.get()));
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl>
GrColorSpaceXformEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            const GrColorSpaceXformEffect& proc = args.fFp.cast<GrColorSpaceXformEffect>();
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            GrGLSLUniformHandler* uniformHandler = args.fUniformHandler;

            fColorSpaceHelper.emitCode(uniformHandler, proc.colorXform());

            SkString childColor = this->invokeChild(0, args);

            SkString xformedColor;
            fragBuilder->appendColorGamutXform(
                    &xformedColor, childColor.c_str(), &fColorSpaceHelper);
            fragBuilder->codeAppendf("return %s;", xformedColor.c_str());
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& fp) override {
            const GrColorSpaceXformEffect& proc = fp.cast<GrColorSpaceXformEffect>();
            fColorSpaceHelper.setData(pdman, proc.colorXform());
        }

        GrGLSLColorSpaceXformHelper fColorSpaceHelper;
    };

    return std::make_unique<Impl>();
}

GrFragmentProcessor::OptimizationFlags GrColorSpaceXformEffect::OptFlags(
        const GrFragmentProcessor* child) {
    return ProcessorOptimizationFlags(child) & (kCompatibleWithCoverageAsAlpha_OptimizationFlag |
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
