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

    bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const override;

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrPorterDuffXPFactory(SkXfermode::Mode);

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
        const GrPorterDuffXPFactory& xpf = xpfBase.cast<GrPorterDuffXPFactory>();
        return fXfermode == xpf.fXfermode;
    }

    GR_DECLARE_XP_FACTORY_TEST;
    static void TestGetXPOutputTypes(const GrXferProcessor*, int* outPrimary, int* outSecondary);

    SkXfermode::Mode fXfermode;

    friend class GrPorterDuffTest; // for TestGetXPOutputTypes()
    typedef GrXPFactory INHERITED;
};

#endif
