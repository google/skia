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

class GrPDXPFactory : public GrXPFactory {
public:
    static const GrXPFactory* Create(SkXfermode::Mode mode); 

    bool supportsRGBCoverage(GrColor /*knownColor*/, uint32_t /*knownColorFlags*/) const override {
        return true;
    }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

private:
    GrPDXPFactory(SkXfermode::Mode);

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
        const GrPDXPFactory& xpf = xpfBase.cast<GrPDXPFactory>();
        return fXfermode == xpf.fXfermode;
    }

    GR_DECLARE_XP_FACTORY_TEST;
    static void TestGetXPOutputTypes(const GrXferProcessor*, int* outPrimary, int* outSecondary);

    SkXfermode::Mode fXfermode;

    friend class GrPorterDuffTest; // for TestGetXPOutputTypes()
    typedef GrXPFactory INHERITED;
};

class GrSrcOverPDXPFactory : public GrXPFactory {
public:
    GrSrcOverPDXPFactory();

    bool supportsRGBCoverage(GrColor /*knownColor*/, uint32_t /*knownColorFlags*/) const override {
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

    bool onIsEqual(const GrXPFactory& /*xpfBase*/) const override {
        return true;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    typedef GrXPFactory INHERITED;
};

namespace GrPorterDuffXPFactory {
    const GrSrcOverPDXPFactory gSrcOverPDXPFactory;

    inline const GrXPFactory* Create(SkXfermode::Mode mode) {
        if (SkXfermode::kSrcOver_Mode == mode) {
            return SkRef(&gSrcOverPDXPFactory);
        }
        return GrPDXPFactory::Create(mode);
    }
};

#endif
