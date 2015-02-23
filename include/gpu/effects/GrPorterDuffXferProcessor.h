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

class GrPorterDuffXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create(SkXfermode::Mode mode); 

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const SK_OVERRIDE;

    bool canTweakAlphaForCoverage() const SK_OVERRIDE;

    void getInvariantOutput(const GrProcOptInfo& colorPOI, const GrProcOptInfo& coveragePOI,
                            GrXPFactory::InvariantOutput*) const SK_OVERRIDE;

private:
    GrPorterDuffXPFactory(GrBlendCoeff src, GrBlendCoeff dst); 

    GrXferProcessor* onCreateXferProcessor(const GrDrawTargetCaps& caps,
                                           const GrProcOptInfo& colorPOI,
                                           const GrProcOptInfo& coveragePOI,
                                           const GrDeviceCoordTexture* dstCopy) const SK_OVERRIDE;

    bool willReadDstColor(const GrDrawTargetCaps& caps,
                          const GrProcOptInfo& colorPOI,
                          const GrProcOptInfo& coveragePOI) const SK_OVERRIDE;

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
