/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixelXorXfermode.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

// we always return an opaque color, 'cause I don't know what to do with
// the alpha-component and still return a valid premultiplied color.
SkPMColor SkPixelXorXfermode::xferColor(SkPMColor src, SkPMColor dst) const {
    SkPMColor res = src ^ dst ^ fOpColor;

    res |= (SK_A32_MASK << SK_A32_SHIFT);   // force it to be opaque
    return res;
}

void SkPixelXorXfermode::flatten(SkWriteBuffer& wb) const {
    wb.writeColor(SkColorSetRGB(SkGetPackedR32(fOpColor),
                                SkGetPackedG32(fOpColor),
                                SkGetPackedB32(fOpColor)));
}

SkFlattenable* SkPixelXorXfermode::CreateProc(SkReadBuffer& buffer) {
    return Create(buffer.readColor());
}

#ifndef SK_IGNORE_TO_STRING
void SkPixelXorXfermode::toString(SkString* str) const {
    str->append("SkPixelXorXfermode: ");
    str->appendHex(fOpColor);
}
#endif

#if SK_SUPPORT_GPU
#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrXferProcessor.h"

#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

static void add_pixelxor_code(GrGLSLFragmentBuilder* fragBuilder,
                              const char* srcColor,
                              const char* dstColor,
                              const char* outputColor,
                              const char* opColor) {
    static const GrGLSLShaderVar gXorArgs[] = {
        GrGLSLShaderVar("f1", kFloat_GrSLType),
        GrGLSLShaderVar("f2", kFloat_GrSLType),
        GrGLSLShaderVar("f3", kFloat_GrSLType),
        GrGLSLShaderVar("fPowerOf2Divisor", kFloat_GrSLType),
    };
    SkString xorFuncName;

    // The xor function checks if the three passed in floats (f1, f2, f3) would
    // have a bit in the log2(fPowerOf2Divisor)-th position if they were
    // represented by an int. It then performs an xor of the 3 bits (using
    // the property that serial xors can be treated as a sum of 0s & 1s mod 2).
    fragBuilder->emitFunction(kFloat_GrSLType,
                              "xor",
                              SK_ARRAY_COUNT(gXorArgs),
                              gXorArgs,
                              "float bit1 = floor(f1 / fPowerOf2Divisor);"
                              "float bit2 = floor(f2 / fPowerOf2Divisor);"
                              "float bit3 = floor(f3 / fPowerOf2Divisor);"
                              "return mod(bit1 + bit2 + bit3, 2.0);",
                              &xorFuncName);

    fragBuilder->codeAppend("float red = 0.0, green = 0.0, blue = 0.0;");

    if (srcColor) {
        fragBuilder->codeAppendf("vec3 src = 255.99 * %s.rgb;", srcColor);
    } else {
        fragBuilder->codeAppendf("vec3 src = vec3(255.99);");
    }
    fragBuilder->codeAppendf("vec3 dst = 255.99 * %s.rgb;", dstColor);
    fragBuilder->codeAppendf("vec3 op  = 255.99 * %s;", opColor);

    fragBuilder->codeAppend("float modValue = 128.0;");

    fragBuilder->codeAppend("for (int i = 0; i < 8; i++) {");

    fragBuilder->codeAppendf("float bit = %s(src.r, dst.r, op.r, modValue);", xorFuncName.c_str());
    fragBuilder->codeAppend("red += modValue * bit;");
    fragBuilder->codeAppend("src.r = mod(src.r, modValue);");
    fragBuilder->codeAppend("dst.r = mod(dst.r, modValue);");
    fragBuilder->codeAppend("op.r = mod(op.r, modValue);");

    fragBuilder->codeAppendf("bit = %s(src.g, dst.g, op.g, modValue);", xorFuncName.c_str());
    fragBuilder->codeAppend("green += modValue * bit;");
    fragBuilder->codeAppend("src.g = mod(src.g, modValue);");
    fragBuilder->codeAppend("dst.g = mod(dst.g, modValue);");
    fragBuilder->codeAppend("op.g = mod(op.g, modValue);");

    fragBuilder->codeAppendf("bit = %s(src.b, dst.b, op.b, modValue);", xorFuncName.c_str());
    fragBuilder->codeAppend("blue += modValue * bit;");
    fragBuilder->codeAppend("src.b = mod(src.b, modValue);");
    fragBuilder->codeAppend("dst.b = mod(dst.b, modValue);");
    fragBuilder->codeAppend("op.b = mod(op.b, modValue);");

    fragBuilder->codeAppend("modValue /= 2.0;");

    fragBuilder->codeAppend("}");

    fragBuilder->codeAppendf("%s = vec4(red/255.0, green/255.0, blue/255.0, 1.0);", outputColor);
}

class GLPixelXorFP;

class PixelXorFP : public GrFragmentProcessor {
public:
    static const GrFragmentProcessor* Create(SkPMColor opColor, const GrFragmentProcessor* dst) {
        return new PixelXorFP(opColor, dst);
    }

    ~PixelXorFP() override {};

    const char* name() const override { return "PixelXor"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Color: 0x%08x", fOpColor);
        return str;
    }

    SkPMColor opColor() const { return fOpColor; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& fpBase) const override {
        const PixelXorFP& fp = fpBase.cast<PixelXorFP>();
        return fOpColor == fp.fOpColor;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }

