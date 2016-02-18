/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAvoidXfermode.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkString.h"

SkAvoidXfermode::SkAvoidXfermode(SkColor opColor, U8CPU tolerance, Mode mode) {
    if (tolerance > 255) {
        tolerance = 255;
    }
    fTolerance = SkToU8(tolerance);
    fOpColor = opColor;
    fDistMul = (256 << 14) / (tolerance + 1);
    fMode = mode;
}

SkFlattenable* SkAvoidXfermode::CreateProc(SkReadBuffer& buffer) {
    const SkColor color = buffer.readColor();
    const unsigned tolerance = buffer.readUInt();
    const unsigned mode = buffer.readUInt();
    return Create(color, tolerance, (Mode)mode);
}

void SkAvoidXfermode::flatten(SkWriteBuffer& buffer) const {
    buffer.writeColor(fOpColor);
    buffer.writeUInt(fTolerance);
    buffer.writeUInt(fMode);
}

// returns 0..31
static unsigned color_dist16(uint16_t c, unsigned r, unsigned g, unsigned b) {
    SkASSERT(r <= SK_R16_MASK);
    SkASSERT(g <= SK_G16_MASK);
    SkASSERT(b <= SK_B16_MASK);

    unsigned dr = SkAbs32(SkGetPackedR16(c) - r);
    unsigned dg = SkAbs32(SkGetPackedG16(c) - g) >> (SK_G16_BITS - SK_R16_BITS);
    unsigned db = SkAbs32(SkGetPackedB16(c) - b);

    return SkMax32(dr, SkMax32(dg, db));
}

// returns 0..255
static unsigned color_dist32(SkPMColor c, U8CPU r, U8CPU g, U8CPU b) {
    SkASSERT(r <= 0xFF);
    SkASSERT(g <= 0xFF);
    SkASSERT(b <= 0xFF);

    unsigned dr = SkAbs32(SkGetPackedR32(c) - r);
    unsigned dg = SkAbs32(SkGetPackedG32(c) - g);
    unsigned db = SkAbs32(SkGetPackedB32(c) - b);

    return SkMax32(dr, SkMax32(dg, db));
}

static int scale_dist_14(int dist, uint32_t mul, uint32_t sub) {
    int tmp = dist * mul - sub;
    int result = (tmp + (1 << 13)) >> 14;

    return result;
}

static inline unsigned Accurate255To256(unsigned x) {
    return x + (x >> 7);
}

void SkAvoidXfermode::xfer32(SkPMColor dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[]) const {
    unsigned    opR = SkColorGetR(fOpColor);
    unsigned    opG = SkColorGetG(fOpColor);
    unsigned    opB = SkColorGetB(fOpColor);
    uint32_t    mul = fDistMul;
    uint32_t    sub = (fDistMul - (1 << 14)) << 8;

    int MAX, mask;

    if (kTargetColor_Mode == fMode) {
        mask = -1;
        MAX = 255;
    } else {
        mask = 0;
        MAX = 0;
    }

    for (int i = 0; i < count; i++) {
        int d = color_dist32(dst[i], opR, opG, opB);
        // now reverse d if we need to
        d = MAX + (d ^ mask) - mask;
        SkASSERT((unsigned)d <= 255);
        d = Accurate255To256(d);

        d = scale_dist_14(d, mul, sub);
        SkASSERT(d <= 256);

        if (d > 0) {
            if (aa) {
                d = SkAlphaMul(d, Accurate255To256(*aa++));
                if (0 == d) {
                    continue;
                }
            }
            dst[i] = SkFourByteInterp256(src[i], dst[i], d);
        }
    }
}

static inline U16CPU SkBlend3216(SkPMColor src, U16CPU dst, unsigned scale) {
    SkASSERT(scale <= 32);
    scale <<= 3;

    return SkPackRGB16(SkAlphaBlend(SkPacked32ToR16(src), SkGetPackedR16(dst), scale),
        SkAlphaBlend(SkPacked32ToG16(src), SkGetPackedG16(dst), scale),
        SkAlphaBlend(SkPacked32ToB16(src), SkGetPackedB16(dst), scale));
}

