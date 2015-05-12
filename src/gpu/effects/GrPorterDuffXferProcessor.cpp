/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "effects/GrPorterDuffXferProcessor.h"

#include "GrBlend.h"
#include "GrDrawTargetCaps.h"
#include "GrProcessor.h"
#include "GrProcOptInfo.h"
#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "gl/GrGLXferProcessor.h"
#include "gl/builders/GrGLFragmentShaderBuilder.h"
#include "gl/builders/GrGLProgramBuilder.h"

static bool can_tweak_alpha_for_coverage(GrBlendCoeff dstCoeff) {
    /*
     The fractional coverage is f.
     The src and dst coeffs are Cs and Cd.
     The dst and src colors are S and D.
     We want the blend to compute: f*Cs*S + (f*Cd + (1-f))D. By tweaking the source color's alpha
     we're replacing S with S'=fS. It's obvious that that first term will always be ok. The second
     term can be rearranged as [1-(1-Cd)f]D. By substituting in the various possibilities for Cd we
     find that only 1, ISA, and ISC produce the correct destination when applied to S' and D.
     */
    return kOne_GrBlendCoeff == dstCoeff ||
           kISA_GrBlendCoeff == dstCoeff ||
           kISC_GrBlendCoeff == dstCoeff;
}

class PorterDuffXferProcessor : public GrXferProcessor {
public:
    static GrXferProcessor* Create(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend,
                                   GrColor constant, const GrDeviceCoordTexture* dstCopy,
                                   bool willReadDstColor) {
        return SkNEW_ARGS(PorterDuffXferProcessor, (srcBlend, dstBlend, constant, dstCopy,
                                                    willReadDstColor));
    }

    ~PorterDuffXferProcessor() override;

    const char* name() const override { return "Porter Duff"; }

    GrGLXferProcessor* createGLInstance() const override;

    bool hasSecondaryOutput() const override;

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stage Output Types
    ////

    enum PrimaryOutputType {
        kNone_PrimaryOutputType,
        kColor_PrimaryOutputType,
        kCoverage_PrimaryOutputType,
        // Modulate color and coverage, write result as the color output.
        kModulate_PrimaryOutputType,
        // Custom Porter-Duff output, used for when we explictly are reading the dst and blending
        // in the shader. Secondary Output must be none if you use this. The custom blend uses the
        // equation: cov * (coeffS * S + coeffD * D) + (1 - cov) * D
        kCustom_PrimaryOutputType
    };

    enum SecondaryOutputType {
        // There is no secondary output
        kNone_SecondaryOutputType,
        // Writes coverage as the secondary output. Only set if dual source blending is supported
        // and primary output is kModulate.
        kCoverage_SecondaryOutputType,
        // Writes coverage * (1 - colorA) as the secondary output. Only set if dual source blending
        // is supported and primary output is kModulate.
        kCoverageISA_SecondaryOutputType,
        // Writes coverage * (1 - colorRGBA) as the secondary output. Only set if dual source
        // blending is supported and primary output is kModulate.
        kCoverageISC_SecondaryOutputType,

        kSecondaryOutputTypeCnt,
    };

    PrimaryOutputType primaryOutputType() const { return fPrimaryOutputType; }
    SecondaryOutputType secondaryOutputType() const { return fSecondaryOutputType; }

    GrBlendCoeff getSrcBlend() const { return fSrcBlend; }
    GrBlendCoeff getDstBlend() const { return fDstBlend; }

private:
    PorterDuffXferProcessor(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend, GrColor constant,
                            const GrDeviceCoordTexture* dstCopy, bool willReadDstColor);

