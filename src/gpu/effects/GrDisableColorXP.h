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
#include "SkRefCnt.h"

class GrProcOptInfo;

class GrDisableColorXPFactory : public GrXPFactory {
public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
    ~GrDisableColorXPFactory() = default;
#pragma GCC diagnostic pop

    static const GrXPFactory* Get();

    void getInvariantBlendedColor(const GrProcOptInfo& colorPOI,
                                  GrXPFactory::InvariantBlendedColor* blendedColor) const override {
        blendedColor->fKnownColorFlags = kNone_GrColorComponentFlags;
        blendedColor->fWillBlendWithDst = false;
    }

private:
    constexpr GrDisableColorXPFactory() {}

    GrXferProcessor* onCreateXferProcessor(const GrCaps& caps,
                                           const GrPipelineAnalysis&,
                                           bool hasMixedSamples,
                                           const DstTexture* dstTexture) const override;

    bool onWillReadDstColor(const GrCaps&, const GrPipelineAnalysis&) const override {
        return false;
    }

    GR_DECLARE_XP_FACTORY_TEST;

    typedef GrXPFactory INHERITED;
};

inline const GrXPFactory* GrDisableColorXPFactory::Get() {
    static constexpr const GrDisableColorXPFactory gDisableColorXPFactory;
    return &gDisableColorXPFactory;
}

#endif
