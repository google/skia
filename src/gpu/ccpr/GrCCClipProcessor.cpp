/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ccpr/GrCCClipProcessor.h"

#include "include/gpu/GrTexture.h"
#include "src/core/SkMakeUnique.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/ccpr/GrCCClipPath.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"

GrCCClipProcessor::GrCCClipProcessor(const GrCCClipPath* clipPath, MustCheckBounds mustCheckBounds,
                                     SkPath::FillType overrideFillType)
        : INHERITED(kGrCCClipProcessor_ClassID, kCompatibleWithCoverageAsAlpha_OptimizationFlag)
        , fClipPath(clipPath)
        , fMustCheckBounds((bool)mustCheckBounds)
        , fOverrideFillType(overrideFillType)
        , fAtlasAccess(sk_ref_sp(fClipPath->atlasLazyProxy()), GrSamplerState::Filter::kNearest,
                       GrSamplerState::WrapMode::kClamp) {
    SkASSERT(fAtlasAccess.proxy());
    this->setTextureSamplerCnt(1);
}

std::unique_ptr<GrFragmentProcessor> GrCCClipProcessor::clone() const {
    return skstd::make_unique<GrCCClipProcessor>(fClipPath, MustCheckBounds(fMustCheckBounds),
                                                 fOverrideFillType);
}

void GrCCClipProcessor::onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const {
    b->add32((fOverrideFillType << 1) | (int)fMustCheckBounds);
}

bool GrCCClipProcessor::onIsEqual(const GrFragmentProcessor& fp) const {
    const GrCCClipProcessor& that = fp.cast<GrCCClipProcessor>();
    // Each ClipPath path has a unique atlas proxy, so hasSameSamplersAndAccesses should have
    // already weeded out FPs with different ClipPaths.
    SkASSERT(that.fClipPath->deviceSpacePath().getGenerationID() ==
             fClipPath->deviceSpacePath().getGenerationID());
    return that.fOverrideFillType == fOverrideFillType;
}

class GrCCClipProcessor::Impl : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const GrCCClipProcessor& proc = args.fFp.cast<GrCCClipProcessor>();
        GrGLSLUniformHandler* uniHandler = args.fUniformHandler;
        GrGLSLFPFragmentBuilder* f = args.fFragBuilder;

        f->codeAppend ("half coverage;");
        if (proc.fMustCheckBounds) {
            const char* pathIBounds;
            fPathIBoundsUniform = uniHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                         "path_ibounds", &pathIBounds);
            f->codeAppendf("if (all(greaterThan(float4(sk_FragCoord.xy, %s.zw), "
                                               "float4(%s.xy, sk_FragCoord.xy)))) {",
                                               pathIBounds, pathIBounds);
        }

        const char* atlasTransform;
        fAtlasTransformUniform = uniHandler->addUniform(kFragment_GrShaderFlag, kFloat4_GrSLType,
                                                        "atlas_transform", &atlasTransform);
        f->codeAppendf("float2 texcoord = sk_FragCoord.xy * %s.xy + %s.zw;",
                       atlasTransform, atlasTransform);

        f->codeAppend ("half coverage_count = ");
        f->appendTextureLookup(args.fTexSamplers[0], "texcoord", kHalf2_GrSLType);
        f->codeAppend (".a;");

        if (SkPath::kEvenOdd_FillType == proc.fOverrideFillType ||
            SkPath::kInverseEvenOdd_FillType == proc.fOverrideFillType) {
            f->codeAppend ("half t = mod(abs(coverage_count), 2);");
            f->codeAppend ("coverage = 1 - abs(t - 1);");
        } else {
            f->codeAppend ("coverage = min(abs(coverage_count), 1);");
        }

        if (proc.fMustCheckBounds) {
            f->codeAppend ("} else {");
            f->codeAppend (    "coverage = 0;");
            f->codeAppend ("}");
        }

        if (SkPath::IsInverseFillType(proc.fOverrideFillType)) {
            f->codeAppend ("coverage = 1 - coverage;");
        }

        f->codeAppendf("%s = %s * coverage;", args.fOutputColor, args.fInputColor);
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& fp) override {
        const GrCCClipProcessor& proc = fp.cast<GrCCClipProcessor>();
        if (proc.fMustCheckBounds) {
            const SkRect pathIBounds = SkRect::Make(proc.fClipPath->pathDevIBounds());
            pdman.set4f(fPathIBoundsUniform, pathIBounds.left(), pathIBounds.top(),
                        pathIBounds.right(), pathIBounds.bottom());
        }
        const SkVector& scale = proc.fClipPath->atlasScale();
        const SkVector& trans = proc.fClipPath->atlasTranslate();
        pdman.set4f(fAtlasTransformUniform, scale.x(), scale.y(), trans.x(), trans.y());
    }

private:
    UniformHandle fPathIBoundsUniform;
    UniformHandle fAtlasTransformUniform;
};

GrGLSLFragmentProcessor* GrCCClipProcessor::onCreateGLSLInstance() const {
    return new Impl();
}