    GrXferProcessor::OptFlags onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                 const GrProcOptInfo& coveragePOI,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrDrawTargetCaps& caps) override;

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        if (!this->willReadDstColor()) {
            blendInfo->fSrcBlend = fSrcBlend;
            blendInfo->fDstBlend = fDstBlend;
        } else {
            blendInfo->fSrcBlend = kOne_GrBlendCoeff;
            blendInfo->fDstBlend = kZero_GrBlendCoeff;
        }
        blendInfo->fBlendConstant = fBlendConstant;
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PorterDuffXferProcessor& xp = xpBase.cast<PorterDuffXferProcessor>();
        if (fSrcBlend != xp.fSrcBlend ||
            fDstBlend != xp.fDstBlend ||
            fBlendConstant != xp.fBlendConstant ||
            fPrimaryOutputType != xp.fPrimaryOutputType || 
            fSecondaryOutputType != xp.fSecondaryOutputType) {
            return false;
        }
        return true;
    }

    GrXferProcessor::OptFlags internalGetOptimizations(const GrProcOptInfo& colorPOI,
                                                       const GrProcOptInfo& coveragePOI,
                                                       bool doesStencilWrite);

    void calcOutputTypes(GrXferProcessor::OptFlags blendOpts, const GrDrawTargetCaps& caps,
                         bool hasSolidCoverage);

    GrBlendCoeff fSrcBlend;
    GrBlendCoeff fDstBlend;
    GrColor      fBlendConstant;
    PrimaryOutputType fPrimaryOutputType;
    SecondaryOutputType fSecondaryOutputType;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

bool append_porterduff_term(GrGLXPFragmentBuilder* fsBuilder, GrBlendCoeff coeff,
                            const char* colorName, const char* srcColorName,
                            const char* dstColorName, bool hasPrevious) {
    if (kZero_GrBlendCoeff == coeff) {
        return hasPrevious;
    } else {
        if (hasPrevious) {
            fsBuilder->codeAppend(" + ");
        }
        fsBuilder->codeAppendf("%s", colorName);
        switch (coeff) {
            case kOne_GrBlendCoeff:
                break;
            case kSC_GrBlendCoeff:
                fsBuilder->codeAppendf(" * %s", srcColorName); 
                break;
            case kISC_GrBlendCoeff:
                fsBuilder->codeAppendf(" * (vec4(1.0) - %s)", srcColorName); 
                break;
            case kDC_GrBlendCoeff:
                fsBuilder->codeAppendf(" * %s", dstColorName); 
                break;
            case kIDC_GrBlendCoeff:
                fsBuilder->codeAppendf(" * (vec4(1.0) - %s)", dstColorName); 
                break;
            case kSA_GrBlendCoeff:
                fsBuilder->codeAppendf(" * %s.a", srcColorName); 
                break;
            case kISA_GrBlendCoeff:
                fsBuilder->codeAppendf(" * (1.0 - %s.a)", srcColorName); 
                break;
            case kDA_GrBlendCoeff:
                fsBuilder->codeAppendf(" * %s.a", dstColorName); 
                break;
            case kIDA_GrBlendCoeff:
                fsBuilder->codeAppendf(" * (1.0 - %s.a)", dstColorName); 
                break;
            default:
                SkFAIL("Unsupported Blend Coeff");
        }
        return true;
    }
}

class GLPorterDuffXferProcessor : public GrGLXferProcessor {
public:
    GLPorterDuffXferProcessor(const GrProcessor&) {}

    virtual ~GLPorterDuffXferProcessor() {}

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps& caps,
                       GrProcessorKeyBuilder* b) {
        const PorterDuffXferProcessor& xp = processor.cast<PorterDuffXferProcessor>();
        b->add32(xp.primaryOutputType());
        b->add32(xp.secondaryOutputType());
        if (xp.willReadDstColor()) {
            b->add32(xp.getSrcBlend());
            b->add32(xp.getDstBlend());
        }
    };

