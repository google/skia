/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"
#include "GrPipeline.h"
#include "GrProcessorAnalysis.h"
#include "effects/GrConstColorProcessor.h"
#include "effects/GrXfermodeFragmentProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

bool GrFragmentProcessor::isEqual(const GrFragmentProcessor& that) const {
    if (this->classID() != that.classID() ||
        !this->hasSameSamplersAndAccesses(that)) {
        return false;
    }
    if (!this->hasSameTransforms(that)) {
        return false;
    }
    if (!this->onIsEqual(that)) {
        return false;
    }
    if (this->numChildProcessors() != that.numChildProcessors()) {
        return false;
    }
    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).isEqual(that.childProcessor(i))) {
            return false;
        }
    }
    return true;
}

GrGLSLFragmentProcessor* GrFragmentProcessor::createGLSLInstance() const {
    GrGLSLFragmentProcessor* glFragProc = this->onCreateGLSLInstance();
    glFragProc->fChildProcessors.push_back_n(fChildProcessors.count());
    for (int i = 0; i < fChildProcessors.count(); ++i) {
        glFragProc->fChildProcessors[i] = fChildProcessors[i]->createGLSLInstance();
    }
    return glFragProc;
}

void GrFragmentProcessor::addCoordTransform(const GrCoordTransform* transform) {
    fCoordTransforms.push_back(transform);
    fFlags |= kUsesLocalCoords_Flag;
    SkDEBUGCODE(transform->setInProcessor();)
}

bool GrFragmentProcessor::instantiate(GrResourceProvider* resourceProvider) const {
    if (!INHERITED::instantiate(resourceProvider)) {
        return false;
    }

    for (int i = 0; i < this->numChildProcessors(); ++i) {
        if (!this->childProcessor(i).instantiate(resourceProvider)) {
            return false;
        }
    }

    return true;
}

int GrFragmentProcessor::registerChildProcessor(gr_fp<GrFragmentProcessor> child) {
    this->combineRequiredFeatures(*child);

    if (child->usesLocalCoords()) {
        fFlags |= kUsesLocalCoords_Flag;
    }

    int index = fChildProcessors.count();
    fChildProcessors.push_back(std::move(child));
    return index;
}

bool GrFragmentProcessor::hasSameTransforms(const GrFragmentProcessor& that) const {
    if (this->numCoordTransforms() != that.numCoordTransforms()) {
        return false;
    }
    int count = this->numCoordTransforms();
    for (int i = 0; i < count; ++i) {
        if (!this->coordTransform(i).hasSameEffectAs(that.coordTransform(i))) {
            return false;
        }
    }
    return true;
}

gr_fp<GrFragmentProcessor> GrFragmentProcessor::MulOutputByInputAlpha(
    gr_fp<GrFragmentProcessor> fp) {
    if (!fp) {
        return nullptr;
    }
    return GrXfermodeFragmentProcessor::MakeFromDstProcessor(std::move(fp), SkBlendMode::kDstIn);
}

namespace {

class PremulInputFragmentProcessor : public GrFragmentProcessor {
public:
    static gr_fp<GrFragmentProcessor> Make() {
        return gr_fp<GrFragmentProcessor>(new PremulInputFragmentProcessor);
    }

    const char* name() const override { return "PremultiplyInput"; }

    gr_fp<GrFragmentProcessor> clone() const override { return Make(); }

private:
    PremulInputFragmentProcessor()
            : INHERITED(kPreservesOpaqueInput_OptimizationFlag |
                        kConstantOutputForConstantInput_OptimizationFlag) {
        this->initClassID<PremulInputFragmentProcessor>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class GLFP : public GrGLSLFragmentProcessor {
        public:
            void emitCode(EmitArgs& args) override {
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, args.fInputColor);
                fragBuilder->codeAppendf("%s.rgb *= %s.a;",
                                            args.fOutputColor, args.fInputColor);
            }
        };
        return new GLFP;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        return input.premul();
    }

    typedef GrFragmentProcessor INHERITED;
};

class UnpremulInputFragmentProcessor : public GrFragmentProcessor {
public:
    static gr_fp<GrFragmentProcessor> Make() {
        return gr_fp<GrFragmentProcessor>(new UnpremulInputFragmentProcessor);
    }

    const char* name() const override { return "UnpremultiplyInput"; }

