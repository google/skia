/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorPriv.h"
#include "SkOverdrawMode.h"
#include "SkString.h"
#include "SkXfermode.h"

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrXferProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLXferProcessor.h"

 ///////////////////////////////////////////////////////////////////////////////
 // Fragment Processor
 ///////////////////////////////////////////////////////////////////////////////

class GLOverdrawFP;

class GrOverdrawFP : public GrFragmentProcessor {
public:
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> dst) {
        return sk_sp<GrFragmentProcessor>(new GrOverdrawFP(std::move(dst)));
    }

    ~GrOverdrawFP() override { }

    const char* name() const override { return "Overdraw"; }

    SkString dumpInfo() const override {
        SkString str;
        return str;
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override { return true; }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }

    GrOverdrawFP(sk_sp<GrFragmentProcessor> dst) {
        this->initClassID<GrOverdrawFP>();

        SkASSERT(dst);
        SkDEBUGCODE(int dstIndex = )this->registerChildProcessor(std::move(dst));
        SkASSERT(0 == dstIndex);
    }

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static void add_overdraw_code(GrGLSLFragmentBuilder* fragBuilder,
                              const char* dstColor,
                              const char* outputColor) {

    static const GrGLSLShaderVar gColorTableArgs[] = {
        // TODO: once kInt_GrSLType lands - switch this over
        GrGLSLShaderVar("index", kFloat_GrSLType),
    };
    SkString colorTableFuncName;

    // The 'colorTable' function exists to work around older GLSL's prohibition
    // of initialized arrays. It takes an integer index and just returns the
    // corresponding color.
   fragBuilder->emitFunction(kVec4f_GrSLType,
                             "colorTable",
                             SK_ARRAY_COUNT(gColorTableArgs),
                             gColorTableArgs,
                             "if (index < 1.5) { return vec4(0.5,   0.617, 1.0,   1.0); }"
                             "if (index < 2.5) { return vec4(0.664, 0.723, 0.83,  1.0); }"
                             "if (index < 3.5) { return vec4(0.832, 0.762, 0.664, 1.0); }"
                             "if (index < 4.5) { return vec4(1,     0.75,  0.496, 1.0); }"
                             "if (index < 5.5) { return vec4(1,     0.723, 0.332, 1.0); }"
                             "if (index < 6.5) { return vec4(1,     0.645, 0.164, 1.0); }"
                             "if (index < 7.5) { return vec4(1,     0.527, 0,     1.0); }"
                             "if (index < 8.5) { return vec4(1,     0.371, 0,     1.0); }"
                             "if (index < 9.5) { return vec4(1,     0.195, 0,     1.0); }"
                             "return vec4(1,     0,     0,     1.0);",
                             &colorTableFuncName);

    fragBuilder->codeAppend("int nextIdx;");
    fragBuilder->codeAppendf("vec4 dst = %s;", dstColor);
    fragBuilder->codeAppend("if (dst.r < 0.25) { nextIdx = 1; }");
    // cap 'idx' at 10
    fragBuilder->codeAppend("else if (dst.g < 0.0977) { nextIdx = 10; }");
    fragBuilder->codeAppend("else if (dst.b > 0.08) { nextIdx = 8 - int(6.0 * dst.b + 0.5); }");
    fragBuilder->codeAppend("else { nextIdx = 11 - int(5.7 * dst.g + 0.5); }");
    fragBuilder->codeAppendf("%s = %s(float(nextIdx));", outputColor, colorTableFuncName.c_str());
}

class GLOverdrawFP : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        GrGLSLFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString dstColor("dstColor");
        this->emitChild(0, nullptr, &dstColor, args);

        add_overdraw_code(fragBuilder, dstColor.c_str(), args.fOutputColor);
    }

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) { }

private:
    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* GrOverdrawFP::onCreateGLSLInstance() const {
    return new GLOverdrawFP;
}

void GrOverdrawFP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLOverdrawFP::GenKey(*this, caps, b);
}