    PixelXorFP(SkPMColor opColor, const GrFragmentProcessor* dst)
        : fOpColor(opColor) {
        this->initClassID<PixelXorFP>();

        SkASSERT(dst);
        SkDEBUGCODE(int dstIndex = )this->registerChildProcessor(dst);
        SkASSERT(0 == dstIndex);
    }

    SkPMColor fOpColor;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLPixelXorFP : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString dstColor("dstColor");
        this->emitChild(0, nullptr, &dstColor, args);

        fOpColorUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag,
                                                       kVec3f_GrSLType, kHigh_GrSLPrecision,
                                                       "opColor");
        const char* kOpColorUni = args.fUniformHandler->getUniformCStr(fOpColorUni);

        add_pixelxor_code(fragBuilder, args.fInputColor, dstColor.c_str(),
                          args.fOutputColor, kOpColorUni);
    }

    static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) { }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
        const PixelXorFP& pixXor = proc.cast<PixelXorFP>();
        pdman.set3f(fOpColorUni,
                    SkGetPackedR32(pixXor.opColor())/255.0f,
                    SkGetPackedG32(pixXor.opColor())/255.0f,
                    SkGetPackedB32(pixXor.opColor())/255.0f);
    }

private:
    GrGLSLProgramDataManager::UniformHandle fOpColorUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* PixelXorFP::onCreateGLSLInstance() const {
    return new GLPixelXorFP;
}

void PixelXorFP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLPixelXorFP::GenKey(*this, caps, b);
}

const GrFragmentProcessor* PixelXorFP::TestCreate(GrProcessorTestData* d) {
    SkColor color = d->fRandom->nextU();

    SkAutoTUnref<const GrFragmentProcessor> dst(GrProcessorUnitTest::CreateChildFP(d));
    return new PixelXorFP(SkPreMultiplyColor(color), dst);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(PixelXorFP);

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class PixelXorXP : public GrXferProcessor {
public:
    PixelXorXP(const DstTexture* dstTexture, bool hasMixedSamples, SkPMColor opColor)
        : INHERITED(dstTexture, true, hasMixedSamples)
        , fOpColor(opColor) {
        this->initClassID<PixelXorXP>();
    }

    const char* name() const override { return "PixelXor"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkPMColor opColor() const { return fOpColor; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override {
        return GrXferProcessor::kNone_OptFlags;
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PixelXorXP& xp = xpBase.cast<PixelXorXP>();

        return fOpColor == xp.fOpColor;
    }

    SkPMColor fOpColor;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLPixelXorXP : public GrGLSLXferProcessor {
public:
    GLPixelXorXP(const PixelXorXP& pixelXorXP) { }

    ~GLPixelXorXP() override {}

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
        fOpColorUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                 kVec3f_GrSLType, kHigh_GrSLPrecision,
                                                 "opColor");
        const char* kOpColorUni = uniformHandler->getUniformCStr(fOpColorUni);

        add_pixelxor_code(fragBuilder, srcColor, dstColor, outColor, kOpColorUni);

        // Apply coverage.
        INHERITED::DefaultCoverageModulation(fragBuilder, srcCoverage, dstColor, outColor,
                                             outColorSecondary, proc);
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrXferProcessor& processor) override {
        const PixelXorXP& pixelXor = processor.cast<PixelXorXP>();
        pdman.set3f(fOpColorUni, 
                    SkGetPackedR32(pixelXor.opColor())/255.0f,
                    SkGetPackedG32(pixelXor.opColor())/255.0f,
                    SkGetPackedB32(pixelXor.opColor())/255.0f);
    };

    GrGLSLProgramDataManager::UniformHandle fOpColorUni;

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void PixelXorXP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLPixelXorXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* PixelXorXP::createGLSLInstance() const { return new GLPixelXorXP(*this); }

///////////////////////////////////////////////////////////////////////////////

class GrPixelXorXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkPMColor opColor) {
        return new GrPixelXorXPFactory(opColor);
    }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor* blendedColor) const override {
        blendedColor->fWillBlendWithDst = true;
        blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
    }

private:
    GrPixelXorXPFactory(SkPMColor opColor)
        : fOpColor(opColor) {
        this->initClassID<GrPixelXorXPFactory>();
    }

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture* dstTexture) const override {
        return new PixelXorXP(dstTexture, hasMixedSamples, fOpColor);
    }

    bool onWillReadDstColor(const GrCaps& caps,
                            const GrPipelineOptimizations& optimizations,
                            bool hasMixedSamples) const override {
        return true;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        const GrPixelXorXPFactory& xpf = xpfBase.cast<GrPixelXorXPFactory>();
        return fOpColor == xpf.fOpColor;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkPMColor fOpColor;

    typedef GrXPFactory INHERITED;
};

GR_DEFINE_XP_FACTORY_TEST(GrPixelXorXPFactory);

const GrXPFactory* GrPixelXorXPFactory::TestCreate(GrProcessorTestData* d) {
    SkColor color = d->fRandom->nextU();

    return GrPixelXorXPFactory::Create(SkPreMultiplyColor(color));
}

///////////////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* SkPixelXorXfermode::getFragmentProcessorForImageFilter(
                                                        const GrFragmentProcessor* dst) const {
    return PixelXorFP::Create(fOpColor, dst);
}

GrXPFactory* SkPixelXorXfermode::asXPFactory() const {
    return GrPixelXorXPFactory::Create(fOpColor);
}

#endif