void SkAvoidXfermode::xfer16(uint16_t dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[]) const {
    unsigned    opR = SkColorGetR(fOpColor) >> (8 - SK_R16_BITS);
    unsigned    opG = SkColorGetG(fOpColor) >> (8 - SK_G16_BITS);
    unsigned    opB = SkColorGetB(fOpColor) >> (8 - SK_R16_BITS);
    uint32_t    mul = fDistMul;
    uint32_t    sub = (fDistMul - (1 << 14)) << SK_R16_BITS;

    int MAX, mask;

    if (kTargetColor_Mode == fMode) {
        mask = -1;
        MAX = 31;
    } else {
        mask = 0;
        MAX = 0;
    }

    for (int i = 0; i < count; i++) {
        int d = color_dist16(dst[i], opR, opG, opB);
        // now reverse d if we need to
        d = MAX + (d ^ mask) - mask;
        SkASSERT((unsigned)d <= 31);
        // convert from 0..31 to 0..32
        d += d >> 4;
        d = scale_dist_14(d, mul, sub);
        SkASSERT(d <= 32);

        if (d > 0) {
            if (aa) {
                d = SkAlphaMul(d, Accurate255To256(*aa++));
                if (0 == d) {
                    continue;
                }
            }
            dst[i] = SkBlend3216(src[i], dst[i], d);
        }
    }
}

void SkAvoidXfermode::xferA8(SkAlpha dst[], const SkPMColor src[], int count,
                             const SkAlpha aa[]) const {
}


#if SK_SUPPORT_GPU

#include "GrFragmentProcessor.h"
#include "GrInvariantOutput.h"
#include "GrXferProcessor.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLUniformHandler.h"
#include "glsl/GrGLSLXferProcessor.h"

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GLAvoidFP;

class AvoidFP : public GrFragmentProcessor {
public:
    static const GrFragmentProcessor* Create(SkColor opColor, uint8_t tolerance,
                                             SkAvoidXfermode::Mode mode,
                                             const GrFragmentProcessor* dst) {
        return new AvoidFP(opColor, tolerance, mode, dst);
    }

    ~AvoidFP() override { }

    const char* name() const override { return "Avoid"; }

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Color: 0x%08x Tol: %d Mode: %s",
                    fOpColor, fTolerance,
                    fMode == SkAvoidXfermode::kAvoidColor_Mode ? "Avoid" : "Target");
        return str;
    }

    SkColor opColor() const { return fOpColor; }
    uint8_t tol() const { return fTolerance; }
    SkAvoidXfermode::Mode mode() const { return fMode; }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor& fpBase) const override {
        const AvoidFP& fp = fpBase.cast<AvoidFP>();

        return fOpColor == fp.fOpColor &&
               fTolerance == fp.fTolerance &&
               fMode == fp.fMode;
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        inout->setToUnknown(GrInvariantOutput::kWill_ReadInput);
    }

    AvoidFP(SkColor opColor, uint8_t tolerance,
            SkAvoidXfermode::Mode mode, const GrFragmentProcessor* dst) 
        : fOpColor(opColor), fTolerance(tolerance), fMode(mode) {
        this->initClassID<AvoidFP>();

        SkASSERT(dst);
        SkDEBUGCODE(int dstIndex = )this->registerChildProcessor(dst);
        SkASSERT(0 == dstIndex);
    }

    SkColor               fOpColor;
    uint8_t               fTolerance;
    SkAvoidXfermode::Mode fMode;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;
    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

// Add common code for calculating avoid's distance value
static void add_avoid_code(GrGLSLFragmentBuilder* fragBuilder,
                           const char* dstColor,
                           const char* srcCoverage,
                           const char* kColorAndTolUni,
                           const char* kCoverageName,
                           SkAvoidXfermode::Mode mode) {

    fragBuilder->codeAppendf("vec3 temp = %s.rgb - %s.rgb;", dstColor, kColorAndTolUni);
    fragBuilder->codeAppendf("float dist = max(max(abs(temp.r), abs(temp.g)), abs(temp.b));");

    if (SkAvoidXfermode::kTargetColor_Mode == mode) {
        fragBuilder->codeAppendf("dist = 1.0 - dist;");
    }

    // the 'a' portion of the uniform is the scaled and inverted tolerance
    fragBuilder->codeAppendf("dist = dist * %s.a - (%s.a - 1.0);",
                             kColorAndTolUni, kColorAndTolUni);

    fragBuilder->codeAppendf("vec4 %s = vec4(dist);", kCoverageName);
    if (srcCoverage) {
        fragBuilder->codeAppendf("%s *= %s;", kCoverageName, srcCoverage);
    }
}

