/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/effects/GrModulateAtlasCoverageEffect.h"

#include "src/gpu/GrDynamicAtlas.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/effects/GrTextureEffect.h"

GrModulateAtlasCoverageEffect::GrModulateAtlasCoverageEffect(
        Flags flags,
        std::unique_ptr<GrFragmentProcessor> inputFP,
        GrSurfaceProxyView atlasView,
        const SkMatrix& devToAtlasMatrix,
        const SkIRect& devIBounds)
        : GrFragmentProcessor(kTessellate_GrModulateAtlasCoverageEffect_ClassID,
                              kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fFlags(flags)
        , fBounds((fFlags & Flags::kCheckBounds) ? devIBounds : SkIRect{0,0,0,0}) {
    this->registerChild(std::move(inputFP));
    this->registerChild(GrTextureEffect::Make(std::move(atlasView), kUnknown_SkAlphaType,
                                              devToAtlasMatrix, GrSamplerState::Filter::kNearest),
                        SkSL::SampleUsage::Explicit());
}

GrModulateAtlasCoverageEffect::GrModulateAtlasCoverageEffect(
        const GrModulateAtlasCoverageEffect& that)
        : GrFragmentProcessor(that)
        , fFlags(that.fFlags)
        , fBounds(that.fBounds) {}

void GrModulateAtlasCoverageEffect::onAddToKey(const GrShaderCaps&,
                                               skgpu::KeyBuilder* b) const {
    b->add32(fFlags & Flags::kCheckBounds);
}

std::unique_ptr<GrFragmentProcessor::ProgramImpl>
GrModulateAtlasCoverageEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
        void emitCode(EmitArgs& args) override {
            auto fp = args.fFp.cast<GrModulateAtlasCoverageEffect>();
            auto f = args.fFragBuilder;
            auto uniHandler = args.fUniformHandler;
            SkString inputColor = this->invokeChild(0, args);
            f->codeAppend("half coverage = 0;");
            if (fp.fFlags & Flags::kCheckBounds) {
                const char* boundsName;
                fBoundsUniform = uniHandler->addUniform(&fp, kFragment_GrShaderFlag,
                                                        kFloat4_GrSLType, "bounds", &boundsName);
                // Are we inside the path's valid atlas bounds?
                f->codeAppendf("if (all(greaterThan(sk_FragCoord.xy, %s.xy)) && "
                                   "all(lessThan(sk_FragCoord.xy, %s.zw))) ",
                               boundsName, boundsName);
            }
            f->codeAppendf("{");
            SkString atlasCoverage = this->invokeChild(1, args, "sk_FragCoord.xy");
            f->codeAppendf("coverage = %s.a;", atlasCoverage.c_str());
            f->codeAppendf("}");
            const char* coverageMaybeInvertName;
            fCoverageMaybeInvertUniform = uniHandler->addUniform(&fp, kFragment_GrShaderFlag,
                                                                 kHalf2_GrSLType, "coverageInvert",
                                                                 &coverageMaybeInvertName);
            // Invert coverage, if needed.
            f->codeAppendf("coverage = coverage * %s.x + %s.y;",
                           coverageMaybeInvertName, coverageMaybeInvertName);
            f->codeAppendf("return %s * coverage;", inputColor.c_str());
        }

    private:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& processor) override {
            auto fp = processor.cast<GrModulateAtlasCoverageEffect>();
            if (fp.fFlags & Flags::kCheckBounds) {
                pdman.set4fv(fBoundsUniform, 1, SkRect::Make(fp.fBounds).asScalars());
            }
            if (fp.fFlags & Flags::kInvertCoverage) {
                pdman.set2f(fCoverageMaybeInvertUniform, -1, 1);  // -1*coverage + 1 = 1 - coverage.
            } else {
                pdman.set2f(fCoverageMaybeInvertUniform, 1, 0);  // 1*coverage + 0 = coverage.
            }
        }
        UniformHandle fBoundsUniform;
        UniformHandle fCoverageMaybeInvertUniform;
    };

    return std::make_unique<Impl>();
}
