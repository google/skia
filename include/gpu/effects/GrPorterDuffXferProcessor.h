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

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor*) const override;

    static GrXferProcessor* CreateSrcOverXferProcessor(const GrCaps& caps,
                                                       const GrPipelineOptimizations& optimizations,
                                                       bool hasMixedSamples,
                                                       const GrXferProcessor::DstTexture*);

    static inline void SrcOverInvariantBlendedColor(
                                                GrColor inputColor,
                                                GrColorComponentFlags validColorFlags,
                                                bool isOpaque,
                                                GrXPFactory::InvariantBlendedColor* blendedColor) {
        if (!isOpaque) {
            blendedColor->fWillBlendWithDst = true;
            blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
            return;
        }
        blendedColor->fWillBlendWithDst = false;

        blendedColor->fKnownColor = inputColor;
        blendedColor->fKnownColorFlags = validColorFlags;
    }

    static bool SrcOverWillNeedDstTexture(const GrCaps& caps,
                                          const GrPipelineOptimizations& optimizations,
                                          bool hasMixedSamples);

private:
    GrPorterDuffXPFactory(SkXfermode::Mode);

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture*) const override;

    bool willReadDstColor(const GrCaps& caps,
                          const GrPipelineOptimizations& optimizations,
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