class GLAvoidFP : public GrGLSLFragmentProcessor {
public:
    void emitCode(EmitArgs& args) override {
        const AvoidFP& avoid = args.fFp.cast<AvoidFP>();

        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString dstColor("dstColor");
        this->emitChild(0, nullptr, &dstColor, args);

        fColorAndTolUni = args.fUniformHandler->addUniform(
                                                 kFragment_GrShaderFlag,
                                                 kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                 "colorAndTol");
        const char* kColorAndTolUni = args.fUniformHandler->getUniformCStr(fColorAndTolUni);

        const char* kCoverageName = "newCoverage";

        // add_avoid_code emits the code needed to compute the new coverage
        add_avoid_code(fragBuilder,
                       dstColor.c_str(), nullptr,
                       kColorAndTolUni, kCoverageName, avoid.mode());

        // The raster implementation's quantization and behavior yield a very noticeable
        // effect near zero (0.0039 = 1/256).
        fragBuilder->codeAppendf("if (%s.r < 0.0039) { %s = %s; } else {",
                                 kCoverageName, args.fOutputColor, dstColor.c_str());
        fragBuilder->codeAppendf("%s = %s * %s + (vec4(1.0)-%s) * %s;",
                                 args.fOutputColor,
                                 kCoverageName, args.fInputColor ? args.fInputColor : "vec4(1.0)",
                                 kCoverageName, dstColor.c_str());
        fragBuilder->codeAppend("}");
    }

    static void GenKey(const GrProcessor& proc, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        const AvoidFP& avoid = proc.cast<AvoidFP>();
        uint32_t key = avoid.mode() == SkAvoidXfermode::kTargetColor_Mode ? 1 : 0;
        b->add32(key);
    }

protected:
    void onSetData(const GrGLSLProgramDataManager& pdman, const GrProcessor& proc) override {
        const AvoidFP& avoid = proc.cast<AvoidFP>();
        pdman.set4f(fColorAndTolUni,
                    SkColorGetR(avoid.opColor())/255.0f,
                    SkColorGetG(avoid.opColor())/255.0f,
                    SkColorGetB(avoid.opColor())/255.0f,
                    256.0f/(avoid.tol()+1.0f));
    }

private:
    GrGLSLProgramDataManager::UniformHandle fColorAndTolUni;

    typedef GrGLSLFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

GrGLSLFragmentProcessor* AvoidFP::onCreateGLSLInstance() const {
    return new GLAvoidFP;
}

void AvoidFP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLAvoidFP::GenKey(*this, caps, b);
}

const GrFragmentProcessor* AvoidFP::TestCreate(GrProcessorTestData* d) {
    SkColor opColor = d->fRandom->nextU();
    uint8_t tolerance = d->fRandom->nextBits(8);
    SkAvoidXfermode::Mode mode = d->fRandom->nextBool() ? SkAvoidXfermode::kAvoidColor_Mode
                                                        : SkAvoidXfermode::kTargetColor_Mode;

    SkAutoTUnref<const GrFragmentProcessor> dst(GrProcessorUnitTest::CreateChildFP(d));
    return new AvoidFP(opColor, tolerance, mode, dst);
}

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(AvoidFP);

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class AvoidXP : public GrXferProcessor {
public:
    AvoidXP(const DstTexture* dstTexture, bool hasMixedSamples,
            SkColor opColor, uint8_t tolerance, SkAvoidXfermode::Mode mode)
        : INHERITED(dstTexture, true, hasMixedSamples)
        , fOpColor(opColor)
        , fTolerance(tolerance)
        , fMode(mode) {
        this->initClassID<AvoidXP>();
    }

    const char* name() const override { return "Avoid"; }

    GrGLSLXferProcessor* createGLSLInstance() const override;

    SkColor opColor() const { return fOpColor; }
    uint8_t tol() const { return fTolerance; }
    SkAvoidXfermode::Mode mode() const { return fMode; }

private:
    GrXferProcessor::OptFlags onGetOptimizations(const GrPipelineOptimizations& optimizations,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrCaps& caps) const override {
        return GrXferProcessor::kNone_OptFlags;
    }

    void onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrXferProcessor& xpBase) const override { 
        const AvoidXP& xp = xpBase.cast<AvoidXP>();

        return fOpColor == xp.fOpColor &&
               fTolerance == xp.fTolerance &&
               fMode == xp.fMode;
    }

    SkColor               fOpColor;
    uint8_t               fTolerance;
    SkAvoidXfermode::Mode fMode;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLAvoidXP : public GrGLSLXferProcessor {
public:
    static void GenKey(const GrProcessor& processor, const GrGLSLCaps&, GrProcessorKeyBuilder* b) {
        const AvoidXP& avoid = processor.cast<AvoidXP>();
        uint32_t key = SkAvoidXfermode::kTargetColor_Mode == avoid.mode() ? 1 : 0;
        b->add32(key);
    }

private:
    void emitBlendCodeForDstRead(GrGLSLXPFragmentBuilder* fragBuilder,
                                 GrGLSLUniformHandler* uniformHandler,
                                 const char* srcColor,
                                 const char* srcCoverage,
                                 const char* dstColor,
                                 const char* outColor,
                                 const char* outColorSecondary,
                                 const GrXferProcessor& proc) override {
        const AvoidXP& avoid = proc.cast<AvoidXP>();

        fColorAndTolUni = uniformHandler->addUniform(kFragment_GrShaderFlag,
                                                     kVec4f_GrSLType, kDefault_GrSLPrecision,
                                                     "colorAndTol");
        const char* kColorandTolUni = uniformHandler->getUniformCStr(fColorAndTolUni);

        const char* kCoverageName = "newCoverage";

        // add_avoid_code emits the code needed to compute the new coverage
        add_avoid_code(fragBuilder,
                       dstColor, srcCoverage,
                       kColorandTolUni, kCoverageName, avoid.mode());

        // The raster implementation's quantization and behavior yield a very noticeable
        // effect near zero (0.0039 = 1/256).
        fragBuilder->codeAppendf("if (%s.r < 0.0039) { %s = %s; } else {",
                                 kCoverageName, outColor, dstColor);
        fragBuilder->codeAppendf("%s = %s;", outColor, srcColor ? srcColor : "vec4(1.0)");
        INHERITED::DefaultCoverageModulation(fragBuilder, kCoverageName, dstColor, outColor,
                                             outColorSecondary, proc);
        fragBuilder->codeAppend("}");
    }

    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrXferProcessor& processor) override {
        const AvoidXP& avoid = processor.cast<AvoidXP>();
        pdman.set4f(fColorAndTolUni,
                    SkColorGetR(avoid.opColor())/255.0f,
                    SkColorGetG(avoid.opColor())/255.0f,
                    SkColorGetB(avoid.opColor())/255.0f,
                    256.0f/(avoid.tol()+1.0f));
    };

    GrGLSLProgramDataManager::UniformHandle fColorAndTolUni;

    typedef GrGLSLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

