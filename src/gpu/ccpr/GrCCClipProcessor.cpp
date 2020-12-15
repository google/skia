/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCClipProcessor.h"

#include "src/gpu/ccpr/GrCCClipPath.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

static GrSurfaceProxyView make_view(const GrCaps& caps, GrSurfaceProxy* proxy,
                                    bool isCoverageCount) {
    GrColorType ct = isCoverageCount ? GrColorType::kAlpha_F16 : GrColorType::kAlpha_8;
    GrSwizzle swizzle = caps.getReadSwizzle(proxy->backendFormat(), ct);
    return { sk_ref_sp(proxy), GrCCAtlas::kTextureOrigin, swizzle };
}

GrCCClipProcessor::GrCCClipProcessor(std::unique_ptr<GrFragmentProcessor> inputFP,
                                     const GrCaps& caps,
                                     const GrCCClipPath* clipPath,
                                     IsCoverageCount isCoverageCount,
                                     MustCheckBounds mustCheckBounds)
        : INHERITED(kGrCCClipProcessor_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fClipPath(clipPath)
        , fIsCoverageCount(IsCoverageCount::kYes == isCoverageCount)
        , fMustCheckBounds(MustCheckBounds::kYes == mustCheckBounds) {
    auto view = make_view(caps, clipPath->atlasLazyProxy(), fIsCoverageCount);
    auto texEffect = GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType);
    this->registerChild(std::move(texEffect), SkSL::SampleUsage::Explicit());
    this->registerChild(std::move(inputFP));
}

GrCCClipProcessor::GrCCClipProcessor(const GrCCClipProcessor& that)
        : INHERITED(kGrCCClipProcessor_ClassID, that.optimizationFlags())
        , fClipPath(that.fClipPath)
        , fIsCoverageCount(that.fIsCoverageCount)
        , fMustCheckBounds(that.fMustCheckBounds) {
    this->cloneAndRegisterAllChildProcessors(that);
}

std::unique_ptr<GrFragmentProcessor> GrCCClipProcessor::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrCCClipProcessor(*this));
}

void GrCCClipProcessor::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    const SkPath& clipPath = fClipPath->deviceSpacePath();
    uint32_t key = (fIsCoverageCount) ? (uint32_t)GrFillRuleForSkPath(clipPath) : 0;
    key = (key << 1) | ((clipPath.isInverseFillType()) ? 1 : 0);
    key = (key << 1) | ((fMustCheckBounds) ? 1 : 0);
    b->add32(key);
}

bool GrCCClipProcessor::onIsEqual(const GrFragmentProcessor& fp) const {
    const GrCCClipProcessor& that = fp.cast<GrCCClipProcessor>();
    return that.fClipPath->deviceSpacePath().getGenerationID() ==
                   fClipPath->deviceSpacePath().getGenerationID() &&
           that.fClipPath->deviceSpacePath().getFillType() ==
                   fClipPath->deviceSpacePath().getFillType() &&
           that.fIsCoverageCount == fIsCoverageCount && that.fMustCheckBounds == fMustCheckBounds;
}

class GrCCClipProcessor::Impl : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrCCClipProcessor& proc = args.fFp.cast<GrCCClipProcessor>();
        GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        f->codeAppend("half coverage;");

        if (proc.fMustCheckBounds) {
            const char* pathIBounds;
            fPathIBoundsUniform = uniHandler->addUniform(&proc, kFragment_GrShaderFlag,
                                                         kFloat4_GrSLType, "path_ibounds",
                                                         &pathIBounds);
            f->codeAppendf("if (all(greaterThan(float4(sk_FragCoord.xy, %s.RB), "
                                               "float4(%s.LT, sk_FragCoord.xy)))) {",
                                               pathIBounds, pathIBounds);
        }

        const char* atlasTranslate;
        fAtlasTranslateUniform = uniHandler->addUniform(&proc, kFragment_GrShaderFlag,
                                                        kFloat2_GrSLType, "atlas_translate",
                                                        &atlasTranslate);
        SkString coord;
        coord.printf("sk_FragCoord.xy + %s.xy", atlasTranslate);
        constexpr int kTexEffectFPIndex = 0;
        SkString sample = this->invokeChild(kTexEffectFPIndex, args, coord.c_str());
        f->codeAppendf("coverage = %s.a;", sample.c_str());

        if (proc.fIsCoverageCount) {
            auto fillRule = GrFillRuleForSkPath(proc.fClipPath->deviceSpacePath());
            if (GrFillRule::kEvenOdd == fillRule) {
                f->codeAppend("half t = mod(abs(coverage), 2);");
                f->codeAppend("coverage = 1 - abs(t - 1);");
            } else {
                SkASSERT(GrFillRule::kNonzero == fillRule);
                f->codeAppend("coverage = min(abs(coverage), 1);");
            }
        }

        if (proc.fMustCheckBounds) {
            f->codeAppend("} else {");
            f->codeAppend(    "coverage = 0;");
            f->codeAppend("}");
        }

        if (proc.fClipPath->deviceSpacePath().isInverseFillType()) {
            f->codeAppend("coverage = 1 - coverage;");
        }

        constexpr int kInputFPIndex = 1;
        SkString inputColor = this->invokeChild(kInputFPIndex, args);

        f->codeAppendf("return %s * coverage;", inputColor.c_str());
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& fp) override {
        const GrCCClipProcessor& proc = fp.cast<GrCCClipProcessor>();
        if (proc.fMustCheckBounds) {
            const SkRect pathIBounds = SkRect::Make(proc.fClipPath->pathDevIBounds());
            pdman.set4f(fPathIBoundsUniform, pathIBounds.left(), pathIBounds.top(),
                        pathIBounds.right(), pathIBounds.bottom());
        }
        const SkIVector& trans = proc.fClipPath->atlasTranslate();
        pdman.set2f(fAtlasTranslateUniform, trans.x(), trans.y());
    }

private:
    UniformHandle fPathIBoundsUniform;
    UniformHandle fAtlasTranslateUniform;
};

GrGLSLFragmentProcessor* GrCCClipProcessor::onCreateGLSLInstance() const {
    return new Impl();
}