private:
    void onEmitCode(const EmitArgs& args) override {
        const PorterDuffXferProcessor& xp = args.fXP.cast<PorterDuffXferProcessor>();
        GrGLXPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();
        if (PorterDuffXferProcessor::kCustom_PrimaryOutputType != xp.primaryOutputType()) {
            SkASSERT(!xp.willReadDstColor());
            switch(xp.secondaryOutputType()) {
                case PorterDuffXferProcessor::kNone_SecondaryOutputType:
                    break;
                case PorterDuffXferProcessor::kCoverage_SecondaryOutputType:
                    fsBuilder->codeAppendf("%s = %s;", args.fOutputSecondary,
                                           args.fInputCoverage);
                    break;
                case PorterDuffXferProcessor::kCoverageISA_SecondaryOutputType:
                    fsBuilder->codeAppendf("%s = (1.0 - %s.a) * %s;",
                                           args.fOutputSecondary, args.fInputColor,
                                           args.fInputCoverage);
                    break;
                case PorterDuffXferProcessor::kCoverageISC_SecondaryOutputType:
                    fsBuilder->codeAppendf("%s = (vec4(1.0) - %s) * %s;",
                                           args.fOutputSecondary, args.fInputColor,
                                           args.fInputCoverage);
                    break;
                default:
                    SkFAIL("Unexpected Secondary Output");
            }

            switch (xp.primaryOutputType()) {
                case PorterDuffXferProcessor::kNone_PrimaryOutputType:
                    fsBuilder->codeAppendf("%s = vec4(0);", args.fOutputPrimary);
                    break;
                case PorterDuffXferProcessor::kColor_PrimaryOutputType:
                    fsBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputColor);
                    break;
                case PorterDuffXferProcessor::kCoverage_PrimaryOutputType:
                    fsBuilder->codeAppendf("%s = %s;", args.fOutputPrimary, args.fInputCoverage);
                    break;
                case PorterDuffXferProcessor::kModulate_PrimaryOutputType:
                    fsBuilder->codeAppendf("%s = %s * %s;", args.fOutputPrimary, args.fInputColor,
                                           args.fInputCoverage);
                    break;
                default:
                    SkFAIL("Unexpected Primary Output");
            }
        } else {
            SkASSERT(xp.willReadDstColor());

            const char* dstColor = fsBuilder->dstColor();

            fsBuilder->codeAppend("vec4 colorBlend =");
            // append src blend
            bool didAppend = append_porterduff_term(fsBuilder, xp.getSrcBlend(),
                                                    args.fInputColor, args.fInputColor,
                                                    dstColor, false);
            // append dst blend
            SkAssertResult(append_porterduff_term(fsBuilder, xp.getDstBlend(),
                                                  dstColor, args.fInputColor,
                                                  dstColor, didAppend));
            fsBuilder->codeAppend(";");

            fsBuilder->codeAppendf("%s = %s * colorBlend + (vec4(1.0) - %s) * %s;",
                                   args.fOutputPrimary, args.fInputCoverage, args.fInputCoverage,
                                   dstColor);
        }
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) override {};

    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

PorterDuffXferProcessor::PorterDuffXferProcessor(GrBlendCoeff srcBlend,
                                                 GrBlendCoeff dstBlend,
                                                 GrColor constant,
                                                 const GrDeviceCoordTexture* dstCopy,
                                                 bool willReadDstColor)
    : INHERITED(dstCopy, willReadDstColor)
    , fSrcBlend(srcBlend)
    , fDstBlend(dstBlend)
    , fBlendConstant(constant)
    , fPrimaryOutputType(kModulate_PrimaryOutputType) 
    , fSecondaryOutputType(kNone_SecondaryOutputType) {
    this->initClassID<PorterDuffXferProcessor>();
}

PorterDuffXferProcessor::~PorterDuffXferProcessor() {
}

void PorterDuffXferProcessor::onGetGLProcessorKey(const GrGLSLCaps& caps,
                                                  GrProcessorKeyBuilder* b) const {
    GLPorterDuffXferProcessor::GenKey(*this, caps, b);
}

GrGLXferProcessor* PorterDuffXferProcessor::createGLInstance() const {
    return SkNEW_ARGS(GLPorterDuffXferProcessor, (*this));
}

GrXferProcessor::OptFlags
PorterDuffXferProcessor::onGetOptimizations(const GrProcOptInfo& colorPOI,
                                            const GrProcOptInfo& coveragePOI,
                                            bool doesStencilWrite,
                                            GrColor* overrideColor,
                                            const GrDrawTargetCaps& caps) {
    GrXferProcessor::OptFlags optFlags = this->internalGetOptimizations(colorPOI,
                                                                        coveragePOI,
                                                                        doesStencilWrite);
    this->calcOutputTypes(optFlags, caps, coveragePOI.isSolidWhite());
    return optFlags;
}

