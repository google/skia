/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDisableColorXP_DEFINED
#define GrDisableColorXP_DEFINED

#include "GrTypes.h"
#include "GrXferProcessor.h"

class GrProcOptInfo;

class GrDisableColorXPFactory : public GrXPFactory {
public:
    static GrXPFactory* Create() { return new GrDisableColorXPFactory; }

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor* blendedColor) const override {
        blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
        blendedColor->fWillBlendWithDst = false;
    }

private:
    GrDisableColorXPFactory();

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineOptimizations& optimizations,
                                           bool hasMixedSamples,
                                           const DstTexture* dstTexture) const override;

    bool onWillReadDstColor(const GrCaps& caps,
                            const GrPipelineOptimizations& optimizations,
                            bool hasMixedSamples) const override {
        return false;
    }

    bool onIsEqual(const GrXPFactory& xpfBase) const override {
        return true;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    typedef GrXPFactory INHERITED;
};

#endif