    gr_fp<GrFragmentProcessor> clone() const override { return Make(); }

private:
    UnpremulInputFragmentProcessor()
            : INHERITED(kPreservesOpaqueInput_OptimizationFlag |
                        kConstantOutputForConstantInput_OptimizationFlag) {
        this->initClassID<UnpremulInputFragmentProcessor>();
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class GLFP : public GrGLSLFragmentProcessor {
        public:
            void emitCode(EmitArgs& args) override {
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, args.fInputColor);
                fragBuilder->codeAppendf("float invAlpha = %s.a <= 0.0 ? 0.0 : 1.0 / %s.a;",
                                         args.fInputColor, args.fInputColor);
                fragBuilder->codeAppendf("%s.rgb *= invAlpha;", args.fOutputColor);
            }
        };
        return new GLFP;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
        return input.unpremul();
    }

    typedef GrFragmentProcessor INHERITED;
};

}

gr_fp<GrFragmentProcessor> GrFragmentProcessor::PremulInput(gr_fp<GrFragmentProcessor> fp) {
    return RunInSeries(PremulInputFragmentProcessor::Make(), std::move(fp));
}

gr_fp<GrFragmentProcessor> GrFragmentProcessor::PremulOutput(gr_fp<GrFragmentProcessor> fp) {
    return RunInSeries(std::move(fp), PremulInputFragmentProcessor::Make());
}

gr_fp<GrFragmentProcessor> GrFragmentProcessor::UnpremulOutput(gr_fp<GrFragmentProcessor> fp) {
    return RunInSeries(std::move(fp), UnpremulInputFragmentProcessor::Make());
}

gr_fp<GrFragmentProcessor> GrFragmentProcessor::SwizzleOutput(gr_fp<GrFragmentProcessor> fp,
                                                              const GrSwizzle& swizzle) {
    class SwizzleFragmentProcessor : public GrFragmentProcessor {
    public:
        static gr_fp<GrFragmentProcessor> Make(const GrSwizzle& swizzle) {
            return gr_fp<GrFragmentProcessor>(new SwizzleFragmentProcessor(swizzle));
        }

        const char* name() const override { return "Swizzle"; }
        const GrSwizzle& swizzle() const { return fSwizzle; }

        gr_fp<GrFragmentProcessor> clone() const override { return Make(fSwizzle); }

    private:
        SwizzleFragmentProcessor(const GrSwizzle& swizzle)
                : INHERITED(kAll_OptimizationFlags)
                , fSwizzle(swizzle) {
            this->initClassID<SwizzleFragmentProcessor>();
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    const SwizzleFragmentProcessor& sfp = args.fFp.cast<SwizzleFragmentProcessor>();
                    const GrSwizzle& swizzle = sfp.swizzle();
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;

                    fragBuilder->codeAppendf("%s = %s.%s;",
                                             args.fOutputColor, args.fInputColor, swizzle.c_str());
                }
            };
            return new GLFP;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
            b->add32(fSwizzle.asKey());
        }

        bool onIsEqual(const GrFragmentProcessor& other) const override {
            const SwizzleFragmentProcessor& sfp = other.cast<SwizzleFragmentProcessor>();
            return fSwizzle == sfp.fSwizzle;
        }

        GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
            return fSwizzle.applyTo(input);
        }

        GrSwizzle fSwizzle;

        typedef GrFragmentProcessor INHERITED;
    };

    if (GrSwizzle::RGBA() == swizzle) {
        return fp;
    }
    return RunInSeries(std::move(fp), SwizzleFragmentProcessor::Make(swizzle));
}

gr_fp<GrFragmentProcessor> GrFragmentProcessor::MakeInputPremulAndMulByOutput(
        gr_fp<GrFragmentProcessor> fp) {
    class PremulFragmentProcessor : public GrFragmentProcessor {
    public:
        static gr_fp<GrFragmentProcessor> Make(gr_fp<GrFragmentProcessor> processor) {
            return gr_fp<GrFragmentProcessor>(new PremulFragmentProcessor(std::move(processor)));
        }

        const char* name() const override { return "Premultiply"; }

        gr_fp<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0).clone());
        }

    private:
        PremulFragmentProcessor(gr_fp<GrFragmentProcessor> processor)
                : INHERITED(OptFlags(processor.get())) {
            this->initClassID<PremulFragmentProcessor>();
            this->registerChildProcessor(std::move(processor));
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                    this->emitChild(0, args);
                    fragBuilder->codeAppendf("%s.rgb *= %s.rgb;", args.fOutputColor,
                                                                args.fInputColor);
                    fragBuilder->codeAppendf("%s *= %s.a;", args.fOutputColor, args.fInputColor);
                }
            };
            return new GLFP;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

        static OptimizationFlags OptFlags(const GrFragmentProcessor* inner) {
            OptimizationFlags flags = kNone_OptimizationFlags;
            if (inner->preservesOpaqueInput()) {
                flags |= kPreservesOpaqueInput_OptimizationFlag;
            }
            if (inner->hasConstantOutputForConstantInput()) {
                flags |= kConstantOutputForConstantInput_OptimizationFlag;
            }
            return flags;
        }

        GrColor4f constantOutputForConstantInput(GrColor4f input) const override {
            GrColor4f childColor = ConstantOutputForConstantInput(this->childProcessor(0),
                                                                  GrColor4f::OpaqueWhite());
            return GrColor4f(input.fRGBA[3] * input.fRGBA[0] * childColor.fRGBA[0],
                             input.fRGBA[3] * input.fRGBA[1] * childColor.fRGBA[1],
                             input.fRGBA[3] * input.fRGBA[2] * childColor.fRGBA[2],
                             input.fRGBA[3] * childColor.fRGBA[3]);
        }

        typedef GrFragmentProcessor INHERITED;
    };
    if (!fp) {
        return nullptr;
    }
    return PremulFragmentProcessor::Make(std::move(fp));
}

