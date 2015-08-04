/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomXfermodePriv_DEFINED
#define GrCustomXfermodePriv_DEFINED

#include "GrCaps.h"
#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrTextureAccess.h"
#include "GrXferProcessor.h"
#include "SkXfermode.h"

class GrGLCaps;
class GrGLFragmentProcessor;
class GrInvariantOutput;
class GrProcessorKeyBuilder;
class GrTexture;

///////////////////////////////////////////////////////////////////////////////
// Fragment Processor
///////////////////////////////////////////////////////////////////////////////

class GrCustomXferFP : public GrFragmentProcessor {
public:
    GrCustomXferFP(GrProcessorDataManager*, SkXfermode::Mode mode, GrTexture* background);

    GrGLFragmentProcessor* createGLInstance() const override;

    const char* name() const override { return "Custom Xfermode"; }

    SkXfermode::Mode mode() const { return fMode; }
    const GrTextureAccess&  backgroundAccess() const { return fBackgroundAccess; }

private:
    void onGetGLProcessorKey(const GrGLSLCaps& caps, GrProcessorKeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& other) const override; 

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST;

    SkXfermode::Mode fMode;
    GrCoordTransform fBackgroundTransform;
    GrTextureAccess  fBackgroundAccess;

    typedef GrFragmentProcessor INHERITED;
};

///////////////////////////////////////////////////////////////////////////////
// Xfer Processor
///////////////////////////////////////////////////////////////////////////////

class GrCustomXPFactory : public GrXPFactory {
public:
    GrCustomXPFactory(SkXfermode::Mode mode); 

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const override {
        return true;
    }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override; 

    bool willReadDstColor(const GrCaps& caps,
                          const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI,
                          bool hasMixedSamples) const override;

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        const GrCustomXPFactory& xpf = xpfBase.cast<GrCustomXPFactory>();
        return fMode == xpf.fMode;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    SkXfermode::Mode fMode;
    GrBlendEquation  fHWBlendEquation;

    typedef GrXPFactory INHERITED;
};
#endif

