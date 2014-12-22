/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPorterDuffXferProcessor_DEFINED
#define GrPorterDuffXferProcessor_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"
#include "SkXfermode.h"

class GrProcOptInfo;

class GrPorterDuffXferProcessor : public GrXferProcessor {
public:
    static GrXferProcessor* Create(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend,
                                   GrColor constant = 0) {
        return SkNEW_ARGS(GrPorterDuffXferProcessor, (srcBlend, dstBlend, constant));
    }

    ~GrPorterDuffXferProcessor() SK_OVERRIDE;

    const char* name() const SK_OVERRIDE { return "Porter Duff"; }

    void getGLProcessorKey(const GrGLCaps& caps, GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    GrGLXferProcessor* createGLInstance() const SK_OVERRIDE;

    bool hasSecondaryOutput() const SK_OVERRIDE;

    ///////////////////////////////////////////////////////////////////////////
    /// @name Stage Output Types
    ////

    enum PrimaryOutputType {
        kNone_PrimaryOutputType,
        kColor_PrimaryOutputType,
        kCoverage_PrimaryOutputType,
        // Modulate color and coverage, write result as the color output.
        kModulate_PrimaryOutputType,
        // Combines the coverage, dst, and color as coverage * color + (1 - coverage) * dst. This
        // can only be set if fDstReadKey is non-zero.
        kCombineWithDst_PrimaryOutputType,
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

    GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                               const GrProcOptInfo& coveragePOI,
                                               bool doesStencilWrite,
                                               GrColor* overrideColor,
                                               const GrDrawTargetCaps& caps) SK_OVERRIDE;

    void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE {
        blendInfo->fSrcBlend = fSrcBlend;
        blendInfo->fDstBlend = fDstBlend;
        blendInfo->fBlendConstant = fBlendConstant;
    }

private:
    GrPorterDuffXferProcessor(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend, GrColor constant);

    bool onIsEqual(const GrXferProcessor& xpBase) const SK_OVERRIDE {
        const GrPorterDuffXferProcessor& xp = xpBase.cast<GrPorterDuffXferProcessor>();
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
                         bool hasSolidCoverage, bool readDst);

    GrBlendCoeff fSrcBlend;
    GrBlendCoeff fDstBlend;
    GrColor      fBlendConstant;
    PrimaryOutputType fPrimaryOutputType;
    SecondaryOutputType fSecondaryOutputType;

    typedef GrXferProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

class GrPorterDuffXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkXfermode::Mode mode); 

    static GrXPFactory* Create(SkXfermode::Coeff src, SkXfermode::Coeff dst) {
        return SkNEW_ARGS(GrPorterDuffXPFactory, ((GrBlendCoeff)(src), (GrBlendCoeff)(dst)));
    }

    static GrXPFactory* Create(GrBlendCoeff src, GrBlendCoeff dst) {
        return SkNEW_ARGS(GrPorterDuffXPFactory, (src, dst));
    }

    GrXferProcessor* createXferProcessor(const GrProcOptInfo& colorPOI,
                                         const GrProcOptInfo& coveragePOI) const SK_OVERRIDE;

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE;

    bool canApplyCoverage(const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE;

    bool canTweakAlphaForCoverage() const SK_OVERRIDE;

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput*) const SK_OVERRIDE;

    bool willReadDst(const GrProcOptInfo& colorPOI,
                     const GrProcOptInfo& coveragePOI) const SK_OVERRIDE;

private:
    GrPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst); 

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        const GrPorterDuffXPFactory& xpf = xpfBase.cast<GrPorterDuffXPFactory>();
        return (fSrcCoeff == xpf.fSrcCoeff && fDstCoeff == xpf.fDstCoeff);
    }

    GR_DECLARE_XP_FACTORY_TEST;

    GrBlendCoeff fSrcCoeff;
    GrBlendCoeff fDstCoeff;

    typedef GrXPFactory INHERITED;
};

#endif