void PorterDuffXferProcessor::calcOutputTypes(GrXferProcessor::OptFlags optFlags,
                                              const GrDrawTargetCaps& caps,
                                              bool hasSolidCoverage) {
    if (this->willReadDstColor()) {
        fPrimaryOutputType = kCustom_PrimaryOutputType;
        return;
    }

    if (optFlags & kIgnoreColor_OptFlag) {
        if (optFlags & kIgnoreCoverage_OptFlag) {
            fPrimaryOutputType = kNone_PrimaryOutputType;
            return;
        } else {
            fPrimaryOutputType = kCoverage_PrimaryOutputType;
            return;
        }
    } else if (optFlags & kIgnoreCoverage_OptFlag) {
        fPrimaryOutputType = kColor_PrimaryOutputType;
        return;
    }

    // If we do have coverage determine whether it matters.  Dual source blending is expensive so
    // we don't do it if we are doing coverage drawing.  If we aren't then We always do dual source
    // blending if we have any effective coverage stages OR the geometry processor doesn't emits
    // solid coverage.
    if (!(optFlags & kSetCoverageDrawing_OptFlag) && !hasSolidCoverage) {
        if (caps.shaderCaps()->dualSourceBlendingSupport()) {
            if (kZero_GrBlendCoeff == fDstBlend) {
                // write the coverage value to second color
                fSecondaryOutputType = kCoverage_SecondaryOutputType;
                fDstBlend = kIS2C_GrBlendCoeff;
            } else if (kSA_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                fSecondaryOutputType = kCoverageISA_SecondaryOutputType;
                fDstBlend = kIS2C_GrBlendCoeff;
            } else if (kSC_GrBlendCoeff == fDstBlend) {
                // SA dst coeff becomes 1-(1-SA)*coverage when dst is partially covered.
                fSecondaryOutputType = kCoverageISC_SecondaryOutputType;
                fDstBlend = kIS2C_GrBlendCoeff;
            }
        }
    }
}