sk_sp<GrFragmentProcessor> GrOverdrawFP::TestCreate(GrProcessorTestData* d) {
    return GrOverdrawFP::Make(GrProcessorUnitTest::MakeChildFP(d));
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(GrOverdrawFP);

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class OverdrawXP : public GrXferProcessor {
public:
    OverdrawXP(const DstTexture* dstTexture, bool hasMixedSamples)
        : INHERITED(dstTexture, true, hasMixedSamples) {
        this->initClassID<OverdrawXP>();
    }

    const char* name() const override { return "Overdraw"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override {
        // We never look at the color input
        return GrXferProcessor::kIgnoreColor_OptFlag;
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor&) const override { return true; }

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLOverdrawXP : public GrGLSLXferProcessor {
public:
    GLOverdrawXP(const OverdrawXP&) { }

    ~GLOverdrawXP() override {}

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) { }

private:
    void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                 GrGLSLUniformHandler* uniformHandler,
                                 const char* srcColor,
                                 const char* srcCoverage,
                                 const char* dstColor,
                                 const char* outColor,
                                 const char* outColorSecondary,
                                 const GrXferProcessor& proc) override {
        add_overdraw_code(fragBuilder, dstColor, outColor);

        // Apply coverage.
        INHERITED::DefaultCoverageModulation(fragBuilder, srcCoverage, dstColor, outColor,
                                             outColorSecondary, proc);
    }

    void onSetData(const GrGLSLProgramDataManager&, const GrXferProcessor&) override { };

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void OverdrawXP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLOverdrawXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* OverdrawXP::createGLSLInstance() const { return new GLOverdrawXP(*this); }

///////////////////////////////////////////////////////////////////////////////
class GrOverdrawXPFactory : public GrXPFactory {
public:
    static sk_sp<GrXPFactory> Make() { return sk_sp<GrXPFactory>(new GrOverdrawXPFactory()); }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor* blendedColor) const override {
        blendedColor->fWillBlendWithDst = true;
        blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
    }

private:
    GrOverdrawXPFactory() {
        this->initClassID<GrOverdrawXPFactory>();
    }

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture* dstTexture) const override {
        return new OverdrawXP(dstTexture, hasMixedSamples);
    }

    bool onWillReadDstColor(const GrCaps&, const GrPipelineOptimizations&) const override {
        return true;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const override { return true; }

    GR_DECLARE_XP_FACTORY_TEST;

    typedef GrXPFactory INHERITED;
};

GR_DEFINE_XP_FACTORY_TEST(GrOverdrawXPFactory);

sk_sp<GrXPFactory> GrOverdrawXPFactory::TestCreate(GrProcessorTestData* d) {
    return GrOverdrawXPFactory::Make();
}
#endif

///////////////////////////////////////////////////////////////////////////////
class SkOverdrawXfermode : public SkXfermode {
public:
    static SkXfermode* Create() {
        return new SkOverdrawXfermode;
    }

    SkPMColor xferColor(SkPMColor src, SkPMColor dst) const override {
        // This table encodes the color progression of the overdraw visualization
        static const SkPMColor gTable[] = {
            SkPackARGB32(0x00, 0x00, 0x00, 0x00),
            SkPackARGB32(0xFF, 128, 158, 255),
            SkPackARGB32(0xFF, 170, 185, 212),
            SkPackARGB32(0xFF, 213, 195, 170),
            SkPackARGB32(0xFF, 255, 192, 127),
            SkPackARGB32(0xFF, 255, 185, 85),
            SkPackARGB32(0xFF, 255, 165, 42),
            SkPackARGB32(0xFF, 255, 135, 0),
            SkPackARGB32(0xFF, 255,  95, 0),
            SkPackARGB32(0xFF, 255,  50, 0),
            SkPackARGB32(0xFF, 255,  0, 0)
        };


        int nextIdx;
        if (SkColorGetR(dst) < 64) { // dst color is the 0th color so the next color is 1
            nextIdx = 1;
        } else if (SkColorGetG(dst) < 25) { // dst color is the 10th color so cap there
            nextIdx = 10;
        } else if ((SkColorGetB(dst)+21)/42 > 0) { // dst color is one of 1-6
            nextIdx = 8 - (SkColorGetB(dst)+21)/42;
        } else { // dst color is between 7 and 9
            nextIdx = 11 - (SkColorGetG(dst)+22)/45;
        }
        SkASSERT(nextIdx < (int)SK_ARRAY_COUNT(gTable));

        return gTable[nextIdx];
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkOverdrawXfermode)

#if SK_SUPPORT_GPU
    sk_sp<GrFragmentProcessor> makeFragmentProcessorForImageFilter(
                                                sk_sp<GrFragmentProcessor> dst) const override {
        return GrOverdrawFP::Make(dst);
    }

    sk_sp<GrXPFactory> asXPFactory() const override {
        return GrOverdrawXPFactory::Make();
    }
#endif

#ifndef SK_IGNORE_TO_STRING
    void toString(SkString* str) const override { str->set("SkOverdrawXfermode"); }
#endif

private:
    friend class SkOverdrawMode;

    void flatten(SkWriteBuffer& buffer) const override { }

    typedef SkXfermode INHERITED;
};

sk_sp<SkFlattenable> SkOverdrawXfermode::CreateProc(SkReadBuffer& buffer) {
    return sk_sp<SkFlattenable>(Create());
}

sk_sp<SkXfermode> SkOverdrawMode::Make() { return sk_make_sp<SkOverdrawXfermode>(); }

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkOverdrawMode)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkOverdrawXfermode)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