void AvoidXP::onGetGLSLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const {
    GLAvoidXP::GenKey(*this, caps, b);
}

GrGLSLXferProcessor* AvoidXP::createGLSLInstance() const { return new GLAvoidXP; }

///////////////////////////////////////////////////////////////////////////////
class GrAvoidXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkColor opColor, uint8_t tolerance,
                               SkAvoidXfermode::Mode mode) {
        return new GrAvoidXPFactory(opColor, tolerance, mode);
    }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor* blendedColor) const override {
        blendedColor->fWillBlendWithDst = true;
        blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
    }

private:
    GrAvoidXPFactory(SkColor opColor, uint8_t tolerance, SkAvoidXfermode::Mode mode)
        : fOpColor(opColor)
        , fTolerance(tolerance)
        , fMode(mode) {
        this->initClassID<GrAvoidXPFactory>();
    }

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture* dstTexture) const override {
        return new AvoidXP(dstTexture, hasMixedSamples, fOpColor, fTolerance, fMode);
    }

    bool onWillReadDstColor(const GrCaps& caps,
                            const GrPipelineOptimizations& optimizations,
                            bool hasMixedSamples) const override {
        return true;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        const GrAvoidXPFactory& xpf = xpfBase.cast<GrAvoidXPFactory>();
        return fOpColor == xpf.fOpColor &&
               fTolerance == xpf.fTolerance &&
               fMode == xpf.fMode;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkColor               fOpColor;
    uint8_t               fTolerance;
    SkAvoidXfermode::Mode fMode;

    typedef GrXPFactory INHERITED;
};

GR_DEFINE_XP_FACTORY_TEST(GrAvoidXPFactory);

const GrXPFactory* GrAvoidXPFactory::TestCreate(GrProcessorTestData* d) {
    SkColor opColor = d->fRandom->nextU();
    uint8_t tolerance = d->fRandom->nextBits(8);
    SkAvoidXfermode::Mode mode = d->fRandom->nextBool() ? SkAvoidXfermode::kAvoidColor_Mode
                                                        : SkAvoidXfermode::kTargetColor_Mode;
    return GrAvoidXPFactory::Create(opColor, tolerance, mode);
}

///////////////////////////////////////////////////////////////////////////////

const GrFragmentProcessor* SkAvoidXfermode::getFragmentProcessorForImageFilter(
                                                            const GrFragmentProcessor* dst) const {
    return AvoidFP::Create(fOpColor, fTolerance, fMode, dst);
}

GrXPFactory* SkAvoidXfermode::asXPFactory() const {
    return GrAvoidXPFactory::Create(fOpColor, fTolerance, fMode);
}
#endif

#ifndef SK_IGNORE_TO_STRING
void SkAvoidXfermode::toString(SkString* str) const {
    str->append("AvoidXfermode: opColor: ");
    str->appendHex(fOpColor);
    str->appendf("tolerance: %d ", fTolerance);

    static const char* gModeStrings[] = { "Avoid", "Target" };

    str->appendf("mode: %s", gModeStrings[fMode]);
}
#endif