GrXferProcessor::OptFlags
PorterDuffXferProcessor::internalGetOptimizations(const GrProcOptInfo& colorPOI,
                                                  const GrProcOptInfo& coveragePOI,
                                                  bool doesStencilWrite) {
    if (this->willReadDstColor()) {
        return GrXferProcessor::kNone_Opt;
    }

    bool srcAIsOne = colorPOI.isOpaque();
    bool hasCoverage = !coveragePOI.isSolidWhite();

    bool dstCoeffIsOne = kOne_GrBlendCoeff == fDstBlend ||
                         (kSA_GrBlendCoeff == fDstBlend && srcAIsOne);
    bool dstCoeffIsZero = kZero_GrBlendCoeff == fDstBlend ||
                         (kISA_GrBlendCoeff == fDstBlend && srcAIsOne);

    // When coeffs are (0,1) there is no reason to draw at all, unless
    // stenciling is enabled. Having color writes disabled is effectively
    // (0,1).
    if ((kZero_GrBlendCoeff == fSrcBlend && dstCoeffIsOne)) {
        if (doesStencilWrite) {
            return GrXferProcessor::kIgnoreColor_OptFlag |
                   GrXferProcessor::kSetCoverageDrawing_OptFlag;
        } else {
            fDstBlend = kOne_GrBlendCoeff;
            return GrXferProcessor::kSkipDraw_OptFlag;
        }
    }

    // if we don't have coverage we can check whether the dst
    // has to read at all. If not, we'll disable blending.
    if (!hasCoverage) {
        if (dstCoeffIsZero) {
            if (kOne_GrBlendCoeff == fSrcBlend) {
                // if there is no coverage and coeffs are (1,0) then we
                // won't need to read the dst at all, it gets replaced by src
                fDstBlend = kZero_GrBlendCoeff;
                return GrXferProcessor::kNone_Opt |
                       GrXferProcessor::kIgnoreCoverage_OptFlag;
            } else if (kZero_GrBlendCoeff == fSrcBlend) {
                // if the op is "clear" then we don't need to emit a color
                // or blend, just write transparent black into the dst.
                fSrcBlend = kOne_GrBlendCoeff;
                fDstBlend = kZero_GrBlendCoeff;
                return GrXferProcessor::kIgnoreColor_OptFlag |
                       GrXferProcessor::kIgnoreCoverage_OptFlag;
            }
        }
        return GrXferProcessor::kIgnoreCoverage_OptFlag;
    }

    // check whether coverage can be safely rolled into alpha
    // of if we can skip color computation and just emit coverage
    if (can_tweak_alpha_for_coverage(fDstBlend)) {
        if (colorPOI.allStagesMultiplyInput()) {
            return GrXferProcessor::kSetCoverageDrawing_OptFlag |
                GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag;
        } else {
            return GrXferProcessor::kSetCoverageDrawing_OptFlag;

        }
    }
    if (dstCoeffIsZero) {
        if (kZero_GrBlendCoeff == fSrcBlend) {
            // the source color is not included in the blend
            // the dst coeff is effectively zero so blend works out to:
            // (c)(0)D + (1-c)D = (1-c)D.
            fDstBlend = kISA_GrBlendCoeff;
            return GrXferProcessor::kIgnoreColor_OptFlag |
                GrXferProcessor::kSetCoverageDrawing_OptFlag;
        } else if (srcAIsOne) {
            // the dst coeff is effectively zero so blend works out to:
            // cS + (c)(0)D + (1-c)D = cS + (1-c)D.
            // If Sa is 1 then we can replace Sa with c
            // and set dst coeff to 1-Sa.
            fDstBlend = kISA_GrBlendCoeff;
            if (colorPOI.allStagesMultiplyInput()) {
                return GrXferProcessor::kSetCoverageDrawing_OptFlag |
                    GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag;
            } else {
                return GrXferProcessor::kSetCoverageDrawing_OptFlag;

            }
        }
    } else if (dstCoeffIsOne) {
        // the dst coeff is effectively one so blend works out to:
        // cS + (c)(1)D + (1-c)D = cS + D.
        fDstBlend = kOne_GrBlendCoeff;
        if (colorPOI.allStagesMultiplyInput()) {
            return GrXferProcessor::kSetCoverageDrawing_OptFlag |
                GrXferProcessor::kCanTweakAlphaForCoverage_OptFlag;
        } else {
            return GrXferProcessor::kSetCoverageDrawing_OptFlag;

        }
        return GrXferProcessor::kSetCoverageDrawing_OptFlag;
    }

    return GrXferProcessor::kNone_Opt;
}

bool PorterDuffXferProcessor::hasSecondaryOutput() const {
    return kNone_SecondaryOutputType != fSecondaryOutputType;
}

///////////////////////////////////////////////////////////////////////////////

class PDLCDXferProcessor : public GrXferProcessor {
public:
    static GrXferProcessor* Create(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend,
                                   const GrProcOptInfo& colorPOI);

    ~PDLCDXferProcessor() override;

    const char* name() const override { return "Porter Duff LCD"; }

    GrGLXferProcessor* createGLInstance() const override;

    bool hasSecondaryOutput() const override { return false; }

private:
    PDLCDXferProcessor(GrColor blendConstant, uint8_t alpha);

    GrXferProcessor::OptFlags onGetOptimizations(const GrProcOptInfo& colorPOI,
                                                 const GrProcOptInfo& coveragePOI,
                                                 bool doesStencilWrite,
                                                 GrColor* overrideColor,
                                                 const GrDrawTargetCaps& caps) override;

    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    void onGetBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const override {
        blendInfo->fSrcBlend = kConstC_GrBlendCoeff;
        blendInfo->fDstBlend = kISC_GrBlendCoeff;
        blendInfo->fBlendConstant = fBlendConstant;
    }

