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

class GrDrawState;
class GrInvariantOutput;

class GrPorterDuffXferProcessor : public GrXferProcessor {
public:
    static GrXferProcessor* Create(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend,
                                   GrColor constant = 0) {
        return SkNEW_ARGS(GrPorterDuffXferProcessor, (srcBlend, dstBlend, constant));
    }

    virtual ~GrPorterDuffXferProcessor();

    virtual const char* name() const { return "Porter Duff"; }

    virtual void getGLProcessorKey(const GrGLCaps& caps,
                                   GrProcessorKeyBuilder* b) const SK_OVERRIDE;

    virtual GrGLFragmentProcessor* createGLInstance() const SK_OVERRIDE;

    virtual GrXferProcessor::OptFlags getOptimizations(const GrProcOptInfo& colorPOI,
                                                       const GrProcOptInfo& coveragePOI,
                                                       bool isCoverageDrawing,
                                                       bool colorWriteDisabled,
                                                       bool doesStencilWrite,
                                                       GrColor* color,
                                                       uint8_t* coverage) SK_OVERRIDE;

    virtual void getBlendInfo(GrXferProcessor::BlendInfo* blendInfo) const SK_OVERRIDE {
        blendInfo->fSrcBlend = fSrcBlend;
        blendInfo->fDstBlend = fDstBlend;
        blendInfo->fBlendConstant = fBlendConstant;
    }

private:
    GrPorterDuffXferProcessor(GrBlendCoeff srcBlend, GrBlendCoeff dstBlend, GrColor constant);

    virtual bool onIsEqual(const GrFragmentProcessor& fpBase) const SK_OVERRIDE {
        const GrPorterDuffXferProcessor& xp = fpBase.cast<GrPorterDuffXferProcessor>();
        if (fSrcBlend != xp.fSrcBlend ||
            fDstBlend != xp.fDstBlend ||
            fBlendConstant != xp.fBlendConstant) {
            return false;
        }
        return true;
    }

    virtual void onComputeInvariantOutput(GrInvariantOutput* inout) const SK_OVERRIDE;

    GrBlendCoeff fSrcBlend;
    GrBlendCoeff fDstBlend;
    GrColor      fBlendConstant;

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

    bool canApplyCoverage(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                          bool isCoverageDrawing, bool colorWriteDisabled) const SK_OVERRIDE;

    bool willBlendWithDst(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                          bool isCoverageDrawing, bool colorWriteDisabled) const SK_OVERRIDE;

    bool canTweakAlphaForCoverage(bool isCoverageDrawing) const SK_OVERRIDE;

    bool getOpaqueAndKnownColor(const GrProcOptInfo& colorPOI,
                                const GrProcOptInfo& coveragePOI,
                                GrColor* solidColor,
                                uint32_t* solidColorKnownComponents) const SK_OVERRIDE;

private:
    GrPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst); 

    bool onIsEqual(const GrXPFactory& xpfBase) const SK_OVERRIDE {
        const GrPorterDuffXPFactory& xpf = xpfBase.cast<GrPorterDuffXPFactory>();
        return (fSrcCoeff == xpf.fSrcCoeff && fDstCoeff == xpf.fDstCoeff);
    }

    GrBlendCoeff fSrcCoeff;
    GrBlendCoeff fDstCoeff;

    typedef GrXPFactory INHERITED;
};

#endif
