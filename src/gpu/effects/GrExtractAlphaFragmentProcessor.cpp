/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrExtractAlphaFragmentProcessor.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

class GLExtractAlphaFragmentProcessor : public GrGLFragmentProcessor {
public:
    GLExtractAlphaFragmentProcessor() {}

    void emitCode(EmitArgs& args) override {
        if (args.fInputColor) {
            GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();
            fsBuilder->codeAppendf("vec4 alpha4 = %s.aaaa;", args.fInputColor);
            this->emitChild(0, "alpha4", args.fOutputColor, args);
        } else {
            this->emitChild(0, nullptr, args.fOutputColor, args);
        }
    }

private:
    typedef GrGLFragmentProcessor INHERITED;
};

GrGLFragmentProcessor* GrExtractAlphaFragmentProcessor::onCreateGLInstance() const {
    return SkNEW(GLExtractAlphaFragmentProcessor);
}

void GrExtractAlphaFragmentProcessor::onGetGLProcessorKey(const GrGLSLCaps&,
                                                          GrProcessorKeyBuilder*) const {
}

bool GrExtractAlphaFragmentProcessor::onIsEqual(const GrFragmentProcessor&) const { return true; }

void GrExtractAlphaFragmentProcessor::onComputeInvariantOutput(GrInvariantOutput* inout) const {
    if (inout->validFlags() & kA_GrColorComponentFlag) {
        GrColor color = GrColorPackA4(GrColorUnpackA(inout->color()));
        inout->setToOther(kRGBA_GrColorComponentFlags, color,
                          GrInvariantOutput::kWill_ReadInput);
    } else {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }
    this->childProcessor(0).computeInvariantOutput(inout);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrExtractAlphaFragmentProcessor);

const GrFragmentProcessor* GrExtractAlphaFragmentProcessor::TestCreate(GrProcessorTestData* d) {
    SkAutoTUnref<const GrFragmentProcessor> child(GrProcessorUnitTest::CreateChildFP(d));
    return SkNEW_ARGS(GrExtractAlphaFragmentProcessor, (child));
}