    bool onIsEqual(const GrXferProcessor& xpBase) const override {
        const PDLCDXferProcessor& xp = xpBase.cast<PDLCDXferProcessor>();
        if (fBlendConstant != xp.fBlendConstant ||
            fAlpha != xp.fAlpha) {
            return false;
        }
        return true;
    }

    GrColor      fBlendConstant;
    uint8_t      fAlpha;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GLPDLCDXferProcessor : public GrGLXferProcessor {
public:
    GLPDLCDXferProcessor(const GrProcessor&) {}

    virtual ~GLPDLCDXferProcessor() {}

    static void GenKey(const GrProcessor& processor, const GrGLSLCaps& caps,
                       GrProcessorKeyBuilder* b) {}

private:
    void onEmitCode(const EmitArgs& args) override {
        GrGLXPFragmentBuilder* fsBuilder = args.fPB->getFragmentShaderBuilder();

        fsBuilder->codeAppendf("%s = %s * %s;", args.fOutputPrimary, args.fInputColor,
                               args.fInputCoverage);
    }

    void onSetData(const GrGLProgramDataManager&, const GrXferProcessor&) override {};

    typedef GrGLXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

PDLCDXferProcessor::PDLCDXferProcessor(GrColor blendConstant, uint8_t alpha)
    : fBlendConstant(blendConstant)
    , fAlpha(alpha) {
    this->initClassID<PDLCDXferProcessor>();
}

GrXferProcessor* PDLCDXferProcessor::Create(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend,
                                            const GrProcOptInfo& colorPOI) {
    if (kOne_GrBlendCoeff != srcBlend || kISA_GrBlendCoeff != dstBlend) {
        return NULL;
    }

    if (kRGBA_GrColorComponentFlags != colorPOI.validFlags()) {
        return NULL;
    }

    GrColor blendConstant = GrUnPreMulColor(colorPOI.color());
    uint8_t alpha = GrColorUnpackA(blendConstant);
    blendConstant |= (0xff << GrColor_SHIFT_A);

    return SkNEW_ARGS(PDLCDXferProcessor, (blendConstant, alpha));
}

PDLCDXferProcessor::~PDLCDXferProcessor() {
}

void PDLCDXferProcessor::onGetGLProcessorKey(const GrGLSLCaps& caps,
                                             GrProcessorKeyBuilder* b) const {
    GLPDLCDXferProcessor::GenKey(*this, caps, b);
}

GrGLXferProcessor* PDLCDXferProcessor::createGLInstance() const {
    return SkNEW_ARGS(GLPDLCDXferProcessor, (*this));
}

GrXferProcessor::OptFlags
PDLCDXferProcessor::onGetOptimizations(const GrProcOptInfo& colorPOI,
                                       const GrProcOptInfo& coveragePOI,
                                       bool doesStencilWrite,
                                       GrColor* overrideColor,
                                       const GrDrawTargetCaps& caps) {
        // We want to force our primary output to be alpha * Coverage, where alpha is the alpha
        // value of the blend the constant. We should already have valid blend coeff's if we are at
        // a point where we have RGB coverage. We don't need any color stages since the known color
        // output is already baked into the blendConstant.
        *overrideColor = GrColorPackRGBA(fAlpha, fAlpha, fAlpha, fAlpha);
        return GrXferProcessor::kOverrideColor_OptFlag;
}

///////////////////////////////////////////////////////////////////////////////
GrPorterDuffXPFactory::GrPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst)
    : fSrcCoeff(src), fDstCoeff(dst) {
    this->initClassID<GrPorterDuffXPFactory>();
}

GrXPFactory* GrPorterDuffXPFactory::Create(SkXfermode::Mode mode) {
    switch (mode) {
        case SkXfermode::kClear_Mode: {
            static GrPorterDuffXPFactory gClearPDXPF(kZero_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gClearPDXPF);
            break;
        }
        case SkXfermode::kSrc_Mode: {
            static GrPorterDuffXPFactory gSrcPDXPF(kOne_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gSrcPDXPF);
            break;
        }
        case SkXfermode::kDst_Mode: {
            static GrPorterDuffXPFactory gDstPDXPF(kZero_GrBlendCoeff, kOne_GrBlendCoeff);
            return SkRef(&gDstPDXPF);
            break;
        }
        case SkXfermode::kSrcOver_Mode: {
            static GrPorterDuffXPFactory gSrcOverPDXPF(kOne_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gSrcOverPDXPF);
            break;
        }
        case SkXfermode::kDstOver_Mode: {
            static GrPorterDuffXPFactory gDstOverPDXPF(kIDA_GrBlendCoeff, kOne_GrBlendCoeff);
            return SkRef(&gDstOverPDXPF);
            break;
        }
        case SkXfermode::kSrcIn_Mode: {
            static GrPorterDuffXPFactory gSrcInPDXPF(kDA_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gSrcInPDXPF);
            break;
        }
        case SkXfermode::kDstIn_Mode: {
            static GrPorterDuffXPFactory gDstInPDXPF(kZero_GrBlendCoeff, kSA_GrBlendCoeff);
            return SkRef(&gDstInPDXPF);
            break;
        }
        case SkXfermode::kSrcOut_Mode: {
            static GrPorterDuffXPFactory gSrcOutPDXPF(kIDA_GrBlendCoeff, kZero_GrBlendCoeff);
            return SkRef(&gSrcOutPDXPF);
            break;
        }
        case SkXfermode::kDstOut_Mode: {
            static GrPorterDuffXPFactory gDstOutPDXPF(kZero_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gDstOutPDXPF);
            break;
        }
        case SkXfermode::kSrcATop_Mode: {
            static GrPorterDuffXPFactory gSrcATopPDXPF(kDA_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gSrcATopPDXPF);
            break;
        }
        case SkXfermode::kDstATop_Mode: {
            static GrPorterDuffXPFactory gDstATopPDXPF(kIDA_GrBlendCoeff, kSA_GrBlendCoeff);
            return SkRef(&gDstATopPDXPF);
            break;
        }
        case SkXfermode::kXor_Mode: {
            static GrPorterDuffXPFactory gXorPDXPF(kIDA_GrBlendCoeff, kISA_GrBlendCoeff);
            return SkRef(&gXorPDXPF);
            break;
        }
        case SkXfermode::kPlus_Mode: {
            static GrPorterDuffXPFactory gPlusPDXPF(kOne_GrBlendCoeff, kOne_GrBlendCoeff);
            return SkRef(&gPlusPDXPF);
            break;
        }
        case SkXfermode::kModulate_Mode: {
            static GrPorterDuffXPFactory gModulatePDXPF(kZero_GrBlendCoeff, kSC_GrBlendCoeff);
            return SkRef(&gModulatePDXPF);
            break;
        }
        case SkXfermode::kScreen_Mode: {
            static GrPorterDuffXPFactory gScreenPDXPF(kOne_GrBlendCoeff, kISC_GrBlendCoeff);
            return SkRef(&gScreenPDXPF);
            break;
        }
        default:
            return NULL;
    }
}

GrXferProcessor*
GrPorterDuffXPFactory::onCreateXferProcessor(const GrDrawTargetCaps& caps,
                                             const GrProcOptInfo& colorPOI,
                                             const GrProcOptInfo& covPOI,
                                             const GrDeviceCoordTexture* dstCopy) const {
    if (covPOI.isFourChannelOutput()) {
        return PDLCDXferProcessor::Create(fSrcCoeff, fDstCoeff, colorPOI);
    } else {
        return PorterDuffXferProcessor::Create(fSrcCoeff, fDstCoeff, 0, dstCopy,
                                               this->willReadDstColor(caps, colorPOI, covPOI));
    }
}

bool GrPorterDuffXPFactory::supportsRGBCoverage(GrColor /*knownColor*/,
                                                uint32_t knownColorFlags) const {
    if (kOne_GrBlendCoeff == fSrcCoeff && kISA_GrBlendCoeff == fDstCoeff &&
        kRGBA_GrColorComponentFlags == knownColorFlags) {
        return true;
    }
    return false;
}

void GrPorterDuffXPFactory::getInvariantOutput(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               GrXPFactory::InvariantOutput* output) const {
    if (!coveragePOI.isSolidWhite()) {
        output->fWillBlendWithDst = true;
        output->fBlendedColorFlags = 0;
        return;
    }

    GrBlendCoeff srcCoeff = fSrcCoeff;
    GrBlendCoeff dstCoeff = fDstCoeff;

    // TODO: figure out to merge this simplify with other current optimization code paths and
    // eventually remove from GrBlend
    GrSimplifyBlend(&srcCoeff, &dstCoeff, colorPOI.color(), colorPOI.validFlags(),
                    0, 0, 0);

    if (GrBlendCoeffRefsDst(srcCoeff)) {
        output->fWillBlendWithDst = true;
        output->fBlendedColorFlags = 0;
        return;
    }

    if (kZero_GrBlendCoeff != dstCoeff) {
        bool srcAIsOne = colorPOI.isOpaque();
        if (kISA_GrBlendCoeff != dstCoeff || !srcAIsOne) {
            output->fWillBlendWithDst = true;
        }
        output->fBlendedColorFlags = 0;
        return;
    }

    switch (srcCoeff) {
        case kZero_GrBlendCoeff:
            output->fBlendedColor = 0;
            output->fBlendedColorFlags = kRGBA_GrColorComponentFlags;
            break;

        case kOne_GrBlendCoeff:
            output->fBlendedColor = colorPOI.color();
            output->fBlendedColorFlags = colorPOI.validFlags();
            break;

            // The src coeff should never refer to the src and if it refers to dst then opaque
            // should have been false.
        case kSC_GrBlendCoeff:
        case kISC_GrBlendCoeff:
        case kDC_GrBlendCoeff:
        case kIDC_GrBlendCoeff:
        case kSA_GrBlendCoeff:
        case kISA_GrBlendCoeff:
        case kDA_GrBlendCoeff:
        case kIDA_GrBlendCoeff:
        default:
            SkFAIL("srcCoeff should not refer to src or dst.");
            break;

            // TODO: update this once GrPaint actually has a const color.
        case kConstC_GrBlendCoeff:
        case kIConstC_GrBlendCoeff:
        case kConstA_GrBlendCoeff:
        case kIConstA_GrBlendCoeff:
            output->fBlendedColorFlags = 0;
            break;
    }

    output->fWillBlendWithDst = false;
}

bool GrPorterDuffXPFactory::willReadDstColor(const GrDrawTargetCaps& caps,
                                             const GrProcOptInfo& colorPOI,
                                             const GrProcOptInfo& coveragePOI) const {
    // We can always blend correctly if we have dual source blending.
    if (caps.shaderCaps()->dualSourceBlendingSupport()) {
        return false;
    }

    if (can_tweak_alpha_for_coverage(fDstCoeff)) {
        return false;
    }

    bool srcAIsOne = colorPOI.isOpaque();

    if (kZero_GrBlendCoeff == fDstCoeff) {
        if (kZero_GrBlendCoeff == fSrcCoeff || srcAIsOne) {
            return false;
        }
    }

    // Reduces to: coeffS * (Cov*S) + D
    if (kSA_GrBlendCoeff == fDstCoeff && srcAIsOne) {
        return false;
    }

    // We can always blend correctly if we have solid coverage.
    if (coveragePOI.isSolidWhite()) {
        return false;
    }

    return true;
}

GR_DEFINE_XP_FACTORY_TEST(GrPorterDuffXPFactory);

GrXPFactory* GrPorterDuffXPFactory::TestCreate(SkRandom* random,
                                               GrContext*,
                                               const GrDrawTargetCaps&,
                                               GrTexture*[]) {
    SkXfermode::Mode mode = SkXfermode::Mode(random->nextULessThan(SkXfermode::kLastCoeffMode));
    return GrPorterDuffXPFactory::Create(mode);
}