//////////////////////////////////////////////////////////////////////////////

gr_fp<GrFragmentProcessor> GrFragmentProcessor::OverrideInput(gr_fp<GrFragmentProcessor> fp,
                                                              GrColor4f color) {
    class ReplaceInputFragmentProcessor : public GrFragmentProcessor {
    public:
        static gr_fp<GrFragmentProcessor> Make(gr_fp<GrFragmentProcessor> child, GrColor4f color) {
            return gr_fp<GrFragmentProcessor>(new ReplaceInputFragmentProcessor(std::move(child),
                                                                                color));
        }

        const char* name() const override { return "Replace Color"; }

        gr_fp<GrFragmentProcessor> clone() const override {
            return Make(this->childProcessor(0).clone(), fColor);
        }

    private:
        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLFP : public GrGLSLFragmentProcessor {
            public:
                GLFP() : fHaveSetColor(false) {}
                void emitCode(EmitArgs& args) override {
                    const char* colorName;
                    fColorUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                                 kVec4f_GrSLType,
                                                                 kDefault_GrSLPrecision,
                                                                 "Color", &colorName);
                    this->emitChild(0, colorName, args);
                }

            private:
                void onSetData(const GrGLSLProgramDataManager& pdman,
                               const GrFragmentProcessor& fp) override {
                    GrColor4f color = fp.cast<ReplaceInputFragmentProcessor>().fColor;
                    if (!fHaveSetColor || color != fPreviousColor) {
                        pdman.set4fv(fColorUni, 1, color.fRGBA);
                        fPreviousColor = color;
                        fHaveSetColor = true;
                    }
                }

                GrGLSLProgramDataManager::UniformHandle fColorUni;
                bool      fHaveSetColor;
                GrColor4f fPreviousColor;
            };

            return new GLFP;
        }

        ReplaceInputFragmentProcessor(gr_fp<GrFragmentProcessor> child, GrColor4f color)
                : INHERITED(OptFlags(child.get(), color)), fColor(color) {
            this->initClassID<ReplaceInputFragmentProcessor>();
            this->registerChildProcessor(std::move(child));
        }

        static OptimizationFlags OptFlags(const GrFragmentProcessor* child, GrColor4f color) {
            OptimizationFlags childFlags = child->optimizationFlags();
            OptimizationFlags flags = kNone_OptimizationFlags;
            if (childFlags & kConstantOutputForConstantInput_OptimizationFlag) {
                flags |= kConstantOutputForConstantInput_OptimizationFlag;
            }
            if ((childFlags & kPreservesOpaqueInput_OptimizationFlag) && color.isOpaque()) {
                flags |= kPreservesOpaqueInput_OptimizationFlag;
            }
            return flags;
        }

        void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override
        {}

        bool onIsEqual(const GrFragmentProcessor& that) const override {
            return fColor == that.cast<ReplaceInputFragmentProcessor>().fColor;
        }

        GrColor4f constantOutputForConstantInput(GrColor4f) const override {
            return ConstantOutputForConstantInput(this->childProcessor(0), fColor);
        }

        GrColor4f fColor;

        typedef GrFragmentProcessor INHERITED;
    };

    if (!fp) {
        return nullptr;
    }
    return ReplaceInputFragmentProcessor::Make(std::move(fp), color);
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor::Iter::Iter(const GrPipeline& pipeline) {
    for (auto fp : Series(pipeline.headColorFragmentProcessor())) {
        fFPStack.push_back(fp);
    }
    for (auto fp : Series(pipeline.headCoverageFragmentProcessor())) {
        fFPStack.push_back(fp);
    }
}

const GrFragmentProcessor* GrFragmentProcessor::Iter::next() {
    if (fFPStack.empty()) {
        return nullptr;
    }
    const GrFragmentProcessor* back = fFPStack.back();
    fFPStack.pop_back();
    for (int i = back->numChildProcessors() - 1; i >= 0; --i) {
        fFPStack.push_back(&back->childProcessor(i));
    }
    return back;
}

